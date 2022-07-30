#include "pch.h"
#include "Mesh.h"
#include "Effect.h"
#include "FlatEffect.h"
#include "BaseEffect.h"
#include "EHelper.h"

Mesh::Mesh(ID3D11Device* pDevice, std::vector<Elite::Vertex_Input> vertices, std::vector<uint32_t> indices, bool flat)
	:m_Flat{flat}
	,m_Timer{0.f}
{
	m_World = Elite::FMatrix4();
	m_WorldViewProjection = Elite::FMatrix4();
	if (flat)
	{
		//Load & initalize flat effect
		m_pEffect = new FlatEffect(pDevice, L"Resources/PosCol3D.fx");
		m_pDiffuseTexture = new Elite::Texture(pDevice, "Resources/fireFX_diffuse.png");
	}
	else
	{
		//Load & initalize normal effect
		m_pEffect = new Effect(pDevice, L"Resources/PosCol3D.fx");
		m_pDiffuseTexture = new Elite::Texture(pDevice, "Resources/vehicle_diffuse.png");
		m_pNormalTexture = new Elite::Texture(pDevice, "Resources/vehicle_normal.png");
		m_pSpecularTexture = new Elite::Texture(pDevice, "Resources/vehicle_specular.png");
		m_pGlosinessTexture = new Elite::Texture(pDevice, "Resources/vehicle_gloss.png");
	}

	//Create Vertex Layout
	HRESULT result = S_OK;
	static const uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 16;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create Input Layout
	D3DX11_PASS_DESC passDesc;
	m_pEffect->GetTechniques()[0]->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pVertexLayout);
	if (FAILED(result))
		return;

	//Create vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Elite::Vertex_Input) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	//Create Index Buffer
	m_AmountIndices = (uint32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}

Mesh::~Mesh()
{
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
	}
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
	}

	delete m_pEffect;
	m_pEffect = nullptr;

	delete m_pDiffuseTexture;
	m_pDiffuseTexture = nullptr;

	delete m_pGlosinessTexture;
	m_pGlosinessTexture = nullptr;

	delete m_pNormalTexture;
	m_pNormalTexture = nullptr;

	delete m_pSpecularTexture;
	m_pSpecularTexture = nullptr;
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, Elite::Filtering filter, Elite::CullMode cull, const Elite::FMatrix4& world)
{
	//Set Vertex Buffer
	UINT stride = sizeof(Elite::Vertex_Input);
	UINT offset = 0;

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Input Buffer
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set effect states
	if (m_Flat)
	{
		//Set flat effect cullmode to no culling
		pDeviceContext->RSSetState(m_pEffect->GetNoneRasterizerState());
	}
	else
	{
		//Set normal effect cullmode
		if(cull == Elite::CullMode::back)
			pDeviceContext->RSSetState(m_pEffect->GetBackRasterizerState());
		else if(cull == Elite::CullMode::front)
			pDeviceContext->RSSetState(m_pEffect->GetFrontRasterizerState());
		else if(cull == Elite::CullMode::none)
			pDeviceContext->RSSetState(m_pEffect->GetNoneRasterizerState());
	}
	
	//Set depth & blend states
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT sampleMask = 0xffffffff;
	pDeviceContext->OMSetDepthStencilState(m_pEffect->GetDepthState(), 0);
	pDeviceContext->OMSetBlendState(m_pEffect->GetBlendState(), blendFactor, sampleMask);

	//Render mesh
	if (filter == Elite::Filtering::point)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		m_pEffect->GetTechniques()[0]->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechniques()[0]->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
		}
	}
	if (filter == Elite::Filtering::linear)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		m_pEffect->GetTechniques()[1]->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechniques()[1]->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
		}
	}
	if (filter == Elite::Filtering::anisotropic)
	{
		D3DX11_TECHNIQUE_DESC techDesc;
		m_pEffect->GetTechniques()[2]->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechniques()[2]->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
		}
	}
}

void Mesh::SetCamera(Elite::Camera* pCamera)
{
	m_pCamera = pCamera;
}

void Mesh::Update(float dT, const Elite::FMatrix4& world)
{
	//Update Matrices & maps
	m_WorldViewProjection = m_pCamera->GetProjectionMatrix() * m_pCamera->GetWorldToView() * world;
	m_pEffect->SetMatrices(m_WorldViewProjection, world, m_pCamera->GetViewToWorld());

	if (m_Flat)
	{
		m_pEffect->SetMaps(m_pDiffuseTexture->GetResourceView());
	}
	else
	{
		m_pEffect->SetMaps(m_pDiffuseTexture->GetResourceView(), m_pNormalTexture->GetResourceView(), m_pSpecularTexture->GetResourceView(), m_pGlosinessTexture->GetResourceView());
	}
}
