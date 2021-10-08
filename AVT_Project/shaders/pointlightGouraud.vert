#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_positions[3];
uniform int l_states[3];

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria

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

	for (int lightIndex = 0 ; lightIndex < l_positions.length() ; lightIndex++ ) {

		if (l_states[lightIndex] == 0) { continue; }

		vec4 l_pos = l_positions[lightIndex];
		
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
		
	
	
	color = max(finalLightsColor, mat.ambient);

}