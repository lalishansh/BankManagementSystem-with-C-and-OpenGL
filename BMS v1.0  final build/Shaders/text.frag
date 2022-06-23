#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform float forcedAlpha;
uniform float minAlpha;
void main()
{
    float sampledAlpha = texture(text, TexCoords).r;
    if(forcedAlpha > 0){
	sampledAlpha = forcedAlpha;
    }
    color = vec4(textColor, sampledAlpha);
    
    if(sampledAlpha < 0.1){ discard; }
    else if(sampledAlpha < minAlpha){ 
	discard;
	//color = vec4(textColor, minAlpha); // if alpha is less than src(prev-fragments alpha)
    }
}