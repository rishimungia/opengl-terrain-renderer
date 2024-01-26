#version 420 core

in vec2 TexCoords;
in float height;

out vec4 color;

layout (binding=1) uniform sampler2D billboardTexture;

void main()
{
	float terrainHeight = height * 20;

	// draw billboard only till cretain height
	if (terrainHeight < 20) {
		color = texture(billboardTexture, TexCoords);

		// discard black part of the texture (background)
		if (color.r == 0 && color.g == 0 && color.b == 0) {
			discard;
		}
	} else {
		discard;
	}
}