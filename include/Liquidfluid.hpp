#pragma once
#include "RigidBody.hpp"
class LiquidParticle : public RigidBody {
    private:
        float lifetime;
        float age;
        float fadeFactor;
        
    public:
        LiquidParticle(const sf::Vector2f& center, float radius, sf::Vector2f velocity, 
                      sf::Vector2f acceleration, sf::Color color, float density = 0.8f, 
                      float lifetime = 5.0f, float fadeFactor = 0.8f);
        
        void update(float dt, float window_width, float window_height) override;
        bool isDead() const;
        void draw(sf::RenderWindow& window) override;
    };