#include "GameObject.h"

glm::mat4 GameObject::updateGameObject(float deltaTime)
{
    return glm::mat4(1);
}

bool GameObject::checkCollided()
{
    if(position.y > 5)
        return true;
    else
        return false;
    
}
