#version 450

layout(binding = 1) uniform sampler3D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
