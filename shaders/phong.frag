#version 150

// Simplified Phong: No materials, only one, hard coded light source
// (in view coordinates) and no ambient

// Note: Simplified! In particular, the light source is given in view
// coordinates, which means that it will follow the camera.
// You usually give light sources in world coordinates.

out vec4 outColor;
in vec3 exNormal; // Phong
in vec3 exSurface; // Phong (specular)
uniform vec4 surface_color;

void main(void)
{
	const vec3 light = normalize(vec3(1,2,1)); // Given in VIEW coordinates! You usually specify light sources in world coordinates.
	float diffuse, specular, shade;

	// Fog
	float fogStart = 10.0; // Distance fog starts at
	float fogEnd = 400;	//Distance fog ends at
	float pointDistance = gl_FragCoord.z / gl_FragCoord.w;
	float fogFactor = (fogEnd - pointDistance)/(fogEnd - fogStart);
	fogFactor = clamp(fogFactor, 0, 1);

	// Diffuse
	diffuse = dot(normalize(exNormal), light);
	diffuse = max(0.0, diffuse); // No negative light

	// Specular
	vec3 r = reflect(-light, normalize(exNormal));
	vec3 v = normalize(-exSurface); // View direction
	specular = dot(r, v);
	if (specular > 0.0)
		specular = 10.0 * pow(specular, 150.0);
	specular = max(specular, 0.0);
	shade = 0.7*diffuse + 1.0*specular;
	outColor = surface_color * vec4(shade, shade, shade, 1.0);
	// outColor = surface_color * vec4(shade, shade, shade, 1.0);
	// outColor = vec4(fogFactor, fogFactor, fogFactor, 1.0);
	outColor = mix(vec4(0.5, 0.5, 0.5, 1.0), outColor, fogFactor);

}
