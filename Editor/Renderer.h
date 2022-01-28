#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <glm/fwd.hpp>


class Renderer
{
public:

	class Vertex
	{
		vec3 pPosition;
		vec3 pNormal;
		vec2 pTexcoord;
		struct _rgba { uint8_t r, g, b, a; } pColour;

	public:

		Vertex();
		Vertex(vec3& position, vec3 normal, float texcoord[2], uint32_t rgba);
		Vertex(float x, float y, float z, vec3 normal, float s, float t, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		Vertex(vec3 position, vec3 normal, float texcoord[2], uint8_t rgba[4]);
		Vertex(const Vertex& v);

		Vertex& operator= (const Vertex& rhs);
	};

	Renderer(HWND hWnd);
	~Renderer();

	void ClearColor(float color[4]);
	void ClearDepth(float);
	void ClearBuffers();
	void SwapBuffers();

	void Viewport(int x, int y, int width, int height);
	void Perspective(float field_of_view, float aspect_ratio, float depth_range[2]);
	void Orthographic();

	uint32_t CreateVertexBuffer(std::vector<Vertex>& vertices);
	void UpdateVertexBuffer(uint32_t vertex_buffer, std::vector<Vertex>& vertices);
	void DrawVertexBuffer(uint32_t vertex_buffer, const vec3& position, const vec3& rotation);

	uint32_t LoadTexture(uint32_t width, uint32_t height, void* data);
	void BindTexture(uint32_t texture);
};