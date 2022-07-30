#pragma once
#include "BaseEffect.h"

class FlatEffect final : public BaseEffect
{
public:
	FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~FlatEffect();

	ID3DX11Effect* GetEffect() const override { return m_pEffect; };

	void SetMaps(ID3D11ShaderResourceView* pDiffuseMap, ID3D11ShaderResourceView* pNormalMap = nullptr, ID3D11ShaderResourceView* pSpecularMap = nullptr, ID3D11ShaderResourceView* pGlossinessMap = nullptr) override;
	void SetMatrices(const Elite::FMatrix4& worldViewProj, const Elite::FMatrix4& world = Elite::FMatrix4(), const Elite::FMatrix4& viewInverse = Elite::FMatrix4()) override;

	ID3D11RasterizerState* GetFrontRasterizerState() const override { return m_pRasterizerStateFront; };
	ID3D11RasterizerState* GetBackRasterizerState() const override { return m_pRasterizerStateBack; };
	ID3D11RasterizerState* GetNoneRasterizerState() const override { return m_pRasterizerStateNone; };

	ID3D11DepthStencilState* GetDepthState() const override { return m_pDepthState; };
	ID3D11BlendState* GetBlendState() const override { return m_pBlendState; };

private:
};

