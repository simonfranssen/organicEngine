struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

Texture2D myTexture : register(t0, space2);
SamplerState mySampler : register(s0, space2);

cbuffer MaterialData : register(b0, space3)
{
    float4 colorMultiplier;
};

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;

    float4 texColor = myTexture.Sample(
        mySampler,
        input.TexCoord
    );

    output.Color = texColor * colorMultiplier;

    return output;
}