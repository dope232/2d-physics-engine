
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

namespace gui {

class Widget {
public:
    virtual ~Widget() = default;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual bool handleEvent(const sf::Event& event) = 0;
    virtual void update() = 0;
};

class Button : public Widget {
public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& text,
           const sf::Font& font, unsigned int characterSize = 14,
           sf::Color idleColor = sf::Color(100, 100, 100),
           sf::Color hoverColor = sf::Color(150, 150, 150),
           sf::Color activeColor = sf::Color(20, 20, 20),
           sf::Color textColor = sf::Color::White)
        : m_position(position), m_size(size), m_idleColor(idleColor),
          m_hoverColor(hoverColor), m_activeColor(activeColor), m_textColor(textColor),
          m_state(ButtonState::IDLE), m_callback(nullptr) {
        
        m_shape.setPosition(m_position);
        m_shape.setSize(m_size);
        m_shape.setFillColor(m_idleColor);
        
        m_text.setFont(font);
        m_text.setString(text);
        m_text.setCharacterSize(characterSize);
        m_text.setFillColor(textColor);
        

        sf::FloatRect textRect = m_text.getLocalBounds();
        m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_text.setPosition(m_position.x + m_size.x / 2.0f, m_position.y + m_size.y / 2.0f);
    }
    
    void setCallback(std::function<void()> callback) {
        m_callback = callback;
    }
    
    void draw(sf::RenderWindow& window) override {
        window.draw(m_shape);
        window.draw(m_text);
    }
    
    bool handleEvent(const sf::Event& event) override {
        if (event.type == sf::Event::MouseMoved) {

            if (isMouseOver(sf::Vector2f(event.mouseMove.x, event.mouseMove.y))) {
                m_state = ButtonState::HOVER;
            } else {
                m_state = ButtonState::IDLE;
            }
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left &&
                isMouseOver(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))) {
                m_state = ButtonState::ACTIVE;
            }
        } else if (event.type == sf::Event::MouseButtonReleased) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (m_state == ButtonState::ACTIVE) {

                    if (m_callback) {
                        m_callback();
                    }
                }

                if (isMouseOver(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))) {
                    m_state = ButtonState::HOVER;
                } else {
                    m_state = ButtonState::IDLE;
                }
            }
        }
        
        return false;
    }
    
    void update() override {
        switch (m_state) {
            case ButtonState::IDLE:
                m_shape.setFillColor(m_idleColor);
                break;
            case ButtonState::HOVER:
                m_shape.setFillColor(m_hoverColor);
                break;
            case ButtonState::ACTIVE:
                m_shape.setFillColor(m_activeColor);
                break;
        }
    }
    
protected:
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::RectangleShape m_shape;
    sf::Text m_text;

private:
    enum class ButtonState {
        IDLE,
        HOVER,
        ACTIVE
    };
    
    bool isMouseOver(const sf::Vector2f& mousePos) const {
        return mousePos.x >= m_position.x && mousePos.x <= m_position.x + m_size.x &&
               mousePos.y >= m_position.y && mousePos.y <= m_position.y + m_size.y;
    }
    
    sf::Color m_idleColor;
    sf::Color m_hoverColor;
    sf::Color m_activeColor;
    sf::Color m_textColor;
    
    ButtonState m_state;
    std::function<void()> m_callback;
};

class ToggleButton : public Button {
public:
    ToggleButton(const sf::Vector2f& position, const sf::Vector2f& size, 
               const std::string& textOn, const std::string& textOff,
               const sf::Font& font, unsigned int characterSize = 14,
               sf::Color idleColor = sf::Color(100, 100, 100),
               sf::Color hoverColor = sf::Color(150, 150, 150),
               sf::Color activeColor = sf::Color(20, 20, 20),
               sf::Color toggledColor = sf::Color(0, 150, 0),
               sf::Color textColor = sf::Color::White)
        : Button(position, size, textOff, font, characterSize, idleColor, hoverColor, activeColor, textColor),
          m_toggled(false), m_textOn(textOn), m_textOff(textOff), 
          m_toggledColor(toggledColor), m_font(font), m_characterSize(characterSize) {
        
        m_toggleCallback = nullptr;
    }
    
    void setToggleCallback(std::function<void(bool)> callback) {
        m_toggleCallback = callback;
        

        Button::setCallback([this]() {
            m_toggled = !m_toggled;
            if (m_toggleCallback) {
                m_toggleCallback(m_toggled);
            }
            updateText();
        });
    }
    
    void setToggled(bool toggled) {
        m_toggled = toggled;
        updateText();
    }
    
    bool isToggled() const {
        return m_toggled;
    }
    
    void draw(sf::RenderWindow& window) override {
        Button::draw(window);
    }
    
    void update() override {
        Button::update();
        

        if (m_toggled) {
            m_shape.setFillColor(m_toggledColor);
        }
    }
    
private:
    void updateText() {
        m_text.setString(m_toggled ? m_textOn : m_textOff);

        sf::FloatRect textRect = m_text.getLocalBounds();
        m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_text.setPosition(m_position.x + m_size.x / 2.0f, m_position.y + m_size.y / 2.0f);
    }
    
    bool m_toggled;
    std::string m_textOn;
    std::string m_textOff;
    sf::Color m_toggledColor;
    std::function<void(bool)> m_toggleCallback;
    

    using Button::m_text;
    using Button::m_position;
    using Button::m_shape;
    
    sf::Font m_font;
    unsigned int m_characterSize;
};


class Slider : public Widget {
    public:
        Slider(const sf::Vector2f& position, const sf::Vector2f& size, 
               float minValue, float maxValue, float currentValue,
               const std::string& label, const sf::Font& font, 
               unsigned int characterSize = 12,
               sf::Color backgroundColor = sf::Color(50, 50, 50),
               sf::Color sliderColor = sf::Color(100, 100, 100),
               sf::Color handleColor = sf::Color(200, 200, 200),
               sf::Color textColor = sf::Color::White)
            : m_position(position), m_size(size), 
              m_minValue(minValue), m_maxValue(maxValue), m_currentValue(currentValue),
              m_draggingHandle(false), m_label(label) {
            

            m_background.setPosition(position);
            m_background.setSize(size);
            m_background.setFillColor(backgroundColor);
            

            m_slider.setPosition(position);
            m_slider.setSize(sf::Vector2f(size.x, size.y * 0.5f));
            m_slider.setFillColor(sliderColor);
            m_slider.setPosition(position.x, position.y + size.y * 0.25f);
  
            float handleWidth = size.y * 0.75f;
            m_handle.setSize(sf::Vector2f(handleWidth, size.y * 0.75f));
            m_handle.setFillColor(handleColor);
            updateHandlePosition();
 
            m_text.setFont(font);
            m_text.setString(label + ": " + std::to_string(m_currentValue));
            m_text.setCharacterSize(characterSize);
            m_text.setFillColor(textColor);
            m_text.setPosition(position.x, position.y - characterSize - 5);
        }
        
        void setValue(float value) {
            m_currentValue = std::max(m_minValue, std::min(m_maxValue, value));
            updateHandlePosition();
            updateText();
            
            if (m_callback) {
                m_callback(m_currentValue);
            }
        }
        
        float getValue() const {
            return m_currentValue;
        }
        
        void setCallback(std::function<void(float)> callback) {
            m_callback = callback;
        }
        
        void draw(sf::RenderWindow& window) override {
            window.draw(m_background);
            window.draw(m_slider);
            window.draw(m_handle);
            window.draw(m_text);
        }
        
        bool handleEvent(const sf::Event& event) override {
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                if (isHandleHovered(mousePos)) {
                    m_draggingHandle = true;
                    return true;
                }
                else if (isSliderHovered(mousePos)) {
   
                    float ratio = (mousePos.x - m_position.x) / m_size.x;
                    setValue(m_minValue + ratio * (m_maxValue - m_minValue));
                    m_draggingHandle = true;
                    return true;
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                m_draggingHandle = false;
            }
            else if (event.type == sf::Event::MouseMoved && m_draggingHandle) {
                float ratio = (event.mouseMove.x - m_position.x) / m_size.x;
                ratio = std::max(0.0f, std::min(1.0f, ratio));
                setValue(m_minValue + ratio * (m_maxValue - m_minValue));
                return true;
            }
            
            return false;
        }
        
        void update() override {
       
        }
        
    private:
        void updateHandlePosition() {
            float ratio = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
            float xPos = m_position.x + ratio * m_size.x - m_handle.getSize().x / 2;
            xPos = std::max(m_position.x, std::min(m_position.x + m_size.x - m_handle.getSize().x, xPos));
            m_handle.setPosition(xPos, m_position.y + m_size.y / 2 - m_handle.getSize().y / 2);
        }
        
        void updateText() {
            
            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << m_currentValue;
            m_text.setString(m_label + ": " + stream.str());
        }
        
        bool isHandleHovered(const sf::Vector2f& mousePos) const {
            sf::FloatRect handleBounds = m_handle.getGlobalBounds();
            return handleBounds.contains(mousePos);
        }
        
        bool isSliderHovered(const sf::Vector2f& mousePos) const {
            sf::FloatRect sliderBounds = m_slider.getGlobalBounds();
            return sliderBounds.contains(mousePos);
        }
        
        sf::Vector2f m_position;
        sf::Vector2f m_size;
        sf::RectangleShape m_background;
        sf::RectangleShape m_slider;
        sf::RectangleShape m_handle;
        sf::Text m_text;
        
        float m_minValue;
        float m_maxValue;
        float m_currentValue;
        
        bool m_draggingHandle;
        std::string m_label;
        std::function<void(float)> m_callback;
    };

class GUI {
public:
    template<typename T, typename... Args>
    std::shared_ptr<T> addWidget(Args&&... args) {
        static_assert(std::is_base_of<Widget, T>::value, "T must derive from Widget");
        auto widget = std::make_shared<T>(std::forward<Args>(args)...);
        m_widgets.push_back(widget);
        return widget;
    }
    
    void handleEvent(const sf::Event& event) {
        for (auto& widget : m_widgets) {
            widget->handleEvent(event);
        }
    }
    
    void update() {
        for (auto& widget : m_widgets) {
            widget->update();
        }
    }
    
    void draw(sf::RenderWindow& window) {
        for (auto& widget : m_widgets) {
            widget->draw(window);
        }
    }
    
private:
    std::vector<std::shared_ptr<Widget>> m_widgets;
};

} 

