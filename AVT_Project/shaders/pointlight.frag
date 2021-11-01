#version 430

out vec4 colorOut;

uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform int texMode;

uniform int fogActivated;

// Pointlights
uniform int lp_states[6];

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

uniform bool normalMap;  //for normal mapping
uniform bool specularMap;
uniform uint diffMapCount;

in Data {
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
} DataIn;

//Exponential Fog:
vec4 applyFog( in vec3 rgb, in float distance, in float alpha) {

	float fogAmount = exp( -distance*0.05 );
	vec3 fogColor = vec3(0.5,0.6,0.7);
	vec3 final_color = mix(fogColor, rgb, fogAmount );
	return vec4(final_color, alpha);
}

vec4 diff, auxSpec;

void main() {

	// ============================== COMMON CALCULATIONS ==============================

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	if(normalMap)
		n = normalize(2.0 * texture(texUnitNormalMap, DataIn.tex_coord).rgb - 1.0);  //normal in tangent space

	vec4 texel0, texel1, texel2;
	
	if (texMode != 0) {
		texel0 = texture(texmap0, DataIn.tex_coord);  // texel from roadGrass2.jpg
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from lighwood.tga
		texel2 = texture(texmap2, DataIn.tex_coord);  // texel from orange.jpg
	}

	// Auxiliary Variables for OBJs
	diff = mat.diffuse;
	auxSpec = mat.specular;

	// Compute Correct Difuse with OBJs
	if (mat.texCount != 0) {
		if(diffMapCount == 0) diff = mat.diffuse;
		else if(diffMapCount == 1) diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord);
		else diff = mat.diffuse * texture(texUnitDiff, DataIn.tex_coord) * texture(texUnitDiff1, DataIn.tex_coord);

		if(specularMap) auxSpec = mat.specular * texture(texUnitSpec, DataIn.tex_coord);
		else auxSpec = mat.specular;
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
			spec = auxSpec * pow(intSpec, mat.shininess);
		}

		if (texMode == 3) finalLightsColor += intensity * texel0 * texel1 + spec;
		else if (texMode == 4) finalLightsColor += intensity * texel2 + spec;
		else finalLightsColor += max(intensity * diff, diff * 0.15) + spec;
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
			spec = auxSpec * pow(intSpec, mat.shininess);
		}

		if (texMode == 3) finalLightsColor += intensity * texel0 * texel1 + spec;
		else if (texMode == 4) finalLightsColor += intensity * texel2 + spec;
		else finalLightsColor += max(intensity * diff, diff * 0.15) + spec;
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
			spec = auxSpec * pow(intSpec, mat.shininess);
		}

		if (texMode == 3) finalLightsColor += intensity * texel0 * texel1 + spec;
		else if (texMode == 4) finalLightsColor += intensity * texel2 + spec;
		else finalLightsColor += max(intensity * diff, diff * 0.15) + spec;
	}

	colorOut = finalLightsColor;
	
	if (texMode == 3) colorOut = max(finalLightsColor, 0.37 * texel0 * texel1 );
	else if (texMode == 4) colorOut = max(finalLightsColor, 0.37 * texel2 );
	else if (texMode == 5) {

		if ((texel0.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
		else colorOut = mat.diffuse * texel0;
		return;

	} else if (mat.texCount == 0) colorOut = max(finalLightsColor, mat.ambient);

	colorOut = vec4(vec3(colorOut), mat.diffuse.a);

	if (fogActivated == 1) {
		float dist = length(DataIn.pos);
		colorOut = applyFog(vec3(colorOut), dist, colorOut.a);
	}
}