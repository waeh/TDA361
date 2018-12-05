#version 420

// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

///////////////////////////////////////////////////////////////////////////////
// Material
///////////////////////////////////////////////////////////////////////////////
uniform vec3 material_color;
uniform float material_reflectivity;
uniform float material_metalness;
uniform float material_fresnel;
uniform float material_shininess;
uniform float material_emission;

uniform int has_emission_texture;
layout(binding = 5) uniform sampler2D emissiveMap;

///////////////////////////////////////////////////////////////////////////////
// Environment
///////////////////////////////////////////////////////////////////////////////
layout(binding = 6) uniform sampler2D environmentMap;
layout(binding = 7) uniform sampler2D irradianceMap;
layout(binding = 8) uniform sampler2D reflectionMap;
uniform float environment_multiplier;

///////////////////////////////////////////////////////////////////////////////
// Light source
///////////////////////////////////////////////////////////////////////////////
uniform vec3 point_light_color = vec3(1.0, 1.0, 1.0);
uniform float point_light_intensity_multiplier = 50.0;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// Input varyings from vertex shader
///////////////////////////////////////////////////////////////////////////////
in vec2 texCoord;
in vec3 viewSpaceNormal;
in vec3 viewSpacePosition;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform mat4 viewInverse;
uniform vec3 viewSpaceLightPosition;

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;


float F(vec3 wi, vec3 wo) {
	return material_fresnel + (1 - material_fresnel) * pow(1 - dot(normalize(wi + wo), wi), 5);
}

vec3 calculateDirectIllumiunation(vec3 wo, vec3 n)
{
	///////////////////////////////////////////////////////////////////////////
	// Task 1.2 - Calculate the radiance Li from the light, and the direction
	//            to the light. If the light is backfacing the triangle, 
	//            return vec3(0); 
	///////////////////////////////////////////////////////////////////////////

	vec3 temp = vec3(
		viewSpaceLightPosition.x - viewSpacePosition.x,
		viewSpaceLightPosition.y - viewSpacePosition.y,
		viewSpaceLightPosition.z - viewSpacePosition.z);

	float d = length(temp);

	vec3 wi = normalize(temp);

	if (dot(n, wi) <= 0) {
		return vec3(0.0f);
	}

	///////////////////////////////////////////////////////////////////////////
	// Task 1.3 - Calculate the diffuse term and return that as the result
	///////////////////////////////////////////////////////////////////////////
	vec3 Li = point_light_intensity_multiplier * point_light_color * (1 / (pow(d, 2)));
	vec3 diffuse_term = pow(PI, -1) * abs(dot(n, wi)) * material_color * Li;

	///////////////////////////////////////////////////////////////////////////
	// Task 2 - Calculate the Torrance Sparrow BRDF and return the light 
	//          reflected from that instead
	///////////////////////////////////////////////////////////////////////////
	vec3 wh = normalize(wi + wo);
	float F = F(wo, wo);
	float D = (material_shininess + 2) / 2 * PI * pow(dot(n, wh), material_shininess);
	float G = min(1, min(2 * dot(n, wh)*dot(n, wh) / dot(wo, wh), 2 * dot(n, wh)*dot(n, wi) / dot(wo, wh)));
	float brdf = F * D * G / 4 * dot(n, wo)*dot(n, wi);

	///////////////////////////////////////////////////////////////////////////
	// Task 3 - Make your shader respect the parameters of our material model.
	///////////////////////////////////////////////////////////////////////////
	vec3 dielectric_term = brdf * dot(n, wi) * Li + (1 - F) * diffuse_term;
	vec3 metal_term = brdf * material_color * dot(n, wi) * Li;
	vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;


	
	//return diffuse_term;
	//return brdf * dot(n,wi) * Li;
	return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
}

vec3 calculateIndirectIllumination(vec3 wo, vec3 n)
{
	///////////////////////////////////////////////////////////////////////////
	// Task 5 - Lookup the irradiance from the irradiance map and calculate
	//          the diffuse reflection
	///////////////////////////////////////////////////////////////////////////
	vec3 normal_ws = mat3(viewInverse) * n;
	float theta = acos(max(-1.0f, min(1.0f, normal_ws.y)));
	float phi = atan(normal_ws.z, normal_ws.x);
	if (phi < 0.0f) phi = phi + 2.0f * PI;
	// Use these to lookup the color in the environment map
	vec2 lookup = vec2(phi / (2.0 * PI), theta / PI);

	vec4 irradience = environment_multiplier * texture(irradianceMap, lookup);
	vec3 diffuse_term =  material_color * (1.0 / PI) * irradience.xyz;

	//return diffuse_term;

	///////////////////////////////////////////////////////////////////////////
	// Task 6 - Look up in the reflection map from the perfect specular 
	//          direction and calculate the dielectric and metal terms. 
	///////////////////////////////////////////////////////////////////////////
	vec3 wi = mat3(viewInverse) * reflect(-wo, n);

	float theta2 = acos(max(-1.0f, min(1.0f, wi.y)));
	float phi2 = atan(wi.z, wi.x);
	if (phi2 < 0.0f) phi2 = phi2 + 2.0f * PI;
	vec2 lookup2 = vec2(phi2 / (2.0 * PI), theta2 / PI);

	float roughness = sqrt(sqrt(2/(material_shininess + 2)));
	vec3 Li = environment_multiplier * textureLod(reflectionMap, lookup2, roughness * 7.0).xyz;

	float F = F(wi, wo);

	vec3 dielectric_term = (F * Li + (1 - F)) * diffuse_term;

	vec3 metal_term = F * material_color * Li;

	vec3 microfacet_term = material_metalness * metal_term + (1 - material_metalness) * dielectric_term;

	return material_reflectivity * microfacet_term + (1 - material_reflectivity) * diffuse_term;
	
}



void main()
{
	vec3 wo = -normalize(viewSpacePosition);
	vec3 n = normalize(viewSpaceNormal);

	// Direct illumination
	vec3 direct_illumination_term = calculateDirectIllumiunation(wo, n);

	// Indirect illumination
	vec3 indirect_illumination_term = calculateIndirectIllumination(wo, n);

	///////////////////////////////////////////////////////////////////////////
	// Add emissive term. If emissive texture exists, sample this term.
	///////////////////////////////////////////////////////////////////////////
	vec3 emission_term = material_emission * material_color;
	if (has_emission_texture == 1) {
		emission_term = texture(emissiveMap, texCoord).xyz;
	}

	fragmentColor.xyz =
		direct_illumination_term +
		indirect_illumination_term +
		emission_term;
}
