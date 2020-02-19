#version 450 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertCol;
layout(location = 2) in vec4 vertNor;

layout(location = 0) out vec4 fragVtxColor;

layout(binding = 0) uniform UboInstance {
    mat4 model;    
    //mat4 View;  
    //mat4 Projection;
} uboInstance;

//layout(binding = 1) uniform AnimationInfo{
//    float time;
//} uAnimInfo;

void main(){
    gl_Position =  uboInstance.model * vertPos;
    //fragVtxColor = mix(vertCol, vec4(1.0, 1.0, 1.0, 0.0) - vertCol, (sin(uAnimInfo.time*2.5)+1.0) / 2.0);
    fragVtxColor = vertNor*.5+.5;
}