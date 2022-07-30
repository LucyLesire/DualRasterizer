#pragma once
#include "pch.h"

namespace Elite
{
	enum class Filtering
	{
		point = 0,
		linear = 1,
		anisotropic = 2
	};

	enum class CullMode
	{
		back = 0,
		front = 1,
		none = 3
	};

	struct Vertex_Input
	{
		Elite::FPoint4 position;
		Elite::FVector2 uv;
		Elite::FVector3 normal;
		Elite::FVector3 tangent;
		Elite::FVector3 viewDirection;
	};

	struct Triangle
	{
		Elite::Vertex_Input v0;
		Elite::Vertex_Input v1;
		Elite::Vertex_Input v2;
	};
}