#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>
#include "Forces.hpp" 

class PhysicsObject {
public:
    enum class shapetype { CIRCLE, POLYGON };

    float radius;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    sf::Color color;
    std::vector<sf::Vector2f> vertices;
    shapetype type;
    float density;
    float mass;
    float area;
    sf::Vector2f com;

    std::vector<Forces*> forces; 

    PhysicsObject(std::vector<sf::Vector2f> vertices, sf::Vector2f velocity, sf::Vector2f acceleration, sf::Color color, float density)
      : vertices(vertices), velocity(velocity), acceleration(acceleration),
        color(color), density(density), mass(0.0f), area(0.0f), com({0, 0}) {
        type = (vertices.size() == 1) ? shapetype::CIRCLE : shapetype::POLYGON;
    }

    PhysicsObject(std::vector<sf::Vector2f> vertices, float radius, sf::Vector2f velocity, sf::Vector2f acceleration, sf::Color color, float density)
      : vertices(vertices), radius(radius), velocity(velocity), acceleration(acceleration),
        color(color), density(density), mass(0.0f), area(0.0f), com({0, 0}) {
        type = shapetype::CIRCLE;
    }

    virtual ~PhysicsObject() {
        for (Forces* f : forces) delete f;
    }

    virtual void update(float dt, float window_width, float window_height) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual float computeArea() = 0;
    virtual void computeMass() = 0;
    virtual void computeCOM() = 0;

    float getMass() const { return mass; }
    sf::Vector2f getCOM() const { return com; }

    void removeForcesByType(ForceType type) {
        forces.erase(
            std::remove_if(forces.begin(), forces.end(),
                [type](Forces* force) {
                    if (force->getType() == type) {
                        delete force;
                        return true;
                    }
                    return false;
                }),
            forces.end()
        );
    }
};
