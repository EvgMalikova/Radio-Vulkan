#version 450



layout (location = 0) in vec4 inColor;
layout (location = 1) in float inRad;

layout (location = 0) out vec4 outFragColor;

float sdfSphere(vec2 p,float r)
{
vec2 point=p-vec2(0.5,0.5);
float v=(r-(length(point)))*2.0; //normalise to 1.0

return clamp(v,0.0,1.0);

}

void main() 
{
vec4 color = inColor;

float rad=0.5; //just in case as I am still experimenting with radius

float v=sdfSphere(gl_PointCoord,rad); //try 1-exp(v) as alpha
float alpha=exp(-v*color.w);//sdfSphere(gl_PointCoord,rad);
vec4 fullc=color*(1.0-alpha);////vec4(1.0-alpha,1.0-alpha,1.0-alpha,alpha);
fullc.w=alpha;
outFragColor = fullc;
}
