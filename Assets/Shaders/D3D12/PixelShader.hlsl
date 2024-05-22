Texture2D	g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );

struct PS_INPUT
{
	float3 vNormal		: NORMAL;
	float2 vTexcoord	: TEXCOORD0;
};
float4 PSMain(PS_INPUT Input) : SV_TARGET {
    
    float4 vDiffuse = g_txDiffuse.Sample( g_samLinear, Input.vTexcoord );
    return vDiffuse;
}