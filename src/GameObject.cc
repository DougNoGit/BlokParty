#include "GameObject.h"
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
    bool collidedX = false;
    bool collidedY = false;

    glm::vec3 nextTranslation = deltaTime * velocity;
    glm::vec3 nextTranslationX = glm::vec3(nextTranslation.x, 0, 0);
    glm::vec3 nextTranslationY = glm::vec3(0, nextTranslation.y, 0);

    glm::vec3 nextPosition = position + nextTranslation;
    glm::vec3 nextPositionX = glm::vec3(nextPosition.x, position.y, position.z);
    glm::vec3 nextPositionY = glm::vec3(position.x, nextPosition.y, position.z);

    glm::vec4 nextBounds = updateBounds(bounds, nextTranslation);
    glm::vec4 nextBoundsX = updateBounds(bounds, nextTranslationX);
    glm::vec4 nextBoundsY = updateBounds(bounds, nextTranslationY);

    // check against other objects
    for(int i = 0; i < allObjects.size(); i++)
    {
        isCurrentlyCollided |= (checkCollidedAt(nextBounds, allObjects[i]) && (this != allObjects[i]));
        collidedX |= (checkCollidedAt(nextBoundsX, allObjects[i]) && (this != allObjects[i]));
        collidedY |= (checkCollidedAt(nextBoundsY, allObjects[i]) && (this != allObjects[i]));
    }

    // check against the ground 
    collidedY |= nextPosition.y > BOTTOM;
    
    // update collision status if either axis is collided 
    isCurrentlyCollided |= (collidedY || collidedX);

    if(isCurrentlyCollided)
    {
        if((!collidedY) && (!collidedX))
        {
            // break ties by falling instead of strafing,
            // this case occurs when diagonal movement causes a 
            // collision
            position = nextPositionY;
            bounds = nextBoundsY;
            velocity.y += GRAVITY * deltaTime;
            velocity.x *= -1 * BOUNCE_COEFFICIENT;
        }
        else if(collidedY && (!collidedX))
        {
            position = nextPositionX;
            bounds = nextBoundsX;
            velocity.x += (-velocity.x * deltaTime * FRICTION_COEFFICIENT);
            velocity.y *= -1 * BOUNCE_COEFFICIENT;
        }
        else if((!collidedY) && collidedX)
        {
            position = nextPositionY;
            bounds = nextBoundsY;
            velocity.y += GRAVITY * deltaTime;
            velocity.x *= -1 * BOUNCE_COEFFICIENT;
        }
        else
        { 
            velocity.y *= -1 * BOUNCE_COEFFICIENT;
            velocity.x *= -1 * BOUNCE_COEFFICIENT;
            return glm::translate(position);
        }
    } else {            
        position = nextPosition;
        bounds = nextBounds;
        velocity.y += GRAVITY * deltaTime;
        velocity.x += (-velocity.x * deltaTime * FRICTION_COEFFICIENT);
    }

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

void GameObject::triggerJump(float magnitude)
{
    velocity.y = -1 * magnitude;
}

bool GameObject::isCollided()
{
    return isCurrentlyCollided;
}


