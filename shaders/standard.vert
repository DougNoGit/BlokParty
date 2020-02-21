#version 450 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertCol;
layout(location = 2) in vec4 vertNor;

layout(location = 0) out vec4 fragVtxColor;

layout(binding = 0) uniform Transforms {
    mat4 Model;    
    mat4 View;  
    mat4 Projection;
} uTransforms;

layout(binding = 1) uniform AnimationInfo{
    float time;
} uAnimInfo;

void main(){
    gl_Position =  uTransforms.Projection * uTransforms.View * uTransforms.Model * vertPos;
    //fragVtxColor = mix(vertCol, vec4(1.0, 1.0, 1.0, 0.0) - vertCol, (sin(uAnimInfo.time*2.5)+1.0) / 2.0);
    vec4 lightpos = vec4(5,10,-2,0);
    vec4 lightdir = vertPos - lightpos;
    vec4 fragNor = uTransforms.View * uTransforms.Model * vec4(vertNor.xyz, 0);
    float diffComp = (dot(fragNor, lightdir) + 1) * 0.3;
    fragVtxColor = diffComp * vec4(1,1,1,1);
    fragVtxColor.rgb *= 0.2;
    fragVtxColor.rgb += 0.2;
    fragVtxColor.a = 1;
}