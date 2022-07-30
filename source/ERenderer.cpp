#include "pch.h"

//Project includes
#include "ERenderer.h"
#include "EOBJParser.h"
#include "Mesh.h"
#include "EBRDF.h"

Elite::Renderer::Renderer(SDL_Window * pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
	, m_TextureDiffuse{ "Resources/vehicle_diffuse.png" }
	, m_TextureNormal{ "Resources/vehicle_normal.png" }
	, m_TextureGlossiness{ "Resources/vehicle_gloss.png" }
	, m_TexureSpecularMap{ "Resources/vehicle_specular.png" }
{
	//Initialize Window
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	//Initialize Software Rasterizer
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//Initialize DirectX pipeline
	InitializeDirectX();
	m_IsInitialized = true;
	std::cout << "DirectX is ready\n";

	std::vector<Elite::Vertex_Input> vertices;
	std::vector<uint32_t> indices;
	Elite::ParseOBJ("Resources/vehicle.obj", vertices, indices);

	//Calculate tangents
	for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t index0 = indices[i];
		uint32_t index1 = indices[i + 1];
		uint32_t index2 = indices[i + 2];

		const FPoint3& p0 = vertices[index0].position.xyz;
		const FPoint3& p1 = vertices[index1].position.xyz;
		const FPoint3& p2 = vertices[index2].position.xyz;
		const FVector2& uv0 = vertices[index0].uv;
		const FVector2& uv1 = vertices[index1].uv;
		const FVector2& uv2 = vertices[index2].uv;

		const FVector3& edge0 = p1 - p0;
		const FVector3& edge1 = p2 - p0;
		const FVector2& diffX = FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
		const FVector2& diffY = FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Cross(diffX, diffY);

		FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
		vertices[index0].tangent += tangent;
		vertices[index1].tangent += tangent;
		vertices[index2].tangent += tangent;
	}

	for (auto& v : vertices)
	{
		v.tangent = GetNormalized(Reject(v.tangent, v.normal));
		v.position.z = -v.position.z;
		v.normal.z = -v.normal.z;
		v.tangent.z = -v.tangent.z;
	}

	//Vehicle Mesh
	m_pMesh = new Mesh(m_pDevice, vertices, indices);

	//Store vertices vehicle
	m_Vertices = vertices;
	m_Indices = indices;
	m_TransformedVertices.resize(m_Vertices.size());

	//Fire Mesh
	Elite::ParseOBJ("Resources/fireFX.obj", vertices, indices);
	m_pCombustion = new Mesh(m_pDevice, vertices, indices, true);

	//Initialize WorldMatrix
	m_World[0] = { 1.f, 0.f, 0.f, 0.f };
	m_World[1] = { 0.f, 1.f, 0.f, 0.f };
	m_World[2] = { 0.f, 0.f, 1.f, 0.f };
	m_World[3] = { 0.f, 0.f, -50.f, 1.f };

	//Initialize Depth Buffer
	m_DepthBuffer.resize(m_Height * m_Width);
	std::fill(m_DepthBuffer.begin(), m_DepthBuffer.end(), FLT_MAX);

	//Information output
	std::cout << "Rotation: starting without rotating\n";
	std::cout << "Cull mode: starting with backface culling\n";
	std::cout << "Render mode: starting with software rasterizer\n";
	std::cout << "Sample mode: starting with point filtering\n";
	std::cout << "Fire mesh: starting without fire mesh\n";
}

Elite::Renderer::~Renderer()
{
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
	}
	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
	}
	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
	}
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
	}
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
	}
	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
	}

	delete m_pMesh;
	m_pMesh = nullptr;

	delete m_pCombustion;
	m_pCombustion = nullptr;
}

void Elite::Renderer::Render()
{
	if (m_UsingDirectx11)
	{
		if (!m_IsInitialized)
			return;

		//Clear Buffers
		RGBColor clearColor = RGBColor(0.1f, 0.1f, 0.1f);
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Render
		m_pMesh->Render(m_pDeviceContext, m_Filter, m_Cull, m_World);
		if(m_UsingFireMesh)
			m_pCombustion->Render(m_pDeviceContext, m_Filter, m_Cull, m_World);

		//Present
		m_pSwapChain->Present(0, 0);
	}
	else
	{
		//Clear Buffers
		SDL_LockSurface(m_pBackBuffer);
		SDL_FillRect(m_pBackBuffer, NULL, 0x191919);

		//Render
		ProjectionStage();
		RasterizerStage();

		//Reset Depth Buffer
		std::fill(m_DepthBuffer.begin(), m_DepthBuffer.end(), FLT_MAX);

		//Clean up
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
}

void Elite::Renderer::ProjectionStage()
{
	for (int i{}; i < m_Vertices.size(); i++)
	{	
		FMatrix4 ONB = (m_pCamera->GetWorldToView());
		FMatrix4 worldViewProjectionMatrix = m_pCamera->GetProjectionMatrix() * (ONB) * m_World;

		//Transform vertices
		Vertex_Input transFormedVertix{};
		transFormedVertix.position = worldViewProjectionMatrix * m_Vertices[i].position;
		transFormedVertix.uv = m_Vertices[i].uv;
		transFormedVertix.normal = Elite::FVector3((m_World * Elite::FVector4{ m_Vertices[i].normal.x, m_Vertices[i].normal.y, m_Vertices[i].normal.z, 1.f }).xyz);
		transFormedVertix.tangent = Elite::FVector3((m_World * Elite::FVector4{ m_Vertices[i].tangent.x, m_Vertices[i].tangent.y, m_Vertices[i].tangent.z, 1.f }).xyz);
		transFormedVertix.viewDirection = Elite::FVector3((m_World * m_Vertices[i].position - ONB[3]).xyz);

		//Make the mesh visible
		transFormedVertix.position.w *= 10;

		//Perspective divide
		transFormedVertix.position.x = transFormedVertix.position.x / transFormedVertix.position.w;
		transFormedVertix.position.y = transFormedVertix.position.y / transFormedVertix.position.w;
		transFormedVertix.position.z = transFormedVertix.position.z / transFormedVertix.position.w;

		transFormedVertix.viewDirection = GetNormalized(transFormedVertix.viewDirection);

		m_TransformedVertices[i] = (transFormedVertix);
	}
}

void Elite::Renderer::RasterizerStage()
{
	for (int i{}; i < m_Indices.size(); i+=3)
	{
		std::vector<Elite::Vertex_Input> NDCVertices = { m_TransformedVertices[m_Indices[i]], m_TransformedVertices[m_Indices[i + 1]], m_TransformedVertices[m_Indices[i + 2]] };

		//Frustum culling
		bool culling = false;
		for (int i{}; i < NDCVertices.size(); i++)
		{
			if (NDCVertices[i].position.x < -1.0f || NDCVertices[i].position.x > 1.0f)
			{
				culling = true;
			}
			if (NDCVertices[i].position.y < -1.0f || NDCVertices[i].position.y > 1.0f)
			{
				culling = true;
			}
			if (NDCVertices[i].position.z < 0.0f || NDCVertices[i].position.z > 1.0f)
			{
				culling = true;
			}

			//Rasterization stage
			NDCVertices[i].position.x = ((NDCVertices[i].position.x + 1) / 2.0f) * m_Width;
			NDCVertices[i].position.y = ((1 - NDCVertices[i].position.y) / 2.0f) * m_Height;
		}

		if (!culling)
		{
			//Bounding Box
			Elite::FPoint2 topLeft = Elite::FPoint2{ std::min(std::min(NDCVertices[0].position.x, NDCVertices[1].position.x), NDCVertices[2].position.x),
				std::min(std::min(NDCVertices[0].position.y, NDCVertices[1].position.y), NDCVertices[2].position.y) };

			Elite::FPoint2 bottomRight = Elite::FPoint2{ std::max(std::max(NDCVertices[0].position.x, NDCVertices[1].position.x), NDCVertices[2].position.x),
				std::max(std::max(NDCVertices[0].position.y, NDCVertices[1].position.y), NDCVertices[2].position.y) };

			topLeft.x = Elite::Clamp(topLeft.x, 0.f, float(m_Width));
			topLeft.y = Elite::Clamp(topLeft.y, 0.f, float(m_Height));
			bottomRight.x = Elite::Clamp(bottomRight.x, 0.f, float(m_Width));
			bottomRight.y = Elite::Clamp(bottomRight.y, 0.f, float(m_Height));

			for (uint32_t r = uint32_t(topLeft.y); r < bottomRight.y; ++r)
			{
				for (uint32_t c = uint32_t(topLeft.x); c < bottomRight.x; ++c)
				{
					Elite::Vertex_Input pixel{};
					pixel.position = { float(c), float(r), 0, 0 };

					if (IsInTriangle(pixel, NDCVertices))
					{
						if (pixel.position.z < m_DepthBuffer[c + (r * m_Width)])
						{
							m_DepthBuffer[c + (r * m_Width)] = pixel.position.z;
							Elite::RGBColor finalColor{};
							finalColor += PixelShading(pixel);

							finalColor.MaxToOne();
							m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(uint8_t(finalColor.r * 255)),
								static_cast<uint8_t>(uint8_t(finalColor.g * 255)),
								static_cast<uint8_t>(uint8_t(finalColor.b * 255)));
						}

					}
				}
			}
		}
	}
}

bool Elite::Renderer::IsInTriangle(Elite::Vertex_Input& pointToHit, const std::vector<Elite::Vertex_Input>& ndcPoints) const
{
	//Baycentric
	Elite::FVector2 pointToSideA = FVector2((pointToHit.position - ndcPoints[0].position));
	Elite::FVector2 pointToSideB = FVector2((pointToHit.position - ndcPoints[1].position));
	Elite::FVector2 pointToSideC = FVector2((pointToHit.position - ndcPoints[2].position));

	const Elite::FVector2 edgeA{ ndcPoints[1].position - ndcPoints[0].position };
	const Elite::FVector2 edgeB{ ndcPoints[2].position - ndcPoints[1].position };
	const Elite::FVector2 edgeC{ ndcPoints[0].position - ndcPoints[2].position };

	float W0 = Elite::Cross(pointToSideB, edgeB);
	float W1 = Elite::Cross(pointToSideC, edgeC);
	float W2 = Elite::Cross(pointToSideA, edgeA);

	//Cull mode
	if (m_Cull == CullMode::back)
	{
		if (W2 < 0 || W1 < 0 || W0 < 0)
		{
			return false;
		}
	}
	else if (m_Cull == CullMode::front)
	{
		if (W2 > 0 || W1 > 0 || W0 > 0)
		{
			return false;
		}
	}
	else if (m_Cull == CullMode::none)
	{
		if (!((W2 < 0 && W1 < 0 && W0 < 0) || (W2 > 0 && W1 > 0 && W0 > 0)))
		{
			return false;
		}
	}

	//Interpolate between vertex values
	W0 = abs(W0 / Elite::Cross(Elite::FVector2(ndcPoints[0].position - ndcPoints[1].position), edgeC));
	W1 = abs(W1 / Elite::Cross(Elite::FVector2(ndcPoints[0].position - ndcPoints[1].position), edgeC));
	W2 = abs(W2 / Elite::Cross(Elite::FVector2(ndcPoints[0].position - ndcPoints[1].position), edgeC));

	auto interpolatedZ = (1 / (((1 / (ndcPoints[0].position.z)) * W0) + ((1 / (ndcPoints[1].position.z)) * W1) + ((1 / (ndcPoints[2].position.z)) * W2)));
	pointToHit.position.z = interpolatedZ;

	auto interpolatedW = (1 / (((1 / (ndcPoints[0].position.w)) * W0) + ((1 / (ndcPoints[1].position.w)) * W1) + ((1 / (ndcPoints[2].position.w)) * W2)));

	pointToHit.uv = (((ndcPoints[0].uv / ndcPoints[0].position.w) * W0) + ((ndcPoints[1].uv / ndcPoints[1].position.w) * W1)
		+ ((ndcPoints[2].uv / ndcPoints[2].position.w) * W2)) * interpolatedW;

	pointToHit.normal = ndcPoints[0].normal * W0 + ndcPoints[1].normal * W1 + ndcPoints[2].normal * W2;
	pointToHit.normal = GetNormalized(pointToHit.normal);

	pointToHit.tangent = ndcPoints[0].tangent * W0 + ndcPoints[1].tangent * W1 + ndcPoints[2].tangent * W2;

	pointToHit.viewDirection = ndcPoints[0].viewDirection * W0 + ndcPoints[1].viewDirection * W1 + ndcPoints[2].viewDirection * W2;
	pointToHit.viewDirection = GetNormalized(pointToHit.viewDirection);
	return true;
}

Elite::RGBColor Elite::Renderer::PixelShading(const Elite::Vertex_Input& v) const
{
	//Direction light
	FVector3 lightDir = { 0.577f, -0.577f, -0.577f };
	Elite::RGBColor lightColor = { 1.f,1.f,1.f };
	float intensity = 7.f;

	float observedArea{};

	//Calculate normals in tangent space
	Elite::RGBColor normalRgb = m_TextureNormal.Sample(v.uv);
	Elite::FVector3 normal{ normalRgb.r, normalRgb.g, normalRgb.b };
	Elite::FVector3 binormal = Cross(v.tangent, v.normal);
	Elite::FMatrix3 tangentSpaceAxis = FMatrix3(v.tangent, binormal, v.normal);

	normal.x = 2.f * normal.x - 1.f;
	normal.y = 2.f * normal.y - 1.f;
	normal.z = 2.f * normal.z - 1.f;

	normal = tangentSpaceAxis * normal;
	normal = GetNormalized(normal);

	//Calculate cosine law
	observedArea = Elite::Dot(normal, -lightDir);
	observedArea = Clamp(observedArea, 0.f, 1.f);
	observedArea /= float(M_PI);

	//Calculate Phong
	float shininess = 25.f;
	auto phongColor = BRDF::Phong(m_TexureSpecularMap.Sample(v.uv), shininess * m_TextureGlossiness.Sample(v.uv).r, lightDir, v.viewDirection, v.normal);
	auto diffuseColor = m_TextureDiffuse.Sample(v.uv);
	auto ambientColor = Elite::RGBColor{ 0.025f, 0.025f, 0.025f };

	diffuseColor += phongColor + ambientColor;
	diffuseColor.MaxToOne();

	auto finalColor = lightColor * intensity * diffuseColor * observedArea;
	return finalColor;
}

void Elite::Renderer::Update(float dT)
{
	if (m_Rotating)
	{
		m_Timer += dT;
		Elite::FMatrix4 rotation{};

		rotation = Elite::MakeRotationY((m_Timer));

		m_World[0] = rotation[0];
		m_World[1] = rotation[1];
		m_World[2] = rotation[2];
	}

	m_pMesh->Update(dT, m_World);
	m_pCombustion->Update(dT, m_World);
}

void Elite::Renderer::SetCamera(Camera* pCamera)
{
	m_pCamera = pCamera;
	m_pMesh->SetCamera(pCamera);
	m_pCombustion->SetCamera(pCamera);
}

void Elite::Renderer::ToggleCullMode()
{
	if (m_Cull == CullMode::back)
	{
		m_Cull = CullMode::front;
		std::cout << "CullMode: changed to front culling\n";
	}
	else if (m_Cull == CullMode::front)
	{
		m_Cull = CullMode::none;
		std::cout << "CullMode: changed to no culling\n";
	}
	else if (m_Cull == CullMode::none)
	{
		m_Cull = CullMode::back;
		std::cout << "CullMode: changed to back culling\n";
	}
}

void Elite::Renderer::ToggleSample()
{
	if (m_UsingDirectx11)
	{
		if (m_Filter == Filtering::point)
		{
			m_Filter = Filtering::linear;
			std::cout << "Sample State: changed to linear sampling\n";
		}
		else if (m_Filter == Filtering::linear)
		{
			m_Filter = Filtering::anisotropic;
			std::cout << "Sample State: changed to anisotropic sampling\n";
		}
		else if (m_Filter == Filtering::anisotropic)
		{
			m_Filter = Filtering::point;
			std::cout << "Sample State: changed to point sampling\n";
		}
	}
}

void Elite::Renderer::ToggleRotation()
{
	m_Rotating = !m_Rotating;
	if (m_Rotating)
		std::cout << "Rotation: started rotating\n";
	else
		std::cout << "Rotation: stopped rotating\n";
}

void Elite::Renderer::ToggleFireMesh()
{
	if (m_UsingDirectx11)
	{
		m_UsingFireMesh = !m_UsingFireMesh;
		if(m_UsingFireMesh)
			std::cout << "Fire Mesh: rendering fire mesh\n";
		else
			std::cout << "Fire Mesh: not rendering fire mesh\n";
	}
}

void Elite::Renderer::ToggleRenderMode()
{
	m_UsingDirectx11 = !m_UsingDirectx11;
	m_pCamera->SetRenderMode();
	if (m_UsingDirectx11)
		std::cout << "Render mode: changed to Directx11\n";
	else
		std::cout << "Render mode: changed to Software Rasterizer\n";
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create device & device context using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);

	//Create DGXI Factory to create Swapchain based on hardware
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
		return result;

	//Create Swapchain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the handle HWMD from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create Swapchain and hook it into the handle of the SDL winwow
	if(m_pDevice)
		result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}



