#version 430

out vec4 colorOut;

uniform mat4 m_View;

uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;
uniform sampler2D texmap4;
uniform sampler2D texmap5;

uniform sampler2D texmapBump0;

uniform	sampler2D texUnitDiff;
uniform	sampler2D texUnitDiff1;
uniform	sampler2D texUnitSpec;
uniform	sampler2D texUnitNormalMap;

uniform samplerCube cubeMap;

uniform int texMode;
uniform int reflect_perFrag;
uniform int bumpMode;

uniform bool shadowMode;

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

	vec3 skyboxTexCoord;
	vec3 reflected;
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

	int numberOfLights = 0;

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);

	if (normalMap) n = normalize(2.0 * texture(texUnitNormalMap, DataIn.tex_coord).rgb - 1.0);

	if (bumpMode == 1) n = normalize(2.0 * texture(texmapBump0, DataIn.tex_coord).rgb - 1.0);

	vec4 texel0, texel1, texel2, texel3, texel4, cube_texel, texel5;
	
	if (texMode != 0) {
		texel0 = texture(texmap0, DataIn.tex_coord);  // texel from roadGrass2.jpg
		texel1 = texture(texmap1, DataIn.tex_coord);  // texel from lighwood.tga
		texel2 = texture(texmap2, DataIn.tex_coord);  // texel from orange.jpg
		texel3 = texture(texmap3, DataIn.tex_coord);  // texel from tree.tga
		texel4 = texture(texmap4, DataIn.tex_coord);  // texel from particle.tga
		texel5 = texture(texmap5, DataIn.tex_coord);  // texel from cheerio.png
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

		numberOfLights = numberOfLights + 1;

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
		else if (texMode == 6) finalLightsColor += intensity * texel3 + spec;
		else if (texMode == 7) finalLightsColor += intensity * texel4 + spec;
		else if (texMode == 10) finalLightsColor += intensity * texel5 + spec;
		else finalLightsColor += max(intensity * diff, diff * 0.15) + spec;
	}

	// DirectionalLights
	for (int lightIndex = 0 ; lightIndex < DataIn.ld_directions.length() ; lightIndex++ ) {

		if (ld_states[lightIndex] == 0) { continue; }

		numberOfLights = numberOfLights + 1;
		
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
		else if (texMode == 6) finalLightsColor += intensity * texel3 + spec;
		else if (texMode == 7) finalLightsColor += intensity * texel4 + spec;
		else if (texMode == 10) finalLightsColor += intensity * texel5 + spec;
		else finalLightsColor += max(intensity * diff, diff * 0.15) + spec;
	}
	
	// Pointlights
	for (int lightIndex = 0 ; lightIndex < DataIn.lp_directions.length() ; lightIndex++ ) {

		if (lp_states[lightIndex] == 0) { continue; }

		numberOfLights = numberOfLights + 1;

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
		else if (texMode == 6) finalLightsColor += intensity * texel3 + spec;
		else if (texMode == 7) finalLightsColor += intensity * texel4 + spec;
		else if (texMode == 10) finalLightsColor += intensity * texel5 + spec;
		else finalLightsColor += max(intensity * diff, diff * 0.15) + spec;
	}

	colorOut = finalLightsColor;
	
	if (texMode == 3) colorOut = max(finalLightsColor, 0.37 * texel0 * texel1 );
	else if (texMode == 4) colorOut = max(finalLightsColor / numberOfLights, 0.15 * texel2 );
	else if (texMode == 5) {

		if ((texel0.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
		else colorOut = mat.diffuse * texel0;
		return;

	} else if (texMode == 6) {
	
		if(texel3.a == 0.0) discard;
		else colorOut = vec4(max(vec3(finalLightsColor / numberOfLights), 0.1*texel3.rgb), texel3.a);

	} else if (texMode == 7) {

		if ((texel4.a == 0.0)  || (mat.diffuse.a == 0.0) ) discard;
		else colorOut = max(finalLightsColor / 5, mat.diffuse) * texel4;

		return;

	} else if (texMode == 8) {

		colorOut = texture(cubeMap, DataIn.skyboxTexCoord);
		return;

	} else if (texMode == 9) {

		if (reflect_perFrag == 1) {

			vec3 reflected1 = vec3 (transpose(m_View) * vec4 (vec3(reflect(-e, n)), 0.0));
			reflected1.x= -reflected1.x;   
			cube_texel = texture(cubeMap, reflected1);

		} else cube_texel = texture(cubeMap, DataIn.reflected);

		//texel = texture(texmap1, DataIn.tex_coord);
		//vec4 aux_color = mix(texel, cube_texel, reflect_factor);
		//aux_color = max(intensity * aux_color + spec, 0.1 * aux_color);
	    //colorOut = vec4(aux_color.rgb, 1.0); 
		colorOut = vec4(cube_texel.rgb, 1.0);

	} else if (texMode == 10) {

		colorOut = max(finalLightsColor / numberOfLights, 0.15 * texel5 );
	
	} else if (mat.texCount == 0) colorOut = max(finalLightsColor / numberOfLights, diff * 0.1);

	colorOut = vec4(vec3(colorOut), mat.diffuse.a);

	if (shadowMode) colorOut = vec4(0.8, 0.8, 0.8, 1.0);

	if (fogActivated == 1) {
		float dist = length(DataIn.pos);
		colorOut = applyFog(vec3(colorOut), dist, colorOut.a);
	}
}