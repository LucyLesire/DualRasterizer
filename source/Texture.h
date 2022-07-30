#pragma once

#include<string>
#include <SDL_image.h>
#include "ERGBColor.h"

namespace Elite
{
	class Texture
	{
	public:
		Texture(ID3D11Device* pDevice, const char* filePath);
		Texture(const char* filePath);
		~Texture();

		Elite::RGBColor Sample(const Elite::FVector2& uv) const;

		ID3D11ShaderResourceView* GetResourceView() { return m_pTextureResourceView; };

	private:
		//Directx11
		ID3D11Texture2D* m_pDX11Texture = nullptr;
		ID3D11ShaderResourceView* m_pTextureResourceView = nullptr;

		//SRAS
		SDL_Surface* m_pSRASTexture = nullptr;
	};
}


