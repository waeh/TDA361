#version 420
// required by GLSL spec Sect 4.5.3 (though nvidia does not, amd does)
precision highp float;

///////////////////////////////////////////////////////////////////////////////
// Input vertex attributes
///////////////////////////////////////////////////////////////////////////////
layout(binding = 0) uniform sampler2D frameBufferTexture;
layout(binding = 1) uniform sampler2D depthTexture;
layout(binding = 2) uniform sampler2D rotateTexture;

///////////////////////////////////////////////////////////////////////////////
// Input varyings from vertex shader
///////////////////////////////////////////////////////////////////////////////
in vec2 texCoord;

///////////////////////////////////////////////////////////////////////////////
// Input uniform variables
///////////////////////////////////////////////////////////////////////////////
uniform vec3 uniformlyDistributedSamples[16];
uniform mat4 projectionMatrix;
uniform mat4 inverseProjectionMatrix;
uniform float kernel_size;

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////
#define PI 3.14159265359

///////////////////////////////////////////////////////////////////////////////
// Output color
///////////////////////////////////////////////////////////////////////////////
layout(location = 0) out vec4 fragmentColor;

vec3 homogenize(vec4 v) { return vec3((1.0 / v.w) * v); }

// Computes one vector in the plane perpendicular to v
vec3 perpendicular(vec3 v)
{
	vec3 av = abs(v);
	if (av.x < av.y)
		if (av.x < av.z) return vec3(0.0f, -v.z, v.y);
		else return vec3(-v.y, v.x, 0.0f);
	else
		if (av.y < av.z) return vec3(-v.z, 0.0f, v.x);
		else return vec3(-v.y, v.x, 0.0f);
}

void main()
{
	vec4 ssaoInTexture = texture(frameBufferTexture, texCoord);

	float fragmentDepth = texture(depthTexture, texCoord).x;

	vec3 vs_normal = ssaoInTexture.xyz;

	// Normalized Device Coordinates (clip space)
	vec4 ndc = vec4(texCoord.x * 2.0 - 1.0, texCoord.y * 2.0 - 1.0, fragmentDepth * 2.0 - 1.0, 1.0);

	// Transform to view space
	vec3 vs_pos = homogenize(inverseProjectionMatrix * ndc).xyz;

	vec3 vs_tangent = normalize(perpendicular(vs_normal));
	vec3 vs_bitangent = normalize(cross(vs_normal, vs_tangent));

	float randomPoint = texture(rotateTexture, texCoord).x;
	float theta = randomPoint * PI * 2;
	
	mat3 rotMat(
				cos(theta), -sin(theta),0,
				sin(theta), cos(theta),0,
				0,0,1);

	// Base vectors for the fragment
	mat3 tbn = rotmat * mat3(vs_tangent, vs_bitangent, vs_normal); 

	int num_visible_samples = 0;
	int num_valid_samples = 0;

	for (int i = 0; i < 5; i++) {
		// Project hemishere sample onto the local base
		vec3 s = tbn * uniformlyDistributedSamples[i];

		// compute view-space position of sample
		vec3 vs_sample_position = vs_pos + s * kernel_size;

		// compute the ndc-coords of the sample
		vec3 sample_coords_ndc = homogenize(projectionMatrix * vec4(vs_sample_position, 1.0));


		// Sample the depth-buffer at a texture coord based on the ndc-coord of the sample
		float blocker_depth = texture(depthTexture, (sample_coords_ndc.xy*0.5) + 0.5).x;

		// Find the view-space coord of the blocker
		vec3 vs_blocker_pos = homogenize(inverseProjectionMatrix *
			vec4(sample_coords_ndc.xy, blocker_depth * 2.0 - 1.0, 1.0));

		// Check that the blocker is closer than kernel_size to vs_pos
		// (otherwise skip this sample)
		if (length(vs_blocker_pos - vs_pos) > kernel_size && vs_blocker_pos.z >= vs_sample_position.z) continue;
		
		// Check if the blocker pos is closer to the camera than our
		// fragment, otherwise, increase num_visible_samples
		if (vs_blocker_pos.z < vs_sample_position.z)
			num_visible_samples++;

		num_valid_samples += 1;
	}

	float hemisphericalVisibility = float(num_visible_samples) / float(num_valid_samples);

	if (num_valid_samples == 0)
		hemisphericalVisibility = 1.0;

	fragmentColor = vec4(vec3(hemisphericalVisibility), 1.0f);
}
