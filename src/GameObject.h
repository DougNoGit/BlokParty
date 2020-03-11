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
    glm::vec4 size;   // -x, x, -y, y
    glm::vec4 bounds; // -x, x, -y, y

public:
    GameObject() 
    {
        velocity = glm::vec3(0);
        size = glm::vec4(-0.5, 0.5, -0.5, 0.5);
        position = glm::vec3(0,-7,-10);
        bounds = glm::vec4(
                        size.x + position.x, 
                        size.y + position.x, 
                        size.z + position.y, 
                        size.a + position.y);
    }
    ~GameObject() {}    
    bool checkCollided();
    void triggerStrafe(float magnitude);
    void triggerImpulse(glm::vec3 direction, float magnitude);
    glm::mat4 updateGameObject(float deltaTime, std::vector<GameObject> allObjects);
};



#endif //GAME_OBJECT_H