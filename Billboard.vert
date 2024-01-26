#version 330 core
layout(location = 0) in vec3 vertexPosition_ocs;
layout(location = 1) in vec2 vertexUV;

uniform float heightScaler;

uniform sampler2D heightmapTexture;

out vec4 vertexPosition;
out float vertexHeight;
out float random;

float heightScale = 0.000002f; 
float calculateHeight(vec2 vertex) {
	vec3 RGBValue = texture(heightmapTexture, vec2(vertex.x, vertex.y)).rgb * 255.0f;
	// perform shift
	int height = int(int(RGBValue.r) << 16) + int(int(RGBValue.g) << 8)  + int(RGBValue.b);
	return (heightScale * heightScaler * float(height));
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	float height = calculateHeight(vec2(vertexUV.x, vertexUV.y));
	vertexHeight = height;

	random = rand(vertexPosition_ocs.xz);

	vertexPosition = vec4(vec3(vertexPosition_ocs.x, height + 0.1f, vertexPosition_ocs.z), 1);
}