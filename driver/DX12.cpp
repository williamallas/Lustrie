#define MULTILINE(...) #__VA_ARGS__

namespace dx12
{
	const char* g_shaderSrc = R"(

	struct VertexShaderInput 
	{ 
		float3 vertex : VERTEX; 
		float3 normal : NORMAL; 
	};

	struct PixelShaderInput 
	{ 
		float4 position : SV_POSITION;
	};

	PixelShaderInput vs_main(VertexShaderInput input)
	{
		PixelShaderInput output;
		output.position = float4(input.vertex+input.normal, 1);
		return output;
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		return float4(1,0,0,1);
	}
	)";
}
