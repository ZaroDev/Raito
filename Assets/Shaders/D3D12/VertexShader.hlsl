struct VS_INPUT
{
	float3 vPosition	: POSITION;
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};

struct VS_OUTPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
	float4 vPosition	: SV_POSITION;
};

struct Constants
{
    float4x4 World;
    float4x4 WorldViewProjection;  
};

ConstantBuffer<Constants> Globals       : register(b0);

VS_OUTPUT VSMain(VS_INPUT Input) {
    VS_OUTPUT Output;

    Output.vPosition = mul(float4(Input.vPosition, 1 ), Globals.WorldViewProjection);
	Output.vNormal = mul(float4(Input.vPosition, 0), Globals.World).xyz;
	Output.vTexcoord = Input.vTexcoord;

    return Output;
}