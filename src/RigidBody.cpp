#include "RigidBody.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

RigidBody::RigidBody(const std::vector<sf::Vector2f>& vertices, sf::Vector2f velocity, sf::Vector2f acceleration, sf::Color color, float density)
    : PhysicsObject(vertices, velocity, acceleration, color, density) {
    type = shapetype::POLYGON;
    computeArea();
    computeCOM();
    computeMass();
    std::cout << "RigidBody (Polygon) created with " << vertices.size() << " vertices.\n";
}

RigidBody::RigidBody(const sf::Vector2f& center, float radius, sf::Vector2f velocity, sf::Vector2f acceleration, sf::Color color, float density)
    : PhysicsObject({center}, radius, velocity, acceleration, color, density) {
    type = shapetype::CIRCLE;
    computeArea();
    computeCOM();
    computeMass();
    std::cout << "RigidBody (Circle) created at (" << center.x << ", " << center.y << ") with radius: " << radius << "\n";
}

void RigidBody::addForce(Forces* force) {
    forces.push_back(force);
}

void RigidBody::applyForces(float dt) {
    sf::Vector2f totalForce(0, 0);
    for (Forces* force : forces) {
        totalForce += force->computeForce(*this);
    }
    
    if (mass > 0) {
        acceleration = totalForce / mass;
    }
}

void RigidBody::update(float dt, float window_width, float window_height) {
    applyForces(dt);
    velocity += acceleration * dt;
    sf::Vector2f oldCom = com;
    com += velocity * dt;
    sf::Vector2f displacement = com - oldCom;
    
    for (auto& vertex : vertices) {
        vertex += displacement;
    }
    
    float bounceFactor = 0.5f;
    
    if (type == shapetype::CIRCLE) {
        bool collided = false;
        
        if (com.x - radius < 0) {
            com.x = radius;
            velocity.x *= -bounceFactor;
            collided = true;
        }
        else if (com.x + radius > window_width) {
            com.x = window_width - radius;
            velocity.x *= -bounceFactor;
            collided = true;
        }
        
        if (com.y - radius < 0) {
            com.y = radius;
            velocity.y *= -bounceFactor;
            collided = true;
        }
        else if (com.y + radius > window_height) {
            com.y = window_height - radius;
            velocity.y *= -bounceFactor;
            collided = true;
        }
        
        if (collided) {
            vertices[0] = com;
        }
    }
    else if (type == shapetype::POLYGON) {
        float minX = window_width, minY = window_height, maxX = 0, maxY = 0;
        for (const auto& vertex : vertices) {
            minX = std::min(minX, vertex.x);
            minY = std::min(minY, vertex.y);
            maxX = std::max(maxX, vertex.x);
            maxY = std::max(maxY, vertex.y);
        }
        
        float adjustX = 0, adjustY = 0;
        bool collidedX = false, collidedY = false;
        
        if (minX < 0) {
            adjustX = -minX;
            collidedX = true;
        } 
        else if (maxX > window_width) {
            adjustX = window_width - maxX;
            collidedX = true;
        }
        
        if (minY < 0) {
            adjustY = -minY;
            collidedY = true;
        } 
        else if (maxY > window_height) {
            adjustY = window_height - maxY;
            collidedY = true;
        }
        
        if (collidedX || collidedY) {
            for (auto& vertex : vertices) {
                vertex.x += adjustX;
                vertex.y += adjustY;
            }
            com.x += adjustX;
            com.y += adjustY;
            
            if (collidedX) {
                velocity.x *= -bounceFactor;
            }
            if (collidedY) {
                velocity.y *= -bounceFactor;
            }
        }
    }
}

float RigidBody::computeArea() {
    if (type == shapetype::POLYGON) {
        float signedArea = 0;
        for (size_t i = 0; i < vertices.size(); ++i) {
            sf::Vector2f current = vertices[i];
            sf::Vector2f next = vertices[(i + 1) % vertices.size()];
            signedArea += current.x * next.y - next.x * current.y;
        }
        area = std::abs(signedArea) / 2;
    } else if (type == shapetype::CIRCLE) {
        area = 3.14159f * radius * radius;
    }
    return area;
}

void RigidBody::computeCOM() {
    if (type == shapetype::POLYGON) {
        sf::Vector2f centerOfMass(0, 0);
        float signedArea = 0;
        for (size_t i = 0; i < vertices.size(); ++i) {
            sf::Vector2f current = vertices[i];
            sf::Vector2f next = vertices[(i + 1) % vertices.size()];
            float crossProduct = current.x * next.y - next.x * current.y;
            signedArea += crossProduct;
            centerOfMass += (current + next) * crossProduct;
        }
        signedArea = std::abs(signedArea) / 2;
        if (signedArea > 0) {
            centerOfMass /= (6 * signedArea);
            com = centerOfMass;
        }
    } else if (type == shapetype::CIRCLE) {
        com = vertices[0];
    }
}

void RigidBody::computeMass() {
    mass = density * area;
}

void RigidBody::draw(sf::RenderWindow& window) {
    if (type == shapetype::POLYGON) {
        sf::ConvexShape polygon;
        polygon.setPointCount(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            polygon.setPoint(i, vertices[i]);
        }
        polygon.setFillColor(color);
        window.draw(polygon);
    } else if (type == shapetype::CIRCLE) {
        sf::CircleShape circle(radius);
        circle.setPosition(vertices[0] - sf::Vector2f(radius, radius)); 
        circle.setFillColor(color);
        window.draw(circle);
    }
}
