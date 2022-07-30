#pragma once
#include <sstream>
#include <vector>

class BaseEffect
{
public:
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	~BaseEffect();

	virtual ID3D11RasterizerState* GetFrontRasterizerState() const = 0;
	virtual ID3D11RasterizerState* GetBackRasterizerState() const = 0;
	virtual ID3D11RasterizerState* GetNoneRasterizerState() const = 0;
	virtual ID3D11DepthStencilState* GetDepthState() const = 0;
	virtual ID3D11BlendState* GetBlendState() const = 0;
	virtual ID3DX11Effect* GetEffect() const = 0;

	std::vector<ID3DX11EffectTechnique*> GetTechniques() { return m_pTechniques; };

	virtual void SetMatrices(const Elite::FMatrix4& worldViewProj, const Elite::FMatrix4& world = Elite::FMatrix4(), const Elite::FMatrix4& viewInverse = Elite::FMatrix4()) = 0;
	virtual void SetMaps(ID3D11ShaderResourceView* pDiffuseMap, ID3D11ShaderResourceView* pNormalMap = nullptr, ID3D11ShaderResourceView* pSpecularMap = nullptr, ID3D11ShaderResourceView* pGlossinessMap = nullptr) = 0;

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result = S_OK;
		ID3D10Blob* pErrorBlob = nullptr;
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				char* pErrors = (char*)pErrorBlob->GetBufferPointer();

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
					ss << pErrors[i];

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}

		return pEffect;
	}
protected:
	ID3DX11Effect* m_pEffect;
	
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	std::vector<ID3DX11EffectTechnique*> m_pTechniques;

	ID3D11RasterizerState* m_pRasterizerStateNone;
	ID3D11RasterizerState* m_pRasterizerStateBack;
	ID3D11RasterizerState* m_pRasterizerStateFront;
	ID3D11BlendState* m_pBlendState;
	ID3D11DepthStencilState* m_pDepthState;
};

