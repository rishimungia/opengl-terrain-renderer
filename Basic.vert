#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_ocs;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 vertexNormal;
out float vertexHeight;
out vec3 lightDirection;
out vec3 cameraDirection;
out mat3 TBN;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform int nPoints; 
uniform vec3 lightPosition;
uniform float heightScaler;

uniform sampler2D heightmapTexture;

float heightScale = 0.000002f; 
float calculateHeight(vec2 vertex) {
	vec3 RGBValue = texture(heightmapTexture, vec2(vertex.x, vertex.y)).rgb * 255.0f;
	// perform shift
	int height = int(int(RGBValue.r) << 16) + int(int(RGBValue.g) << 8)  + int(RGBValue.b);
	return (heightScale * heightScaler * float(height));
}

vec3 getNearbyVertex (int xPos, int yPos) {
	float one = 1.0f / (nPoints - 1);
	vec3 v = vec3(
				vertexUV.x + (xPos * one), 
				calculateHeight(vec2(vertexUV.x + (xPos*one), vertexUV.y + (yPos*one))),
				vertexUV.y + (yPos * one)
			); 
	return v;
}

void main() {
	// Output position of the vertex, in clip space : MVP * position
	float height = calculateHeight(vec2(vertexUV.x, vertexUV.y));
	vec3 vertexPos = vec3(vertexPosition_ocs.x, height, vertexPosition_ocs.z);
    gl_Position = MVP * vec4(vertexPos, 1.0f);

	vertexHeight = height / heightScale;

	// calculate vertex normals
	// Nx = differences between 0-6, 1-7, 2-8
	vec3 v0_v6 = getNearbyVertex(-1, -1) - getNearbyVertex(1, -1);
	vec3 v1_v7 = getNearbyVertex(-1, 0) - getNearbyVertex(1, 0);
	vec3 v2_v8 = getNearbyVertex(-1, 1) - getNearbyVertex(1, 1);

	float Nx = v0_v6.y + 2*(v1_v7.y) + v2_v8.y;

	// Nz = differences between 0-2, 3-5, 6-8
	vec3 v0_v2 = getNearbyVertex(-1, -1) - getNearbyVertex(-1, 1); 
	vec3 v3_v5 = getNearbyVertex(0, -1) - getNearbyVertex(0, 1);
	vec3 v6_v8 = getNearbyVertex(1, -1) - getNearbyVertex(1, 1);

	float Nz = v0_v2.y + 2*(v3_v5.y) + v6_v8.y;

	// Ny = fixed value = small value
	float Ny = 0.2;

	// normal of the vertex
	vertexNormal = normalize(vec3(Nx, Ny, Nz));

	// TODO: calculate tangent and bit tangent for normal mapping
	vec3 tangent = vec3(1, 0, 0);
	vec3 bitangent = vec3(0, 0, 1);

	vec3 T = normalize(vec3(viewMatrix * vec4(tangent, 0.0f)));
	vec3 B = normalize(vec3(viewMatrix * vec4(bitangent, 0.0f)));
	vec3 N = vertexNormal;

	// Gram-Schmidt Proces
	// substract the projection from the original vector
	T = normalize(T - dot(T, N) * N);

	// set TBN matrix
	TBN = mat3(T, B, N);

	// light vector: anywhere -> lightPos
	lightDirection = -1 * (viewMatrix * modelMatrix * vec4(lightPosition, 0.0f)).xyz;

	// camera vector: vertexPos -> cameraPos 
	// camera is at (0, 0, 0)
	cameraDirection = vec3(0, 0, 0) - vec4(viewMatrix * modelMatrix * vec4(vertexPos, 0.0f)).xyz;

	// UV of the vertex
	UV = vertexUV;
	
}