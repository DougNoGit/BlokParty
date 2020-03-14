#include "InputAdapterIntermediate.h"

#include <iostream>

InputAdapterIntermediate * InputAdapterIntermediate::instance = nullptr;

InputAdapterIntermediate::InputAdapterIntermediate()
{
	if (instance)
	{
		std::cerr << "One instance of WindowManager has already been created, event callbacks of new instance will not work." << std::endl;
	}

	instance = this;
}

InputAdapterIntermediate::~InputAdapterIntermediate()
{
	if (instance == this)
	{
		instance = nullptr;
	}
}

void InputAdapterIntermediate::setEventCallbacks(KeyEventHandler *in_keyEventHandler) 
{
    keyEventHandler = in_keyEventHandler;
}


void InputAdapterIntermediate::init(GLFWwindow *window)
{
    glfwSetKeyCallback(window, key_callback);
}

void InputAdapterIntermediate::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (instance && instance->keyEventHandler)
	{
		instance->keyEventHandler->keyCallback(window, key, scancode, action, mods);
	}
}