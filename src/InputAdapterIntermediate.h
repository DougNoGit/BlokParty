#ifndef INPUT_ADAPTER_INTERMEDIATE_H
#define INPUT_ADAPTER_INTERMEDIATE_H

#include <GLFW/glfw3.h>

typedef struct _inputData {
    bool W,A,S,D;
} InputData;

class KeyEventHandler
{
public:
	virtual void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) = 0;
};

class InputAdapterIntermediate
{
public:
    InputAdapterIntermediate();
    ~InputAdapterIntermediate();
    void init(GLFWwindow *window);
	void setEventCallbacks(KeyEventHandler *in_keyEventHandler);

protected:
    // This class implements the singleton design pattern
	static InputAdapterIntermediate * instance;

	KeyEventHandler *keyEventHandler = nullptr;

private:
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
};

#endif 