#define GLAD_GL_IMPLEMENTATION
#include "../glad/gl.h"
#include <SFML/Window.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/component_wise.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <math.h>

#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include "../include/matrices.hh"
#include "../include/mesh.hh"
#include "../include/hotshaders.hh"
#include "../include/rawmouse.hh"

bool parte3D = false;

struct Box
{
    glm::vec3 min;
    glm::vec3 max;

    bool contains (const glm::vec3& p) const
    {
        return p.x >= min.x && p.x <= max.x
            && p.y >= min.y && p.y <= max.y
            && p.z >= min.z && p.z <= max.z;
    }
};

Box expand_box(const Box& box, float margin)
{
    glm::vec3 offset(margin, margin, margin);

    return {
        box.min - offset,
        box.max + offset
    };
}

bool ray_triangle_intersection(
    const glm::vec3& ray_origin,
    const glm::vec3& ray_direction,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2)
{
    const float EPSILON = 0.000001f;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec3 h = glm::cross(ray_direction, edge2);
    float a = glm::dot(edge1, h);

    // Il raggio è parallelo al triangolo
    if (std::abs(a) < EPSILON)
        return false;

    float f = 1.0f / a;

    glm::vec3 s = ray_origin - v0;

    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f)
        return false;

    glm::vec3 q = glm::cross(s, edge1);

    float v = f * glm::dot(ray_direction, q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    float t = f * glm::dot(edge2, q);

    // Intersezione davanti alla telecamera
    return t > EPSILON;
}

/////////////////////////////
// Window and OpenGL setup //
/////////////////////////////

class Setup
{
public:
    static const int window_width = 800;
    static const int window_height = 600;
    static const int min_window_width = 400;
    static const int min_window_height = 300;

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

        gladLoadGL(reinterpret_cast<GLADloadfunc>(sf::Context::getFunction));
    }
};

////////////////////
// Camera + World //
////////////////////

class Lights
{
public:
    glm::vec3 light_direct_pos_relative = {2.0, 2.0, 0.0}; // xyz (relative to camera position)
    glm::vec3 light_direct_pos = {0.0, 0.0, 0.0};   // xyz (absolute, in world coordinates)
    glm::vec3 light_direct_val = {1.0, 1.0, 1.0};   // rgb
    glm::vec3 light_ambient_val = {0.1, 0.1, 0.1};  // rgb
    glm::vec3 material_diffuse = {0.8, 0.7, 0.6};   // rgb
    glm::vec3 material_ambient = {0.5, 0.5, 0.8};   // rgb
    glm::vec3 material_specular = {1.0, 1.0, 1.0};  // rgb
    float material_shininess = 1000.0; // scalar

    bool bianco = true;

private:
    
    GLint light_direct_pos_loc_bianco;   // xyz
    GLint light_direct_val_loc_bianco;   // rgb
    GLint light_ambient_val_loc_bianco;  // rgb
    GLint material_diffuse_loc_bianco;   // rgb
    GLint material_ambient_loc_bianco;   // rgb
    GLint material_specular_loc_bianco;  // rgb
    GLint material_shininess_loc_bianco; // scalar

    GLint light_direct_pos_loc_colorato;   // xyz
    GLint light_direct_val_loc_colorato;   // rgb
    GLint light_ambient_val_loc_colorato;  // rgb
    GLint material_diffuse_loc_colorato;   // rgb
    GLint material_ambient_loc_colorato;   // rgb
    GLint material_specular_loc_colorato;  // rgb
    GLint material_shininess_loc_colorato; // scalar

public:
    Lights (fcg::Shaders& shaders__bianco, fcg::Shaders& shaders__colorato)
    {
        locations (shaders__bianco, shaders__colorato);
    }

    void locations (fcg::Shaders& shaders__bianco, fcg::Shaders& shaders__colorato)
    {
        light_direct_pos_loc_bianco = glGetUniformLocation (shaders__bianco.program, "light.direct_pos");
        light_direct_val_loc_bianco = glGetUniformLocation (shaders__bianco.program, "light.direct_val");
        light_ambient_val_loc_bianco = glGetUniformLocation (shaders__bianco.program, "light.ambient_val");
        material_diffuse_loc_bianco = glGetUniformLocation (shaders__bianco.program, "material.diffuse");
        material_ambient_loc_bianco = glGetUniformLocation (shaders__bianco.program, "material.ambient");
        material_specular_loc_bianco = glGetUniformLocation (shaders__bianco.program, "material.specular");
        material_shininess_loc_bianco = glGetUniformLocation (shaders__bianco.program, "material.shininess");

        light_direct_pos_loc_colorato = glGetUniformLocation(shaders__colorato.program, "light.direct_pos");
        light_direct_val_loc_colorato = glGetUniformLocation(shaders__colorato.program, "light.direct_val");
        light_ambient_val_loc_colorato = glGetUniformLocation(shaders__colorato.program, "light.ambient_val");
        material_diffuse_loc_colorato = glGetUniformLocation(shaders__colorato.program, "material.diffuse");
        material_ambient_loc_colorato = glGetUniformLocation(shaders__colorato.program, "material.ambient");
        material_specular_loc_colorato = glGetUniformLocation(shaders__colorato.program, "material.specular");
        material_shininess_loc_colorato = glGetUniformLocation(shaders__colorato.program, "material.shininess");
    }

    void send_parameters_bianco ()
    {
        glUniform3fv (light_direct_val_loc_bianco, 1, &light_direct_val[0]);
        glUniform3fv (light_ambient_val_loc_bianco, 1, &light_ambient_val[0]);
        glUniform3fv (material_diffuse_loc_bianco, 1, &material_diffuse[0]);
        glUniform3fv (material_ambient_loc_bianco, 1, &material_ambient[0]);
        glUniform3fv (material_specular_loc_bianco, 1, &material_specular[0]);
        glUniform1fv (material_shininess_loc_bianco, 1, &material_shininess);
    }

    void send_parameters_colorato ()
    {
        glUniform3fv (light_direct_val_loc_colorato, 1, &light_direct_val[0]);
        glUniform3fv (light_ambient_val_loc_colorato, 1, &light_ambient_val[0]);
        glUniform3fv (material_diffuse_loc_colorato, 1, &material_diffuse[0]);
        glUniform3fv (material_ambient_loc_colorato, 1, &material_ambient[0]);
        glUniform3fv (material_specular_loc_colorato, 1, &material_specular[0]);
        glUniform1fv (material_shininess_loc_colorato, 1, &material_shininess);
    }

    void send_position_relative (const glm::mat4& inverse_view_matrix)
    {
        glm::vec4 p = glm::vec4 (light_direct_pos_relative, 1.0);
        p = inverse_view_matrix * p;
        light_direct_pos = {p.x, p.y, p.z};
        if(bianco)    send_position_bianco();
        else    send_position_colorato ();
    }

    void send_position_bianco ()
    {
        glUniform3fv (light_direct_pos_loc_bianco, 1, &light_direct_pos[0]);
    }

    void send_position_colorato ()
    {
        glUniform3fv (light_direct_pos_loc_colorato, 1, &light_direct_pos[0]);
    }
};

class Camera
{
public:
    glm::mat4 v;
    glm::mat4 inv_v;
    glm::mat4 vp;
    int asse = 0; // x=1, y=2, z=3

    bool bianco = true;

private:
    /** Intrinsic camera parameters **/
    const float normal_fd = 50.0 / 18.0;
    float fd; // focal distance
    float ar; // aspect ratio
    const glm::vec3 camera_default = {0.0, 0.0, 5.0};

    /** Extrinsic camera parameters **/
    // xyz, starting point of dynamic camera position
    glm::vec3 camera_pos = camera_default; // xyz
    GLint camera_pos_loc_bianco;
    GLint camera_pos_loc_colorato;
    // Angles defining the in-place camera rotation
    float phi_deg = 0.0;
    float theta_deg = 0.0;

    /** Camera movement **/
    bool pan_tilt_on = false;
    bool move_on = false;
    bool move_add = false;

    // Bounding boxes of all fixed objects in world coordinates.
    const std::vector<Box>* collision_boxes = nullptr;
    bool collision = false;

public:
    Camera (fcg::Shaders& shaders_bianco, fcg::Shaders& shaders_colorato)
    {
        locations (shaders_bianco, shaders_colorato);
        lens_normal ();
        set_window_size (Setup::window_width, Setup::window_height);
        view_projection ();
        if (bianco)     send_parameters_bianco();
        else    send_parameters_colorato();
    }

    void locations (fcg::Shaders& shaders_bianco, fcg::Shaders& shaders_colorato)
    {
        camera_pos_loc_bianco = glGetUniformLocation (shaders_bianco.program, "camera_pos");
        camera_pos_loc_colorato = glGetUniformLocation (shaders_colorato.program, "camera_pos");
    }

    GLint get_camera_pos_loc_bianco () { return camera_pos_loc_bianco; }
    GLint get_camera_pos_loc_colorato () { return camera_pos_loc_colorato; }
    glm::vec3 get_camera_pos () { return camera_pos; }
    bool is_moving() const { return pan_tilt_on || move_on; }

    void set_window_size(int w, int h)
    {
        ar = ((float) w) / (float) h;
        view_projection ();
        if (bianco)     send_parameters_bianco();
        else    send_parameters_colorato();
    }   

    bool pan_tilt_toggle ()
    {
        return pan_tilt_on = !pan_tilt_on;
    }

    void pan_tilt (float dx, float dy)
    {
        if (!pan_tilt_on)
            return;

        phi_deg += dx * 0.1;
        theta_deg += dy * 0.1;
        theta_deg = theta_deg > 90.0? 90.0 : theta_deg;
        theta_deg = theta_deg < -90.0? -90.0 : theta_deg;
        view_projection ();
        if (bianco)     send_parameters_bianco();
        else    send_parameters_colorato();
    }

    void set_collision_boxes (const std::vector<Box>& boxes)
    {
        collision_boxes = &boxes;
    }

    bool collision_happened () const
    {
        return collision;
    }

    void clear_collision_feedback ()
    {
        collision = false;
    }

    bool collides (const glm::vec3& position) const
    {
        if (collision_boxes == nullptr)
            return false;

        for (const Box& box : *collision_boxes)
        {
            if (box.contains(position))
                return true;
        }

        return false;
    }

    void move_start (bool add)
    {
        move_add = add;
        move_on = true;
    }

    void move_stop ()
    {
        move_on = false;
    }

    // move
    void move (float delta)
    {
        if (!move_on)   return;

        glm::vec3 candidate = camera_pos;
        if (asse == 1)
        {
            if (move_add)    candidate.x += delta;
            else            candidate.x -= delta;
        }
        if (asse == 2)
        {
            if (move_add)    candidate.y += delta;
            else            candidate.y -= delta;

        }
        if (asse == 3)
        {
            if (move_add)    candidate.z += delta;
            else            candidate.z -= delta;
        }

        // Calcola la posizione che raggiungeremmo durante questo frame. // Non entrare nel riquadro di delimitazione di un oggetto fisso.
        if (collides(candidate))
        {
            collision = true;
            return;
        }
        collision = false;
        camera_pos = candidate;
        view_projection ();
        if (bianco)     send_parameters_bianco();
        else    send_parameters_colorato();
    }

    void lens_normal ()
    {
        fd = normal_fd;
        view_projection ();
        if (bianco)     send_parameters_bianco();
        else    send_parameters_colorato();
    }

    void set_default ()
    {
        camera_pos = camera_default;
        phi_deg = 0.0f;
        theta_deg = 0.0f;

        pan_tilt_on = false;
        move_on = false;
        move_add = false;

        lens_normal();
        collision = false;
    }

    void view_projection ()
    {
        float ncp = 0.1f;
        float fcp = 100.0f;

        // prepare rotations and translation matrices
        glm::mat4 ry = fcg::rotation_y (phi_deg);
        glm::mat4 rx = fcg::rotation_x (theta_deg);
        glm::mat4 t = fcg::translation (-camera_pos.x, -camera_pos.y, -camera_pos.z);

        // prepare projection matrix
        float a = (fcp + ncp) / (ncp - fcp);       // coefficient 3rd col
        float b = 2.0 * fcp * ncp / (ncp - fcp);   // coefficient 4th col

        glm::mat4 pr = glm::mat4(
                                 fd,  0.0, 0.0,  0.0,    // 1st column
                                 0.0,  fd * ar, 0.0,  0.0,    // 2nd column
                                 0.0, 0.0,   a, -1.0,    // 3rd column
                                 0.0, 0.0,   b,  0.0     // 4th column
                                 );

        // Compute VP matrix and update it
        v = rx * ry * t;
        vp = pr * v;
        inv_v = glm::inverse (v);

    }

    void send_parameters_bianco ()
    {
        glUniform3fv(camera_pos_loc_bianco, 1, &camera_pos[0]);
    }

    void send_parameters_colorato ()
    {
        glUniform3fv(camera_pos_loc_colorato, 1, &camera_pos[0]);
    }
};

class Menu
{
public:
    sf::RenderWindow& window;
    sf::Vector2u size_window;
    sf::RectangleShape sfondo;
    int help = 0;
    bool success = false;
    sf::Clock success_clock;  //timer di 3 secondi

private:
    sf::Font font{"data/GeorgiaLike-Regular.ttf"};
    sf::Font font_c{"data/tuffy.ttf"};
    unsigned int lv;
    float width, height;
    std::vector <sf::RectangleShape> buttons;
    unsigned int lv_chose = 0;

public:
    Menu (Setup& s) : window (s.window)
    {
        lv = 3;
        size_window = window.getSize();
        width = size_window.x /2.f;
        height = size_window.y /2.f;
        reset_level ();
    }

    void reset_size(int w, int h)
    {
        reset_level ();
        if (w < Setup::min_window_width)    w = Setup::min_window_width;
        if (h < Setup::min_window_height)   h = Setup::min_window_height;

        size_window = {static_cast<unsigned>(w), static_cast<unsigned>(h)};
        width = w/2.f;
        height = h/2.f;
        sf::View view = window.getDefaultView();
        view.setSize({static_cast<float>(w), static_cast<float>(h)});
        view.setCenter({w / 2.f, h / 2.f});
        window.setView(view);
        if (window.getSize().x != static_cast<unsigned>(w) || window.getSize().y != static_cast<unsigned>(h))
        {
            window.setSize({static_cast<unsigned>(w), static_cast<unsigned>(h) });
        }
    }

    void reset_level()
    {
        lv_chose = 0;
    }

    unsigned get_level () {return lv_chose;}

    void draw()
    {
        draw_sfondo();
        draw_button();
        draw_text();
        draw_comment();
    }

    void draw_help()
    {
        if(help==0)
            return;
        else if(help==1){
            draw_sfondo();
            draw_h1();
        }
        else if(help==2){
            draw_sfondo();
            draw_h2();
        }
        else if(help==3){
            draw_sfondo();
            draw_h3();
        }
    }

    void draw_success()
    {
        draw_sfondo();
        draw_succ();
    }

    void which_level(sf::Vector2f mousePos)
    {
        reset_level ();
        for(unsigned i=0; i<buttons.size(); i++)
        {
            sf::FloatRect bounds = buttons[i].getGlobalBounds();
            if(bounds.contains(mousePos))
            {
                lv_chose = i+1;
                break;
            }
        }
    }

    void start_success()
    {
        success = true;
        success_clock.restart();
    }

private:
    void draw_sfondo()
    {
        sfondo.setSize({width, height});
        sfondo.setPosition({
            (size_window.x - width) / 2.f,
            (size_window.y - height) / 2.f
        });
        sfondo.setFillColor(sf::Color(128, 128, 128, 255)); // riempimento grigio
        sfondo.setOutlineColor(sf::Color(139, 69, 19)); // bordo marrone
        sfondo.setOutlineThickness(5.f); // spessore bordo
        window.draw(sfondo);
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
            if(i==0 || i!=lv_chose)
                text.setFillColor(sf::Color::Blue);
            else
                text.setFillColor(sf::Color::Red);
            if(i==0)
                text_centre(text, sfondo.getPosition(), {width, height}, 0);
            else
                text_centre(text, buttons[i-1].getPosition(), buttons[i-1].getSize(), 1);
            window.draw(text);
        }
    }

    void draw_button()
    {
        buttons.clear();
        float size = 50.f;
        float y = sfondo.getPosition().y + sfondo.getSize().y * 1.f / 2.f;
        float w_sfondo = sfondo.getSize().x;
        float distanza = (w_sfondo - lv * size) / (lv + 1);
        for(unsigned i=1; i<=lv; i++)
        {
            sf::RectangleShape button;
            set_button(button, i, size, distanza, y);
            window.draw(button);
            buttons.push_back(button);
        }
    }

    void draw_succ()
    {
        std::string s = "COMPLIMENTI!\nhai trovato il coniglio";
        sf::Text text(font, s, 30);
        text.setFillColor(sf::Color::Black);
        text_centre(text, sfondo.getPosition(), {width, height}, 3);
        window.draw(text);
    }

    void draw_comment()
    {
        std::string c = "Trova il coniglio e clicca,\npremere tasto H per aiuto";
        sf::Text text (font_c, c, 20);
        text.setFillColor(sf::Color::Black);
        text_centre(text, sfondo.getPosition(), {width, height}, 2);
        window.draw(text);
    }

    void draw_h1()
    {
        std::string c = "Per ruotre la visione,\nfai un clic sul sinistro del muose\nper smettere di ruotare\n fai secondo clic";
        sf::Text text (font_c, c, 20);
        text.setFillColor(sf::Color::Black);
        text_centre(text, sfondo.getPosition(), {width, height}, 3);
        window.draw(text);
    }

    void draw_h2()
    {
        std::string c = "Usare i tasti frecce,\nNumpadPlus e NumpadMinus\nper spostare la telecamera";
        sf::Text text (font_c, c, 20);
        text.setFillColor(sf::Color::Black);
        text_centre(text, sfondo.getPosition(), {width, height}, 3);
        window.draw(text);
    }
    
    void draw_h3()
    {
        std::string c = "fai un clic sul destro del muose\nsulla sfera per selezionarla\npoi usa i tasti W,S,A,D\nper spostare la sfera";
        sf::Text text (font_c, c, 20);
        text.setFillColor(sf::Color::Black);
        text_centre(text, sfondo.getPosition(), {width, height}, 3);
        window.draw(text);
    }

    void set_button(sf::RectangleShape& button, unsigned i, float size, float distanza, float y)
    {
        button.setSize({size, size});
        button.setFillColor(sf::Color::White); // riempimento bianco
        if(i!=lv_chose)
            button.setOutlineColor(sf::Color::Yellow); // bordo giallo
        else
            button.setOutlineColor(sf::Color::Red); // bordo rosso
        button.setOutlineThickness(3.f); // spessore bordo
        sf::FloatRect bounds = button.getLocalBounds();
        button.setOrigin({
            bounds.position.x,
            bounds.position.y + bounds.size.y / 2.f
        });
        float x = sfondo.getPosition().x + distanza + (i-1) * (size + distanza);
        button.setPosition({x, y});
    }

    // testo, posizione padre di riferimento, size padre, altezza: 0->1/4, 1->centro button, 2->3/4, 3->1/2
    void text_centre(sf::Text& text, sf::Vector2f padre, sf::Vector2f size, int altezza) 
    {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin({
            bounds.position.x + bounds.size.x / 2.f,
            bounds.position.y + bounds.size.y / 2.f
        });
        float x = padre.x + size.x / 2.f;   // centro X
        float y;
        if(altezza == 0) 
            y = padre.y + size.y / 4.f;   // 1/4 dall'alto
        else if (altezza == 1)
            y = padre.y;   // centro Y
        else if(altezza == 2) 
            y = padre.y + size.y*3.f / 4.f;   // 3/4 dall'alto
        else if(altezza == 3) 
            y = padre.y + size.y / 2.f;   // 1/4 dall'alto
            
        text.setPosition({x, y});
    }
};

class GPUMesh
{
public:
    glm::vec3 min_bounds;
    glm::vec3 max_bounds;
    glm::vec3 center;
    glm::vec3 extent;
    float span;
    glm::mat4 to_unit_extent; // normalization model matrix
    glm::vec3 unit_center;
    glm::vec3 unit_extent;
    float unit_span;

private:
    std::vector<float> points = {};
    std::vector<unsigned int> indices = {};

    GLuint vbo;
    GLuint ebo;
    GLuint vao;
    bool initialized = false;

public:
    GPUMesh (std::string filename){ load (filename); }

    ~GPUMesh () { clean (); }

    void load (std::string filename)
    {
        fcg::Mesh mesh (filename);
        mesh.pack4gpu (points, indices);
        send_arrays_2a3f ();

        min_bounds = mesh.min_bounds;
        max_bounds = mesh.max_bounds;
        center = (min_bounds + max_bounds) * 0.5f;
        span = glm::distance (max_bounds, min_bounds);
        extent = max_bounds - min_bounds;

        to_unit_extent =
            fcg::scaling (1.0 / glm::compMax (extent)) *
            fcg::translation (-center);

        unit_center = {0.0, 0.0, 0.0};
        unit_span = glm::distance (extent, {0.0, 0.0, 0.0});
        unit_extent = extent / glm::compMax (extent);

        initialized = true;
    }

    void clean ()
    {
        if (initialized) {
            glDeleteVertexArrays (1, &vao);
            glDeleteBuffers (1, &vbo);
            glDeleteBuffers(1, &ebo);
        }
    }

    Box world_bounds (const glm::mat4& model) const
    {
        // 8 angoli del bounding box locale
        glm::vec3 corners[8] = {
            {min_bounds.x, min_bounds.y, min_bounds.z},
            {min_bounds.x, min_bounds.y, max_bounds.z},
            {min_bounds.x, max_bounds.y, min_bounds.z},
            {min_bounds.x, max_bounds.y, max_bounds.z},
            {max_bounds.x, min_bounds.y, min_bounds.z},
            {max_bounds.x, min_bounds.y, max_bounds.z},
            {max_bounds.x, max_bounds.y, min_bounds.z},
            {max_bounds.x, max_bounds.y, max_bounds.z}
        };
        glm::vec3 world_min = glm::vec3(model * glm::vec4(corners[0], 1.0f));
        glm::vec3 world_max = world_min;

        for (int i = 1; i < 8; ++i)
        {
            glm::vec3 p = glm::vec3(model * glm::vec4(corners[i], 1.0f));
            world_min = glm::min(world_min, p);
            world_max = glm::max(world_max, p);
        }

        return {world_min, world_max};
    }

    void draw ()
    {
        glBindVertexArray (vao);
        glDrawElements(GL_TRIANGLES, indices.size (), GL_UNSIGNED_INT, 0);
    }

    std::vector<float> get_points() const { return points;}
    std::vector<unsigned int> get_indices() const { return indices;}

protected:
    void send_arrays_2a3f ()
    {
        // we want just one buffer, and we retrieve the name OpenGL assigns to it.
        glGenBuffers (1, &vbo);
        // bind it as the current VBO
        glBindBuffer (GL_ARRAY_BUFFER, vbo);
        // transfer data from CPU RAM to GPU RAM.
        glBufferData (GL_ARRAY_BUFFER, points.size () * sizeof (float), points.data (), GL_STATIC_DRAW);

        // we want just one buffer container, and we retrieve the name OpenGL assigns to it.
        glGenVertexArrays (1, &vao);
        // bind it as the current vao.
        glBindVertexArray (vao);

        // Attribute 0: position (x, y, z)
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray (0);

        // Attribute 1: 3 generic floats (u, v, w)
        glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray (1);

        glGenBuffers(1, &ebo); 
        // MUST be bound after the VAO's binding!
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size () * sizeof (unsigned int), indices.data (), GL_STATIC_DRAW);
    }
};

class Scene
{
public:
    int level;
    fcg::Shaders& shaders_bianco;
    fcg::Shaders& shaders_colorato;
    Camera camera;
    Lights lights;

    GPUMesh cube;
    GPUMesh bunny;
    GPUMesh sphere;

    bool bunny_hover = false;
    bool sphere_hover = false;
    bool sphere_move_on = false;
    int direzione = -1;

private:
    GLint model_loc_bianco;
    GLint vp_loc_bianco;
    GLint tr_inv_model_loc_bianco;

    GLint model_loc_colorato;
    GLint vp_loc_colorato;
    GLint tr_inv_model_loc_colorato;

    std::vector<Box> collision_boxes;
    glm::mat4 bunny_model, wall_model, sphere_model;

public:
    Scene (std::string dirname, fcg::Shaders& shaders_bianco, fcg::Shaders& shaders_colorato, int n) :
        shaders_bianco (shaders_bianco), 
        shaders_colorato (shaders_colorato), 
        camera (shaders_bianco, shaders_colorato), 
        lights (shaders_bianco, shaders_colorato),
        cube (dirname + "cube.off"),
        bunny (dirname + "bunny.off"),
        sphere (dirname + "sphere.off")
    {
        level = n;
        locations();
        update_all_bianco ();
        build_collision_boxes ();
        camera.set_collision_boxes (collision_boxes);
    }
    
    void locations()
    {
        camera.locations (shaders_bianco, shaders_colorato);
        lights.locations (shaders_bianco, shaders_colorato);
        model_loc_bianco = glGetUniformLocation(shaders_bianco.program, "model");
        vp_loc_bianco = glGetUniformLocation(shaders_bianco.program, "vp");
        tr_inv_model_loc_bianco = glGetUniformLocation(shaders_bianco.program, "tr_inv_model");
        model_loc_colorato = glGetUniformLocation(shaders_colorato.program, "model");
        vp_loc_colorato = glGetUniformLocation(shaders_colorato.program, "vp");
        tr_inv_model_loc_colorato = glGetUniformLocation(shaders_colorato.program, "tr_inv_model");
    }

    void update_all_bianco ()
    {
        shaders_bianco.use();
        camera.bianco = true;
        camera.view_projection ();
        lights.bianco = true;
        lights.send_parameters_bianco ();
        lights.send_position_relative (camera.inv_v);
    }

    void update_all_colorato ()
    {
        shaders_colorato.use();
        camera.bianco = false;
        camera.view_projection ();
        lights.bianco = false;
        lights.send_parameters_colorato ();
        lights.send_position_relative (camera.inv_v);
    }

    void update ()
    {
        if (camera.is_moving())
        {
            bunny_hover = false;
            sphere_hover = false;
            update_all_bianco();
            return;
        }
        if (bunny_hover || sphere_hover)    update_all_colorato();
        else    update_all_bianco();
    }

    void reload (int n)
    {
        level = n;
        build_collision_boxes ();
        camera.set_collision_boxes (collision_boxes); 
    }

    void draw ()
    {
        // Feedback di collisione: lampeggia lo sfondo di rosso per il fotogramma in cui
        if (camera.collision_happened ())
            glClearColor (1.0f, 0.0f, 0.0f, 1.0f);
        else
            glClearColor (0.0f, 0.0f, 0.0f, 1.0f);

        // clear the buffers
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // the view-projection matrix is the same for all the scene
        // vp = Projection(prospettiva della telecamera) × View(posizione della telecamera)

        if (bunny_hover)
        {
            shaders_colorato.use();
            glUniformMatrix4fv(vp_loc_colorato, 1, GL_FALSE, &camera.vp[0][0]);
            draw_bunny (bunny_model);
        }
        else{
            shaders_bianco.use();
            glUniformMatrix4fv(vp_loc_bianco, 1, GL_FALSE, &camera.vp[0][0]);
            draw_bunny (bunny_model);
        }
        if (level == 2){
            shaders_bianco.use();
            glUniformMatrix4fv(vp_loc_bianco, 1, GL_FALSE, &camera.vp[0][0]);
            draw_wall();
        }
        else if(level == 3)
        {
            if (sphere_hover)
            {
                shaders_colorato.use();
                glUniformMatrix4fv(vp_loc_colorato, 1, GL_FALSE, &camera.vp[0][0]);
                draw_sphere(sphere_model);
            }
            else{
                shaders_bianco.use();
                glUniformMatrix4fv(vp_loc_bianco, 1, GL_FALSE, &camera.vp[0][0]);
                draw_sphere(sphere_model);
            }
        }

        camera.clear_collision_feedback ();
    }

    void move_sphere (float delta)
    {
        if (!sphere_move_on || direzione == -1)
            return;
        else if (direzione == 1)
        {
            glm::mat4 translate = fcg::translation (0, delta, 0);
            sphere_model = translate * sphere_model;
        }
        else if (direzione == 2)
        {
            glm::mat4 translate = fcg::translation (0, -delta, 0);
            sphere_model = translate * sphere_model;
        }
        else if (direzione == 3)
        {
            glm::mat4 translate = fcg::translation (-delta, 0, 0);
            sphere_model = translate * sphere_model;
        }
        else if (direzione == 4)
        {
            glm::mat4 translate = fcg::translation (delta, 0, 0);
            sphere_model = translate * sphere_model;
        }
    }

    bool mouse_su_bunny (sf::Vector2i position, sf::Vector2u window_size)
    {
        float x =
            2.0f * static_cast<float>(position.x)
            / static_cast<float>(window_size.x) - 1.0f;

        float y =
            1.0f - 2.0f * static_cast<float>(position.y)
            / static_cast<float>(window_size.y);

        glm::vec4 near_point(x, y, -1.0f, 1.0f);
        glm::vec4 far_point (x, y,  1.0f, 1.0f);

        glm::mat4 inv_vp = glm::inverse(camera.vp);

        near_point = inv_vp * near_point;
        far_point  = inv_vp * far_point;

        near_point /= near_point.w;
        far_point  /= far_point.w;

        glm::vec3 ray_origin = glm::vec3(near_point);

        glm::vec3 ray_direction =
            glm::normalize(
                glm::vec3(far_point - near_point)
            );

        glm::mat4 inv_bunny = glm::inverse(bunny_model); 
        glm::vec3 local_origin = glm::vec3( inv_bunny * glm::vec4(ray_origin, 1.0f) ); 
        glm::vec3 local_direction = glm::normalize( glm::vec3( inv_bunny * glm::vec4(ray_direction, 0.0f) ) );

        const auto& points = bunny.get_points(); 
        const auto& indices = bunny.get_indices(); 
        for (size_t i = 0; i < indices.size(); i += 3) { 
            unsigned int i0 = indices[i]; 
            unsigned int i1 = indices[i + 1]; 
            unsigned int i2 = indices[i + 2];
            glm::vec3 v0( points[i0 * 6 + 0], points[i0 * 6 + 1], points[i0 * 6 + 2] ); 
            glm::vec3 v1( points[i1 * 6 + 0], points[i1 * 6 + 1], points[i1 * 6 + 2] ); 
            glm::vec3 v2( points[i2 * 6 + 0], points[i2 * 6 + 1], points[i2 * 6 + 2] );
            if (ray_triangle_intersection( local_origin, local_direction, v0, v1, v2)) 
                { return true; }
        }

        return false;
    }

    bool mouse_su_sphere (sf::Vector2i position, sf::Vector2u window_size)
    {
        float x =
            2.0f * static_cast<float>(position.x)
            / static_cast<float>(window_size.x) - 1.0f;

        float y =
            1.0f - 2.0f * static_cast<float>(position.y)
            / static_cast<float>(window_size.y);

        glm::vec4 near_point(x, y, -1.0f, 1.0f);
        glm::vec4 far_point (x, y,  1.0f, 1.0f);

        glm::mat4 inv_vp = glm::inverse(camera.vp);

        near_point = inv_vp * near_point;
        far_point  = inv_vp * far_point;

        near_point /= near_point.w;
        far_point  /= far_point.w;

        glm::vec3 ray_origin = glm::vec3(near_point);

        glm::vec3 ray_direction =
            glm::normalize(
                glm::vec3(far_point - near_point)
            );

        glm::mat4 inv_sphere = glm::inverse(sphere_model); 
        glm::vec3 local_origin = glm::vec3( inv_sphere * glm::vec4(ray_origin, 1.0f) ); 
        glm::vec3 local_direction = glm::normalize( glm::vec3( inv_sphere * glm::vec4(ray_direction, 0.0f) ) );

        const auto& points = sphere.get_points(); 
        const auto& indices = sphere.get_indices(); 
        for (size_t i = 0; i < indices.size(); i += 3) { 
            unsigned int i0 = indices[i]; 
            unsigned int i1 = indices[i + 1]; 
            unsigned int i2 = indices[i + 2];
            glm::vec3 v0( points[i0 * 6 + 0], points[i0 * 6 + 1], points[i0 * 6 + 2] ); 
            glm::vec3 v1( points[i1 * 6 + 0], points[i1 * 6 + 1], points[i1 * 6 + 2] ); 
            glm::vec3 v2( points[i2 * 6 + 0], points[i2 * 6 + 1], points[i2 * 6 + 2] );
            if (ray_triangle_intersection( local_origin, local_direction, v0, v1, v2)) 
                { return true; }
        }

        return false;
    }

private: 
    void draw_bunny (glm::mat4 bunny_trasforme)
    {
        glm::mat4 mm = bunny_trasforme;
        glm::mat3 ti_mm = glm::transpose (glm::inverse (glm::mat3 (mm)));
        if (!bunny_hover){
            glUniformMatrix4fv(model_loc_bianco, 1, GL_FALSE, &mm[0][0]);
            glUniformMatrix3fv (tr_inv_model_loc_bianco, 1, GL_FALSE, &ti_mm[0][0]);
        }else{
            glUniformMatrix4fv(model_loc_colorato, 1, GL_FALSE, &mm[0][0]);
            glUniformMatrix3fv (tr_inv_model_loc_colorato, 1, GL_FALSE, &ti_mm[0][0]);
        }
        bunny.draw ();
    }
    
    void draw_cube (glm::mat4 cube_trasforme)
    {
        glm::mat4 mm = cube_trasforme;
        glm::mat3 ti_mm = glm::transpose (glm::inverse (glm::mat3 (mm)));
        glUniformMatrix4fv(model_loc_bianco, 1, GL_FALSE, &mm[0][0]);
        glUniformMatrix3fv (tr_inv_model_loc_bianco, 1, GL_FALSE, &ti_mm[0][0]);
        cube.draw ();
    }

    void draw_wall ()
    {
        draw_cube (wall_model);
    }

    void draw_sphere (glm::mat4 sphere_trasforme)
    {
        glm::mat4 mm = sphere_trasforme;
        glm::mat3 ti_mm = glm::transpose (glm::inverse (glm::mat3 (mm)));        
        if (!sphere_hover){
            glUniformMatrix4fv(model_loc_bianco, 1, GL_FALSE, &mm[0][0]);
            glUniformMatrix3fv (tr_inv_model_loc_bianco, 1, GL_FALSE, &ti_mm[0][0]);
        }else{
            glUniformMatrix4fv(model_loc_colorato, 1, GL_FALSE, &mm[0][0]);
            glUniformMatrix3fv (tr_inv_model_loc_colorato, 1, GL_FALSE, &ti_mm[0][0]);
        }
        sphere.draw ();
    }

    void build_collision_boxes ()
    {
        glm::mat4 scale, translate;
        collision_boxes.clear ();
        if (level == 1){
            translate = fcg::translation (0, 0, 8.0f); // push it away
        }
        else{
            translate = fcg::translation (0, 0, -3.0f); // push it away
        }
        bunny_model = translate * bunny.to_unit_extent;
        float margin = 0.5f;
        Box bunny_box = bunny.world_bounds(bunny_model);
        bunny_box = expand_box(bunny_box, margin);
        collision_boxes.push_back (bunny_box);
        
        if (level == 2){
            // Dimensioni del muro: spessore, base, altezza
            float depth = 1.0f;
            float width = bunny.extent.x * 1.5;
            float height = width *(3.0f/4.0f);
            // draw back wall
            scale = fcg::scaling (width, height, depth); // flatten the cube!
            translate = fcg::translation (0, 0, -1.0f); // push it away
            wall_model = translate * scale * cube.to_unit_extent;
            float wall_margin = 1.5f;
            Box wall_box = cube.world_bounds(wall_model);
            wall_box = expand_box(wall_box, wall_margin);
            collision_boxes.push_back (wall_box);
        }
        else if (level == 3){
            translate = fcg::translation (0, 0, -1.0f); // push it away
            scale = fcg::scaling (1.5f, 1.5f, 1.5f); // flatten the cube!
            sphere_model = translate * scale * sphere.to_unit_extent;
            Box sphere_box = sphere.world_bounds(sphere_model);
            sphere_box = expand_box(sphere_box, margin);
            collision_boxes.push_back (sphere_box);
        }
    }
};


////////////////////
// SFML Callbacks //
////////////////////

void handle (const sf::Event::KeyPressed& key, Scene& scene, Menu& menu)
{
    if (parte3D){
        if (key.scancode == sf::Keyboard::Scancode::Escape)
        {
            parte3D = false;
            menu.help = 0;
            menu.success = false;
        }
        else if (key.scancode == sf::Keyboard::Scancode::Right)
        {
            scene.camera.asse = 1;
            scene.camera.move_start(true);
        }
        else if (key.scancode == sf::Keyboard::Scancode::Left)
        {
            scene.camera.asse = 1;
            scene.camera.move_start(false);
        }
        else if (key.scancode == sf::Keyboard::Scancode::Up)
        {
            scene.camera.asse = 2;
            scene.camera.move_start(true);
        }
        else if (key.scancode == sf::Keyboard::Scancode::Down)
        {
            scene.camera.asse = 2;
            scene.camera.move_start(false);
        }
        else if (key.scancode == sf::Keyboard::Scancode::NumpadPlus) // allontanare oggetto
        {
            scene.camera.asse = 3;
            scene.camera.move_start(true);
        }
        else if (key.scancode == sf::Keyboard::Scancode::NumpadMinus) // avvicinare oggetto
        {
            scene.camera.asse = 3;
            scene.camera.move_start(false);
        }
        else if (key.scancode == sf::Keyboard::Scancode::W)
        {
            if (scene.sphere_move_on)     scene.direzione = 1;
        }
        else if (key.scancode == sf::Keyboard::Scancode::S)
        {
            if (scene.sphere_move_on)     scene.direzione = 2;
        }
        else if (key.scancode == sf::Keyboard::Scancode::A)
        {
            if (scene.sphere_move_on)     scene.direzione = 3;
        }
        else if (key.scancode == sf::Keyboard::Scancode::D)
        {
            if (scene.sphere_move_on)     scene.direzione = 4;
        }
        else if (key.scancode == sf::Keyboard::Scancode::H)
        {
            menu.help = scene.level;
            parte3D = false;
        }
    }
    else{
        if (key.scancode == sf::Keyboard::Scancode::Escape)
        {
            if(menu.help == 0)
                exit (0);
            else if(menu.help == 1){
                parte3D = true;
                scene.reload(1);
            }
            else if(menu.help == 2){
                parte3D = true;
                scene.reload(2);
            }
            else if(menu.help == 3){
                parte3D = true;
                scene.reload(3);
            }
        }
    }
}

void handle (const sf::Event::KeyReleased& key, Scene& scene)
{
    if (key.scancode == sf::Keyboard::Scancode::Right
        || key.scancode == sf::Keyboard::Scancode::Left
        || key.scancode == sf::Keyboard::Scancode::Up
        || key.scancode == sf::Keyboard::Scancode::Down
        || key.scancode == sf::Keyboard::Scancode::NumpadPlus
        || key.scancode == sf::Keyboard::Scancode::NumpadMinus)
    {
        scene.camera.move_stop();
    }
}

void handle (const sf::Event::Resized& resized, Menu& menu, Camera& camera)
{
    if(!parte3D)
        menu.reset_size(resized.size.x, resized.size.y);
    else
    {
        glViewport (0, 0, resized.size.x, resized.size.y);
        camera.set_window_size (resized.size.x, resized.size.y);
    }
}

void handle (const sf::Event::MouseMoved& mouse_moved, Menu& menu, sf::RenderWindow& window, Scene& scene)
{
    if(!parte3D)
    {
        sf::FloatRect bounds = menu.sfondo.getGlobalBounds();
        if(bounds.contains({(float)mouse_moved.position.x, (float)mouse_moved.position.y}))
            menu.which_level({(float)mouse_moved.position.x, (float)mouse_moved.position.y});
        else menu.reset_level ();
    }
    else
    {
        // reset
        scene.bunny_hover = false;
        scene.sphere_hover = false;

        // Durante la rotazione della telecamera NON fare il ray casting sugli oggetti.
        if (scene.camera.is_moving())
            return;

        if (scene.level == 3)
        {
            scene.sphere_hover = scene.mouse_su_sphere (mouse_moved.position, window.getSize());
        }
        if (!scene.sphere_hover)    scene.bunny_hover = scene.mouse_su_bunny (mouse_moved.position, window.getSize());
    }
}

void handle (sf::Vector2f delta, Camera& camera)
{
    camera.pan_tilt (delta.x, delta.y);
}

void handle (const sf::Event::MouseButtonPressed& mouse_pressed, Camera& camera, sf::RenderWindow& window, Scene& scene, Menu& menu)
{
    if(parte3D)
    {
        if (mouse_pressed.button == sf::Mouse::Button::Left) {
            bool pan_tilt_on = camera.pan_tilt_toggle ();
            window.setMouseCursorGrabbed (pan_tilt_on);
            window.setMouseCursorVisible (!pan_tilt_on);
        }
        else if (mouse_pressed.button == sf::Mouse::Button::Right)
        {
            bool sphere = scene.mouse_su_sphere (mouse_pressed.position, window.getSize());
            if (scene.level == 3)
            {
                if (!scene.sphere_move_on)
                    scene.sphere_move_on = sphere;
                else{
                    scene.sphere_move_on = false;
                    scene.direzione = -1;
                }
            }
            if (!sphere && scene.mouse_su_bunny (mouse_pressed.position, window.getSize())){
                parte3D = false;
                menu.help = 0;
                menu.start_success();
            }
            else    parte3D = true;
        }
    }
    else
    {
        if (mouse_pressed.button == sf::Mouse::Button::Left) 
        {
            if(menu.get_level () == 1){
                parte3D = true;
                scene.reload(1);
            }
            else if(menu.get_level () == 2){  
                parte3D = true;  
                scene.reload(2);
            }
            else if(menu.get_level () == 3){
                parte3D = true;
                scene.reload(3);
            }
        }
    }
}


//////////
// Main //
//////////

int main ()
{
    Setup setup;
    sf::RenderWindow& window = setup.window;
    Menu menu (setup);

    fcg::Shaders shaders_bianco ("shader/shader_flat.vert", "shader/shader_flat.frag");
    fcg::Shaders shaders_colorato ("shader/shader_normals.vert", "shader/shader_normals.frag");
    shaders_bianco.use();
    Scene scene ("data/", shaders_bianco, shaders_colorato, 3);

    bool last_2D = false;
    sf::Clock clock;
    bool running = true;
    fcg::RawMouse raw_mouse;
    while(running)
    {
        window.clear();
        while (const std::optional event = window.pollEvent ())
        {
            if (event->is<sf::Event::Closed> ())
                running = false;
            else if(const auto* resized = event->getIf<sf::Event::Resized> ())
                handle (* resized, menu, scene.camera);
            else if(const auto* key_pressed = event->getIf<sf::Event::KeyPressed> ())
                handle (* key_pressed, scene, menu);
            else if (const auto* key_released = event->getIf<sf::Event::KeyReleased> ())
                handle (*key_released, scene);
            else if(const auto* mouse_moved = event->getIf<sf::Event::MouseMoved> ())
                handle (* mouse_moved, menu, window, scene);
            else if (const auto* mouse_pressed = event->getIf<sf::Event::MouseButtonPressed> ())
                handle (*mouse_pressed, scene.camera, window, scene, menu);
            else if (const auto* mouse_moved_raw = event->getIf<sf::Event::MouseMovedRaw> ())
                raw_mouse.event (*mouse_moved_raw);
        }

        if (!parte3D)
        {
            last_2D = true;
            setup.window.pushGLStates();

            glBindVertexArray(0);
            glUseProgram(0);

            window.resetGLStates();

            window.clear(sf::Color::Black);
            if (menu.success){
                if (menu.success_clock.getElapsedTime().asSeconds() < 2.0f)
                    menu.draw_success();
                else
                {
                    menu.success = false;
                    menu.draw();
                }
            }
            else if(menu.help==0){    
                menu.draw();
            }
            else    menu.draw_help();

            setup.window.popGLStates();
        }
        else
        {
            window.resetGLStates();

            glViewport(0, 0, window.getSize().x, window.getSize().y);
            glEnable (GL_CULL_FACE);
            glCullFace (GL_BACK);
            glEnable (GL_DEPTH_TEST);

                scene.update();
            if (!last_2D)
            {
                handle (raw_mouse.delta (), scene.camera);
                float elapsed = clock.restart().asSeconds();
                scene.camera.move (elapsed);
                scene.move_sphere (elapsed/3);
            }
            else //reset di camera
            {
                scene.camera.set_default();
                last_2D = false;
            }

            scene.draw();

            // lascia OpenGL in uno stato pulito
            glBindVertexArray(0);

            menu.reset_level ();
        }        
    
        window.display();
    }
}