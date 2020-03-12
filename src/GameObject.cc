#include "GameObject.h"

#define BOTTOM 2.0

// TODO better float comparison

void outVec3(glm::vec3 vec)
{
    std::cout << "X: " << vec.x << " Y: " << vec.y << " Z: " << vec.z << std::endl;
} 

void outVec4(glm::vec4 vec)
{
    std::cout << "X: " << vec.x << " Y: " << vec.y << " Z: " << vec.z << " A: " << vec.a << std::endl;
} 

glm::vec4 GameObject::getBounds()
{
    return bounds;
}

bool GameObject::checkCollidedAt(glm::vec4 newBounds, GameObject* other)
{
    glm::vec4 otherBounds = other->getBounds();
    //  bounds are  -x, x, -y, y
    //               x, y,  z, a
    return ((
                ((newBounds.y >= otherBounds.x) && (newBounds.y <= otherBounds.y)) ||
                ((newBounds.x <= otherBounds.y) && (newBounds.x >= otherBounds.y))
            ) &&
            (
                ((newBounds.z <= otherBounds.a) && (newBounds.z >= otherBounds.z)) || 
                ((newBounds.a >= otherBounds.z) && (newBounds.a <= otherBounds.a))
            ));
}

void GameObject::setPosition(glm::vec3 pos)
{
    position = pos;
    bounds = updateBounds(size, pos);
}

glm::vec4 GameObject::updateBounds(glm::vec4 oldBounds, glm::vec3 translation)
{
    //  bounds are  -x, x, -y, y
    oldBounds.x += translation.x;
    oldBounds.y += translation.x;
    oldBounds.z += translation.y;
    oldBounds.a += translation.y;

    return oldBounds;
}

glm::mat4 GameObject::updateGameObject(float deltaTime, std::vector<GameObject*> allObjects)
{
    isCurrentlyCollided = false;
    glm::vec3 nextTranslation = deltaTime * velocity;
    glm::vec3 nextPosition = position + nextTranslation;
    glm::vec4 nextBounds = updateBounds(bounds, nextTranslation);

    // check against other objects
    for(int i = 0; i < allObjects.size(); i++)
    {
        isCurrentlyCollided |= (checkCollidedAt(nextBounds, allObjects[i]) && (this != allObjects[i]));
    }

    // check against the ground 
    isCurrentlyCollided |= nextPosition.y > BOTTOM;

    if(isCurrentlyCollided)
    {
        // bounce
        velocity = -0.3f * velocity;
    } else {            
        position = nextPosition;
        bounds = nextBounds;
        velocity.y += 10 * deltaTime;
    }

    velocity.x += (-velocity.x * deltaTime * 2.0);
    return glm::translate(position);
}

void GameObject::triggerImpulse(glm::vec3 direction, float magnitude)
{
    velocity += direction * magnitude;
}

void GameObject::triggerImpulse(glm::vec3 combinedVelocityVector)
{
    velocity += combinedVelocityVector;
}

void GameObject::triggerStrafe(float magnitude)
{
    velocity.x = magnitude;
}

bool GameObject::isCollided()
{
    return isCurrentlyCollided;
}


