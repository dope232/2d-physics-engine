#include "Liquidfluid.hpp"

LiquidParticle::LiquidParticle(const sf::Vector2f& center, float radius, sf::Vector2f velocity, 
    sf::Vector2f acceleration, sf::Color color, float density, 
    float lifetime, float fadeFactor)
: RigidBody(center, radius, velocity, acceleration, color, density), 
lifetime(lifetime), age(0.0f), fadeFactor(fadeFactor) {
}


void LiquidParticle::update(float dt, float window_width, float window_height) {
    age += dt;
    
    applyForces(dt);
    
    velocity += acceleration * dt;
    com += velocity * dt;
    
    vertices[0] = com;
    
    float bounceFactor = 0.2f;
    
    if (com.x - radius < 0) {
        com.x = radius;  
        velocity.x *= -bounceFactor;
        velocity.x *= 0.9f;
    }
    if (com.x + radius > window_width) {
        com.x = window_width - radius;
        velocity.x *= -bounceFactor;
        velocity.x *= 0.9f;
    }
    if (com.y - radius < 0) {
        com.y = radius;
        velocity.y *= -bounceFactor;
    }
    if (com.y + radius > window_height) {
        com.y = window_height - radius;
        velocity.y *= -bounceFactor;
        
        velocity.x *= 0.95f;
    }
}

bool LiquidParticle::isDead() const {
    return age >= lifetime;
}

void LiquidParticle::draw(sf::RenderWindow& window) {
    float alpha = 255.0f * (1.0f - (age / lifetime));
    
    sf::CircleShape circle(radius);
    sf::Color particleColor = color;
    particleColor.a = static_cast<sf::Uint8>(alpha);
    
    circle.setPosition(vertices[0] - sf::Vector2f(radius, radius));
    circle.setFillColor(particleColor);
    window.draw(circle);
}
