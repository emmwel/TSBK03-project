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
uniform float planeWidth;
uniform float zFar;
uniform float zNear;
uniform float camHeight;

// forces parameters
uniform float splitArea;
uniform float airDragCoefficient;
uniform float airDensity;
uniform float mass;
uniform vec3 windDirection;

// lifetime
uniform float maxLifetime;

out vec4 out_Color;

float worldPosZFromDepth(float depth) {

    // scale z between [0, clip-space-size]
    float depth_flipped = (1-depth) * (zFar - zNear);

    return depth_flipped + (camHeight - zFar);
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
    float x_1 = (depthTexCoord1.x * planeWidth) - planeWidth/2;
    float x_2 = (depthTexCoord2.x * planeWidth) - planeWidth/2;
    float z_1 = (-1 * depthTexCoord1.y * planeWidth) + planeWidth/2;
    float z_2 = (-1 * depthTexCoord2.y * planeWidth) + planeWidth/2;

    // create normal
    vec3 pos1 = vec3(x_1, y_1, z_1);
    vec3 pos2 = vec3(x_2, y_2, z_2);
    vec3 normal_out = normalize(cross(pos1.xyz - pos0.xyz, pos2.xyz - pos0.xyz));

    return normal_out;
}


void main(void)
{
  // Fetch position, age and velocity from textures
  vec4 curPos4D = texture(texUnitPosition, outTexCoord);
  vec3 curPos = curPos4D.xyz;
  float age = curPos4D[3];
  vec3 curVel = texture(texUnitVelocity, outTexCoord).xyz;

  // texture values between [0, 1]
  float u = (curPos.x + planeWidth/2) / planeWidth;
  float v = -1 * (curPos.z - planeWidth/2) / planeWidth;
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

  // Calculate wind force
  vec3 windForce;
  if(curPos.y < 0.2){
    windForce = vec3(0.0);
  }
  else{
    float windSpeed = 0.01;
    vec3 windDirection = vec3(0.865, 0.0, 0.5);
    windForce = windSpeed * windDirection;
  }

  // Calculate acceleration from forces
  vec3 acceleration = (1/mass) * (gravityForce + dragForce + windForce);

  // Calculate new velocity from acceleration
  vec3 newVel = curVel + deltaTime * acceleration;

  // Slow down hail closer to the ground (simulated rolling friction)
  if(curPos.y < 5.0){
    float factor = (5.0-curPos.y)/5.0;
    factor = clamp(factor, 0, 0.05);
    newVel.xz -= factor * newVel.xz;
    newVel.y -= factor/2.0 * newVel.y;
  }

  // Check for collision with plane
  vec3 newPos = curPos + deltaTime * newVel;
  if (newPos.y < depth_z) {
    vec3 normal = getSurfaceNormal(vec3(curPos.x, depth_z, curPos.y), depthTexCoord_in);
    vec3 impulse = 1.15 * normal * dot(normal, newVel);
    newVel -= impulse;
  }

  if (age > maxLifetime) {
    newVel = vec3(0.0, 0.0, 0.0);
  }

  // Output velocity
  out_Color = vec4(newVel, 1.0);
}
