#include "CollisionHandler.hpp"
#include <limits>
#include <cmath>

float vectorLength(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float length = vectorLength(v);
    if (length > 0) {
        return sf::Vector2f(v.x / length, v.y / length);
    }
    return v;
}

float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

CollisionHandler::CollisionInfo CollisionHandler::detectCollision(RigidBody* bodyA, RigidBody* bodyB) {
    if (bodyA->type == PhysicsObject::shapetype::CIRCLE && bodyB->type == PhysicsObject::shapetype::CIRCLE) {
        return circleVsCircle(bodyA, bodyB);
    }
    else if (bodyA->type == PhysicsObject::shapetype::POLYGON && bodyB->type == PhysicsObject::shapetype::POLYGON) {
        return polygonVsPolygon(bodyA, bodyB);
    }
    else if (bodyA->type == PhysicsObject::shapetype::CIRCLE && bodyB->type == PhysicsObject::shapetype::POLYGON) {
        return circleVsPolygon(bodyA, bodyB);
    }
    else if (bodyA->type == PhysicsObject::shapetype::POLYGON && bodyB->type == PhysicsObject::shapetype::CIRCLE) {
        CollisionInfo info = circleVsPolygon(bodyB, bodyA);
        if (info.hasCollision) {
            info.normal = -info.normal;
        }
        return info;
    }
    
    return CollisionInfo();
}

CollisionHandler::CollisionInfo CollisionHandler::circleVsCircle(RigidBody* circleA, RigidBody* circleB) {
    CollisionInfo info;
    
    sf::Vector2f delta = circleB->com - circleA->com;
    float distance = vectorLength(delta);
    float sumRadii = circleA->radius + circleB->radius;
    
    if (distance < sumRadii) {
        info.hasCollision = true;
        info.normal = normalize(delta);
        info.penetrationDepth = sumRadii - distance;
        
        info.point = circleA->com + info.normal * circleA->radius;
    }
    
    return info;
}

sf::Vector2f getEdge(const std::vector<sf::Vector2f>& vertices, int index) {
    int nextIndex = (index + 1) % vertices.size();
    return vertices[nextIndex] - vertices[index];
}

CollisionHandler::CollisionInfo CollisionHandler::polygonVsPolygon(RigidBody* polyA, RigidBody* polyB) {
    CollisionInfo info;
    float minOverlap = std::numeric_limits<float>::max();
    sf::Vector2f collisionNormal;
    
    for (size_t i = 0; i < polyA->vertices.size(); i++) {
        sf::Vector2f edge = getEdge(polyA->vertices, i);
        sf::Vector2f axis(-edge.y, edge.x);
        axis = normalize(axis);
        
        float overlap = 0;
        if (!checkOverlapOnAxis(axis, polyA->vertices, polyB->vertices, overlap)) {
            return info;
        }
        
        if (overlap < minOverlap) {
            minOverlap = overlap;
            collisionNormal = axis;
        }
    }
    
    for (size_t i = 0; i < polyB->vertices.size(); i++) {
        sf::Vector2f edge = getEdge(polyB->vertices, i);
        sf::Vector2f axis(-edge.y, edge.x);
        axis = normalize(axis);
        
        float overlap = 0;
        if (!checkOverlapOnAxis(axis, polyA->vertices, polyB->vertices, overlap)) {
            return info;
        }
        
        if (overlap < minOverlap) {
            minOverlap = overlap;
            collisionNormal = axis;
        }
    }
    
    sf::Vector2f centerDiff = polyB->com - polyA->com;
    if (dot(centerDiff, collisionNormal) < 0) {
        collisionNormal = -collisionNormal;
    }
    
    info.hasCollision = true;
    info.normal = collisionNormal;
    info.penetrationDepth = minOverlap;
    
    info.point = polyA->com + collisionNormal * (minOverlap / 2);
    
    return info;
}

CollisionHandler::CollisionInfo CollisionHandler::circleVsPolygon(RigidBody* circle, RigidBody* poly) {
    CollisionInfo info;
    
    sf::Vector2f closestPoint = poly->vertices[0];
    float closestDistSq = std::numeric_limits<float>::max();
    
    for (const auto& vertex : poly->vertices) {
        sf::Vector2f diff = circle->com - vertex;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestPoint = vertex;
        }
    }
    
    for (size_t i = 0; i < poly->vertices.size(); i++) {
        sf::Vector2f start = poly->vertices[i];
        sf::Vector2f end = poly->vertices[(i + 1) % poly->vertices.size()];
        sf::Vector2f edge = end - start;
        sf::Vector2f circleToStart = circle->com - start;
        
        float projection = dot(circleToStart, edge) / dot(edge, edge);
        
        projection = std::max(0.0f, std::min(1.0f, projection));
        
        sf::Vector2f pointOnEdge = start + projection * edge;
        
        sf::Vector2f diff = circle->com - pointOnEdge;
        float distSq = diff.x * diff.x + diff.y * diff.y;
        
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestPoint = pointOnEdge;
        }
    }
    
    sf::Vector2f delta = closestPoint - circle->com;
    float distance = vectorLength(delta);
    
    if (distance < circle->radius) {
        info.hasCollision = true;
        info.normal = normalize(delta);
        info.penetrationDepth = circle->radius - distance;
        info.point = closestPoint;
    }
    
    return info;
}

bool CollisionHandler::checkOverlapOnAxis(const sf::Vector2f& axis, 
                                         const std::vector<sf::Vector2f>& vertsA, 
                                         const std::vector<sf::Vector2f>& vertsB,
                                         float& overlap) {
    float minA = std::numeric_limits<float>::max();
    float maxA = std::numeric_limits<float>::lowest();
    float minB = std::numeric_limits<float>::max();
    float maxB = std::numeric_limits<float>::lowest();
    
    for (const auto& v : vertsA) {
        float proj = dot(v, axis);
        minA = std::min(minA, proj);
        maxA = std::max(maxA, proj);
    }
    
    for (const auto& v : vertsB) {
        float proj = dot(v, axis);
        minB = std::min(minB, proj);
        maxB = std::max(maxB, proj);
    }
    
    float overlapAB = maxA - minB;
    float overlapBA = maxB - minA;
    
    if (maxA < minB || maxB < minA) {
        return false;
    }
    
    overlap = std::min(overlapAB, overlapBA);
    return true;
}

void CollisionHandler::resolveCollision(RigidBody* bodyA, RigidBody* bodyB, const CollisionInfo& info) {
    if (!info.hasCollision) return;
    
    const float restitution = 0.6f;
    
    sf::Vector2f relativeVelocity = bodyB->velocity - bodyA->velocity;
    
    float velAlongNormal = dot(relativeVelocity, info.normal);
    
    if (velAlongNormal > 0) return;
    
    float j = -(1.0f + restitution) * velAlongNormal;
    j /= (1.0f / bodyA->mass) + (1.0f / bodyB->mass);
    
    sf::Vector2f impulse = j * info.normal;
    
    bodyA->velocity -= impulse / bodyA->mass;
    bodyB->velocity += impulse / bodyB->mass;
    
    const float percent = 0.2f;
    const float slop = 0.01f;
    
    sf::Vector2f correction = std::max(info.penetrationDepth - slop, 0.0f) * percent * 
                              info.normal / ((1.0f / bodyA->mass) + (1.0f / bodyB->mass));
    
    bodyA->com -= correction / bodyA->mass;
    bodyB->com += correction / bodyB->mass;
    
    sf::Vector2f correctA = -correction / bodyA->mass;
    sf::Vector2f correctB = correction / bodyB->mass;
    
    for (auto& vertex : bodyA->vertices) {
        vertex += correctA;
    }
    
    for (auto& vertex : bodyB->vertices) {
        vertex += correctB;
    }
}
