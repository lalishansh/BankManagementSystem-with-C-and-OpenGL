#include "text2D.h"

Shader text2D::shader = Shader();
glm::vec3 text2D::color(1.0f, 1.0f, 1.0f);
glm::mat4 text2D::projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));

text2D::text2D()
{
    fileLoc = "fonts/Kingthings_Foundation.ttf";
}
text2D::text2D(std::string fontLoc)
{
    fileLoc = fontLoc;
}
bool text2D::init(){
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // compile and setup the shader
    // ----------------------------
    if(shader.GetShaderID()==0)
        shader.CreateFromFiles("Shaders/text.vert", "Shaders/text.frag");

    shader.UseShader();
    glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }


    // load font as face
    FT_Face face;
    if (FT_New_Face(ft, fileLoc.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return false;
    }
    else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 48);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++)
        {
            // Load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use

            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}
// render line of text
// -------------------
std::vector<float> text2D::RenderText(std::string text, float x, float y, float scale)
{
    //box details
    const float originX = x, originY = y;
    float lastBoxWid = 0, lastBoxHt = 0, lastBoxBearing = 0;

    ////////////////////////////
    scale *= 0.5f;// TEMP MEASURE
    ////////////////////////////

    // activate corresponding render state	
    shader.UseShader();
    glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader.GetShaderID(), "textColor"), color.x, color.y, color.z);
    glUniform1f(glGetUniformLocation(shader.GetShaderID(), "forcedAlpha"), 0.0f);
    glUniform1f(glGetUniformLocation(shader.GetShaderID(), "minAlpha"), 0.2f);

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 1.0f));
    //model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(scale)); //x /= scale, y /= scale;
    glUniformMatrix4fv(shader.GetModelLocation(), 1, GL_FALSE, glm::value_ptr(model));

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    x = 0.0f;
    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x;// *scale;
        float ypos = -ch.Size.y + ch.Bearing.y;// *scale;
        if (ch.Bearing.y > lastBoxBearing) lastBoxBearing = ch.Bearing.y;// getting max char bearing in y dirn // for text box

        float w = ch.Size.x;// *scale;
        float h = ch.Size.y;// *scale;
        if (h > lastBoxHt) lastBoxHt = h; // getting max char size.y // for text box

        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6);// * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    lastBoxWid = x; // getting total shift in x dirn i.e box width // for text box

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    //scale *= 2;// TEMP MEASURE

    std::vector<float> floats;
    floats.reserve(5);
    floats.push_back(originX-5);
    floats.push_back(originY-5);
    floats.push_back(lastBoxWid * scale + 10);
    floats.push_back(lastBoxHt * scale + 10);
    floats.push_back(lastBoxBearing);
    return floats;
}
void text2D::Color(float x, float y, float z) {
    color = glm::vec3(x, y, z);
}
glm::vec3 text2D::Color() {
    return color;
}
void forcingAlpha(GLuint ID, float amount) {
    glUniform1f(glGetUniformLocation(ID, "forcedAlpha"), amount);
}
void text2D::renderHUD(GLuint VAOs, GLuint VBOs,glm::vec4 posnAndSize,float alpha) {
    // activate corresponding render state	
    shader.UseShader();
    glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(shader.GetShaderID(), "textColor"), color.x, color.y, color.z);
    forcingAlpha(shader.GetShaderID(), alpha);
    glUniform1f(glGetUniformLocation(shader.GetShaderID(), "minAlpha"), 0.0f);

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(1.0f));
    glUniformMatrix4fv(shader.GetModelLocation(), 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(VAOs);
    float xpos = posnAndSize.x;
    float ypos = posnAndSize.y;

    float w = posnAndSize.z;
    float h = posnAndSize.w;
    // update VBO for each character
    float vertices[6][4] = {
        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos,     ypos,       0.0f, 1.0f },
        { xpos + w, ypos,       1.0f, 1.0f },

        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos + w, ypos,       1.0f, 1.0f },
        { xpos + w, ypos + h,   1.0f, 0.0f }
    };
    // update content of VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, VBOs);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // render quad
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

