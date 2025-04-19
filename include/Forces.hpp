#pragma once
#include <SFML/Graphics.hpp>

// Forward declaration instead of including the header
class PhysicsObject;

// Define force types for easy identification
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

// Gravity force class
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