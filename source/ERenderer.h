/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include <vector>
#include "ECamera.h"
#include "Texture.h"

struct SDL_Window;
struct SDL_Surface;

class Mesh;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render();

		void Update(float dT);

		void SetCamera(Camera* pCamera);
		void ToggleRenderMode();
		void ToggleCullMode();
		void ToggleSample();
		void ToggleRotation();
		void ToggleFireMesh();

	private:
		//Directx11 Initalization
		HRESULT InitializeDirectX();

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGIFactory* m_pDXGIFactory;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		bool m_IsInitialized;

		//Basic Window
		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;

		//Software Rasterizer
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		std::vector<float> m_DepthBuffer{};

		void ProjectionStage();
		void RasterizerStage();

		bool IsInTriangle(Elite::Vertex_Input& pointToHit, const std::vector<Elite::Vertex_Input>& ndcPoints) const;
		Elite::RGBColor PixelShading(const Elite::Vertex_Input& v) const;

		//Vertices
		std::vector<Elite::Vertex_Input> m_Vertices;
		std::vector<Elite::Vertex_Input> m_TransformedVertices;
		std::vector<uint32_t> m_Indices;

		std::vector<Elite::Triangle> m_Triangles;
		std::vector<Elite::Triangle> m_TransformedTriangles;

		//Textures
		Elite::Texture m_TextureDiffuse;
		Elite::Texture m_TextureNormal;
		Elite::Texture m_TexureSpecularMap;
		Elite::Texture m_TextureGlossiness;

		//Meshes
		Mesh* m_pMesh;
		Mesh* m_pCombustion;

		//Camera
		Camera* m_pCamera;

		//Other
		Elite::Filtering m_Filter;
		Elite::CullMode m_Cull;
		bool m_UsingFireMesh;
		bool m_Rotating;
		bool m_UsingDirectx11;
		float m_Timer;
		Elite::FMatrix4 m_World;
	};
}

#endif