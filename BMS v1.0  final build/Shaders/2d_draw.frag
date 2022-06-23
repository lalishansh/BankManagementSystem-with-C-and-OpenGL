#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D theTexture;
uniform float minAlpha;
void main()
{
    color = texture(theTexture, TexCoords);
    
    if(color.w < minAlpha){ 
	discard;
    }
}