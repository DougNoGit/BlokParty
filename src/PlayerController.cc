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
        playerGameObject.triggerJump(JUMP_STRENGTH); 

    if(data->lf)
        playerGameObject.triggerStrafe(-1 * STRAFE_STRENGTH);

    if(data->rt)
        playerGameObject.triggerStrafe(STRAFE_STRENGTH);
}