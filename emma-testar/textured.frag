#version 150

out vec4 outColor;

in vec2 texCoord;
in float z_dist;
uniform sampler2D texLookUnit;

void main(void)
{
	//Fog color hard coded
	vec4 fogColor = vec4(0.5, 0.5, 0.5, 1.0);

	float fogStart = 100.0; // Distance fog starts at
	float fogEnd = -50;	//Distance fog ends at
	float fogFactor = (fogEnd - z_dist)/(fogEnd - fogStart);
	fogFactor = clamp(fogFactor, 0, 1);

	// Discard fragments of transparent corners of the texture
	vec4 t = texture(texLookUnit, texCoord);
	if (t.a < 0.1) discard;
	else
		outColor = mix(fogColor, t, fogFactor);
}
