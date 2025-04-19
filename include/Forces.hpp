#pragma once
#include <SFML/Graphics.hpp>

class PhysicsObject;


enum class ForceType {
    GRAVITY,
    DRAG,
    SPRING,
    CUSTOM
};

class Forces {
public:
    virtual ~Forces() = default;
    virtual sf::Vector2f computeForce(PhysicsObject& obj) = 0;
    virtual ForceType getType() const = 0;
};


class Gravity : public Forces {
private:
    sf::Vector2f gravity;
public:
    Gravity(float gx = 0.0f, float gy = 9.8f);
    sf::Vector2f computeForce(PhysicsObject& obj) override;
    ForceType getType() const override {
        return ForceType::GRAVITY;
    }
};
