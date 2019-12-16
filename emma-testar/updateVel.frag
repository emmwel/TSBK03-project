#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;
uniform float z_far;
uniform float camHeight;
float z_near = 1.0;

// depth variables
uniform sampler2D texUnitDepth;
uniform mat4 orthogonalProjectionMatrix;
uniform mat4 worldToView;

// forces parameters
uniform float splitArea;
uniform float airDragCoefficient;
uniform float airDensity;
uniform float mass;

out vec4 out_Color;

float worldPosZFromDepth(float depth) {

    // scale z between [0, clip-space-size]
    float depth_flipped = (1-depth) * (z_far - z_near);

    return depth_flipped + (camHeight - z_far);
}

vec3 getSurfaceNormal(vec3 pos0, vec2 depthTexCoord0) {

    // Get texture coordinates for 2 nearby points
    vec2 offset = vec2(1.0) / textureSize(texUnitDepth, 0).xy;
    vec2 depthTexCoord1 = depthTexCoord0 + vec2(offset.x, 0.0);
    vec2 depthTexCoord2 = depthTexCoord0 + vec2(0.0, offset.y);

    // sample depth for nearby points
    float depth_1 = texture(texUnitDepth, depthTexCoord1).x;
    float depth_2 = texture(texUnitDepth, depthTexCoord2).x;

    // transform to world coordinates, note that depth image was taken from
    // above, so the given coordinates are on the y-axis
    float y_1 = worldPosZFromDepth(depth_1);
    float y_2 = worldPosZFromDepth(depth_2);

    // transform texture coordinates into world coordinates
    float x_1 = depthTexCoord1.x * 200 - 100;
    float x_2 = depthTexCoord2.x * 200 - 100;
    float z_1 = -1 * depthTexCoord1.y * 200 + 100;
    float z_2 = -1 * depthTexCoord2.y * 200 + 100;

    // create normal
    vec3 pos1 = vec3(x_1, y_1, z_1);
    vec3 pos2 = vec3(x_2, y_2, z_2);
    vec3 normal_out = normalize(cross(pos1.xyz - pos0.xyz, pos2.xyz - pos0.xyz));

    return normal_out;
}


void main(void)
{
  // Fetch position and velocity from textures
  vec3 curPos = texture(texUnitPosition, outTexCoord).xyz;
  vec3 curVel = texture(texUnitVelocity, outTexCoord).xyz;

  // divide with W and H to get correct texture values between [0, 1]
  float u = ((curPos.x + 100) * 4 )/ 800;
  float v = ((curPos.z - 100) * (-4)) / 800;
  vec2 depthTexCoord_in = vec2(u, v);
  float depth_in = texture(texUnitDepth, depthTexCoord_in).x;
  float depth_z = worldPosZFromDepth(depth_in);

  // Separate velocity direction and magnitude
  float curVelNorm = length(curVel);
  vec3 curVelDirection = curVel / curVelNorm;

  // Calculate forces
  vec3 gravityForce = mass * vec3(0, -9.82, 0);
  vec3 dragForce = -1/2 * airDragCoefficient * airDensity * splitArea * curVelNorm * curVelNorm * curVelDirection;
  //float dragConstants = 0.5; // Simplified drag constants
  //vec3 dragForce = dragConstants * curVelNorm * curVelNorm * -curVelDirection;

  // Calculate acceleration from forces
  vec3 acceleration = (1/mass) * (gravityForce + dragForce);

  // Calculate new velocity from acceleration
  vec3 newVel = curVel + deltaTime * acceleration;

  // Check for collision with plane
  vec3 newPos = curPos + deltaTime * newVel;
  if (newPos.y < depth_z) {
    vec3 normal = getSurfaceNormal(vec3(curPos.x, depth_z, curPos.y), depthTexCoord_in);
    vec3 impulse = 1.001 * normal * dot(normal, newVel);
    newVel -= impulse;
  }

  // Output velocity
  out_Color = vec4(newVel, 1.0);

}
