#version 450 core

layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec4 vertCol;

layout(location = 0) out vec4 fragVtxColor;

void main(){
    gl_Position = vertPos;
    fragVtxColor = vertCol;
}