#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;
out vec4 out_Color;

void main(void)
{
    vec4 curVel = texture(texUnitVelocity, outTexCoord);

    // calculate gravity forces
    vec4 gravity = vec4(0, -2.0, 0, 0);
    vec4 acceleration = gravity;

    out_Color = curVel + deltaTime * acceleration;
}
