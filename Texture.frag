#version 420 core

// Input
in vec2 UV;
in vec3 vertexNormal;
in float vertexHeight;
in vec3 lightDirection;
in vec3 cameraDirection;
in mat3 TBN;

// Output
out vec3 color;

//Uniforms
layout (binding=0) uniform sampler2D heightmapTexture;

// rock
layout (binding=1) uniform sampler2D rockTexture;
layout (binding=2) uniform sampler2D rockRoughness;
layout (binding=3) uniform sampler2D rockNormalmap;

// grass
layout (binding=4) uniform sampler2D grassTexture;
layout (binding=5) uniform sampler2D grassRoughness;
layout (binding=6) uniform sampler2D grassNormalmap;

// snow
layout (binding=7) uniform sampler2D snowTexture;
layout (binding=8) uniform sampler2D snowRoughness;
layout (binding=9) uniform sampler2D snowNormalmap;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

void main() {
	vec3 normalMapColor = vec3(abs(vertexNormal.x),abs(vertexNormal.y),abs(vertexNormal.z));

	// tiling 2K rock texture across the 8K plane
	vec2 tiling = textureSize(heightmapTexture,0) / textureSize(rockTexture,0);
	vec2 tilingUV = tiling * UV;

	// material interpolation
	vec3 interpolation; // x: grass, y: rock, z: snow
	float height = vertexHeight / 20000.0f;
	if (height > 80.0f) { // snow -> rock
		interpolation = vec3(0,
							clamp(1 - (height - 80.0f)/10.0f, 0, 1),
							clamp((height - 80.0f)/10.0f, 0, 1));
	}
	else if (height > 40.0f && height < 80.0f) { // rock -> grass
		interpolation = vec3(clamp(1 - (height - 40.0f)/10.0f, 0, 1),
							clamp((height - 40.0f)/10.0f, 0, 1),
							0);
	}
	else { // grass
		interpolation = vec3(1,0,0);
	}

	// obtain normal from normal map in range [0,1]
	vec3 normal =	interpolation.x * texture(grassNormalmap, tilingUV).rgb +
					interpolation.y * texture(rockNormalmap, tilingUV).rgb +
					interpolation.z * texture(snowNormalmap, tilingUV).rgb;
    // transform normal vector to range [-1,1]
    normal = (normal * 2.0 - 1.0);  
	// normal map with TBN
	normal = normalize(viewMatrix * modelMatrix * vec4(TBN * normal, 0.0f)).xyz;

	// normal = vertexNormal;

	// Phong Shading
//	vec3 lightColor = vec3(0.93, 0.84, 0.62);
	vec3 lightColor = vec3(0.98, 0.9, 0.72);

	// get roughness from texture
	// float roughness = texture(rockRoughness, tilingUV).r;
	float roughness =	interpolation.x * texture(grassRoughness, tilingUV).r +
						interpolation.y * texture(rockRoughness, tilingUV).r +
						interpolation.z * texture(snowRoughness, tilingUV).r;
	float shininess = clamp((2/(pow(roughness,4)+1e-2))-2,0,500.0f);
	vec3 specularColour = vec3(0.25);

	vec3 materialColor =	interpolation.x * texture(grassTexture, tilingUV).rgb +
							interpolation.y * texture(rockTexture, tilingUV).rgb +
							interpolation.z * texture(snowTexture, tilingUV).rgb;

	// I_ambient = l_ambient (light) * r_ambient (material)
	// multiplied by 0.25 to reduce intensity correct way??
	vec3 I_ambient = lightColor * vec3(0.25) * materialColor;

	// I_specular = l_specular (light) * r_specular (material) * (cosTheta)^h_specular (heighlight coeff)
	vec3 l = normalize(lightDirection);
	vec3 e = normalize(cameraDirection);
	vec3 v_b = l + e / 2;

	float cosThetaSpecular = clamp(dot(normal, v_b), 0, 1) / (length(normal) * length(v_b));
	
	vec3 I_specular = lightColor * materialColor * specularColour * clamp(pow(cosThetaSpecular, shininess), 0, 1);

	// I_diffuse = l_diffuse (light) * r_diffuse (material) * (cosTheta)
	float cosThetaDiffuse = clamp(dot(normal, l), 0, 1) / (length(normal) * length(l));
	vec3 I_diffuse = lightColor * materialColor * cosThetaDiffuse;

	// I_total = I_specular + I_diffuse + I_ambient
	color =  I_specular + I_diffuse + I_ambient;
}