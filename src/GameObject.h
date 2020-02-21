#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "ModelContainer.h"

#define GRAVITY 10.0

class GameObject
{
private:
    glm::vec3 velocity;
    glm::vec3 position;

public:
    GameObject() 
    {
        velocity = glm::vec3(0);
        position = glm::vec3(0,-7,-10);
    }
    ~GameObject() {}    
    bool checkCollided();
    void triggerStrafe(float magnitude);
    void triggerImpulse(glm::vec3 direction, float magnitude);
    glm::mat4 updateGameObject(float deltaTime);
};



#endif //GAME_OBJECT_H