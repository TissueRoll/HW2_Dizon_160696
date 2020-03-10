#version 330

in vec3 fragPos;
in vec3 outNormal;
in vec2 outUV;
in mat3 TBN;

out vec4 fragColor;

uniform vec3 eyePos;

struct DirectionalLight
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;	
};

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float kConstant;
	float kLinear;
	float kQuadratic;
};

struct SpotLight
{
	vec3 position;
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float kConstant;
	float kLinear;
	float kQuadratic;

	float cutOffAngle;
};

uniform DirectionalLight dirLight;

uniform PointLight pointLight;

uniform SpotLight spotLight;

// Diffuse map
uniform sampler2D diffuseTex;

// Specular map
uniform sampler2D specularTex;

// Normal map
uniform sampler2D normalTex;

uniform int normalMappingEnable;

void main() {
	// Get the diffuse color from the diffuse map at the given UV coordinates
	vec3 diffuseColor = texture(diffuseTex, outUV).rgb;

	// Get the specular color from the specular map at the given UV coordinates
	vec3 specularColor = texture(specularTex, outUV).rgb;

	vec3 normal = normalize(outNormal);
	if (normalMappingEnable == 1) {
		normal = texture(normalTex, outUV).rgb;
		normal = normalize(normal * 2.0 - 1.0);
		normal = normalize(TBN * normal);
	} 

	vec3 viewDir = normalize(eyePos - fragPos);

	// --- Compute for directional light ---

	vec3 lightDir = normalize(dirLight.direction);
	vec3 fragToLightDir = -lightDir;

	vec3 dirLightAmbient = dirLight.ambient * diffuseColor;

	float dirLightDiffuseCoefficient = max(dot(normal, fragToLightDir), 0.0);
	vec3 dirLightDiffuse = dirLight.diffuse * (dirLightDiffuseCoefficient * diffuseColor);

	vec3 dirLightSpecular = vec3(0.0, 0.0, 0.0);
	if (dirLightDiffuseCoefficient > 0.0)
	{
		vec3 viewDir = normalize(eyePos - fragPos);
		vec3 reflectDir = reflect(lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
		dirLightSpecular = dirLight.specular * (spec * specularColor);
	}

	vec3 dirLightResult = (dirLightAmbient + dirLightDiffuse + dirLightSpecular);

	// --- Compute for point light ---

	vec3 lightToFragDir = normalize(fragPos - pointLight.position);
	fragToLightDir = -lightToFragDir;

	vec3 pointLightAmbient = pointLight.ambient * diffuseColor;

	float pointLightDiffuseCoefficient = max(dot(normal, fragToLightDir), 0.0);
	vec3 pointLightDiffuse = pointLight.diffuse * (pointLightDiffuseCoefficient * diffuseColor);

	vec3 pointLightSpecular = vec3(0.0, 0.0, 0.0);
	if (pointLightDiffuseCoefficient > 0.0)
	{
		vec3 reflectDir = reflect(lightToFragDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
		pointLightSpecular = pointLight.specular * (spec * specularColor);
	}

	float lightToFragDist = length(fragPos - pointLight.position);
	float attenuation = 1.0 / (pointLight.kConstant + pointLight.kLinear * lightToFragDist + pointLight.kQuadratic * lightToFragDist * lightToFragDist);

	vec3 pointLightResult = (pointLightAmbient + pointLightDiffuse + pointLightSpecular) * attenuation;

	// --- Compute for spot light ---

	lightToFragDir = normalize(fragPos - spotLight.position);
	fragToLightDir = -lightToFragDir;

	vec3 spotLightAmbient = spotLight.ambient * diffuseColor;

	vec3 spotLightDiffuse = vec3(0.0, 0.0, 0.0);
	vec3 spotLightSpecular = vec3(0.0, 0.0, 0.0);

	float cosTheta = dot(lightToFragDir, spotLight.direction);
	float cosPhi = cos(spotLight.cutOffAngle);
	if (cosTheta > cosPhi)
	{
		float spotLightDiffuseCoefficient = max(dot(normal, fragToLightDir), 0.0);
		spotLightDiffuse = spotLight.diffuse * (spotLightDiffuseCoefficient * diffuseColor);

		if (spotLightDiffuseCoefficient > 0.0)
		{
			vec3 reflectDir = reflect(lightToFragDir, normal);
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
			spotLightSpecular = spotLight.specular * (spec * specularColor);
		}
	}

	lightToFragDist = length(fragPos - spotLight.position);
	attenuation = 1.0 / (spotLight.kConstant + spotLight.kLinear * lightToFragDist + spotLight.kQuadratic * lightToFragDist * lightToFragDist);

	vec3 spotLightResult = (spotLightAmbient + spotLightDiffuse + spotLightSpecular) * attenuation;
	
	// Get the sum of the effects of all light sources to get the final color of the fragment
    fragColor = vec4(dirLightResult + pointLightResult + spotLightResult, 1.0);
}