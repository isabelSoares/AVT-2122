#version 330

out vec4 colorOut;

uniform sampler2D texmap0;
uniform sampler2D texmap1;

uniform int texMode;

// Pointlights
uniform int lp_states[2];

// Directional Lights
uniform int ld_states[1];

// Spotlights
uniform float ls_angles[2];
uniform int ls_states[2];

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

in Data {
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
} DataIn;

void main() {

	// ============================== COMMON CALCULATIONS ==============================

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	vec4 texel0, texel1;
	
	if (texMode != 0) {
		texel0 = texture(texmap0, DataIn.tex_coord);  // texel from checker.tga
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from lighwood.tga
	}

	// ============================== =================== ==============================

	vec4 finalLightsColor = vec4(0.0);

	// Spotlights
	for (int lightIndex = 0 ; lightIndex < DataIn.ls_directions.length() ; lightIndex++ ) {

		if (ls_states[lightIndex] == 0) { continue; }

		vec4 spec = vec4(0.0);
		vec3 l = normalize(DataIn.ls_directions[lightIndex]);

		float intensity = max(dot(n,l), 0.0);
		float angle = degrees(acos(dot(-l, vec3(normalize(DataIn.ls_realDirections[lightIndex])))));
		if (abs(angle) > ls_angles[lightIndex]) intensity = 0.0;
	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}

		if (texMode == 3) finalLightsColor += intensity * texel0 * texel1 + spec;
		else finalLightsColor += intensity * mat.diffuse + spec;
	}

	// DirectionalLights
	for (int lightIndex = 0 ; lightIndex < DataIn.ld_directions.length() ; lightIndex++ ) {

		if (ld_states[lightIndex] == 0) { continue; }
		
		vec4 spec = vec4(0.0);
		vec3 l = normalize(DataIn.ld_directions[lightIndex]);

		float intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}

		if (texMode == 3) finalLightsColor += intensity * texel0 * texel1 + spec;
		else finalLightsColor += intensity * mat.diffuse + spec;
	}

	// Pointlights
	for (int lightIndex = 0 ; lightIndex < DataIn.lp_directions.length() ; lightIndex++ ) {

		if (lp_states[lightIndex] == 0) { continue; }

		vec4 spec = vec4(0.0);
		vec3 l = normalize(DataIn.lp_directions[lightIndex]);

		float intensity = max(dot(n,l), 0.0);
	
		if (intensity > 0.0) {

			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}

		if (texMode == 3) finalLightsColor += intensity * texel0 * texel1 + spec;
		else finalLightsColor += intensity * mat.diffuse + spec;
	}

	// colorOut = finalLightsColor;

	if (texMode == 3) colorOut = max(finalLightsColor, 0.37 * texel0 * texel1 );
	else colorOut = max(finalLightsColor, mat.ambient);
}