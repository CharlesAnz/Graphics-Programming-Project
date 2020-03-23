#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixModelView;

layout (location = 0) in vec3 aVertex;
layout (location = 3) in vec2 aTexCoord;

out vec2 texCoord0;

void main(void) 
{
	// calculate position
	vec4 position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;

	// calculate texture coordinate
	texCoord0 = aTexCoord;
}

