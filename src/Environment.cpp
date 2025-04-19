#include "Environment.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include "CollisionHandler.hpp"

UserInput::UserInput(Environment& env) 
    : environment(env), dragging(false) {
}

void UserInput::handleInput(sf::Event event) {
    ShapeType currMode = environment.getCurrentMode();
    sf::RenderWindow& window = environment.getWindow();
    sf::Vector2f mousePosition(sf::Mouse::getPosition(window));
    
    if (currMode == ShapeType::LIQUID) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            environment.spawnLiquidObjects(mousePosition, 5);
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (currMode == ShapeType::CIRCLE) {
            if (tempVertices.empty()) {
                tempVertices.push_back(mousePosition);
                std::cout << "Circle center selected.\n";
            } 
            else {
                float radius = std::hypot(tempVertices[0].x - mousePosition.x, tempVertices[0].y - mousePosition.y);
                std::cout << "Circle radius calculated: " << radius << "\n";

                sf::Vector2f center = tempVertices[0];
                auto windowSize = window.getSize();

                if (center.x - radius < 0) radius = center.x;
                if (center.x + radius > windowSize.x) radius = windowSize.x - center.x;
                if (center.y - radius < 0) radius = center.y;
                if (center.y + radius > windowSize.y) radius = windowSize.y - center.y;

                RigidBody* circle = new RigidBody(center, radius, {0, 0}, {0, 0}, sf::Color::Blue);
                circle->addForce(new Gravity(0, 1000.0f));
                environment.addRigidBody(circle);
                std::cout << "Circle created.\n";

                tempVertices.clear();
            }
        }
        else {
            dragStart = mousePosition;
            dragging = true;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left && dragging) {
        dragEnd = mousePosition;
        dragging = false;

        if (std::abs(dragEnd.x - dragStart.x) > 10 && std::abs(dragEnd.y - dragStart.y) > 10) {
            if (currMode == ShapeType::RECTANGLE) {
                environment.createRectangle(dragStart, dragEnd);
            } 
            else if (currMode == ShapeType::TRIANGLE) {
                environment.createTriangle(dragStart, dragEnd);
            }
        }
    }

    if (dragging) {
        dragEnd = mousePosition;
    }
}

void UserInput::drawPreview(sf::RenderWindow& window) {
    if (dragging) {
        ShapeType currMode = environment.getCurrentMode();
        
        if (currMode == ShapeType::RECTANGLE) {
            sf::RectangleShape preview;
            preview.setPosition(dragStart);
            preview.setSize(sf::Vector2f(dragEnd.x - dragStart.x, dragEnd.y - dragStart.y));
            preview.setFillColor(sf::Color::Transparent);
            preview.setOutlineColor(sf::Color::White);
            preview.setOutlineThickness(1.0f);
            window.draw(preview);
        } 
        else if (currMode == ShapeType::TRIANGLE) {
            sf::ConvexShape preview;
            preview.setPointCount(3);
            preview.setPoint(0, sf::Vector2f((dragStart.x + dragEnd.x) / 2, dragStart.y));
            preview.setPoint(1, sf::Vector2f(dragStart.x, dragEnd.y));
            preview.setPoint(2, sf::Vector2f(dragEnd.x, dragEnd.y));
            preview.setFillColor(sf::Color::Transparent);
            preview.setOutlineColor(sf::Color::White);
            preview.setOutlineThickness(1.0f);
            window.draw(preview);
        }
    }

    if (!tempVertices.empty() && environment.getCurrentMode() == ShapeType::CIRCLE) {
        sf::CircleShape point(5);
        point.setFillColor(sf::Color::Yellow);
        point.setPosition(tempVertices[0] - sf::Vector2f(5, 5));
        window.draw(point);
    }
}

Environment::Environment(int width, int height, const std::string& title)
    : window(sf::VideoMode(width, height), title), 
      currMode(ShapeType::RECTANGLE),
      userInput(*this) {
    
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cout << "No font could be loaded" << std::endl;
    }
    
    mainView = window.getDefaultView();
    
    setupGUI();
    setupPropertiesPanel();
}

void Environment::updateGravityOnObjects() {
    sf::Vector2f newGravity(gravityX, gravityY);
    
    for (auto* obj : rigidobjs) {
        obj->removeForcesByType(ForceType::GRAVITY);
        obj->addForce(new Gravity(newGravity.x, newGravity.y));
    }
    
    for (auto* particle : liquidobjs) {
        particle->removeForcesByType(ForceType::GRAVITY);
        particle->addForce(new Gravity(newGravity.x, newGravity.y));
    }
}

void Environment::setupPropertiesPanel() {
    float panelWidth = 250.0f;
    float panelHeight = window.getSize().y;
    float panelX = window.getSize().x - panelWidth;
    
    propertiesPanel.setSize(sf::Vector2f(panelWidth, panelHeight));
    propertiesPanel.setPosition(panelX, 0);
    propertiesPanel.setFillColor(sf::Color(30, 30, 30, 200));
    
    float sliderY = 70.0f;
    float sliderSpacing = 60.0f;
    float sliderWidth = panelWidth - 20.0f;
    float sliderHeight = 20.0f;
    float sliderX = panelX + 10.0f;
    
    densitySlider = gui.addWidget<gui::Slider>(
        sf::Vector2f(sliderX, sliderY),
        sf::Vector2f(sliderWidth, sliderHeight),
        1000.0f, 15000.0f, defaultDensity,
        "Rigid Body Density", font, 12
    );
    
    densitySlider->setCallback([this](float value) {
        defaultDensity = value;
    });
    
    liquidDensitySlider = gui.addWidget<gui::Slider>(
        sf::Vector2f(sliderX, sliderY + sliderSpacing),
        sf::Vector2f(sliderWidth, sliderHeight),
        0.1f, 2.0f, liquidDensity,
        "Liquid Density", font, 12
    );
    
    liquidDensitySlider->setCallback([this](float value) {
        liquidDensity = value;
    });
    
    gravityXSlider = gui.addWidget<gui::Slider>(
        sf::Vector2f(sliderX, sliderY + sliderSpacing * 2),
        sf::Vector2f(sliderWidth, sliderHeight),
        -2000.0f, 2000.0f, gravityX,
        "Gravity X", font, 12
    );
    
    gravityXSlider->setCallback([this](float value) {
        gravityX = value;
        updateGravityOnObjects();
    });
    
    gravityYSlider = gui.addWidget<gui::Slider>(
        sf::Vector2f(sliderX, sliderY + sliderSpacing * 3),
        sf::Vector2f(sliderWidth, sliderHeight),
        -2000.0f, 2000.0f, gravityY,
        "Gravity Y", font, 12
    );
    
    gravityYSlider->setCallback([this](float value) {
        gravityY = value;
        updateGravityOnObjects();
    });
    
    lifetimeSlider = gui.addWidget<gui::Slider>(
        sf::Vector2f(sliderX, sliderY + sliderSpacing * 4),
        sf::Vector2f(sliderWidth, sliderHeight),
        1.0f, 10.0f, liquidLifetime,
        "Liquid Lifetime", font, 12
    );
    
    lifetimeSlider->setCallback([this](float value) {
        liquidLifetime = value;
    });
    
    fadeFactorSlider = gui.addWidget<gui::Slider>(
        sf::Vector2f(sliderX, sliderY + sliderSpacing * 5),
        sf::Vector2f(sliderWidth, sliderHeight),
        0.1f, 1.0f, liquidFadeFactor,
        "Liquid Fade Factor", font, 12
    );
    
    fadeFactorSlider->setCallback([this](float value) {
        liquidFadeFactor = value;
    });
}

void Environment::setupGUI() {
    const float buttonWidth = 120.0f;
    const float buttonHeight = 30.0f;
    const float buttonSpacing = 10.0f;
    const float startX = 10.0f;
    const float startY = 10.0f;
    
    rectButton = gui.addWidget<gui::ToggleButton>(
        sf::Vector2f(startX, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Rectangle selected", "Rectangle",
        font, 14, 
        sf::Color(100, 100, 100), 
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(200, 80, 80)
    );
    
    triangleButton = gui.addWidget<gui::ToggleButton>(
        sf::Vector2f(startX + buttonWidth + buttonSpacing, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Triangle selected", "Triangle",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 200, 80)
    );
    
    circleButton = gui.addWidget<gui::ToggleButton>(
        sf::Vector2f(startX + (buttonWidth + buttonSpacing) * 2, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Circle selected", "Circle",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 80, 200)
    );
    
    liquidButton = gui.addWidget<gui::ToggleButton>(
        sf::Vector2f(startX + (buttonWidth + buttonSpacing) * 3, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Liquid selected", "Liquid",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 180, 200)
    );
    
    quitbutton = gui.addWidget<gui::Button>(
        sf::Vector2f(window.getSize().x - buttonWidth - 10, window.getSize().y - buttonHeight - 10),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Quit",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(200, 80, 80)
    );

    deletebutton = gui.addWidget<gui::ToggleButton>(
        sf::Vector2f(startX + (buttonWidth + buttonSpacing) * 4, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Delete selected", "Delete",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 200, 80)
    );

    clearbutton = gui.addWidget<gui::Button>(
        sf::Vector2f(startX + (buttonWidth + buttonSpacing) * 5, startY),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Clear",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 80, 200)
    );
    
    pausebutton = gui.addWidget<gui::ToggleButton>(
        sf::Vector2f(window.getSize().x - (buttonWidth * 2) - buttonSpacing - 10, window.getSize().y - buttonHeight - 10),
        sf::Vector2f(buttonWidth, buttonHeight),
        "Pause selected", "Pause",
        font, 14,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 180, 200)
    );
    
   zoomInButton = gui.addWidget<gui::Button>(
        sf::Vector2f(startX, window.getSize().y - buttonHeight - 10),
        sf::Vector2f(buttonWidth/2, buttonHeight),
        "+",
        font, 18,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(80, 200, 80)
    );
    
    zoomOutButton = gui.addWidget<gui::Button>(
        sf::Vector2f(startX + buttonWidth/2, window.getSize().y - buttonHeight - 10),
        sf::Vector2f(buttonWidth/2, buttonHeight),
        "-",
        font, 18,
        sf::Color(100, 100, 100),
        sf::Color(150, 150, 150),
        sf::Color(20, 20, 20),
        sf::Color(200, 80, 80)
    );

    rectButton->setToggleCallback([this](bool toggled) {
        if (toggled) {
            setShapeType(ShapeType::RECTANGLE);
        }
    });
    
    triangleButton->setToggleCallback([this](bool toggled) {
        if (toggled) {
            setShapeType(ShapeType::TRIANGLE);
        }
    });
    
    circleButton->setToggleCallback([this](bool toggled) {
        if (toggled) {
            setShapeType(ShapeType::CIRCLE);
        }
    });
    
    liquidButton->setToggleCallback([this](bool toggled) {
        if (toggled) {
            setShapeType(ShapeType::LIQUID);
        }
    });

    quitbutton->setCallback([this]() {
        window.close();
    });

    deletebutton->setToggleCallback([this](bool toggled) {
        if (toggled) {
            if (!rigidobjs.empty()) {
                delete rigidobjs.back();
                rigidobjs.pop_back();
                std::cout << "Last object deleted.\n";
            }
        }
    });

    clearbutton->setCallback([this]() {
        for (auto* obj : rigidobjs) {
            delete obj;
        }
        rigidobjs.clear();
    });

    pausebutton->setToggleCallback([this](bool toggled) {
        if (toggled) {
            isPaused = true;
        }
        else {
            isPaused = false;
        }
    });
    
    zoomInButton->setCallback([this]() {
        zoomIn();
    });
    
    zoomOutButton->setCallback([this]() {
        zoomOut();
    });

    setShapeType(currMode);
}

void Environment::zoomIn() {
    if (zoomLevel < 3.0f) {
        zoomLevel *= 1.2f;
        updateZoom();
    }
}

void Environment::zoomOut() {
    if (zoomLevel > 1.0f) {
        zoomLevel /= 1.2f;
        updateZoom();
    }
}

void Environment::updateZoom() {
    sf::Vector2f center = mainView.getCenter();
    
    mainView = window.getDefaultView();
    
    mainView.zoom(1.0f / zoomLevel);
    
    mainView.setCenter(center);
    
    window.setView(mainView);
    
    std::cout << "Zoom level: " << zoomLevel << std::endl;
}

void Environment::setShapeType(ShapeType type) {
    currMode = type;
    
    rectButton->setToggled(type == ShapeType::RECTANGLE);
    triangleButton->setToggled(type == ShapeType::TRIANGLE);
    circleButton->setToggled(type == ShapeType::CIRCLE);
    liquidButton->setToggled(type == ShapeType::LIQUID);
    
    std::cout << "Switched to " << getShapeTypeName(currMode) << " mode" << std::endl;
}

void Environment::run() {
    mainView = window.getDefaultView();
    
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
                
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    zoomIn();
                } else if (event.mouseWheelScroll.delta < 0) {
                    zoomOut();
                }
            }

            gui.handleEvent(event);
            
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T) {
                if (currMode == ShapeType::RECTANGLE)
                    setShapeType(ShapeType::TRIANGLE);
                else if (currMode == ShapeType::TRIANGLE)
                    setShapeType(ShapeType::CIRCLE);
                else if (currMode == ShapeType::CIRCLE)
                    setShapeType(ShapeType::LIQUID);
                else
                    setShapeType(ShapeType::RECTANGLE);
            }
            
            userInput.handleInput(event);
        }

       
        if (!isPaused) {
            update();
        } else {
            clock.restart();
        }

        window.clear();
        draw();
        window.display();
    }
}

void Environment::update() {
    float dt = clock.restart().asSeconds();

    gui.update();

    for (auto* obj : rigidobjs) {
        obj->applyForces(dt);
    }
    
    for (size_t i = 0; i < rigidobjs.size(); i++) {
        for (size_t j = i + 1; j < rigidobjs.size(); j++) {
            auto* bodyA = rigidobjs[i];
            auto* bodyB = rigidobjs[j];
            
            CollisionHandler::CollisionInfo info = CollisionHandler::detectCollision(bodyA, bodyB);
            
            if (info.hasCollision) {
                CollisionHandler::resolveCollision(bodyA, bodyB, info);
                std::cout << "Collision detected between objects " << i << " and " << j << std::endl;
            }
        }
    }
    
    for (auto* obj : rigidobjs) {
        obj->update(dt, window.getSize().x, window.getSize().y);
    }
    
    for (auto it = liquidobjs.begin(); it != liquidobjs.end();) {
        (*it)->update(dt, window.getSize().x, window.getSize().y);
        
        if ((*it)->isDead()) {
            delete *it;
            it = liquidobjs.erase(it);
        } else {
            ++it;
        }
    }
    
    for (auto* particle : liquidobjs) {
        for (auto* obj : rigidobjs) {
            CollisionHandler::CollisionInfo info = CollisionHandler::detectCollision(particle, obj);
            
            if (info.hasCollision) {
                CollisionHandler::resolveCollision(particle, obj, info);
            }
        }
    }
}

void Environment::draw() {
    sf::View currentView = window.getView();
    
    window.setView(mainView);
    
    for (auto& body : rigidobjs) {
        body->draw(window);
    }
    
    for (auto& particle : liquidobjs) {
        particle->draw(window);
    }

    userInput.drawPreview(window);
    
    window.setView(window.getDefaultView());
    
    if (showProperties) {
        window.draw(propertiesPanel);
    }
    
    gui.draw(window);
    
    window.setView(currentView);
}

void Environment::spawnLiquidObjects(const sf::Vector2f& position, int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> radiusDist(2.0f, 4.0f);
    static std::uniform_real_distribution<float> velDist(-30.0f, 30.0f);
    static std::uniform_real_distribution<float> lifetimeDist(3.0f, 7.0f);
    
    for (int i = 0; i < count; i++) {
        float radius = radiusDist(gen);
        float lifetime = lifetimeDist(gen);
        
        sf::Vector2f velocity(velDist(gen), velDist(gen) + 30.0f);
        
        sf::Color waterColor(40, 130, 255, 180); 
        
        LiquidParticle* particle = new LiquidParticle(
            position,
            radius,
            velocity,
            {0, 0},
            waterColor,
            0.8f,
            lifetime
        );
        
        particle->addForce(new Gravity(0, 1000.0f));
        liquidobjs.push_back(particle);
    }
}

void Environment::createRectangle(const sf::Vector2f& start, const sf::Vector2f& end) {
    std::vector<sf::Vector2f> vertices;
    
    vertices.push_back(start);
    vertices.push_back({end.x, start.y});
    vertices.push_back(end);
    vertices.push_back({start.x, end.y});

    rigidobjs.push_back(new RigidBody(vertices, {0, 0}, {0, 0}, sf::Color::Red));
    rigidobjs.back()->addForce(new Gravity(0, 1000.0f));
    std::cout << "Rectangle created.\n";
}

void Environment::createTriangle(const sf::Vector2f& start, const sf::Vector2f& end) {
    std::vector<sf::Vector2f> vertices;
    vertices.push_back({(start.x + end.x) / 2, start.y});
    vertices.push_back({start.x, end.y});
    vertices.push_back({end.x, end.y});

    rigidobjs.push_back(new RigidBody(vertices, {0, 0}, {0, 0}, sf::Color::Green));
    rigidobjs.back()->addForce(new Gravity(0, 1000.0f));
    std::cout << "Triangle created.\n";
}

std::string Environment::getShapeTypeName(ShapeType type) {
    switch(type) {
        case ShapeType::RECTANGLE: return "Rectangle";
        case ShapeType::TRIANGLE: return "Triangle";
        case ShapeType::CIRCLE: return "Circle";
        case ShapeType::LIQUID: return "Liquid";
        default: return "Unknown";
    }
}
