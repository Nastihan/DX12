#pragma once
#include <DirectXMath.h>
#include "Graphics.h"

class AssimpTest;

class TransformCbuf
{
private:
	struct Transforms
	{
		DirectX::XMMATRIX model;
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX modelViewProj;
	};
public:
	TransformCbuf(AssimpTest& parent);
	Transforms GetTransforms(Graphics& gfx);
private:
	AssimpTest* pParent = nullptr;
};
