#pragma once
#include <vector>
#include "ECamera.h"
#include "Texture.h"
class BaseEffect;

class Mesh
{
public:
	Mesh(ID3D11Device* pDevice, std::vector<Elite::Vertex_Input> vertices, std::vector<uint32_t> indices, bool flat = false);
	~Mesh();

	void Render(ID3D11DeviceContext* pDeviceContext, Elite::Filtering filter, Elite::CullMode cull, const Elite::FMatrix4& world);

	void SetCamera(Elite::Camera* pCamera);
	void Update(float dT, const Elite::FMatrix4& world);
private:
	BaseEffect* m_pEffect = nullptr;

	ID3D11InputLayout* m_pVertexLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;

	uint32_t m_AmountIndices{};

	Elite::Camera* m_pCamera = nullptr;

	Elite::FMatrix4 m_WorldViewProjection;
	Elite::FMatrix4 m_World;

	Elite::Texture* m_pDiffuseTexture = nullptr;
	Elite::Texture* m_pNormalTexture = nullptr;
	Elite::Texture* m_pSpecularTexture = nullptr;
	Elite::Texture* m_pGlosinessTexture = nullptr;

	bool m_Flat;
	float m_Timer = 0.f;
};