#include "PlayerController.h"

void PlayerController::setInputData(InputData * _data) 
{
    data = _data;
}
    
glm::mat4 PlayerController::update(float deltaTime)
{
    if(playerGameObject.checkCollided() && data->W)
        playerGameObject.triggerImpulse(glm::vec3(0,-1,0), 10); 

    if(data->A)
        playerGameObject.triggerStrafe(-10);

    if(data->D)
        playerGameObject.triggerStrafe(10);

    return playerGameObject.updateGameObject(deltaTime);
}