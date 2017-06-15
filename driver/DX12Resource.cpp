#include "DX12Resource.h"
#include <core/Logger.h>

namespace dx12
{
	/********************/
	/*     Resource     */
	/********************/

	D3D12_VERTEX_BUFFER_VIEW BaseResource::vertexBufferView(uint32_t numVertices, uint32_t stride, uint32_t offset) const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = _gpuVirtualAdress + offset;
		vbv.SizeInBytes = numVertices * stride;
		vbv.StrideInBytes = stride;
		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW BaseResource::indexBufferView(uint32_t numIndices, uint32_t offset, bool is32bit) const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = _gpuVirtualAdress + offset;
		ibv.Format = is32bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		ibv.SizeInBytes = numIndices * (is32bit ? 4 : 2);
		return ibv;
	}
}