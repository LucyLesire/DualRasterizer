#include "pch.h"
#include "BaseEffect.h"

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

	//Create techniques with different sample modes
	ID3DX11EffectTechnique* pTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
	if (!pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";
	m_pTechniques.push_back(pTechnique);

	pTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
	if (!pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";
	m_pTechniques.push_back(pTechnique);

	pTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";
	m_pTechniques.push_back(pTechnique);

	//Create worldViewProj matrix & diffusemap
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::wcout << L"m_pMatWorldViewProjVariable is not valid\n";

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"Variable gDiffuseMap not found\n";

	//Create and set Pi
	m_pEffect->GetVariableByName("gPi")->AsScalar()->SetFloat(float(M_PI));

	//Create Rasterizer states
	D3D11_RASTERIZER_DESC rasterizerStateBack;
	ZeroMemory(&rasterizerStateBack, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateBack.FillMode = D3D11_FILL_SOLID;
	rasterizerStateBack.CullMode = D3D11_CULL_BACK;
	rasterizerStateBack.FrontCounterClockwise = true;
	rasterizerStateBack.DepthBias = false;
	rasterizerStateBack.DepthBiasClamp = 0;
	rasterizerStateBack.SlopeScaledDepthBias = 0;
	rasterizerStateBack.DepthClipEnable = true;
	rasterizerStateBack.ScissorEnable = false;
	rasterizerStateBack.MultisampleEnable = false;
	rasterizerStateBack.AntialiasedLineEnable = false;
	pDevice->CreateRasterizerState(&rasterizerStateBack, &m_pRasterizerStateBack);
	
	D3D11_RASTERIZER_DESC rasterizerStateFront;
	ZeroMemory(&rasterizerStateFront, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateFront.FillMode = D3D11_FILL_SOLID;
	rasterizerStateFront.CullMode = D3D11_CULL_FRONT;
	rasterizerStateFront.FrontCounterClockwise = true;
	rasterizerStateFront.DepthBias = false;
	rasterizerStateFront.DepthBiasClamp = 0;
	rasterizerStateFront.SlopeScaledDepthBias = 0;
	rasterizerStateFront.DepthClipEnable = true;
	rasterizerStateFront.ScissorEnable = false;
	rasterizerStateFront.MultisampleEnable = false;
	rasterizerStateFront.AntialiasedLineEnable = false;

	pDevice->CreateRasterizerState(&rasterizerStateFront, &m_pRasterizerStateFront);

	D3D11_RASTERIZER_DESC rasterizerStateNone;
	ZeroMemory(&rasterizerStateNone, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerStateNone.FillMode = D3D11_FILL_SOLID;
	rasterizerStateNone.CullMode = D3D11_CULL_NONE;
	rasterizerStateNone.FrontCounterClockwise = true;
	rasterizerStateNone.DepthBias = false;
	rasterizerStateNone.DepthBiasClamp = 0;
	rasterizerStateNone.SlopeScaledDepthBias = 0;
	rasterizerStateNone.DepthClipEnable = true;
	rasterizerStateNone.ScissorEnable = false;
	rasterizerStateNone.MultisampleEnable = false;
	rasterizerStateNone.AntialiasedLineEnable = false;
	pDevice->CreateRasterizerState(&rasterizerStateNone, &m_pRasterizerStateNone);
}

BaseEffect::~BaseEffect()
{
	if (m_pRasterizerStateBack)
	{
		m_pRasterizerStateBack->Release();
	}

	if (m_pRasterizerStateFront)
	{
		m_pRasterizerStateFront->Release();
	}

	if (m_pRasterizerStateNone)
	{
		m_pRasterizerStateNone->Release();
	}

	if (m_pBlendState)
	{
		m_pBlendState->Release();
	}

	if (m_pDepthState)
	{
		m_pDepthState->Release();
	}

	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
	}

	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}

	for (int i{}; i < m_pTechniques.size(); i++)
	{
		m_pTechniques[i]->Release();
	}

	m_pTechniques.clear();
	
	if (m_pEffect)
	{
		m_pEffect->Release();
	}

}
