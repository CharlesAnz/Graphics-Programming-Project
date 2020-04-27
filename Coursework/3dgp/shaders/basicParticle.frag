#version 330

in float age;
uniform sampler2D texture0;
out vec4 outColor;

void main()
{
    outColor = texture(texture0, gl_PointCoord);

    // alpha
    float alpha = 1 - outColor.r * outColor.g * outColor.b;
    alpha *= 1 - age;
    
    outColor.a = alpha;
}

