#include "VulkanGraphicsApp.h"
#include "data/VertexGeometry.h"
#include "data/UniformBuffer.h"
#include "data/VertexInput.h"
#include "utils/FpsTimer.h"
#include <iostream>
#include <memory> // Include shared_ptr
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "utils/ModelContainer.h"

using SimpleVertexBuffer = VertexAttributeBuffer<SimpleVertex>;
using SimpleVertexInput = VertexInputTemplate<SimpleVertex>;

struct Transforms {    
    alignas(16) glm::mat4 Model;
    alignas(16) glm::mat4 View;    
    alignas(16) glm::mat4 Projection;
};

struct AnimationInfo {
    alignas(16) float time;
};

using UniformTransformData = UniformStructData<Transforms>;
using UniformTransformDataPtr = std::shared_ptr<UniformTransformData>;
using UniformAnimationData = UniformStructData<AnimationInfo>;
using UniformAnimationDataPtr = std::shared_ptr<UniformAnimationData>;

static glm::mat4 getOrthographicProjection(const VkExtent2D& frameDim);
static glm::mat4 getPerspective(const VkExtent2D& frameDim, float fov, float near, float far);

class Application : public VulkanGraphicsApp
{
 public:
    void init();
    void run();
    void cleanup();

 protected:
    void initGeometry();
    void initShaders();
    void initUniforms(); 

    void render();

    glm::vec2 getMousePos();

    std::shared_ptr<SimpleVertexBuffer> mGeometry = nullptr;
    UniformTransformDataPtr mTransformUniforms = nullptr;
    UniformAnimationDataPtr mAnimationUniforms = nullptr;
};


int main(int argc, char** argv){
    Application app;
    app.init();
    app.run();
    app.cleanup();

    return(0);
}


glm::vec2 Application::getMousePos(){
    static glm::vec2 previous = glm::vec2(0.0);
    if(glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_1) != GLFW_PRESS)
        return previous;

    double posX, posY;
    glfwGetCursorPos(mWindow, &posX, &posY);

    // Get width and height of window as 2D vector 
    VkExtent2D frameExtent = getFramebufferSize();

    //lab 4: FIX this
    glm::vec2 cursorPosDeviceCoords = glm::vec2(0.0);
    glm::vec2 cursorVkCoords = previous = cursorPosDeviceCoords;
    return(cursorVkCoords);
}

void Application::init(){
    // Initialize GPU devices and display setup
    VulkanSetupBaseApp::init(); 

    // Initialize geometry 
    initGeometry();
    // Initialize shaders
    initShaders();
    // Initialize shader uniform variables
    initUniforms();

    // Initialize graphics pipeline and render setup 
    VulkanGraphicsApp::init();
}

void Application::run(){
    FpsTimer globalRenderTimer(0);
    FpsTimer localRenderTimer(1024);

    // Run until the application is closed
    while(!glfwWindowShouldClose(mWindow)){
        // Poll for window events, keyboard and mouse button presses, ect...
        glfwPollEvents();

        // Render the frame 
        globalRenderTimer.frameStart();
        localRenderTimer.frameStart();
        render();
        globalRenderTimer.frameFinish();
        localRenderTimer.frameFinish();

        // Print out framerate statistics if enough data has been collected 
        if(localRenderTimer.isBufferFull()){
            localRenderTimer.reportAndReset();
        }
        ++mFrameNumber;
    }

    std::cout << "Average Performance: " << globalRenderTimer.getReportString() << std::endl;
    
    // Make sure the GPU is done rendering before moving on. 
    vkDeviceWaitIdle(mDeviceBundle.logicalDevice.handle());
}

void Application::cleanup(){
    // Deallocate the buffer holding our geometry and delete the buffer
    mGeometry->freeBuffer();
    mGeometry = nullptr;

    mTransformUniforms = nullptr;
    mAnimationUniforms = nullptr;

    VulkanGraphicsApp::cleanup();
}

void Application::render(){

    // Set the position of the top vertex 
    //if(glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
    //    glm::vec2 mousePos = getMousePos();
    //    mGeometry->getVertices()[1].pos = glm::vec3(mousePos, 0.0);
    //    mGeometry->updateDevice();
    //    VulkanGraphicsApp::setVertexBuffer(mGeometry->getBuffer(), mGeometry->vertexCount());
    //}

    float time = static_cast<float>(glfwGetTime());
    VkExtent2D frameDimensions = getFramebufferSize();
    static float angle = 0;
    angle+=0.01;
    // Set the value of our uniform variable
    mTransformUniforms->pushUniformData({
        glm::translate(glm::vec3(.1*cos(time), .1*sin(time), -5)) * glm::rotate(time, glm::vec3(0,1,0)),
        glm::mat4(1),
        getPerspective(frameDimensions, 120, 0.1, 150)
    });
    mAnimationUniforms->pushUniformData({time});

    // Tell the GPU to render a frame. 
    VulkanGraphicsApp::render();
}

void Application::initGeometry(){

    ModelContainer mc = ModelContainer("../assets/suzanne.gltf");


    // Create a new vertex buffer on the GPU using the given geometry 
    mGeometry = std::make_shared<SimpleVertexBuffer>(mc.verts, mDeviceBundle);

    // Check to make sure the geometry was uploaded to the GPU correctly. 
    assert(mGeometry->getDeviceSyncState() == DEVICE_IN_SYNC);
    // Specify that we wish to render this vertex buffer
    VulkanGraphicsApp::setVertexBuffer(mGeometry->handle(), mGeometry->vertexCount());

    // Define a description of the layout of the geometry data
    const static SimpleVertexInput vtxInput( /*binding = */ 0U,
        /*vertex attribute descriptions = */ {
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, pos)},
            {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(SimpleVertex, color)},
            {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, normal)}
        }
    );
    // Send this description to the GPU so that it knows how to interpret our vertex buffer 
    VulkanGraphicsApp::setVertexInput(vtxInput.getBindingDescription(), vtxInput.getAttributeDescriptions());

}

void Application::initShaders(){

    // Load the compiled shader code from disk. 
    VkShaderModule vertShader = vkutils::load_shader_module(mDeviceBundle.logicalDevice.handle(), STRIFY(SHADER_DIR) "/standard.vert.spv");
    VkShaderModule fragShader = vkutils::load_shader_module(mDeviceBundle.logicalDevice.handle(), STRIFY(SHADER_DIR) "/vertexColor.frag.spv");
    
    assert(vertShader != VK_NULL_HANDLE);
    assert(fragShader != VK_NULL_HANDLE);

    VulkanGraphicsApp::setVertexShader("standard.vert", vertShader);
    VulkanGraphicsApp::setFragmentShader("vertexColor.frag", fragShader);
}

void Application::initUniforms(){
    mTransformUniforms = UniformTransformData::create();
    mAnimationUniforms = UniformAnimationData::create(); 
    
    VulkanGraphicsApp::addUniform(0, mTransformUniforms);
    VulkanGraphicsApp::addUniform(1, mAnimationUniforms);
}

static glm::mat4 getPerspective(const VkExtent2D& frameDim, float fov, float near, float far){
    float aspect = (float)frameDim.width / (float)frameDim.height;
    glm::mat4 perspective = glm::perspective(fov, aspect, near, far);
    perspective[1][1] = perspective[1][1]*-1;
    return perspective;
}

static glm::mat4 getOrthographicProjection(const VkExtent2D& frameDim){
    float left, right, top, bottom;
    left = top = -1.0;
    right = bottom = 1.0;

    if(frameDim.width > frameDim.height){
        float aspect = static_cast<float>(frameDim.width) / frameDim.height;
        left *= aspect;
        right *= aspect;
    }else{
        float aspect = static_cast<float>(frameDim.height) / frameDim.width;
        top *= aspect;
        right *= aspect;
    }

    return(glm::ortho(left, right, bottom, top));
}