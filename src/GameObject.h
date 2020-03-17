#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "ModelContainer.h"

#define BOTTOM 2.0f
#define GRAVITY 10.0f
#define BOUNCE_COEFFICIENT 0.4f
#define FRICTION_COEFFICIENT 4.0f

class GameObject
{
private:
    glm::vec3 velocity;
    glm::vec3 position;
    glm::vec4 size;   // -x, x, -y, y
    glm::vec4 bounds; // -x, x, -y, y
    bool isCurrentlyCollided;

public:
    GameObject() 
    {
        isCurrentlyCollided = false;
        velocity = glm::vec3(0);
        size = glm::vec4(-1, 1, -1, 1);
        position = glm::vec3(0,-7,-10);
        bounds = glm::vec4(
                        size.x + position.x, 
                        size.y + position.x, 
                        size.z + position.y, 
                        size.a + position.y);
    }
    ~GameObject() {}      

    bool isCollided();
    glm::vec4 getBounds();
    void setPosition(glm::vec3 pos);
    void triggerJump(float magnitude);
    void triggerStrafe(float magnitude);
    void triggerImpulse(glm::vec3 combinedVelocityVector);
    bool checkCollidedAt(glm::vec4 newBounds, GameObject* other);
    void triggerImpulse(glm::vec3 direction, float magnitude);
    glm::vec4 updateBounds(glm::vec4 oldBounds, glm::vec3 translation);
    glm::mat4 updateGameObject(float deltaTime, std::vector<GameObject*> allObjects);
};



#endif //GAME_OBJECT_H