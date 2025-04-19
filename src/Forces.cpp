#include <SFML/Graphics.hpp>
#include "Forces.hpp"
#include "PhysicsObject.hpp"

Gravity::Gravity(float gx, float gy) : gravity(gx, gy) {}


sf::Vector2f Gravity::computeForce(PhysicsObject& obj){
    return obj.getMass() * gravity;
}
