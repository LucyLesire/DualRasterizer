#include "pch.h"
#include "FlatEffect.h"

FlatEffect::FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	:BaseEffect(pDevice, assetFile)
{
	//Set flat on true
	m_pEffect->GetVariableByName("gFlat")->AsScalar()->SetBool(true);

	//Create blendstate
	D3D11_BLEND_DESC blendState;
	ZeroMemory(&blendState, sizeof(D3D11_BLEND_DESC));
	blendState.RenderTarget->BlendEnable = TRUE;
	blendState.RenderTarget->SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendState.RenderTarget->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget->BlendOp = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget->SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendState.RenderTarget->DestBlendAlpha = D3D11_BLEND_ZERO;
	blendState.RenderTarget->BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget->RenderTargetWriteMask = 0x0F;
	pDevice->CreateBlendState(&blendState, &m_pBlendState);

	//Create depthstencil state
	D3D11_DEPTH_STENCIL_DESC depthState;
	depthState.DepthEnable = true;
	depthState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthState.DepthFunc = D3D11_COMPARISON_LESS;
	depthState.StencilEnable = false;
	
	depthState.StencilReadMask = 0x0F;
	depthState.StencilWriteMask = 0x0F;

	depthState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthState.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	
	depthState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;

	depthState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	pDevice->CreateDepthStencilState(&depthState, &m_pDepthState);
}

FlatEffect::~FlatEffect()
{
}

void FlatEffect::SetMaps(ID3D11ShaderResourceView* pDiffuseMap, ID3D11ShaderResourceView* pNormalMap, ID3D11ShaderResourceView* pSpecularMap, ID3D11ShaderResourceView* pGlossinessMap)
{
	//Update diffuse map
	if (pDiffuseMap)
	{
		if (m_pDiffuseMapVariable->IsValid())
			m_pDiffuseMapVariable->SetResource(pDiffuseMap);
	}
}

void FlatEffect::SetMatrices(const Elite::FMatrix4& worldViewProj, const Elite::FMatrix4& world, const Elite::FMatrix4& viewInverse)
{
	//Update worldViewProj matrix
	Elite::FMatrix4 worldViewProjectionMatrixTmp = worldViewProj;

	if (m_pMatWorldViewProjVariable->IsValid())
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<float*>(&worldViewProjectionMatrixTmp));

}
