#pragma once
#include "Graphics.h"

class IndexBuffer
{
public:
	IndexBuffer(Graphics& gfx, std::vector<WORD>& indices);
public: 
	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	UINT nIndices;
};