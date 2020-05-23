#pragma once

#include <Windows.h>
#include <cstdint>


class Renderer
{
public:

	Renderer(HWND hWnd);
	~Renderer();

	void Viewport(int x, int y, int width, int height);
	int LoadTexture(uint32_t width, uint32_t height, uint8_t* data);
};