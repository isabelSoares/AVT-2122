#version 430

in vec2 TexCoords;
out vec4 color;

layout(binding = 0) uniform sampler2D text;
uniform vec3 textColor;

uniform bool symbol;

void main()
{    

    if (symbol) {
        
        color = texture(text, TexCoords);

    } else {
        
        vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
        color = vec4(textColor, 1.0) * sampled;
    }
}