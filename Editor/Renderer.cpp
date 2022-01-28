#include "stdafx.h"

#include "Renderer.h"
#include <wingdi.h>
#include <GL/gl.h>

// TODO: list DLL entry points


Renderer::Renderer(HWND hwnd)
{
	// TODO: LoadLibrary and get all entry points
}

Renderer::~Renderer()
{}

void Renderer::ClearColor(float color[4])
{
	glClearColor(color[0], color[1], color[2], color[3]);
}

void Renderer::ClearDepth(float depth)
{
	glClearDepth(depth);
}

void Renderer::ClearBuffers()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::SwapBuffers()
{
}

void Renderer::Viewport(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
}

void Renderer::DrawVertexBuffer(std::vector<Vertex>& vertices, std::vector<int>& indices)
{
}

uint32_t Renderer::LoadTexture(uint32_t width, uint32_t height, void* data)
{
	return 0;
}
