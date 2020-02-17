#ifndef GAME_OBJECT_H
#include "ModelContainer.h"

class GameObject
{
private:

public:
    GameObject(/* args */);
    ~GameObject();
    void update(float deltaTime);
    void draw();
};

GameObject::GameObject(/* args */)
{
}

GameObject::~GameObject()
{
}

#endif //GAME_OBJECT_H