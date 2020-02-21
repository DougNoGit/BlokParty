#include "GameObject.h"

#define BOTTOM 2.0

// TODO better float comparison

void outVec3(glm::vec3 vec)
{
    std::cout << "X: " << vec.x << " Y: " << vec.y << " Z: " << vec.z << std::endl;
} 

bool checkCollidedAt(glm::vec3 pos)
{
    if(pos.y > BOTTOM)
        return true;
    else
        return false;
    
}

glm::mat4 GameObject::updateGameObject(float deltaTime)
{
    glm::vec3 newPostition = deltaTime * velocity + position;

    if(checkCollidedAt(newPostition))
    {
        velocity.y = -0.3 * velocity.y;
        position.y = BOTTOM;
    } else {    
        velocity.y += 10 * deltaTime;
        position = newPostition;
    }
        
    velocity.x += (-velocity.x * deltaTime * 5.0);
    //outVec3(position);
    return glm::translate(position);
}

void GameObject::triggerImpulse(glm::vec3 direction, float magnitude)
{
    velocity += direction * magnitude;
}

void GameObject::triggerStrafe(float magnitude)
{
    velocity.x = magnitude;
}

bool GameObject::checkCollided()
{
    if(abs(BOTTOM - position.y) < 0.0001)
        return true;
    else
        return false;
    
}


