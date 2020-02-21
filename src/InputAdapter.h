#ifndef INPUT_ADAPTER_H
#define INPUT_ADAPTER_H

#include <GLFW/glfw3.h>
#include "InputAdapterIntermediate.h"

class InputAdapter : public KeyEventHandler
{
private:
    InputData *data;
    InputAdapterIntermediate intermediate = InputAdapterIntermediate();

public:
    InputAdapter() {}
    ~InputAdapter() {}
    void init(InputData *in_data, GLFWwindow *window)
    {
        intermediate.setEventCallbacks(this);
        intermediate.init(window);
        data = in_data;
    }
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        switch (key)
        {
        case (GLFW_KEY_ESCAPE):
            glfwSetWindowShouldClose(window, true);
            break;
        case (GLFW_KEY_W):
            data->W = (action != GLFW_RELEASE);
            break;
        case (GLFW_KEY_A):
            data->A = (action != GLFW_RELEASE);
            break;
        case (GLFW_KEY_S):
            data->S = (action != GLFW_RELEASE);
            break;
        case (GLFW_KEY_D):
            data->D = (action != GLFW_RELEASE);
            break;
        }
    }
};

#endif