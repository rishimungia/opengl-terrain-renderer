#version 330 core

layout ( points ) in;
layout ( triangle_strip, max_vertices = 4 ) out;

uniform mat4 gVP;

in vec4 vertexPosition[1];
in float vertexHeight[1];
in float random[1];

out vec2 TexCoords;
out float height;

void main()
{		
	if (random[0] < 0.001 && random[0] > 0.0001) {
        height = vertexHeight[0];

		// Calculate the screen position of the marker using ViewProjection matrix
		vec4 center = gVP * vertexPosition[0];
	
		// scale and align the billboard
		vec2 dir = vec2(1.0f, 3.0f) * 0.05;

		// top right
		gl_Position = vec4( center.x+dir.x, center.y+dir.y, center.z, center.w );
		TexCoords = vec2(1,0);
		EmitVertex();
	
		// top left
		gl_Position = vec4( center.x-dir.x, center.y+dir.y, center.z, center.w );
		TexCoords = vec2(0,0);
		EmitVertex();
	
		// bottom right
		gl_Position = vec4( center.x+dir.x, center.y-dir.y, center.z, center.w );
		TexCoords = vec2(1,1);
		EmitVertex();
	
		// bottom left
		gl_Position = vec4( center.x-dir.x, center.y-dir.y, center.z, center.w );
		TexCoords = vec2(0,1);
		EmitVertex();

		/// Finally emit the whole square.
		EndPrimitive();
    }
	
}