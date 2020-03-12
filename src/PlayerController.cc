#include "PlayerController.h"

void PlayerController::setInputData(FilteredData * _data) 
{
    data = _data;
}

GameObject * PlayerController::getGameObjectPtr()
{
    return &playerGameObject;
}
    
void PlayerController::update()
{
    if(playerGameObject.isCollided() && data->up)
        playerGameObject.triggerImpulse(glm::vec3(0,-1,0), 10); 

    if(data->lf)
        playerGameObject.triggerStrafe(-5);

    if(data->rt)
        playerGameObject.triggerStrafe(5);
}