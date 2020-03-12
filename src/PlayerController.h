#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "InputAdapter.h"
#include "GameObject.h"

class PlayerController
{
private:
    GameObject playerGameObject = GameObject();
    FilteredData * data = nullptr;
public:
    PlayerController(){
        W = A = S = D = false;
    }
    ~PlayerController(){}
    GameObject* getGameObjectPtr();
    void setInputData(FilteredData * _data);
    void update();
    bool W,A,S,D;
};

#endif // PLAYER_CONTROLLER_H