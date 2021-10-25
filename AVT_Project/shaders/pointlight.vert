#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform bool normalMap;

// Pointlights
uniform vec4 lp_positions[6];

// Directional Lights
uniform vec4 ld_directions[1];

// Spotlights
uniform vec4 ls_positions[2];
uniform vec4 ls_directions[2];

in vec4 position;
in vec4 normal, tangent, bitangent;    //por causa do gerador de geometria
in vec4 texCoord;

out Data {
	vec4 pos;

	vec3 normal;
	vec3 eye;
	vec2 tex_coord;

	// Pointlights
	vec3 lp_directions[6];
	// Directional Lights
	vec3 ld_directions[1];
	// Spotlights
	vec3 ls_directions[2];
	vec3 ls_realDirections[2];
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;
	DataOut.pos = pos;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.eye = vec3(-pos);
	DataOut.tex_coord = texCoord.st;

	// =========================== DEAL WITH OBJs ===========================

	vec3 t, b;
	vec3 aux;

	if(normalMap)  {  //transform eye and light vectors by tangent basis
		t = normalize(m_normal * tangent.xyz);
		b = normalize(m_normal * bitangent.xyz);

		aux.x = dot(DataOut.eye, t);
		aux.y = dot(DataOut.eye, b);
		aux.z = dot(DataOut.eye, DataOut.normal);
		DataOut.eye = normalize(aux);
	}

	// ======================================================================

	// Pointlights
	for (int lightIndex = 0 ; lightIndex < lp_positions.length() ; lightIndex++ ) {

		DataOut.lp_directions[lightIndex] = vec3(lp_positions[lightIndex] - pos);
		if (normalMap) {
			aux.x = dot(DataOut.lp_directions[lightIndex], t);
			aux.y = dot(DataOut.lp_directions[lightIndex], b);
			aux.z = dot(DataOut.lp_directions[lightIndex], DataOut.normal);
			DataOut.lp_directions[lightIndex] = normalize(aux);
		}
	}

	// Spotlights
	for (int lightIndex = 0 ; lightIndex < ls_positions.length() ; lightIndex++ ) {
		DataOut.ls_directions[lightIndex] = vec3(ls_positions[lightIndex] - pos);
		DataOut.ls_realDirections[lightIndex] = vec3(ls_directions[lightIndex]);

		if (normalMap) {
			aux.x = dot(DataOut.ls_directions[lightIndex], t);
			aux.y = dot(DataOut.ls_directions[lightIndex], b);
			aux.z = dot(DataOut.ls_directions[lightIndex], DataOut.normal);
			DataOut.ls_directions[lightIndex] = normalize(aux);
		}
	}

	// Directional
	for (int lightIndex = 0 ; lightIndex < ld_directions.length() ; lightIndex++ ) {
		DataOut.ld_directions[lightIndex] = vec3(- ld_directions[lightIndex]);

		if (normalMap) {
			aux.x = dot(DataOut.ld_directions[lightIndex], t);
			aux.y = dot(DataOut.ld_directions[lightIndex], b);
			aux.z = dot(DataOut.ld_directions[lightIndex], DataOut.normal);
			DataOut.ld_directions[lightIndex] = normalize(aux);
		}
	}

	gl_Position = m_pvm * position;	
}