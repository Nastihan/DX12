#pragma once
#include "Graphics.h"
#include <DirectXMath.h>
#include <array>
#include "Drawable.h"
#include <d3dcompiler.h>
#include "GraphicsError.h"
#include <DirectXMath.h>
#include <chrono>
#include <DxTex/include/DirectXTex.h>
#include <ranges>
#include <iostream>
#include "SphereGeo.h"
#include "BindableInclude.h"


class Sphere : public Drawable
{
public:
	Sphere(Graphics& gfx)
	{
		// Vertex data structure
		struct Vertex
		{
			DirectX::XMFLOAT3 pos;
		};
		auto model = SphereGeo::Make<Vertex>();
		model.Transform(DirectX::XMMatrixScaling(0.2f, 0.2f, 0.2f));
		// Vertex buffer stuff
		{
			pVertexBuffer = std::make_unique<VertexBuffer>(gfx, model.vertices);
		}
		// Index buffer stuff
		{
			pIndexBuffer = std::make_unique<IndexBuffer>(gfx, model.indices);
		}
		
		// define root signature with a matrix of 16 32-bit floats used by the vertex shader (rotation matrix) 
		CD3DX12_ROOT_PARAMETER rootParameters[1]{};
		rootParameters[0].InitAsConstants(3 * (sizeof(DirectX::XMMATRIX) / 4), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		
		// static sampler
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		// Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
		const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		rootSignatureDesc.Init((UINT)std::size(rootParameters), rootParameters,
			0U,nullptr , rootSignatureFlags);
		// serialize root signature
		Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
		if (const auto hr = D3D12SerializeRootSignature(
			&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			&signatureBlob, &errorBlob); FAILED(hr)) {
			if (errorBlob) {
				auto errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());

				std::cout << " Error regarding Root Signature Serialization : " << errorBufferPtr << std::endl;
			}
			hr >> chk;
		}
		// Create root signature
		gfx.Device()->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)) >> chk;

		// create pso structure
		Graphics::PipelineStateStream pipelineStateStream;

		// define the vertex input layout
		const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0 ,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		};

		// load the VS & PS
		Microsoft::WRL::ComPtr<ID3DBlob> BlobVS;
		D3DReadFileToBlob(L"SolidLightVS.cso", &BlobVS) >> chk;
		Microsoft::WRL::ComPtr<ID3DBlob> BlobPS;
		D3DReadFileToBlob(L"SolidLightPS.cso", &BlobPS) >> chk;

		// filling pso structure
		pipelineStateStream.RootSignature = pRootSignature.Get();
		pipelineStateStream.InputLayout = { inputLayout,(UINT)std::size(inputLayout) };
		pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE(BlobVS.Get());
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE(BlobPS.Get());
		pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateStream.RTVFormats = {
			.RTFormats{ DXGI_FORMAT_R8G8B8A8_UNORM },
			.NumRenderTargets = 1,
		};

		// building pipeline state object
		D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
			sizeof(pipelineStateStream), &pipelineStateStream
		};
		gfx.Device()->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pPipelineState)) >> chk;
	}
	void Draw(Graphics& gfx) const override
	{
		//gfx.ResetCmd();
		gfx.CommandList()->SetPipelineState(pPipelineState.Get());
		gfx.CommandList()->SetGraphicsRootSignature(pRootSignature.Get());
		gfx.CommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfx.CommandList()->IASetVertexBuffers(0, 1, &pVertexBuffer->vertexBufferView);
		gfx.CommandList()->IASetIndexBuffer(&pIndexBuffer->indexBufferView);
		// trasnform
		auto transform = std::make_unique<TransformCbuf>(*this);
		auto mvp = transform->GetTransforms(gfx);
		gfx.CommandList()->SetGraphicsRoot32BitConstants(0, sizeof(mvp) / 4, &mvp, 0);
		gfx.ConfigForDraw();
		gfx.CommandList()->DrawIndexedInstanced(pIndexBuffer->nIndices, 1, 0, 0, 0);
		//gfx.Execute();
		//gfx.Sync();
	}
	DirectX::XMMATRIX GetTransform() const noexcept override
	{
		return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	}
	void SetPos(DirectX::XMFLOAT3 pos)
	{
		this->pos = pos;
	}
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineState;
	// vertex buffer 
	std::unique_ptr<VertexBuffer> pVertexBuffer;
	// index buffer 
	std::unique_ptr<IndexBuffer> pIndexBuffer;
	// position
	DirectX::XMFLOAT3 pos;

	

};
