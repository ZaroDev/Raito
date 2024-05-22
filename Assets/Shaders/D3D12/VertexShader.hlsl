struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : Color;
};

PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.Position = position;
    result.Color = color;

    return result;
}