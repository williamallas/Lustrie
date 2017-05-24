#include "DX12.h"

#define MULTILINE(...) #__VA_ARGS__

namespace dx12
{
	ID3D12Device* g_device = nullptr;

	CommandQueueManager* g_commandQueues = nullptr;

	const char* g_shaderSrc = R"(

	struct VertexShaderInput 
	{ 
		float3 vertex : VERTEX; 
	};

	struct PixelShaderInput 
	{ 
		float4 position : SV_POSITION;
	};

	PixelShaderInput vs_main(VertexShaderInput input)
	{
		PixelShaderInput output;
		output.position = float4(input.vertex, 1);
		return output;
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		return float4(1,0,0,1);
	}
	)";
}
