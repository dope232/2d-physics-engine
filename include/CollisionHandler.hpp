#pragma once
#include "RigidBody.hpp"

class CollisionHandler {
public:
    struct CollisionInfo {
        bool hasCollision;
        sf::Vector2f normal;
        sf::Vector2f point;
        float penetrationDepth;
        
        CollisionInfo() : hasCollision(false), normal(0, 0), point(0, 0), penetrationDepth(0) {}
    };

    static CollisionInfo detectCollision(RigidBody* bodyA, RigidBody* bodyB);
    static void resolveCollision(RigidBody* bodyA, RigidBody* bodyB, const CollisionInfo& info);

private:
    static CollisionInfo circleVsCircle(RigidBody* circleA, RigidBody* circleB);
    static CollisionInfo polygonVsPolygon(RigidBody* polyA, RigidBody* polyB);
    static CollisionInfo circleVsPolygon(RigidBody* circle, RigidBody* poly);
    
    static bool checkOverlapOnAxis(const sf::Vector2f& axis, const std::vector<sf::Vector2f>& vertsA, 
                                  const std::vector<sf::Vector2f>& vertsB, float& overlap);
};
