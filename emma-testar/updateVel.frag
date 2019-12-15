#version 150

in vec2 outTexCoord;
uniform sampler2D texUnitPosition;
uniform sampler2D texUnitVelocity;
uniform float deltaTime;
uniform float pixelSize;

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

float worldPosFromDepth(float depth, vec2 depthTexCoord) {

    // scale z [-1, 1]
    float zndc = 2.0 * depth - 1.0;

    // values of orthogonal projection
    float z_near = 1.0;
    float z_far = 40.0;

    // can do for only z when we have orthogonal projection
    float z_view = z_near + zndc * (z_far - z_near);

    // don't know why this works
    vec4 onlyZ = vec4(0, 0, z_far-z_view, 1.0);
    vec4 worldSpacePosition = inverse(worldToView) * onlyZ;

    return worldSpacePosition.z;
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
  float depth_z = worldPosFromDepth(depth_in, depthTexCoord_in);

  // TEMPORARY define surface position
  // float plane_y = 0.0 + 0.5;

  // Separate velocity direction and magnitude
  float curVelNorm = length(curVel);
  vec3 curVelDirection = curVel / curVelNorm;

  // Calculate forces
  vec3 gravityForce = mass * vec3(0, -50.0, 0);
  // vec3 dragForce = -1/2 * airDragCoefficient * airDensity * splitArea * curVelNorm * curVelNorm * curVelDirection;
  float dragConstants = 0.5;
  vec3 dragForce = dragConstants * curVelNorm * curVelNorm * -curVelDirection;

  // Calculate acceleration from forces
  vec3 acceleration = (1/mass) * (gravityForce + dragForce);

  vec3 newVel = curVel + deltaTime * acceleration;

  // Check for collision with plane
  vec3 newPos = curPos + deltaTime * newVel;
  if (newPos.y < depth_z) {
    vec3 normal = vec3(0.0, 1.0, 0.0);
    vec3 impulse = 1.4 * normal * dot(normal, newVel);
    newVel -= impulse;
  }

  // Output velocity
  out_Color = vec4(newVel, 1.0);
}
