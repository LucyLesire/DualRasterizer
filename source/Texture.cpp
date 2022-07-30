#include "pch.h"
#include "Texture.h"
#include <iostream>
#include "Effect.h"


Elite::Texture::Texture(ID3D11Device* pDevice, const char* filePath)
{
	SDL_Surface* pSurface = IMG_Load(filePath);
	
	//Initialize Directx11
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pDX11Texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pDX11Texture, &SRVDesc, &m_pTextureResourceView);

	SDL_FreeSurface(pSurface);
}

Elite::Texture::Texture(const char* filePath)
{
	//Initialize SRAS
	m_pSRASTexture = IMG_Load(filePath);
}

Elite::Texture::~Texture()
{
	if (m_pTextureResourceView)
	{
		m_pTextureResourceView->Release();
	}
	if (m_pDX11Texture)
	{
		m_pDX11Texture->Release();
	}

	if (m_pSRASTexture)
	{
		SDL_FreeSurface(m_pSRASTexture);
	}
}

Elite::RGBColor Elite::Texture::Sample(const Elite::FVector2& uv) const
{
	Elite::FVector2 convertedUv{};
	convertedUv.x = uv.x * m_pSRASTexture->w;
	convertedUv.y = (uv.y) * m_pSRASTexture->h;
	Elite::RGBColor finalColor;

	Uint8 r{};
	Uint8 g{};
	Uint8 b{};

	Uint32 pixelCoord = Uint32(roundf(convertedUv.x)) + Uint32((roundf(convertedUv.y) * m_pSRASTexture->w));
	pixelCoord = Clamp(pixelCoord, Uint32(0), Uint32(m_pSRASTexture->w * m_pSRASTexture->h));

	SDL_GetRGB(static_cast<Uint32*>(m_pSRASTexture->pixels)[pixelCoord], m_pSRASTexture->format, &r, &g, &b);

	finalColor.r = static_cast<float>(float(r)) / 255.f;
	finalColor.g = static_cast<float>(float(g)) / 255.f;
	finalColor.b = static_cast<float>(float(b)) / 255.f;

	return finalColor;
}
