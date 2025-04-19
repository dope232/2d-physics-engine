#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "RigidBody.hpp"
#include "Liquidfluid.hpp"
#include "GUI.hpp"

enum class ShapeType {
    RECTANGLE,
    TRIANGLE,
    CIRCLE,
    LIQUID
};

class Environment;

class UserInput {
public:
    UserInput(Environment& env);
    void handleInput(sf::Event event);
    void drawPreview(sf::RenderWindow& window);

private:
    Environment& environment;
    std::vector<sf::Vector2f> tempVertices; 
    sf::Vector2f dragStart, dragEnd;
    bool dragging;
};

class Environment {
public:
    Environment(int width, int height, const std::string& title);
    void run();
    ShapeType getCurrentMode() const { return currMode; }
    sf::RenderWindow& getWindow() { return window; }
    void addRigidBody(RigidBody* obj) { rigidobjs.push_back(obj); }
    void spawnLiquidObjects(const sf::Vector2f& position, int count);
    void createRectangle(const sf::Vector2f& start, const sf::Vector2f& end);
    void createTriangle(const sf::Vector2f& start, const sf::Vector2f& end);


    void togglePropertiesPanel();
    void setupPropertiesPanel();
    void updateGravityOnObjects();
    float getDefaultDensity() const { return defaultDensity; }
    float getLiquidDensity() const { return liquidDensity; }
    float getLiquidLifetime() const { return liquidLifetime; }
    float getLiquidFadeFactor() const { return liquidFadeFactor; }
    sf::Vector2f getGravity() const { return sf::Vector2f(gravityX, gravityY); }

private:
    sf::RenderWindow window;
    sf::Clock clock;
    gui::GUI gui;
    sf::Font font;
    ShapeType currMode;
    UserInput userInput;
    std::vector<RigidBody*> rigidobjs;
    std::vector<LiquidParticle*> liquidobjs;
    bool isPaused = false;

    std::shared_ptr<gui::ToggleButton> rectButton;
    std::shared_ptr<gui::ToggleButton> triangleButton;
    std::shared_ptr<gui::ToggleButton> circleButton;
    std::shared_ptr<gui::ToggleButton> liquidButton;
    std::shared_ptr<gui::Button> quitbutton;
    std::shared_ptr<gui::ToggleButton> deletebutton;
    std::shared_ptr<gui::Button> clearbutton;
    std::shared_ptr<gui::ToggleButton> pausebutton;

    float zoomLevel = 1.0f;
    sf::View mainView;
    std::shared_ptr<gui::Button> zoomInButton;
    std::shared_ptr<gui::Button> zoomOutButton;

    void setupGUI();
    void setShapeType(ShapeType type);
    void update();
    void draw();
    std::string getShapeTypeName(ShapeType type);

 
    void zoomIn();
    void zoomOut();
    void updateZoom();

    float defaultDensity = 7050.0f;  
    float liquidDensity = 0.8f;      
    float gravityX = 0.0f;            
    float gravityY = 1000.0f;        
    float liquidLifetime = 5.0f;     
    float liquidFadeFactor = 0.8f;    

    std::shared_ptr<gui::Slider> densitySlider;
    std::shared_ptr<gui::Slider> liquidDensitySlider;
    std::shared_ptr<gui::Slider> gravityXSlider;
    std::shared_ptr<gui::Slider> gravityYSlider;
    std::shared_ptr<gui::Slider> lifetimeSlider;
    std::shared_ptr<gui::Slider> fadeFactorSlider;

    sf::RectangleShape propertiesPanel;
    bool showProperties = false;
    std::shared_ptr<gui::ToggleButton> propertiesButton;
};
