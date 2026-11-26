struct PS_OUTPUT
{
    float4 Color : SV_Target0;
};

PS_OUTPUT fragMain()
{
    PS_OUTPUT output;
    output.Color = float4(1.0f, 0.0f, 1.0f, 1.0f);


    return output;
}