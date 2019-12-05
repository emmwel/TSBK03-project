#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;
out vec4 out_Color;

void main(void)
{
    vec4 curPos = texture(texUnitPosition, outTexCoord);
    vec4 curVel = texture(texUnitVelocity, outTexCoord);

    //out_Color = curPos - deltaTime * vec4(0.0, 0.1, 0.0, 0.0); // + deltaTime * curVel;
    out_Color = curPos + deltaTime * curVel;
}
