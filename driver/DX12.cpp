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

		float4x4 model : MODEL_MATRIX;

		int4 material_textures : MATERIAL_TEXTURES;
		float4 material : MATERIAL;
	};

	struct PixelShaderInput 
	{ 
		float4 position : SV_POSITION;
		float3 normal : OUT_NORMAL;
		float2 texCoord : OUT_UV;

		int4 textures : OUT_MATERIAL_TEXTURES;
		float4 material : OUT_MATERIAL;
	};

	cbuffer FrameConstants : register(b0)
	{
		matrix <float, 4, 4> view;
		matrix <float, 4, 4> proj;
		matrix <float, 4, 4> projView;
		float4 cameraPos;
	};

	Texture2D<float4> textures[16] : register(t2);
	SamplerState texSampler : register(s2);

	)";

	const char* g_defaultShader = R"(
	PixelShaderInput vs_main(VertexShaderInput input)
	{
		PixelShaderInput output;
		output.position = mul(projView, mul(input.model, float4(input.vertex, 1)));
		output.normal = mul(float3x3(input.model._m00_m01_m02, input.model._m10_m11_m12, input.model._m20_m21_m22), input.normal);
		output.texCoord = input.texCoord;

		output.textures = input.material_textures;
		output.material = input.material;
		return output;
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		float light = dot(normalize(-input.normal), float3(1,0,0));
		return textures[input.textures[0]].Sample(texSampler, input.texCoord)*light;
	}

	)";

	const char* g_planetShader = R"(

	PixelShaderInput vs_main(VertexShaderInput input)
	{
		PixelShaderInput output;
		output.position = mul(projView, float4(input.vertex, 1));
		output.normal = input.normal;
		output.texCoord = input.texCoord;

		output.textures = input.material_textures;
		output.material = input.material;
		return output;
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		float light = dot(normalize(input.normal), float3(1,0,0));
		//return float4(input.texCoord,0,1); 
		return textures[0].Sample(texSampler, input.texCoord*2)*light;
	}

	)";

	const char* g_grassShader = R"(
	
	struct VertexShaderInput 
	{ 
		float3 vertex : VERTEX; 
		float3 normal : NORMAL;

		float4x4 model : MODEL_MATRIX;

		int4 material_textures : MATERIAL_TEXTURES;
		float4 material : MATERIAL;
	};

	struct PixelShaderInput 
	{ 
		float4 position : SV_POSITION;

		int4 textures : OUT_MATERIAL_TEXTURES;
		float4 material : OUT_MATERIAL;

		float3 normal : OUT_NORMAL;
		float darkness : DARK;
		float2 texCoord : OUT_UV;
	};

	struct GeometryShaderInput
	{ 
		float3 vertex : GS_VERTEX; 
		float3 normal : GS_NORMAL; 

		int4 textures : GS_MATERIAL_TEXTURES;
		float4 material : GS_MATERIAL;
	};

	cbuffer FrameConstants : register(b0)
	{
		matrix <float, 4, 4> view;
		matrix <float, 4, 4> proj;
		matrix <float, 4, 4> projView;
		float4 cameraPos;
	};

	Texture2D<float4> textures[16] : register(t2);
	SamplerState texSampler : register(s2);
	
	GeometryShaderInput vs_main(VertexShaderInput input)
	{
		GeometryShaderInput output;
		output.vertex = input.vertex;
		output.normal = input.normal;

		output.textures = input.material_textures;
		output.material = input.material;
		return output;
	}

	float randf(in float3 uv)
	{
		float3 noise = (frac(sin(dot(uv ,float3(12.9898,78.233, 144.609)*2.0)) * 43758.5453));
		return abs(noise.x + noise.y + noise.z) * 0.333333;
	}

	[maxvertexcount(8)]
	void gs_main(point GeometryShaderInput input[1], inout TriangleStream<PixelShaderInput> OutputStream)
	{
		const float BASE_HEIGHT = 0.15;
		const float RAND_HEIGHT = 0.15;		
		const float WIDTH = 0.02;	
		const float SPEED = 1;		
		const float AMPLITUDE = 0.05;
		const float SPREAD_WAVE = 1;	

		matrix <float, 4, 4> projViewIn = mul(proj, view);
		float3 up = normalize(input[0].vertex);
		float3 side = cross(up, normalize(input[0].vertex - float3(cameraPos[0],cameraPos[1],cameraPos[2])));
		
		float3 absDir = cross(up, normalize(float3(0.4,-0.5,0.1)));
		absDir = AMPLITUDE * absDir * sin(cameraPos[3]*SPEED + (input[0].vertex[2] + input[0].vertex[1] + input[0].vertex[0])*SPREAD_WAVE);	
		
		float randHeight = randf(input[0].vertex + float3(234567,32556,11676)) * RAND_HEIGHT;
		float split[2] = {0.55,0.85};		

		PixelShaderInput output;
		output.normal = input[0].normal;
        output.texCoord = float2(0,0);
		
		output.textures = input[0].textures;
		output.material = input[0].material;

		float4 positions[7];
		positions[0] = mul(projViewIn,float4(input[0].vertex-side*WIDTH,1));
		positions[1] = mul(projViewIn,float4(input[0].vertex+side*WIDTH,1));

		positions[2] = mul(projViewIn,float4(absDir*0.4 + up*(BASE_HEIGHT+randHeight)*split[0] + input[0].vertex-side*WIDTH*0.8,1));
		positions[3] = mul(projViewIn,float4(absDir*0.4 + up*(BASE_HEIGHT+randHeight)*split[0] + input[0].vertex+side*WIDTH*0.8,1));

		positions[4] = mul(projViewIn,float4(absDir*0.8 + up*(BASE_HEIGHT+randHeight)*split[1] + input[0].vertex-side*WIDTH*0.4,1));
		positions[5] = mul(projViewIn,float4(absDir*0.8 + up*(BASE_HEIGHT+randHeight)*split[1] + input[0].vertex+side*WIDTH*0.4,1));

		positions[6] = mul(projViewIn,float4(absDir + up*(BASE_HEIGHT+randHeight) + input[0].vertex,1));
		
		float texOff = randf(input[0].vertex) * 0.9;		

		output.darkness = 0.3;
		output.position = positions[0]; output.texCoord = float2(0+texOff,0); OutputStream.Append(output);
		output.position = positions[1]; output.texCoord = float2(0.1+texOff,0); OutputStream.Append(output);
		
		output.darkness = 0.6;
		output.position = positions[2]; output.texCoord = float2(0+texOff,split[0]); OutputStream.Append(output);
		output.position = positions[3]; output.texCoord = float2(0.1+texOff,split[0]); OutputStream.Append(output);

		output.darkness = 1;
		output.position = positions[4]; output.texCoord = float2(0+texOff,split[1]); OutputStream.Append(output);
		output.position = positions[5]; output.texCoord = float2(0.1+texOff,split[1]); OutputStream.Append(output);
		output.position = positions[6]; output.texCoord = float2(0.05+texOff,1); OutputStream.Append(output);	
	}

	float4 ps_main(PixelShaderInput input) : SV_TARGET
	{
		//return float4(input.normal*0.5+0.5, 1);
		float light = dot(normalize(input.normal), float3(1,0,0));
		return input.darkness * light * textures[1].Sample(texSampler, input.texCoord*2);
	}

	)";
}
