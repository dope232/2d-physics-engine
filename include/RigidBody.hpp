#pragma once
#include "PhysicsObject.hpp"
#include "Forces.hpp"
#include <vector>

class RigidBody : public PhysicsObject {
private:
    std::vector<Forces*> forces;

public:
    RigidBody(const std::vector<sf::Vector2f>& vertices, sf::Vector2f velocity, sf::Vector2f acceleration, sf::Color color, float density = 7050.0f); // polygon
    RigidBody(const sf::Vector2f& center, float radius, sf::Vector2f velocity, sf::Vector2f acceleration, sf::Color color, float density = 7050.0f); // circle

    void addForce(Forces* force);
    void applyForces(float dt);
    void update(float dt, float window_width, float window_height) override;
    void draw(sf::RenderWindow& window) override;
    float computeArea() override;
    void computeMass() override;
    void computeCOM() override;
};