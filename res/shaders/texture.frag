#version 450

//shader input
layout (location = 1) in vec2 uv;

//output write
layout (location = 0) out vec4 outFragColor;

// texture
layout(set = 0, binding = 0) uniform sampler2D displayTexture;

void main() 
{
	// TODO: Re-enable alpha eventually
	outFragColor = vec4(texture(displayTexture, vec2(uv.x, 1 - uv.y)).xyz, 1.0);
}
