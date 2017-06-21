#include "DX12.h"

namespace dx12
{
	ID3D12Device* g_device = nullptr;

	CommandQueueManager* g_commandQueues = nullptr;

	const char* g_headerShader = R"(
	
	struct VertexShaderInput 
	{ 
		float3 vertex : VERTEX; 
		float3 normal : NORMAL;
		float2 texCoord : UV; 
	};

	struct PixelShaderInput 
	{ 
		float4 position : SV_POSITION;
		float3 normal : OUT_NORMAL;
		float2 texCoord : OUT_UV;
	};

	cbuffer FrameConstants : register(b0)
	{
		matrix <float, 4, 4> view;
		matrix <float, 4, 4> proj;
	};

	cbuffer WorldMatrix : register(b1)
	{
		matrix <float, 4, 4> model;
	};

	)";

	const char* g_defaultShader = R"(
	PixelShaderInput vs_main(VertexShaderInput input)
	{
		PixelShaderInput output;
		output.position = mul(proj, mul(view,float4(input.vertex, 1)));
		output.normal = input.normal;
		output.texCoord = input.texCoord;
		return output;
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		float light = dot(normalize(input.normal), float3(1,0,0));
		//return ColorTex.Sample(samplerColorTex, input.texCoord) * light;
		return /*float4(light,light,light,1) **/ float4(input.texCoord, 0,1);
	}

	)";

	const char* g_planetShader = R"(
	Texture2D<float4> textures[16] : register(t2);
	SamplerState texSampler : register(s2);

	PixelShaderInput vs_main(VertexShaderInput input)
	{
		PixelShaderInput output;
		output.position = mul(proj, mul(view,float4(input.vertex, 1)));
		output.normal = input.normal;
		output.texCoord = input.texCoord;
		return output;
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		float light = dot(normalize(input.normal), float3(1,0,0));
		//return float4(input.texCoord,0,1); 
		return textures[0].Sample(texSampler, input.texCoord)*light;
	}

	)";
}
