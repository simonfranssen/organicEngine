// Input structure
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD1;
};

// Output structure
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

cbuffer ConstantBuffer : register(b0, space1)
{
    matrix worldViewProj; //Combined world, view, and projection matrix
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.Position = mul(
        worldViewProj,
        float4(input.Position, 1.0f)
    );
    output.TexCoord = input.TexCoord;
    return output;
}
