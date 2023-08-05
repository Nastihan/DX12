#include "TransformCbuf.h"
#include "AssimpTest.h"

TransformCbuf::TransformCbuf(AssimpTest& parent)
	: pParent(&parent)
{

}

TransformCbuf::Transforms TransformCbuf::GetTransforms(Graphics& gfx)
{
	assert(pParent != nullptr);
	const auto model = pParent->GetTransform(gfx);
	const auto modelView = model * gfx.GetCamera();
	return {
		DirectX::XMMatrixTranspose(model),
		DirectX::XMMatrixTranspose(modelView),
		DirectX::XMMatrixTranspose(
			modelView *
			gfx.GetProjection()
		)
	};
}
