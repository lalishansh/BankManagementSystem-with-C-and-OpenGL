#pragma once
#include <map>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Shader.h"
#include "CommonDefs.h"
#include "Texture.h"
#include "Mesh.h"

class text2D
{
public:
    text2D();
    text2D(std::string fontLoc);
    bool init();
    std::vector<float> RenderText(std::string text, float x, float y, float scale);
    
    static void Color(float x, float y, float z);
    static glm::vec3 Color();

    static void renderHUD(GLuint VAOs, GLuint VBOs, glm::vec4 posnAndSize, float alpha);
    static float* UIdrawProjValPtr() { return glm::value_ptr(projection); }
private:
    static glm::vec3 color;
    static glm::mat4 projection;
    struct Character {
        unsigned int TextureID; // ID handle of the glyph texture
        glm::ivec2   Size;      // Size of glyph
        glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
        unsigned int Advance;   // Horizontal offset to advance to next glyph
    };

    std::string fileLoc;
    static Shader shader;
    std::map<GLchar, Character> Characters;
    GLuint VAO, VBO;
};