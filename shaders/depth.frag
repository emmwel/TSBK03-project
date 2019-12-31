#version 150

in vec2 outTexCoord;
uniform sampler2D texUnit;
out vec4 out_Color;

void main(void)
{
    float depth = texture(texUnit, outTexCoord).x;
    // float zNear = 1.0;
    // float zFar = 60.0;
    // //float c =
    // float c = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));

    // out_Color = vec4(c, c, c, 1.0);

    out_Color = vec4(vec3(depth), 1.0);


//    if (depth > 0) {
//        out_Color = vec4(1.0, 0, 0, 1.0);
//    }
//
//    if (depth > 0.09) {
//        out_Color = vec4(0, 1.0, 0, 1.0);
//    }

}
