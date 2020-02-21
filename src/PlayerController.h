#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "InputAdapter.h"
#include "GameObject.h"

class PlayerController
{
private:
    GameObject playerGameObject = GameObject();
    InputData * data = nullptr;
public:
    PlayerController(){
        W = A = S = D = false;
    }
    ~PlayerController(){}
    void setInputData(InputData * _data);
    glm::mat4 update(float deltaTime);
    bool W,A,S,D;

};

#endif // PLAYER_CONTROLLER_H