#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

// Pointlights
uniform vec4 lp_positions[2];
uniform int lp_states[2];

// Directional Lights
uniform vec4 ld_directions[1];
uniform int ld_states[1];

// Spotlights
uniform vec4 ls_positions[3];
uniform float ls_angles[3];
uniform int ls_states[3];

in vec4 position;
in vec4 normal;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

out vec4 color;

void main () {

	vec4 pos = m_viewModel * position;

	vec3 normalGouraud = normalize(m_normal * normal.xyz);

	vec4 finalLightsColor = vec4(0.0);

	// Spotlights
	for (int lightIndex = 0 ; lightIndex < ls_positions.length() ; lightIndex++ ) {

		if (ls_states[lightIndex] == 0) { continue; }

		vec4 l_pos = ls_positions[lightIndex];
		
		vec3 lightDir = vec3(l_pos - pos);
		vec3 eye = vec3(-pos);

		gl_Position = m_pvm * position;

		vec4 spec = vec4(0.0);

		vec3 n = normalize(normalGouraud);
		vec3 l = normalize(lightDir);
		vec3 e = normalize(eye);

		float intensity = max(dot(n,l), 0.0);
		float angle = degrees(acos(dot(n, l)));
		if (angle > ls_angles[lightIndex]) intensity = 0.0;
	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}

		finalLightsColor += intensity * mat.diffuse + spec;
	}

	// DirectionalLights
	for (int lightIndex = 0 ; lightIndex < ld_directions.length() ; lightIndex++ ) {

		if (ld_states[lightIndex] == 0) { continue; }
		
		vec3 lightDir = vec3(- ld_directions[lightIndex]);
		vec3 eye = vec3(-pos);

		gl_Position = m_pvm * position;

		vec4 spec = vec4(0.0);

		vec3 n = normalize(normalGouraud);
		vec3 l = normalize(lightDir);
		vec3 e = normalize(eye);

		float intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}

		finalLightsColor += intensity * mat.diffuse + spec;
	}

	// Pointlights
	for (int lightIndex = 0 ; lightIndex < lp_positions.length() ; lightIndex++ ) {

		if (lp_states[lightIndex] == 0) { continue; }

		vec4 l_pos = lp_positions[lightIndex];
		
		vec3 lightDir = vec3(l_pos - pos);
		vec3 eye = vec3(-pos);

		gl_Position = m_pvm * position;

		vec4 spec = vec4(0.0);

		vec3 n = normalize(normalGouraud);
		vec3 l = normalize(lightDir);
		vec3 e = normalize(eye);

		float intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}

		finalLightsColor += intensity * mat.diffuse + spec;
	}
	
	color = finalLightsColor;
	// color = max(finalLightsColor, mat.ambient);

}