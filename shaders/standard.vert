#version 450 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertCol;
layout(location = 2) in vec4 vertNor;

layout(location = 0) out vec4 fragVtxColor;

vec4 side = vec4(-100, 0, 10, 0);
vec4 magenta = vec4(1.0, 0.44, 0.81, 1.0);
vec4 cyan = vec4(0.0, 0.8, 1.0, 1.0);
vec4 lightpos = vec4(5,10,-2,0);

layout(binding = 0) uniform Transforms {
    mat4 Model;    
    mat4 View;  
    mat4 Projection;
} uTransforms;

//layout(binding = 1) uniform AnimationInfo{
//    float time;
//} uAnimInfo;

//  ~ a ~ e ~ s ~ t ~ h ~ e ~ t ~ i ~ c ~ s ~  // 

void main(){
    gl_Position =  uboInstance.view * uboInstance.model * vertPos;
    //fragVtxColor = mix(vertCol, vec4(1.0, 1.0, 1.0, 0.0) - vertCol, (sin(uAnimInfo.time*2.5)+1.0) / 2.0);
    vec4 lightdir = normalize(vertPos - lightpos);
    vec4 fragNor = normalize(uTransforms.View * uTransforms.Model * vec4(vertNor.xyz, 0));
    float diffComp = dot(fragNor, lightdir) + 0.75;

    float magentaScalar = dot(fragNor, normalize(vertPos - side)) + 1;
    float cyanScalar = 1 - magentaScalar;
    vec4 baseColor = (magentaScalar * magenta) + (cyanScalar * cyan);
    fragVtxColor = diffComp * baseColor;
    // fragVtxColor.rgb *= 0.2;
    // fragVtxColor.rgb += 0.2;
    fragVtxColor.a = 1;
}