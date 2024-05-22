struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : Color;
};


float4 PSMain(PSInput input) : SV_TARGET
{
    return input.Color;
}