# OrganicEngine – Renderingspipelinen: Den första triangeln

## 1. Introduktion

Det här dokumentet beskriver hela renderingsprocessen som OrganicEngine använder för att rita sin första triangel med hjälp av:

- C++20
- SDL3
- SDL3 GPU API
- Metal (macOS)
- HLSL-shaders
- SDL_shadercross
- Apples Metal-kompilator

Målet är att förstå hela resan från vertexdata i CPU-minnet till en färdigrenderad triangel på skärmen.

Detta är grunden för OrganicEngines renderingsarkitektur.

---

# 2. Renderingspipelinen – Översikt

Att rendera en triangel involverar flera olika steg.

Applikationen förbereder data, laddar upp den till GPU-minnet, konfigurerar en grafikpipeline och skickar renderingskommandon till GPU:n.

GPU:n bearbetar sedan vertexarna, sätter ihop triangeln, rasteriserar den och kör fragment-shadern för att bestämma färgen på de slutliga pixlarna.

### Hela pipelinen

```text
                    APPLIKATION (CPU)
                           |
                           v
                       Vertexdata
                     (C++ Vertex[])
                           |
                           v
                      Mesh-skapande
                           |
                           v
                       Vertexbuffer
                           |
                           v
                  Ladda upp data till GPU
                           |
                           |
              +------------+------------+
              |                         |
              v                         v
        Vertex Shader             Fragment Shader
           (HLSL)                      (HLSL)
              |                         |
              +------------+------------+
                           |
                           v
                    Shaderkompilering
                           |
                           v
                      Metal Library
                       (.metallib)
                           |
                           v
                  SDL GPU Shader-objekt
                           |
                           v
                     Grafikpipeline
                           |
                           v
                       Command Buffer
                           |
                           v
                        Render Pass
                           |
                           v
                        Draw-anrop
                           |
                           v
                      GPU-exekvering
                           |
                           v
                       Rasterisering
                           |
                           v
                      Fragment Shader
                           |
                           v
                     Swapchain Texture
                           |
                           v
                         SKÄRM
```

Diagrammet visar den övergripande processen. Shaderkompileringen sker under byggprocessen, innan applikationen startar. Grafikpipelinen använder de kompilerade shaderprogrammen under körning.

---

# 3. Vertexdata – Att definiera triangeln

Innan GPU:n kan rendera något måste vi definiera geometrin.

Vi börjar med att skapa en enkel vertexstruktur.

### Vertex.h

```cpp
#pragma once

struct Vertex
{
    float position[3];
    float texCoord[2];
};
```

Varje vertex innehåller:

| Attribut | Typ | Beskrivning |
|---|---|---|
| position | float[3] | X-, Y- och Z-koordinater |
| texCoord | float[2] | Texturkoordinater (U och V) |

En vertex är helt enkelt en samling attribut som beskriver en punkt i vår geometri.

Vår triangel består av tre vertexar.

### Skapa triangeln

```cpp
Vertex vertices[] =
{
    {{ 0.0f,  0.5f, 0.0f}, {0.5f, 0.0f}},
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}},
    {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}}
};
```

Dessa koordinater definierar en triangel centrerad kring origo.

Positionerna ligger redan inom GPU:ns clip space, vilket innebär att vi inte behöver någon kamera eller projektionsmatris för det här första renderingstestet.

### Clip space

I vårt nuvarande test skickar vertex-shadern positionerna direkt till GPU:ns clip space.

Det synliga området är ungefär:

```text
X: -1 till +1
Y: -1 till +1
Z:  0 till +1  (Metal clip space)
```

Våra vertexpositioner ligger inom detta område.

Det är därför triangeln kan renderas utan en kamera.

En kamera och transformationsmatriser blir nödvändiga när vi introducerar geometri i världskoordinater och en riktig 3D-scen.

---

# 4. Mesh – Hantering av geometri på GPU:n

Mesh-klassen ansvarar för att hantera geometridata.

Istället för att låta Renderer ansvara för varje vertexallokering och dataöverföring separerar vi geometrihanteringen i en egen klass.

Vår Mesh-klass hanterar:

1. Tar emot vertexdata.
2. Skapar en GPU-vertexbuffer.
3. Skapar en tillfällig transferbuffer.
4. Kopierar vertexdata till transferbuffern.
5. Laddar upp datan till GPU-vertexbuffern.
6. Håller reda på antalet vertexar.

### Konceptuellt minnesflöde

```text
             CPU-MINNE
                  |
                  v
              Vertex Array
                  |
                  v
             Transfer Buffer
                  |
                  | GPU-uppladdning
                  v
              GPU-MINNE
                  |
                  v
              Vertex Buffer
                  |
                  v
             GPU-rendering
```

### Skapa meshen

I main.cpp:

```cpp
Mesh triangle(gpuDevice);

if (!triangle.Create(vertices, 3))
{
    SDL_LogError(
        SDL_LOG_CATEGORY_RENDER,
        "Mesh creation failed: %s",
        SDL_GetError()
    );

    return 1;
}
```

Anropet till `Create()` är viktigt.

Att skapa ett Mesh-objekt betyder inte automatiskt att det innehåller någon geometri.

Vi måste uttryckligen skapa vertexbuffern och ladda upp de tre vertexarna.

### Varför använder vi en GPU-vertexbuffer?

GPU:n behöver tillgång till vertexdatan när den kör renderingspipelinen.

En GPU-buffer är en resurs som GPU:n kan läsa effektivt.

Vi laddar upp datan en gång och återanvänder den vid efterföljande draw-anrop.

Detta gör att vi slipper skicka samma geometri från CPU:n varje bildruta.

---

# 5. Shaderprogram

Shaders är små program som körs på GPU:n.

OrganicEngine använder för närvarande HLSL som shaderspråk.

Vi har två shadersteg:

- Vertex Shader
- Fragment Shader

De utför olika uppgifter.

## 5.1 Vertex Shader

Vertex-shadern bearbetar enskilda vertexar.

Dess huvudsakliga ansvar är att transformera vertexpositioner till clip space.

Vår första testshader:

```hlsl
struct VS_INPUT
{
    float3 Position : POSITION;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
};

VS_OUTPUT vertexMain(VS_INPUT input)
{
    VS_OUTPUT output;

    output.Position = float4(input.Position, 1.0f);

    return output;
}
```

### Förstå shadern

Input-strukturen:

```hlsl
struct VS_INPUT
{
    float3 Position : POSITION;
};
```

Tar emot vertexpositionen från vertexbuffern.

`float3` innehåller tre komponenter:

```text
X, Y, Z
```

Output-strukturen:

```hlsl
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
};
```

Innehåller den transformerade positionen.

`SV_POSITION` är en systemsemantik som grafikpipelinen känner igen.

Utdata är en `float4` eftersom GPU:ns positionshantering använder homogena koordinater:

```text
X, Y, Z, W
```

Vår shader gör följande:

```hlsl
output.Position = float4(input.Position, 1.0f);
```

Detta kopierar den ursprungliga positionen och sätter W till 1.

Ingen transformation utförs.

Detta är avsiktligt. Vi vill först verifiera att den grundläggande renderingspipelinen fungerar innan vi introducerar matriser.

## 5.2 Fragment Shader

Fragment-shadern bestämmer färgen på de fragment som skapas genom rasteriseringen.

Vår tillfälliga testshader:

```hlsl
struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

PS_OUTPUT fragMain()
{
    PS_OUTPUT output;

    output.Color = float4(
        1.0f,
        0.0f,
        1.0f,
        1.0f
    );

    return output;
}
```

Utdatafärgen är RGBA:

| Komponent | Värde | Betydelse |
|---|---:|---|
| R | 1.0 | Maximalt rött |
| G | 0.0 | Inget grönt |
| B | 1.0 | Maximalt blått |
| A | 1.0 | Helt opak |

Resultatet blir magenta.

Vi tog avsiktligt bort texturer, samplers och materialbuffertar under detta test.

Det gjorde det möjligt att isolera geometrin och grafikpipelinen.

---

# 6. Shaderkompilering – Från HLSL till Metal

OrganicEngine använder HLSL för att skriva shaders, men grafikbackend på macOS är Metal.

GPU:n kan inte direkt exekvera vår ursprungliga HLSL-källkod.

Därför behöver vi en shaderkompileringspipeline.

Vår nuvarande byggprocess använder:

- SDL_shadercross
- Apples Metal-kompilator (`xcrun metal`)
- Apples Metal library linker (`xcrun metallib`)

### Kompileringsflödet

```text
             HLSL-KÄLLKOD
                  |
                  v
             SDL_shadercross
                  |
                  v
               MSL-KÄLLKOD
           (Metal Shading Language)
                  |
                  v
              xcrun metal
                  |
                  v
               AIR-OBJEKT
                  |
                  v
             xcrun metallib
                  |
                  v
              .metallib-FIL
                  |
                  v
          ORGANICENGINE RUNTIME
```

### Steg 1 – HLSL-källkod

Vi skriver våra shaderprogram i HLSL.

Exempelvis:

```text
Shaders/
    Triangle.vert.hlsl
    Triangle.frag.hlsl
```

### Steg 2 – SDL_shadercross

SDL_shadercross översätter våra HLSL-shaders till Metal Shading Language.

Resultatet är Metal-kompatibel shaderkod.

### Steg 3 – Apples Metal-kompilator

Metal-kompilatorn översätter MSL-källkoden till AIR, en mellanrepresentation.

AIR står för Apple Intermediate Representation.

### Steg 4 – Metal Library

Verktyget `metallib` paketerar de kompilerade AIR-objekten till ett Metal-bibliotek.

Vårt projekt använder för närvarande ett gemensamt bibliotek:

```text
OrganicEngine.metallib
```

Det innehåller både vertex- och fragment-shaderns entry points.

### Steg 5 – Ladda shaders vid körning

Vid körning laddar OrganicEngine det kompilerade Metal-biblioteket genom SDL3 GPU.

Vi skapar SDL GPU shader-objekt med rätt entry points:

```text
vertexMain
fragMain
```

Den viktiga skillnaden är:

**Shaderkompileringen sker under byggprocessen. Shaderinläsningen sker när applikationen körs.**

Det betyder att vi inte behöver kompilera HLSL varje bildruta.

---

# 7. SDL3 GPU – Skapa grafikpipelinen

Grafikpipelinen beskriver hur GPU:n ska bearbeta vår geometri.

Den kopplar samman våra shaderprogram med vertexlayouten, rasteriseringsinställningarna och renderingsmålets format.

I OrganicEngine ansvarar Renderer för att skapa och hantera denna pipeline.

### Pipelinekonfiguration

Vår nuvarande grafikpipeline innehåller följande viktiga inställningar:

| Inställning | Syfte |
|---|---|
| Vertex shader | Bearbetar vertexpositioner |
| Fragment shader | Bestämmer fragmentens färger |
| Vertex input state | Beskriver vertexbufferns layout |
| Primitive type | Definierar hur vertexar bildar geometri |
| Rasterizer state | Styr rasteriseringen av trianglar |
| Multisample state | Styr multisampling |
| Color target format | Definierar utdataformatet |

## 7.1 Vertex Input Layout

GPU:n behöver veta hur vår C++-struktur Vertex är organiserad i minnet.

Vår struktur:

```cpp
struct Vertex
{
    float position[3];
    float texCoord[2];
};
```

Motsvarande layout är:

```text
Vertex
|
+-- position
|      float
|      float
|      float
|
+-- texCoord
       float
       float
```

Vi beskriver layouten med `SDL_GPUVertexInputState`.

Buffer pitch anges med:

```cpp
sizeof(Vertex)
```

Detta talar om hur många byte GPU:n ska hoppa framåt när den går från en vertex till nästa.

Attributens offset anges med:

```cpp
offsetof(Vertex, position)
offsetof(Vertex, texCoord)
```

Det säkerställer att GPU:n läser rätt data från varje vertex.

Vertexlayouten måste stämma överens med shaderns input-attribut, deras platser och format.

Exempel:

| Shader-input | C++-attribut | Format |
|---|---|---|
| POSITION | position | FLOAT3 |
| TEXCOORD | texCoord | FLOAT2 |

Under vårt förenklade triangeltest används endast POSITION.

## 7.2 Primitive Type

Vi konfigurerar:

```cpp
SDL_GPU_PRIMITIVETYPE_TRIANGLELIST
```

Detta talar om för GPU:n att varje grupp om tre vertexar ska tolkas som en separat triangel.

Våra tre vertexar bildar alltså exakt en triangel.

## 7.3 Rasterizer State

Rasterizer-konfigurationen styr hur primitiver omvandlas till fragment.

Vår nuvarande konfiguration använder:

```cpp
fill_mode = SDL_GPU_FILLMODE_FILL;
cull_mode = SDL_GPU_CULLMODE_NONE;
```

`FILL` innebär att triangeln fylls med färg istället för att endast ritas som en kontur.

`CULL_NONE` stänger av face culling.

Det är användbart under den första renderingen eftersom trianglar inte sorteras bort baserat på sin vertexordning.

## 7.4 Color Target Format

Grafikpipelinen måste vara kompatibel med formatet på swapchain-texturen.

Vi hämtar formatet med:

```cpp
SDL_GetGPUSwapchainTextureFormat(
    device,
    window
);
```

Formatet används när grafikpipelinen skapas.

Det säkerställer att pipelinens färgutdata matchar måltexturen.

---

# 8. Rendering – GPU-kommandoflödet

När meshen och grafikpipelinen finns kan vi börja rendera.

Renderer utför renderingsarbetet varje bildruta.

Processen bygger på GPU command buffers och render passes.

## 8.1 Hämta en Command Buffer

```cpp
SDL_GPUCommandBuffer* commandBuffer =
    SDL_AcquireGPUCommandBuffer(device);
```

En command buffer används för att registrera GPU-kommandon.

Vi använder den för att tala om vilka operationer GPU:n ska utföra.

Kommandona skickas sedan in för exekvering.

## 8.2 Hämta Swapchain Texture

```cpp
SDL_GPUTexture* swapchainTexture = nullptr;

SDL_WaitAndAcquireGPUSwapchainTexture(
    commandBuffer,
    window,
    &swapchainTexture,
    nullptr,
    nullptr
);
```

Swapchainen tillhandahåller texturerna som presenteras i applikationens fönster.

Vi hämtar den aktuella texturen så att vi kan rendera till den.

Om texturen är null kan fönstret exempelvis vara minimerat eller inte redo för rendering.

## 8.3 Börja en Render Pass

Vi konfigurerar färgmålet:

```cpp
SDL_GPUColorTargetInfo colorTarget = {};

colorTarget.texture = swapchainTexture;

colorTarget.clear_color = {
    0.08f,
    0.15f,
    0.35f,
    1.0f
};

colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
colorTarget.store_op = SDL_GPU_STOREOP_STORE;
```

`clear_color` är bakgrundsfärgen.

Vi börjar sedan render pass:

```cpp
SDL_GPURenderPass* renderPass =
    SDL_BeginGPURenderPass(
        commandBuffer,
        &colorTarget,
        1,
        nullptr
    );
```

En render pass definierar de renderingsoperationer som utförs mot ett eller flera renderingsmål.

Vår pass har för närvarande ett färgmål.

`CLEAR` innebär att måltexturen rensas innan vi ritar.

`STORE` innebär att resultatet bevaras så att det kan presenteras.

## 8.4 Bind Grafikpipelinen

```cpp
SDL_BindGPUGraphicsPipeline(
    renderPass,
    graphicsPipeline
);
```

Detta talar om för SDL vilken grafikpipeline som ska användas för efterföljande draw-anrop.

Pipelinen innehåller våra shaderprogram och renderingsinställningar.

## 8.5 Bind Vertexbuffern

```cpp
SDL_GPUBufferBinding vertexBinding = {};

vertexBinding.buffer = mesh.GetVertexBuffer();
vertexBinding.offset = 0;

SDL_BindGPUVertexBuffers(
    renderPass,
    0,
    &vertexBinding,
    1
);
```

Vi binder vertexbuffern innan vi gör draw-anropet.

GPU:n kan nu komma åt vertexdatan som tillhör vår triangel.

## 8.6 Rita triangeln

```cpp
SDL_DrawGPUPrimitives(
    renderPass,
    mesh.GetVertexCount(),
    1,
    0,
    0
);
```

Argumenten beskriver:

| Argument | Värde | Betydelse |
|---|---:|---|
| vertex count | 3 | Tre vertexar |
| instance count | 1 | En instans |
| first vertex | 0 | Börja vid första vertexen |
| first instance | 0 | Börja vid första instansen |

Eftersom vi använder `TRIANGLELIST` bildar de tre vertexarna en triangel.

## 8.7 Avsluta Render Pass

```cpp
SDL_EndGPURenderPass(renderPass);
```

Detta avslutar renderingsoperationerna för den aktuella render pass.

## 8.8 Skicka in Command Buffer

```cpp
SDL_SubmitGPUCommandBuffer(commandBuffer);
```

När command buffern skickas in lämnas de registrerade GPU-kommandona över för exekvering.

GPU:n kan nu utföra renderingsarbetet.

---

# 9. Vad händer egentligen inne i GPU:n?

Efter att kommandobuffern skickats in exekverar GPU:n grafikpipelinen.

Det förenklade exekveringsflödet ser ut så här:

```text
          VERTEX BUFFER
                |
                v
           VERTEX FETCH
                |
                v
          VERTEX SHADER
                |
                v
        PRIMITIVE ASSEMBLY
                |
                v
           RASTERIZATION
                |
                v
         FRAGMENT SHADER
                |
                v
           COLOR OUTPUT
                |
                v
        SWAPCHAIN TEXTURE
```

### Vertex Fetch

GPU:n läser vertexattribut från den bundna vertexbuffern.

### Vertex Shader

Vertex-shadern körs för varje vertex.

Vår shader skickar positionen direkt till clip space.

### Primitive Assembly

GPU:n grupperar de tre bearbetade vertexarna till en triangel.

### Rasterization

Rasteriseraren avgör vilka pixlar på skärmen som täcks av triangeln.

Dessa täckta bildpunkter ger upphov till fragment.

### Fragment Shader

Fragment-shadern körs för de genererade fragmenten.

Vår shader skickar ut en konstant magentafärg.

### Color Output

Den resulterande färgen skrivs till swapchain-texturen.

Texturen kan därefter presenteras i fönstret.

---

# 10. Varför vi började med en förenklad shader

Vårt första försök använde mer avancerad shaderfunktionalitet.

Vertex-shadern innehöll:

```hlsl
cbuffer ConstantBuffer : register(b0, space1)
{
    matrix worldViewProj;
};
```

Och transformerade vertexpositionen:

```hlsl
output.Position = mul(
    worldViewProj,
    float4(input.Position, 1.0f)
);
```

Fragment-shadern använde också:

- Texture2D
- SamplerState
- Material constant buffer

Vi hade däremot ännu inte implementerat motsvarande skap