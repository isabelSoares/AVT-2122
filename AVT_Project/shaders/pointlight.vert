#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

// Pointlights
uniform vec4 lp_positions[2];

// Directional Lights
uniform vec4 ld_directions[1];

// Spotlights
uniform vec4 ls_positions[2];
uniform vec4 ls_directions[2];

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec2 tex_coord;

	// Pointlights
	vec3 lp_directions[2];
	// Directional Lights
	vec3 ld_directions[1];
	// Spotlights
	vec3 ls_directions[2];
	vec3 ls_realDirections[2];
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.eye = vec3(-pos);
	DataOut.tex_coord = texCoord.st;

	// Pointlights
	for (int lightIndex = 0 ; lightIndex < lp_positions.length() ; lightIndex++ ) {
		DataOut.lp_directions[lightIndex] = vec3(lp_positions[lightIndex] - pos);
	}

	// Spotlights
	for (int lightIndex = 0 ; lightIndex < ls_positions.length() ; lightIndex++ ) {
		DataOut.ls_directions[lightIndex] = vec3(ls_positions[lightIndex] - pos);
		DataOut.ls_realDirections[lightIndex] = vec3(ls_directions[lightIndex]);
	}

	// Directional
	for (int lightIndex = 0 ; lightIndex < ld_directions.length() ; lightIndex++ ) {
		DataOut.ld_directions[lightIndex] = vec3(- ld_directions[lightIndex]);
	}

	gl_Position = m_pvm * position;	
}