#version 450

//layout(binding = 0) uniform UniformBufferObject {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;


void main() {
    gl_Position = vec4(inPosition, 1.0);
    //gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTexCoord.x=1.0-fragTexCoord.x;
//   fragTexCoord.z=0.0;//gl_Position.z; //normalised 0-1
}
