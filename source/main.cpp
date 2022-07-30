#include "pch.h"
//#undef main

//Standard includes
#include <iostream>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"

#ifdef _DEBUG
	#include <vld.h>
#endif // DEBUG


void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;
	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - Lucy Lesire",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	auto pTimer{ std::make_unique<Elite::Timer>() };
	auto pRenderer{ std::make_unique<Elite::Renderer>(pWindow) };
	Elite::Camera* pCamera = new Elite::Camera(float(width) / float(height));
	pRenderer->SetCamera(pCamera);

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_E)
					pRenderer->ToggleRenderMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_R)
					pRenderer->ToggleRotation();
				if (e.key.keysym.scancode == SDL_SCANCODE_C)
					pRenderer->ToggleCullMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F)
					pRenderer->ToggleSample();
				if (e.key.keysym.scancode == SDL_SCANCODE_T)
					pRenderer->ToggleFireMesh();

				break;
			}
		}

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		pCamera->Update(pTimer->GetElapsed());
		pRenderer->Update(pTimer->GetElapsed());
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			std::cout << "FPS: " << pTimer->GetFPS() << std::endl;
		}

	}
	pTimer->Stop();

	delete pCamera;
	pCamera = nullptr;
	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}