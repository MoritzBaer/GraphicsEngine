#version 450

//shader input
layout (location = 1) in vec2 uv;

//output write
layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = vec4(uv, 0, 1.0f);
}
