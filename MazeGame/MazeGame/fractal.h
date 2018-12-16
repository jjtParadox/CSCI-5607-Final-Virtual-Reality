#pragma once
#include "game_object.h"

class Controller;

class Fractal : public GameObject {
   public:
    Fractal(Model* model) : GameObject(model) {}
    ~Fractal() = default;

    Controller* holder_ = nullptr;
};
