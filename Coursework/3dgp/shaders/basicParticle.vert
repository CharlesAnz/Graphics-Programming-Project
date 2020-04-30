// VERTEX SHADER
#version 330

// Uniforms: Transformation Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Particle-specific Uniforms
uniform vec3 initialPos = vec3(0, 0, 0);		// Initial Position (source of the fountain)
uniform vec3 gravity = vec3(0.0, 0.1, 0.0);	// Gravity Acceleration in world coords
uniform float particleLifetime;			// Max Particle Lifetime
uniform float time;					// Animation Time

// Special Vertex Attributes
layout (location = 0) in vec3 aVelocity;		// Particle initial velocity
layout (location = 1) in float aStartTime;	// Particle "birth" time

// Output Variable (sent to Fragment Shader)
out float age;					// age of the particle (0..1)



void main()
{
	float t = mod(time - aStartTime, particleLifetime);
	vec3 pos = initialPos + aVelocity * t + gravity * t * t; 
	age = t / particleLifetime;

	gl_PointSize = mix(4, 40, age);

	// calculate position (normal calculation not applicable here)
	vec4 position = matrixModelView * vec4(pos, 1.0);
	gl_Position = matrixProjection * position;

	
	
}



