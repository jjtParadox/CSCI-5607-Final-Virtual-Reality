#pragma once
#include "game_object.h"

class Map;
class Controller;

class Key : public GameObject {
   public:
    explicit Key(Model* model, Map* map, char id, glm::vec2 pos);
    ~Key() = default;

    void Update() override;
    void GoAway();
    void SetHolder(Controller* player);
    void Drop();
    bool CanBePickedUp();

   private:
    void InitTransform();

    char id_;
    Controller* holder_;
    int drop_time_ = 0;
};
