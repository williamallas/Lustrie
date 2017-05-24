#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <EASTL/vector.h>
#include <EASTL/array.h>
#include <EASTL/string.h>
#include <core/type.h>

#include <dxgi1_4.h>
#include <wrl.h>

namespace dx12
{
	extern ID3D12Device* g_device;

	class CommandQueueManager;
	extern CommandQueueManager* g_commandQueues;

	extern const char* g_shaderSrc;
}