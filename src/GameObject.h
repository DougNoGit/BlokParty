#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
#include "ModelContainer.h"

#define GRAVITY 10.0

class GameObject
{
private:
    bool checkCollided();
    glm::vec3 velocity;
    glm::vec3 position;

public:
    GameObject() {}
    ~GameObject() {}
    glm::mat4 updateGameObject(float deltaTime);
};



#endif //GAME_OBJECT_H