#version 150
// bump mapping should be calculated
// 1) in view coordinates
// 2) in texture coordinates

in vec2 outTexCoord;
in vec3 out_Normal;
in vec3 Ps;
in vec3 Pt;
in vec3 pixPos;  // Needed for specular reflections
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
  vec3 light = vec3(0.0, 0.7, 0.7); // Light source in view coordinates

	// Calculate gradients here
  float offset = 1.0 / 256.0; // texture size, same in both directions
  float bs = texture(texUnit, outTexCoord + offset * vec2(1.0, 0.0)).r - texture(texUnit, outTexCoord).r;
  float bt = texture(texUnit, outTexCoord + offset * vec2(0.0, 1.0)).r - texture(texUnit, outTexCoord).r;

  //****** 2A ******//
  // vec3 normal = normalize(out_Normal - bs * Ps - bt * Pt);
	// Simplified lighting calculation.
	// A full solution would inc lude material, ambient, specular, light sources, multiply by texture.
  // out_Color = vec4( dot(normal, light));

  //****** 2B ******//
  mat3 Mvt = transpose(mat3(Ps, Pt, normalize(out_Normal)));
  vec3 texNormal = normalize(vec3(bs, bt, 1));

  vec3 lightTex = Mvt * light;
  out_Color = vec4( dot(texNormal, lightTex));
}
