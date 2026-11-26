
struct VS_INPUT
{
    float3 Position : POSITION;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
};

// Data som skickas från CPU till vertex-shadern
[[vk::binding(0, 1)]]
cbuffer TimeUniform : register(b0, space0)
{
    float time;
};

VS_OUTPUT vertexMain(VS_INPUT input)
{
    VS_OUTPUT output;

    float3 position = input.Position;

    position.x += sin(time) * 0.5f;

    output.Position = float4(position, 1.0f);

    return output;
}
