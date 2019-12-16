#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;
uniform float maxLifetime;
out vec4 out_Color;

// function to make random new float
float rand(vec3 pos, vec3 randomVec, float min, float max){
    vec3 small_v = vec3(sin(pos.x), sin(pos.y), sin(pos.z));
    float random = dot(small_v, randomVec);
    random = fract(sin(random) * 143758.5453);
    return random * (max - min) + min;
}

void main(void)
{
    vec4 curPos = texture(texUnitPosition, outTexCoord);
    vec4 curVel = texture(texUnitVelocity, outTexCoord);

    float age = curPos[3] + deltaTime;

    if (age > maxLifetime) {
        float newX = rand(curPos.xyz, vec3(17.975, 88.156, 25.428), -100.0, 100.0);
        float newY = rand(curPos.xyz, vec3(12.989, 78.233, 37.719), 90.0, 1000.0);
        float newZ = rand(curPos.xyz, vec3(39.346, 11.135, 83.155), -100.0, 100.0);
        float newAge = rand(curPos.xyz, vec3(73.156, 52.235, 09.151), 10.0, 50.0);
        out_Color = vec4(newX, newY, newZ, newAge);
        //out_Color = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else {
        out_Color = vec4(curPos.xyz + deltaTime * curVel.xyz, age);
        //out_Color = vec4(age/maxLifetime, age/maxLifetime, age/maxLifetime, age);
        //out_Color = vec4(1.0, 0.0, 0.0, 1.0);
    }

}
