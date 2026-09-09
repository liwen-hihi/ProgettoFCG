#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"
#include <SFML/Window.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <string>
#include <iostream>
#include <math.h>

#include <SFML/Graphics.hpp>

#include <glm/glm.hpp>
/////////////////////////////
// Window and OpenGL setup //
/////////////////////////////

class Setup
{
public:
    static const int window_width = 800;
    static const int window_height = 600;

    sf::RenderWindow window;

    Setup ()
    {
        sf::ContextSettings settings;
        settings.depthBits = 24;
        settings.stencilBits = 8;
        settings.antiAliasingLevel = 4;
        settings.majorVersion = 4;
        settings.minorVersion = 1;

        window.create(sf::VideoMode({window_width, window_height}), "SFML + OpenGL", sf::Style::Default, sf::State::Windowed, settings);
        window.setVerticalSyncEnabled (true);

        if (!window.setActive (true)) {
            std::cerr << "Failure: error during SFML OpenGL Activation." << std::endl;
            exit (1);
        }
    }
};

////////////////////
// Camera + World //
////////////////////

class Menu
{
public:
    sf::RenderWindow& w;

private:
    sf::Font font{"data/GeorgiaLike-Regular.ttf"};
    unsigned int lv;
    float width, height;
    sf::RectangleShape sfondo;
    std::vector <sf::RectangleShape> buttons;

public:
    Menu (Setup& s) : w (s.window)
    {
        lv = 3;
        width = w.getSize().x /2.f;
        height = w.getSize().y /2.f;
    }

    void draw()
    {
        draw_sfondo();
        draw_button();
        draw_text();
    }
private:
    void draw_sfondo()
    {
        sfondo.setSize({width, height});
        sfondo.setPosition({
            (w.getSize().x - width) / 2.f,
            (w.getSize().y - height) / 2.f
        });
        sfondo.setFillColor(sf::Color(128, 128, 128, 255)); // riempimento grigio
        sfondo.setOutlineColor(sf::Color(139, 69, 19)); // bordo marrone
        sfondo.setOutlineThickness(5.f); // spessore bordo
        w.draw(sfondo);
    }

    void draw_text()
    {
        std::vector<std::string> livelli = {"scegli livello"};
        for(unsigned i=0; i<=lv; i++)
        {
            if(i!=0)
            {
                std::string temp = std::to_string(i);
                livelli.push_back(temp);
            }
            sf::Text text(font, livelli[i], 30);
            text.setFillColor(sf::Color::Blue);
            if(i==0)
                text_centre(text, sfondo.getPosition(), {width, height}, false);
            else
                text_centre(text, buttons[i-1].getPosition(), buttons[i-1].getSize(), true);
            w.draw(text);
        }
    }

    void draw_button()
    {
        float size = 50.f;
        float y = sfondo.getPosition().y + sfondo.getSize().y * 3.f / 5.f;
        float w_sfondo = sfondo.getSize().x;
        float distanza = (w_sfondo - lv * size) / (lv + 1);
        for(unsigned i=1; i<=lv; i++)
        {
            sf::RectangleShape button;
            set_button(button, i, size, distanza, y);
            w.draw(button);
            buttons.push_back(button);
        }
    }

    void set_button(sf::RectangleShape& button,int i, float size, float distanza, float y)
    {
        button.setSize({size, size});
        button.setFillColor(sf::Color::White); // riempimento bianco
        button.setOutlineColor(sf::Color::Yellow); // bordo giallo
        button.setOutlineThickness(3.f); // spessore bordo
        sf::FloatRect bounds = button.getLocalBounds();
        button.setOrigin({
            bounds.position.x,
            bounds.position.y + bounds.size.y / 2.f
        });
        float x = sfondo.getPosition().x + distanza + (i-1) * (size + distanza);
        button.setPosition({x, y});
    }

    // testo, posizione padre di riferimento, size padre, true->1/2 altezza o false->1/4 
    void text_centre(sf::Text& text, sf::Vector2f padre, sf::Vector2f size, bool mezzo) 
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
        float x = padre.x + size.x / 2.f;   // centro X
        float y;
        if(mezzo) 
            y = padre.y;   // centro Y
        else
            y = padre.y + size.y / 4.f;   // 1/4 dall'alto
            
        text.setPosition({x, y});
    }
};


////////////////////
// SFML Callbacks //
////////////////////

void handle (const sf::Event::KeyPressed& key)
{
    switch (key.scancode) {
    case sf::Keyboard::Scancode::Escape:
        exit (0); 
    default:
        return;
    }
}

int main ()
{
    Setup setup;
    sf::RenderWindow& window = setup.window;
    Menu menu (setup);

    bool running = true;
    while(running)
    {
        while (const std::optional event = window.pollEvent ())
        {
            if (event->is<sf::Event::Closed> ())
                running = false;
            if(const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ())
                handle (* key_pressed);
        }
        window.clear();
        menu.draw();
        window.display();
    }
    
}