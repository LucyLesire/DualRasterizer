#include "pch.h"
#include "Effect.h"
#include "ECamera.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
	:BaseEffect(pDevice, assetFile)
{
	//Create world and viewInverse matrices
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
		std::wcout << L"m_pMatWorldVariable is not valid\n";

	m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if (!m_pMatViewInverseVariable->IsValid())
		std::wcout << L"m_pMatViewInverseVariable is not valid\n";

	//Create normal, specular & glossiness map
	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::wcout << L"Variable gNormalMap not found\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::wcout << L"Variable gSpecularMap not found\n";

	m_pGlosinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlosinessMapVariable->IsValid())
		std::wcout << L"Variable gGlossinessMap not found\n";

	//Set flat false
	m_pEffect->GetVariableByName("gFlat")->AsScalar()->SetBool(false);

	//Create blend state
	D3D11_BLEND_DESC blendState;
	ZeroMemory(&blendState, sizeof(D3D11_BLEND_DESC));
	blendState.RenderTarget[0].BlendEnable = FALSE;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendState.AlphaToCoverageEnable = false;
	blendState.IndependentBlendEnable = false;
	blendState.RenderTarget->BlendEnable = false;
	pDevice->CreateBlendState(&blendState, &m_pBlendState);

	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthState;
	depthState.DepthEnable = true;
	depthState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthState.DepthFunc = D3D11_COMPARISON_LESS;
	depthState.StencilEnable = false;

	depthState.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthState.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

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

Effect::~Effect()
{
	if (m_pBlendState)
	{
		m_pBlendState->Release();
	}

	if (m_pDepthState)
	{
		m_pDepthState->Release();
	}

	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}

	if (m_pGlosinessMapVariable)
	{
		m_pGlosinessMapVariable->Release();
	}

	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}

	if (m_pMatWorldVariable)
	{
		m_pMatWorldVariable->Release();
	}

	if (m_pMatViewInverseVariable)
	{
		m_pMatViewInverseVariable->Release();
	}
}

void Effect::SetMaps(ID3D11ShaderResourceView* pDiffuseMap, ID3D11ShaderResourceView* pNormalMap, ID3D11ShaderResourceView* pSpecularMap, ID3D11ShaderResourceView* pGlossinessMap)
{
	//Update maps
	if (pDiffuseMap)
	{
		if (m_pDiffuseMapVariable->IsValid())
			m_pDiffuseMapVariable->SetResource(pDiffuseMap);
	}

	if (pNormalMap)
	{
		if (m_pNormalMapVariable->IsValid())
			m_pNormalMapVariable->SetResource(pNormalMap);
	}

	if (pSpecularMap)
	{
		if (m_pSpecularMapVariable->IsValid())
			m_pSpecularMapVariable->SetResource(pSpecularMap);
	}

	if (pGlossinessMap)
	{
		if (m_pGlosinessMapVariable->IsValid())
			m_pGlosinessMapVariable->SetResource(pGlossinessMap);
	}
}

void Effect::SetMatrices(const Elite::FMatrix4& worldViewProj, const Elite::FMatrix4& world, const Elite::FMatrix4& viewInverse)
{
	//Update matrices
	Elite::FMatrix4 worldViewProjectionMatrixTmp = worldViewProj;

	if (m_pMatWorldViewProjVariable->IsValid())
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<float*>(&worldViewProjectionMatrixTmp));

	Elite::FMatrix4 worldMatrix = world;

	if (m_pMatWorldVariable->IsValid())
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<float*>(&worldMatrix));

	Elite::FMatrix4 viewInverseMatrix = viewInverse;

	if (m_pMatViewInverseVariable->IsValid())
		m_pMatViewInverseVariable->SetMatrix(reinterpret_cast<float*>(&viewInverseMatrix));
}