#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out float outRad;

out gl_PerVertex {
	vec4 gl_Position;
	float gl_PointSize;  
};


void main() 
{
	outColor = inColor;
        outColor.w*= 2.0*inPos.w; //premultiply by 2 rad
        const float spriteSize = 0.35 * inPos.w;
        //vec4 pos=ubo.model * vec4(inPos.xyz, 1.0);
        //const float spriteSize = 0.35 * inPos.w*(2.0/(pos.z+0.00001)); //z simmulates camera distance
         //scale by window size as well
        gl_PointSize = spriteSize*ubo.model[0][0];
        outRad=spriteSize;
    gl_Position = ubo.proj * ubo.view * ubo.model* vec4(inPos.xyz,1.0);
  
}
