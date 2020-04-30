// FRAGMENT SHADER

#version 330

// View Matrix
uniform mat4 matrixView;

in vec4 color;
in vec4 shadowCoord;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in mat3 matrixTangent;

out vec4 outColor;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;
uniform sampler2D texture0;
uniform sampler2DShadow shadowMap;
uniform sampler2D textureNormal;
uniform float normalPower;

vec3 normalNew;

struct SPOT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	mat4 matrix;
	vec3 direction;
	float cutoff; 
	float attenuation;
};
uniform SPOT lightSpot;

vec4 SpotLight(SPOT light)
{
	// HERE GOES THE CODE COPIED FROM THE POINT LIGHT FUNCTION
	// Calculate Spot Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(light.matrix * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	// HERE GOES THE NEW CODE TO DETERMINE THE SPOT FACTOR
	//vec3 D = normalize(light.matrix * vec4(light.direction, 1)- position).xyz;
	vec3 D = normalize(mat3(light.matrix) * light.direction);
	float LdotD = dot(-L, D);
	float A = acos(LdotD);
	float Cut = clamp(radians(light.cutoff), 0.0f, 90.0f);
	float spotFactor;
	if (A <= Cut) spotFactor = pow(LdotD, light.attenuation);
	else if (A > Cut) spotFactor = 0;

	// assuming that the Point Light value is stored as color and we have calculated spotFactor:
	return spotFactor * color;
}


struct POINT
{
	int on;
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	float att_quadratic;
};
uniform POINT lightPoint1, lightPoint2;

vec4 PointLight(POINT light)
{
	// Calculate Point Light
	vec4 color = vec4(0, 0, 0, 0);
	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (light.att_quadratic * dist * dist);
	vec3 L = normalize(matrixView * vec4(light.position, 1) - position).xyz;
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normal);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
	    color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);
	

	return color * att;
}

void main(void) 
{
	outColor = color;

	normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
	normalNew = normalize(matrixTangent * normalNew);
  
	if (lightSpot.on == 1) 
		outColor += SpotLight(lightSpot);

	if (lightPoint1.on == 1) 
		outColor += PointLight(lightPoint1);

	if (lightPoint2.on == 1) 
		outColor += PointLight(lightPoint2);
		
	// Calculation of the shadow
	float shadow = 1.0;
	if (shadowCoord.w > 0)	// if shadowCoord.w < 0 fragment is out of the Light POV
		shadow = 0.5 + 0.5 * textureProj(shadowMap, shadowCoord);
	outColor *= shadow;
	

	outColor *= texture(texture0, texCoord0);
}
