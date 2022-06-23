#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "CommonDefs.h"

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "text2D.h"
#include "BMS_commonDef.h"

void renderScene(const Shader&, glm::vec3);
void renderCube();
void createObjs();
void mouseInterAction();
void DrawUIs(text2D& fontking);
void extractTextButton(std::vector<float>,char);
void createTextField(float xPos, float yPos, float width, float height, char identifier, text2D& font);
void bufferManager(bool* ,bool, bool& ,bool&, bool&);
void drawCursor(Shader& shader, glm::vec2 posn, GLuint VAOs, GLuint VBOs);
template<typename Typ>
void clamp(Typ& val, Typ high, Typ low);

// Settings
Window mainWindow;
Camera camera;

Mesh flatPlane;

unsigned int pointLightCount = 0;
unsigned int spotLightCount = 0;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

bool shadowsBOOL = true;


text2D fontking("fonts/Kingthings_Foundation.ttf");

struct {
    char data[100];
    int size;
    char boundTo;
}Buffer;

GLuint VAOsqr;
GLuint VBOsqr;
std::vector<Texture> Textures;
enum {
    Texture_PLAIN,
    Texture_CURSOR,
};

struct TextBoxBounds{
    float x, y;
    float w, h;
    float bearUp;
    char IDENTIFIER;
};
std::vector<TextBoxBounds> textBoxes;
std::string tempVariablesForTextFeilds[7] = {" "," "," "," "," "," "," "};
glm::vec2 mouseCursor(SCR_WIDTH/2,SCR_HEIGHT/2);

char BMS_scene = 0;
int main()
{
	mainWindow = Window(SCR_WIDTH, SCR_HEIGHT, "Bank Managment System");
	mainWindow.Initialise();

	Shader shader, simpleDepthShader,simple2dShader;
	shader.CreateFromFiles("Shaders/point/point_shadows.vs", "Shaders/point/point_shadows.fs");
	simpleDepthShader.CreateFromFiles("Shaders/point/point_shadows_depth.vs", "Shaders/point/point_shadows_depth.gs", "Shaders/point/point_shadows_depth.fs");
    simple2dShader.CreateFromFiles("Shaders/text.vert", "Shaders/2d_draw.frag");
    simple2dShader.use();
    simple2dShader.setInt("texture", 0);

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -60.0f, 0.0f, 5.0f, 0.5f);

	Texture brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
    Texture cursor = Texture("Textures/cursor.png");
    cursor.LoadTextureA();

    // configure depth map FBO
    // -----------------------
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shader.setInt("depthMap", 1);

    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
    glm::vec3 lightColor(0.5f, 0.5f, 0.5f);

    fontking.init();

    //
    bool cameraIsOrbiting = true;
    bool DiscoLights = false;
    bool rideTheLight = false;
    glm::vec3 lightLookAt;
    float lightColorAdjuster = 0;
    //
    createObjs();
    Buffer.data[0] = '\0';
    Buffer.size = 0;
    Buffer.boundTo = -1;
	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime(); // SDL_GetPerformanceCounter();
		deltaTime = now - lastTime; // (now - lastTime)*1000/SDL_GetPerformanceFrequency();
		lastTime = now;

		// Get + Handle User Input
		glfwPollEvents();
        bufferManager(mainWindow.getsKeys(),mainWindow.mouseLeft,cameraIsOrbiting,DiscoLights,rideTheLight);

		//camera.keyControl(mainWindow.getsKeys(), deltaTime);
		//system("CLS");
		camera.joyStickControl(mainWindow.getButtons(), mainWindow.getAxes(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());
        mouseCursor.x += mainWindow.getXChange(), mouseCursor.y += mainWindow.getYChange();
        clamp(mouseCursor.x, (float)(SCR_WIDTH-40), 1.0f);
        clamp(mouseCursor.y, (float)(SCR_HEIGHT-40), 1.0f);

		mainWindow.pollJoystickAxes();

        if (mainWindow.getsKeys()[GLFW_KEY_SPACE])
        {
            shadowsBOOL = !shadowsBOOL;
            mainWindow.getsKeys()[GLFW_KEY_SPACE] = false;
        }

        // move light position over time
        lightPos = glm::vec3(-sin((glfwGetTime()) * 0.05) * 3.0f, -sin((glfwGetTime()) * 0.1) * 3.0f, sin((glfwGetTime()) * 0.2) * 3.0);
        lightLookAt = glm::vec3(-sin((glfwGetTime() + 0.1f) * 0.05) * 3.0f, -sin((glfwGetTime() + 0.1f) * 0.1) * 3.0f, sin((glfwGetTime() + 0.1f) * 0.2) * 3.0);
        // Disco
        if (DiscoLights) {
            lightColorAdjuster += deltaTime;
            lightColor = glm::vec3(sin(lightColorAdjuster * 0.05f) * 0.5f + 0.5f, sin(lightColorAdjuster * 0.15f) * 0.5f + 0.5f, sin(lightColorAdjuster * 0.3f) * 0.5f + 0.5f);
        }
        else {
            if (lightColor.x != 0.5f)
                lightColor.x += (0.5f - lightColor.x) * deltaTime;
            if (lightColor.y != 0.5f)
                lightColor.y += (0.5f - lightColor.y) * deltaTime;
            if (lightColor.z != 0.5f)
                lightColor.z += (0.5f - lightColor.z) * deltaTime;
        }
        // ride

        if(rideTheLight) {
            glm::vec3 camPosn = camera.getCameraPosition();
            if (camPosn.x != lightPos.x)
                camPosn.x += (lightPos.x - camPosn.x) * deltaTime*3;
            if (camPosn.y != lightPos.y)
                camPosn.y += (lightPos.y - camPosn.y) * deltaTime*3;
            if (camPosn.z != lightPos.z)
                camPosn.z += (lightPos.z - camPosn.z) * deltaTime*3;
            camera.setCameraPosition(camPosn);
        }
        else {
            glm::vec3 camPosn = camera.getCameraPosition();
            if (camPosn.x != 0.0f)
                camPosn.x += (0.0f - camPosn.x) * deltaTime;
            if (camPosn.y != 0.0f)
                camPosn.y += (0.0f - camPosn.y) * deltaTime;
            if (camPosn.z != 0.0f)
                camPosn.z += (0.0f - camPosn.z) * deltaTime;
            camera.setCameraPosition(camPosn);
        }

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 0. create depth cubemap transformation matrices
        // -----------------------------------------------
        float near_plane = 1.0f;
        float far_plane = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        // 1. render scene to depth cubemap
        // --------------------------------
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        simpleDepthShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            simpleDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        simpleDepthShader.setFloat("far_plane", far_plane);
        simpleDepthShader.setVec3("lightPos", lightPos);
        renderScene(simpleDepthShader,lightPos);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. render scene as normal 
        // -------------------------
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.25f, 100.0f);
        camera.Orbit(rideTheLight ? lightLookAt:lightPos);
        glm::mat4 view = camera.calculateViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // set lighting uniforms
        shader.setVec3("lightPos", lightPos);
        shader.setVec3("lightColor", lightColor);
        shader.setVec3("viewPos", camera.getCameraPosition());
        shader.setInt("shadows", shadowsBOOL); // enable/disable shadows by pressing 'SPACE'
        shader.setFloat("far_plane", far_plane);
        brickTexture.UseTexture(0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        renderScene(shader,lightPos);

        // UI-Elements
        Textures[Texture_CURSOR].UseTexture(0);
        drawCursor(simple2dShader, mouseCursor, VAOsqr, VBOsqr);
        Textures[Texture_PLAIN].UseTexture();
        DrawUIs(fontking);
        // UI-Interaction
        mouseInterAction();
        text2D::Color(1, 1, 1);
        text2D::renderHUD(VAOsqr, VBOsqr,glm::vec4(100, 100, SCR_WIDTH - 200, SCR_HEIGHT - 200), 0.6f);

		glUseProgram(0);

		mainWindow.swapBuffers();
        mainWindow.ResetKeys();
        mainWindow.getsKeys()[GLFW_KEY_ENTER] = false;
        textBoxes.clear(); // removing all text boxes

        float sleepFor = 16.67f - deltaTime * 1000;
        clamp(sleepFor, 16.0f, 0.0f);
        Sleep(sleepFor);
	}
	return 0;
}

void DrawUIs(text2D& fontking)
{
    switch (BMS_scene) {
    case SCENES::MAIN_MENU:
        text2D::Color(0, 0, 0);
        fontking.RenderText("MAIN MENU", 200, 550, 2);
        extractTextButton(fontking.RenderText("01. NEW ACCOUNT", 200, 500, 1), MAIN_MENU::NEW_ACCOUNT);
        extractTextButton(fontking.RenderText("02. DEPOSIT AMOUNT", 200, 450, 1), MAIN_MENU::DEPOSIT_AMOUNT);
        extractTextButton(fontking.RenderText("03. WITHDRAW AMOUNT", 200, 400, 1), MAIN_MENU::WITHDRAW_AMOUNT);
        extractTextButton(fontking.RenderText("04. BALANCE ENQUIRY", 200, 350, 1), MAIN_MENU::BALANCE_ENQUIRY);
        extractTextButton(fontking.RenderText("05. ALL ACCOUNT HOLDER LIST", 200, 300, 1), MAIN_MENU::ALL_ACCOUNT_HOLDER_LIST);
        extractTextButton(fontking.RenderText("06. CLOSE AN ACCOUNT", 200, 250, 1), MAIN_MENU::CLOSE_AN_ACCOUNT);
        extractTextButton(fontking.RenderText("07. MODIFY AN ACCOUNT", 200, 200, 1), MAIN_MENU::MODIFY_AN_ACCOUNT);
        extractTextButton(fontking.RenderText("08. EXIT", 200, 150, 1), MAIN_MENU::EXIT);
        break;
    case SCENES::NEW_ACCOUNT:
        create_account();
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::DEPOSIT_AMOUNT:
        deposit_deposit();
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::WITHDRAW_AMOUNT:
        deposit_withdraw();
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::BALANCE_ENQUIRY:
        display_sp();
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::ALL_ACCOUNT_HOLDER_LIST:
        display_all();
        if (mainWindow.getsKeys()[GLFW_KEY_UP]) {
            SCENES::scrollRecordsBy > 0 ? SCENES::scrollRecordsBy-- : 0;
            mainWindow.getsKeys()[GLFW_KEY_UP] = false;
        }
        else if (mainWindow.getsKeys()[GLFW_KEY_DOWN]) {
            SCENES::scrollRecordsBy++ ;
            mainWindow.getsKeys()[GLFW_KEY_DOWN] = false;
        }
        if (mainWindow.getsKeys()[GLFW_KEY_ENTER])
            BMS_scene = SCENES::MAIN_MENU;
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::CLOSE_AN_ACCOUNT:
        delete_account();
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::MODIFY_AN_ACCOUNT:
        modify_account();
        if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE])
            BMS_scene = SCENES::MAIN_MENU, mainWindow.getsKeys()[GLFW_KEY_BACKSPACE] = false;
        break;
    case SCENES::EXIT:
        mainWindow.CloseWindow(true); // hack to close window
        break;
    }
    if (BMS_scene != SCENES::MAIN_MENU && !CLOSE_AN_ACCOUNT::CONFIRM_WINDOW) {
        createTextField(110, SCR_HEIGHT - 150, 0, 0, SCENES::BACK, fontking);
    }
}
void mouseInterAction()
{
    for (size_t i = 0;i < textBoxes.size();i++) {
        float xDiff = mouseCursor.x - textBoxes[i].x;
        if (0 < xDiff && xDiff < textBoxes[i].w) {
            float yDiff = mouseCursor.y - textBoxes[i].y + textBoxes[i].bearUp;
            if (0 < yDiff && yDiff < textBoxes[i].h) {
                text2D::Color(1, 1, 1);
                text2D::renderHUD(VAOsqr, VBOsqr,glm::vec4(textBoxes[i].x - 5 , textBoxes[i].y - 5 , textBoxes[i].w + 10, textBoxes[i].h + 10 ), 0.5f);
                text2D::Color(0, 0, 0);
                text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(textBoxes[i].x - 10, textBoxes[i].y - 10, textBoxes[i].w + 20, textBoxes[i].h + 20), 0.5f);
                fontking.RenderText("+", mouseCursor.x - 15, mouseCursor.y + 30, 1);
                if (mainWindow.mouseLeft) {
                    if (BMS_Input_Handler(&BMS_scene, textBoxes[i].IDENTIFIER)){
                        Buffer.boundTo = textBoxes[i].IDENTIFIER;//Also bind buffer load data from temp variable // Since if tempVar is empty then buffer will automatically be empty
                        for(size_t i = 0;i < tempVariablesForTextFeilds[Buffer.boundTo].size() + 1; i++)
                            Buffer.data[i] = tempVariablesForTextFeilds[Buffer.boundTo][i];
                        Buffer.size = tempVariablesForTextFeilds[Buffer.boundTo].size();
                    }
                }
            }
        }
    }
}

void createObjs() {
    GLuint VAO = 0, VBO = 0;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    VAOsqr = VAO;
    VBOsqr = VBO;


    Textures.push_back(Texture("Textures/plain.png"));
    Textures[Texture_PLAIN].LoadTextureA();

    Textures.push_back(Texture("Textures/cursor.png"));
    Textures[Texture_CURSOR].LoadTextureA();
}

GLuint cubeVAO = 0;
GLuint cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderScene(const Shader& shader,glm::vec3 lightPos)
{
    // room cube
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0f));
    shader.setMat4("model", model);
    glDisable(GL_CULL_FACE); // note that we disable culling here since we render 'inside' the cube instead of the usual 'outside' which throws off the normal culling methods.
    shader.setInt("reverse_normals", 1); // A small little hack to invert normals when drawing cube from the inside so lighting still works.
    renderCube();
    shader.setInt("reverse_normals", 0); // and of course disable it
    glEnable(GL_CULL_FACE);
    // cubes
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(8.0f, -6.5f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, 6.0f, 2.0));
    model = glm::scale(model, glm::vec3(0.95f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-6.0f, -2.0f, 0.0));
    model = glm::scale(model, glm::vec3(0.9f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-3.5f, 2.0f, 3.5));
    model = glm::scale(model, glm::vec3(0.75f));
    shader.setMat4("model", model);
    renderCube();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.5f, 2.0f, -3.0));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
    model = glm::scale(model, glm::vec3(0.95f));
    shader.setMat4("model", model);
    renderCube();

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    shader.setMat4("model", model);
    renderCube();
}
void extractTextButton(std::vector<float> params,char identifier) {
    TextBoxBounds fields;
    fields.x = params[0];
    fields.y = params[1];
    fields.w = params[2];
    fields.h = params[3];
    fields.bearUp = params[4];
    fields.IDENTIFIER = identifier;
    textBoxes.push_back(fields);
}
void createTextField(float xPos,float yPos,float width,float height, char identifier,text2D& font) {
    std::vector<float> paramss;
    paramss.reserve(5);
    if (identifier > -1) {
        if (Buffer.boundTo == identifier) {
            auto params = font.RenderText(Buffer.data, xPos + 15, yPos + 8, 1);
            if ((size_t)(glfwGetTime() * 1.5f) % 2)
                font.RenderText("_", xPos + 15 + params[2], yPos + 12, 2);
        }
        else
            font.RenderText(tempVariablesForTextFeilds[identifier], xPos + 10, yPos + 5, 1.2f);//temp
    }
    else {
        glm::vec3 colr = font.Color();
        font.Color(0.8f, 0.5f, 0.1f);
        std::vector<float>tmp = font.RenderText("< BACK", xPos + 10, yPos + 5, 1.2f);
        font.Color(0.0f, 0.0f, 0.0f);
        font.renderHUD(VAOsqr, VBOsqr, glm::vec4(tmp[0]-5, tmp[1]-5, tmp[2]+10, tmp[3]+10), 0.5f);
        width = tmp[2]+10;
        height = tmp[3]+10;
        yPos -= 5;
        font.Color(colr.x, colr.y, colr.z);
    }
    paramss.emplace_back(xPos);
    paramss.emplace_back(yPos);
    paramss.emplace_back(width);
    paramss.emplace_back(height);
    paramss.emplace_back(height);
    extractTextButton(paramss, identifier);
}

void esterOprations1(bool& orbit) {
    orbit = !orbit;
    camera.Orbit(orbit);
    Buffer.data[0] = 32;
    Buffer.data[1] = 0;
    Buffer.size = 0;
}
void esterOprations2(bool& disco) {
    disco = !disco;
    Buffer.data[0] = 32;
    Buffer.data[1] = 0;
    Buffer.size = 0;
}
void esterOprations3(bool& ride) {
    ride = !ride;
    Buffer.data[0] = 32;
    Buffer.data[1] = 0;
    Buffer.size = 0;
}
void bufferManager(bool* keys,bool mouseClick,bool& orbit,bool& disco,bool& ride) {
    if (mouseClick || keys[GLFW_KEY_ENTER]) {
        if (Buffer.boundTo > -1) {
            tempVariablesForTextFeilds[Buffer.boundTo] = std::string(Buffer.data);
            // UNBIND BUFFER //Also load data to temp variable //
            Buffer.boundTo = -1;
        }
        else {
            Buffer.data[0] = 32;
            Buffer.data[1] = 0;
            Buffer.size = 0;
        }
    }
    if (Buffer.size < 50) {
        for (size_t i = GLFW_KEY_0;i < GLFW_KEY_9 + 1;i++) {
            if (keys[i]) {
                Buffer.data[Buffer.size] = i;
                Buffer.size++, keys[i] = false;
                Buffer.data[Buffer.size] = 0;
            }
        }
        for (size_t i = GLFW_KEY_A;i < GLFW_KEY_Z + 1;i++) {
            if (keys[i]) {
                Buffer.data[Buffer.size] = i + 32;
                if (keys[GLFW_KEY_RIGHT_SHIFT] || keys[GLFW_KEY_LEFT_SHIFT]) Buffer.data[Buffer.size] -= 32;
                Buffer.size++, keys[i] = false;
                Buffer.data[Buffer.size] = 0;
            }
        }
        if (keys[GLFW_KEY_SPACE]) {
            Buffer.data[Buffer.size] = 32;
            Buffer.size++, keys[GLFW_KEY_SPACE] = false;
            Buffer.data[Buffer.size] = 0;
        }
    }
    if (keys[GLFW_KEY_BACKSPACE] && Buffer.boundTo > -1) {
        if(Buffer.size > 0) Buffer.size--;//crude clamping
        Buffer.data[Buffer.size] = 0,keys[GLFW_KEY_BACKSPACE] = false;
    }
    if (BMS_scene == SCENES::MAIN_MENU) {
        if (Buffer.data[0] == 'f')
            if (Buffer.data[1] == 'o')
                if (Buffer.data[2] == 'c')
                    if (Buffer.data[3] == 'u')
                        if (Buffer.data[4] == 's')
                            esterOprations1(orbit);
        if (Buffer.data[0] == 'l')
            if (Buffer.data[1] == 'i')
                if (Buffer.data[2] == 'g')
                    if (Buffer.data[3] == 'h')
                        if (Buffer.data[4] == 't')
                            esterOprations2(disco);
        if (Buffer.data[0] == 'r')
            if (Buffer.data[1] == 'i')
                if (Buffer.data[2] == 'd')
                    if (Buffer.data[3] == 'e')
                            esterOprations3(ride);
    }
}

void display_all()
{
    account ac;
    std::ifstream inFile;
    inFile.open("account.dat", std::ios::binary);
    if (!inFile)
    {
        std::cout << "File could not be open !! Press any Key...";
        return;
    }
    fontking.RenderText("All acc holder list:", 250, 600, 1.5f);
    fontking.RenderText("A/c no.      NAME         Type   Balance", 150, 550, 1.5f);

    size_t ScrollBy = SCENES::scrollRecordsBy;
    size_t i = 0;
    while (inFile.read(reinterpret_cast<char*> (&ac), sizeof(account)))
    {
        if (ScrollBy > 0) { ScrollBy--; continue; }
        char* buff = ac.report();
        fontking.RenderText(buff,190,500-i*50,1);
        delete[] buff;
        i++;
    }
    inFile.close();
}
void display_sp()
{
    fontking.RenderText("BALANCE ENQUIRY", 200, 550, 1.5f);
    fontking.RenderText("Your A/C No.", 200, 480, 1.2f);
    text2D::Color(1, 1, 0.3f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(450, 480, 300, 40), 0.9f);
    text2D::Color(0.0f, 0.0f, 0.0f);
    createTextField(450, 480, 300, 40, BALANCE_ENQUIRY::IN_ACCOUNT_NO, fontking);
    if (mainWindow.getsKeys()[GLFW_KEY_ENTER]) {
        if (Buffer.boundTo == BALANCE_ENQUIRY::IN_ACCOUNT_NO) {
            tempVariablesForTextFeilds[Buffer.boundTo] = std::string(Buffer.data);
            // UNBIND BUFFER //Also load data to temp variable // it may occur that enter key is pressed after bufferManager is executed
            Buffer.boundTo = -1;
        }
        display_sp(std::atoi(tempVariablesForTextFeilds[BALANCE_ENQUIRY::IN_ACCOUNT_NO].c_str()));
    }
    text2D::Color(1.0f, 0.3f, 0.1f);
    fontking.RenderText("BALANCE DETAILS:", 250, 420, 1.5f);
    fontking.RenderText("A/c No. :", 210, 337 - 0 * 50, 1);
    fontking.RenderText("Name    :", 210, 337 - 1 * 50, 1);
    fontking.RenderText("Balance :", 210, 337 - 3 * 50, 1);
    fontking.RenderText("A/c Type:", 210, 337 - 2 * 50, 1);
    text2D::Color(0, 0, 0);
    char i = 0;
    createTextField( 350, 330 - i * 50,300,40, BALANCE_ENQUIRY::ACCOUNT_NO,fontking);
    i++;
    createTextField( 350, 330 - i * 50, BALANCE_ENQUIRY::nameWid, 40, BALANCE_ENQUIRY::NAME, fontking);
    i++;
    createTextField( 350, 330 - i * 50, 300, 40, BALANCE_ENQUIRY::TYP, fontking);
    i++;
    createTextField( 350, 330 - i * 50, 300, 40, BALANCE_ENQUIRY::BALANCE, fontking);
}
void display_sp(int n)
{
    account ac;
    bool flag = false;
    std::ifstream inFile;
    inFile.open("account.dat", std::ios::binary);
    if (!inFile)
    {
        std::cout << "File could not be open !! Press any Key...";
        return;
    }
    size_t i = 0;
    while (inFile.read(reinterpret_cast<char*> (&ac), sizeof(account)))
    {
        i = 0;
        if (ac.retacno() == n)
        {
            tempVariablesForTextFeilds[BALANCE_ENQUIRY::ACCOUNT_NO] = std::to_string(ac.retacno());
            tempVariablesForTextFeilds[BALANCE_ENQUIRY::NAME] = ac.retname();
            BALANCE_ENQUIRY::nameWid = (ac.retname().size() > 30) ? (ac.retname().size()*16) : 400 ;
            tempVariablesForTextFeilds[BALANCE_ENQUIRY::TYP] = std::string(1,ac.rettyp());
            tempVariablesForTextFeilds[BALANCE_ENQUIRY::BALANCE] = std::to_string(ac.retbal());
            flag = true;
        }
    }
    inFile.close();
    text2D::Color(1, 0, 0);
    if (flag == false) {
        tempVariablesForTextFeilds[BALANCE_ENQUIRY::ACCOUNT_NO] = "Not Available";
        tempVariablesForTextFeilds[BALANCE_ENQUIRY::NAME] = "Not Available";
        tempVariablesForTextFeilds[BALANCE_ENQUIRY::TYP] = "Not Available";
        tempVariablesForTextFeilds[BALANCE_ENQUIRY::BALANCE] = "Not Available";
    }
}
void deposit_deposit() {
    fontking.RenderText("Your A/C No.", 200, 480, 1.2f);
    text2D::Color(1, 1, 0.3f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(450, 480, 300, 40), 0.9f);
    text2D::Color(0.0f, 0.0f, 0.0f);
    createTextField(450, 480, 300, 40, DEPOSIT_AMOUNT::IN_ACCOUNT_NO, fontking);
    
    fontking.RenderText("Your A/C Details", 200, 420, 1);
    createTextField(200, 370, 600, 40, DEPOSIT_AMOUNT::DETAILS, fontking);
    if ( (mainWindow.mouseLeft||mainWindow.getsKeys()[GLFW_KEY_ENTER]) && tempVariablesForTextFeilds[DEPOSIT_AMOUNT::IN_ACCOUNT_NO] != "" ) {
        tempVariablesForTextFeilds[DEPOSIT_AMOUNT::DETAILS] = getDetails(std::atoi(tempVariablesForTextFeilds[DEPOSIT_AMOUNT::IN_ACCOUNT_NO].c_str()));
    }
    if (tempVariablesForTextFeilds[DEPOSIT_AMOUNT::DETAILS] != "-") {
        fontking.RenderText("ENTER AMOUNT", 200, 310, 1.2f);
        createTextField(450, 310, 300, 40, DEPOSIT_AMOUNT::AMOUNT, fontking);
        if (mainWindow.getsKeys()[GLFW_KEY_ENTER]) {
            if (Buffer.boundTo == DEPOSIT_AMOUNT::AMOUNT) {
                tempVariablesForTextFeilds[Buffer.boundTo] = std::string(Buffer.data);
                Buffer.boundTo = -1;
            }
            if( deposit_withdraw(std::atoi(tempVariablesForTextFeilds[DEPOSIT_AMOUNT::IN_ACCOUNT_NO].c_str()),
                std::atoi(tempVariablesForTextFeilds[DEPOSIT_AMOUNT::AMOUNT].c_str()),1) )
                tempVariablesForTextFeilds[DEPOSIT_AMOUNT::AMOUNT] = "";
        }
    }
    createTextField(200, 230, 600, 40, DEPOSIT_AMOUNT::STATUS, fontking);
}
void deposit_withdraw() {
    fontking.RenderText("Your A/C No.", 200, 480, 1.2f);
    text2D::Color(1, 1, 0.3f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(450, 480, 300, 40), 0.9f);
    text2D::Color(0.0f, 0.0f, 0.0f);
    createTextField(450, 480, 300, 40, WITHDRAW_AMOUNT::IN_ACCOUNT_NO, fontking);

    fontking.RenderText("Your A/C Details", 200, 420, 1);
    createTextField(200, 370, 600, 40, WITHDRAW_AMOUNT::DETAILS, fontking);
    if ((mainWindow.mouseLeft || mainWindow.getsKeys()[GLFW_KEY_ENTER]) && tempVariablesForTextFeilds[WITHDRAW_AMOUNT::IN_ACCOUNT_NO] != "") {
        tempVariablesForTextFeilds[WITHDRAW_AMOUNT::DETAILS] = getDetails(std::atoi(tempVariablesForTextFeilds[WITHDRAW_AMOUNT::IN_ACCOUNT_NO].c_str()));
    }
    if (tempVariablesForTextFeilds[WITHDRAW_AMOUNT::DETAILS] != "-") {
        fontking.RenderText("ENTER AMOUNT", 200, 310, 1.2f);
        createTextField(450, 310, 300, 40, WITHDRAW_AMOUNT::AMOUNT, fontking);
        if (mainWindow.getsKeys()[GLFW_KEY_ENTER]) {
            if (Buffer.boundTo == WITHDRAW_AMOUNT::AMOUNT) {
                tempVariablesForTextFeilds[Buffer.boundTo] = std::string(Buffer.data);
                Buffer.boundTo = -1;
            }
            if (deposit_withdraw(std::atoi(tempVariablesForTextFeilds[WITHDRAW_AMOUNT::IN_ACCOUNT_NO].c_str()),
                std::atoi(tempVariablesForTextFeilds[WITHDRAW_AMOUNT::AMOUNT].c_str()), 2))
                tempVariablesForTextFeilds[WITHDRAW_AMOUNT::AMOUNT] = "";
        }
    }
    createTextField(200, 230, 600, 40, WITHDRAW_AMOUNT::STATUS, fontking);
}
bool deposit_withdraw(int n,int amt, int option)
{
    bool found = false;
    account ac;
    std::fstream File;
    File.open("account.dat", std::ios::binary | std::ios::in | std::ios::out);
    if (!File)
    {
        std::cout << "File could not be open !! Press any Key...";
        return false;
    }
    while (!File.eof() && found == false)
    {
        File.read(reinterpret_cast<char*> (&ac), sizeof(account));
        if (ac.retacno() == n)
        {
            if (option == 1) // from deposit_deposit
            {
                if(ac.dep(amt))
                    tempVariablesForTextFeilds[DEPOSIT_AMOUNT::STATUS] = "SUCCESSFUL, Record Updated";
            }
            if (option == 2) // from deposit_withdraw
            {
                if (!ac.draw(amt))
                    tempVariablesForTextFeilds[WITHDRAW_AMOUNT::STATUS] = "Insufficience balance";
            }
            int pos = (-1) * static_cast<int>(sizeof(ac));
            File.seekp(pos, std::ios::cur);
            File.write(reinterpret_cast<char*> (&ac), sizeof(account));
            std::cout << "\n\n\t Record Updated";
            found = true;
        }
    }
    File.close();
    if (found == false) {
        switch (option) {
        case 1: //deposit
            tempVariablesForTextFeilds[DEPOSIT_AMOUNT::STATUS] = "Record NOT Found";
        case 2: //withdraw
            tempVariablesForTextFeilds[WITHDRAW_AMOUNT::STATUS] = "Record NOT Found";
        }
    }
    return found;
}
std::string getDetails(int n) {
    account ac;
    bool flag = false;
    std::ifstream inFile;
    inFile.open("account.dat", std::ios::binary);
    if (!inFile)
    {
        std::cout << "File could not be open !! Press any Key...";
        return "-";
    }
    size_t i = 0;
    while (inFile.read(reinterpret_cast<char*> (&ac), sizeof(account)))
    {
        i = 0;
        if (ac.retacno() == n)
        {
            inFile.close();
            return ac.report();
        }
    }
    inFile.close();
    return "-";
}
void delete_account() {
    fontking.RenderText("Your A/C No.", 200, 480, 1.2f);
    text2D::Color(1, 1, 0.3f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(450, 480, 300, 40), 0.9f);
    text2D::Color(0.0f, 0.0f, 0.0f);
    createTextField(450, 480, 300, 40, CLOSE_AN_ACCOUNT::IN_ACCOUNT_NO, fontking);

    fontking.RenderText("A/C Details", 200, 420, 1);
    createTextField(200, 370, 600, 40, CLOSE_AN_ACCOUNT::DETAILS, fontking);
    if ((mainWindow.mouseLeft || mainWindow.getsKeys()[GLFW_KEY_ENTER]) && tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::IN_ACCOUNT_NO] != "") {
        tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::DETAILS] = getDetails(std::atoi(tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::IN_ACCOUNT_NO].c_str()));
    }
    if (mainWindow.getsKeys()[GLFW_KEY_BACKSPACE]){
        if (CLOSE_AN_ACCOUNT::CONFIRM_WINDOW)
            CLOSE_AN_ACCOUNT::CONFIRM_WINDOW = false;
        else
            BMS_scene = SCENES::MAIN_MENU;
    }

    if (CLOSE_AN_ACCOUNT::CONFIRM_WINDOW) {
        text2D::Color(0.8f, 0.3f, 0.0f);
        text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(250, 280, SCR_WIDTH - 250 * 2, 60), 1.0f);
        text2D::Color(0, 0, 0);
        fontking.RenderText("ARE YOU SURE(YES/NO)", 260, 285, 1);
        createTextField(650, 290, 100, 50, CLOSE_AN_ACCOUNT::CONFIRMATION, fontking);
        if (mainWindow.getsKeys()[GLFW_KEY_ENTER]) {
            if (tolower(tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::CONFIRMATION][0]) == 'y')
                CLOSE_AN_ACCOUNT::_DELETE = true;
            else {
                Buffer.boundTo = CLOSE_AN_ACCOUNT::IN_ACCOUNT_NO;
                for (size_t i = 0;i < tempVariablesForTextFeilds[Buffer.boundTo].size() + 1; i++)
                    Buffer.data[i] = tempVariablesForTextFeilds[Buffer.boundTo][i];
                Buffer.size = tempVariablesForTextFeilds[Buffer.boundTo].size();
            }
        }
    }
    else
    {
        createTextField(200, 240, 600, 40, CLOSE_AN_ACCOUNT::LAST_DELETED_DETAILS, fontking);
        if(tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::LAST_DELETED_DETAILS] != "" ||
          tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::LAST_DELETED_DETAILS] != "-")
            fontking.RenderText("Last Deleted Account Details:", 260, 285, 1);
    }

    if (mainWindow.getsKeys()[GLFW_KEY_ENTER] && tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::DETAILS] != "-") {
        CLOSE_AN_ACCOUNT::CONFIRM_WINDOW = !CLOSE_AN_ACCOUNT::CONFIRM_WINDOW;
        if (CLOSE_AN_ACCOUNT::CONFIRM_WINDOW) {
            Buffer.boundTo = CLOSE_AN_ACCOUNT::CONFIRMATION;
            Buffer.data[0] = 32;
            Buffer.data[1] = 0;
            Buffer.size = 0;
        }
    }
    if (CLOSE_AN_ACCOUNT::_DELETE) {
        tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::CONFIRMATION] = "";
        tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::DETAILS] = "-";
        tempVariablesForTextFeilds[CLOSE_AN_ACCOUNT::LAST_DELETED_DETAILS] = delete_account(std::atoi(tempVariablesForTextFeilds[WITHDRAW_AMOUNT::IN_ACCOUNT_NO].c_str()));
        CLOSE_AN_ACCOUNT::_DELETE = false;
    }
}
void create_account() {
    fontking.RenderText("Unique A/C No.", 200, 480, 1.2f);
    text2D::Color(1, 1, 0.3f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(450, 480, 300, 40), 0.9f);
    text2D::Color(0.0f, 0.0f, 0.0f);
    createTextField(450, 480, 300, 40, NEW_ACCOUNT::IN_ACCOUNT_NO, fontking);

    if ((mainWindow.mouseLeft || mainWindow.getsKeys()[GLFW_KEY_ENTER]) && tempVariablesForTextFeilds[NEW_ACCOUNT::IN_ACCOUNT_NO] != "") {
        account* ac = accExists(std::atoi(tempVariablesForTextFeilds[NEW_ACCOUNT::IN_ACCOUNT_NO].c_str()));
        if (ac) {
            tempVariablesForTextFeilds[NEW_ACCOUNT::IN_NAME] = ac->retname();
            NEW_ACCOUNT::nameWid = (ac->retname().size() > 30) ? (ac->retname().size() * 16) : 400;
            if (tolower(ac->rettyp()) == 'c')
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE] = "Current", NEW_ACCOUNT::accTYP = NEW_ACCOUNT::TYP_CURRENT;
            if (tolower(ac->rettyp()) == 's')
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE] = "Savings", NEW_ACCOUNT::accTYP = NEW_ACCOUNT::TYP_SAVINGS;

            tempVariablesForTextFeilds[NEW_ACCOUNT::IN_DEPOSIT] = std::to_string(ac->retbal());
            tempVariablesForTextFeilds[NEW_ACCOUNT::LOG] = "Account No. Already Exists.";
            NEW_ACCOUNT::allowAccountCreation = false;
        }
        else {
            if (!NEW_ACCOUNT::allowAccountCreation) {
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_NAME] = "";
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE] = "Not Available";
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_DEPOSIT] = "";

                NEW_ACCOUNT::allowAccountCreation = true;
            }
        }
        delete ac;
    }

    if (tempVariablesForTextFeilds[NEW_ACCOUNT::IN_ACCOUNT_NO] == " " ||
        tempVariablesForTextFeilds[NEW_ACCOUNT::IN_ACCOUNT_NO] == "") {
        text2D::Color(0.6f, 0.6f, 0.0f);
        text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(250, 200, SCR_WIDTH - 250 * 2, 250), 0.9f);
        text2D::Color(0, 0, 0);
        fontking.RenderText("Steps to Create Account:", 290, 250 + 3 * 50, 1);
        fontking.RenderText("01. Select/Enter Unique", 290, 250 + 2 * 50, 1);
        fontking.RenderText("02. Fill in Acount related", 290, 250 + 1 * 50, 1);
        fontking.RenderText("03. Enter Starting Amount", 290, 250 + 0 * 50, 1);
    }
    else
    {
        text2D::Color(1.0f, 0.3f, 0.1f);
        fontking.RenderText("Name    :", 210, 447 - 1 * 50, 1);
        fontking.RenderText("Balance :", 210, 447 - 3 * 50, 1);
        fontking.RenderText("A/c Type:", 210, 447 - 2 * 50, 1);
        text2D::Color(0, 0, 0);
        char i = 1;
        createTextField(350, 440 - i * 50, NEW_ACCOUNT::nameWid, 40, NEW_ACCOUNT::IN_NAME, fontking);
        i++;
        if (NEW_ACCOUNT::allowAccountCreation) {
            if (NEW_ACCOUNT::accTYP == NEW_ACCOUNT::TYP_CURRENT && tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE][0] != 'C')
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE] = "Current";
            if (NEW_ACCOUNT::accTYP == NEW_ACCOUNT::TYP_SAVINGS && tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE][0] != 'S')
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE] = "Savings";
            if (tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE][0] == 'S')
                text2D::Color(0.3, 1, 0.3f);
            else
                text2D::Color(0.3, 0.5, 1);
        }
        else
            text2D::Color(0.5f, 0.5f, 0.5f);
        text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(350, 440 - i * 50, 300, 40), 0.9f);
        text2D::Color(0, 0, 0);
        createTextField(350, 440 - i * 50, 300, 40, NEW_ACCOUNT::IN_TYPE, fontking);
        i++;
        createTextField(350, 440 - i * 50, 300, 40, NEW_ACCOUNT::IN_DEPOSIT, fontking);
    }

    if (mainWindow.getsKeys()[GLFW_KEY_ENTER] && NEW_ACCOUNT::allowAccountCreation) {
        if (Buffer.boundTo > NEW_ACCOUNT::IN_ACCOUNT_NO-1 && Buffer.boundTo < NEW_ACCOUNT::IN_DEPOSIT) {
            Buffer.boundTo++;
            Buffer.data[0] = 32;
            Buffer.data[1] = 0;
            Buffer.size = 0;
        }
        else {
            std::string err = write_account(std::atoi(tempVariablesForTextFeilds[NEW_ACCOUNT::IN_ACCOUNT_NO].c_str()),
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_NAME],
                tempVariablesForTextFeilds[NEW_ACCOUNT::IN_TYPE][0],
                std::atoi(tempVariablesForTextFeilds[NEW_ACCOUNT::IN_DEPOSIT].c_str()));
            if (err != "")
                tempVariablesForTextFeilds[NEW_ACCOUNT::LOG] = err;
            else
                tempVariablesForTextFeilds[NEW_ACCOUNT::LOG] = "Account SUCCESSFULLY Created.";
        }
    }
    createTextField(200, 200, SCR_WIDTH-200*2, 50, NEW_ACCOUNT::LOG,fontking);
}
void modify_account() {
    fontking.RenderText("Your A/C No.", 200, 480, 1.2f);
    text2D::Color(1, 1, 0.3f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(450, 480, 300, 40), 0.9f);
    text2D::Color(0.0f, 0.0f, 0.0f);
    createTextField(450, 480, 300, 40, MODIFY_AN_ACCOUNT::IN_ACCOUNT_NO, fontking);

    if (mainWindow.mouseLeft || mainWindow.getsKeys()[GLFW_KEY_ENTER]) {
        account* ac = accExists(std::atoi(tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::IN_ACCOUNT_NO].c_str()));
        if (ac) {
            if (!MODIFY_AN_ACCOUNT::allowAccountModification) {
                tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_NAME] = ac->retname();
                MODIFY_AN_ACCOUNT::nameWid = (ac->retname().size() > 30) ? (ac->retname().size() * 16) : 400;
                tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_DEPOSIT] = std::to_string(ac->retbal());
                if (tolower(ac->rettyp()) == 'c')
                    MODIFY_AN_ACCOUNT::accTYP = MODIFY_AN_ACCOUNT::TYP_CURRENT;
                if (tolower(ac->rettyp()) == 's')
                    MODIFY_AN_ACCOUNT::accTYP = MODIFY_AN_ACCOUNT::TYP_SAVINGS;
                MODIFY_AN_ACCOUNT::allowAccountModification = true;
            }
        }
        else {
            tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_NAME] = "Not Available";
            tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE] = "Not Avalable";
            tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_DEPOSIT] = "Not Avalable";
            MODIFY_AN_ACCOUNT::allowAccountModification = false;
        }
        delete ac;
    }

    text2D::Color(1.0f, 0.3f, 0.1f);
    fontking.RenderText("Name    :", 210, 447 - 1 * 50, 1);
    fontking.RenderText("Balance :", 210, 447 - 3 * 50, 1);
    fontking.RenderText("A/c Type:", 210, 447 - 2 * 50, 1);
    text2D::Color(0, 0, 0);
    char i = 1;
    createTextField(350, 440 - i * 50, MODIFY_AN_ACCOUNT::nameWid, 40, MODIFY_AN_ACCOUNT::NEW_NAME, fontking);
    i++;
    if (MODIFY_AN_ACCOUNT::allowAccountModification) {
        if (MODIFY_AN_ACCOUNT::accTYP == MODIFY_AN_ACCOUNT::TYP_CURRENT && tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE][0] != 'C')
            tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE] = "Current";
        if (MODIFY_AN_ACCOUNT::accTYP == MODIFY_AN_ACCOUNT::TYP_SAVINGS && tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE][0] != 'S')
            tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE] = "Savings";
        if (tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE][0] == 'S')
            text2D::Color(0.3, 1, 0.3f);
        else
            text2D::Color(0.3, 0.5, 1);
    }
    else
        text2D::Color(0.5f, 0.5f, 0.5f);
    text2D::renderHUD(VAOsqr, VBOsqr, glm::vec4(350, 440 - i * 50, 300, 40), 0.9f);
    text2D::Color(0, 0, 0);
    createTextField(350, 440 - i * 50, 300, 40, MODIFY_AN_ACCOUNT::NEW_TYPE, fontking);
    i++;
    createTextField(350, 440 - i * 50, 300, 40, MODIFY_AN_ACCOUNT::NEW_DEPOSIT, fontking);


    if (mainWindow.getsKeys()[GLFW_KEY_ENTER] && MODIFY_AN_ACCOUNT::allowAccountModification) {
        if (Buffer.boundTo > MODIFY_AN_ACCOUNT::IN_ACCOUNT_NO - 1 && Buffer.boundTo < MODIFY_AN_ACCOUNT::NEW_DEPOSIT) {
            Buffer.boundTo++;
            Buffer.data[0] = 32;
            Buffer.data[1] = 0;
            Buffer.size = 0;
        }
        else {
            std::string err = modify_account(std::atoi(tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::IN_ACCOUNT_NO].c_str()),
                tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_NAME],
                tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_TYPE][0],
                std::atoi(tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::NEW_DEPOSIT].c_str()));
            if (err != "")
                tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::LOG] = err;
            else
                tempVariablesForTextFeilds[MODIFY_AN_ACCOUNT::LOG] = "Account SUCCESSFULLY updated.";
        }
    }
    createTextField(200, 200, SCR_WIDTH - 200 * 2, 50, MODIFY_AN_ACCOUNT::LOG, fontking);
}
void resetGlobVars() {
    for(char i=0;i<7;i++)
        tempVariablesForTextFeilds[i] = "";
}
void drawCursor(Shader& shader,glm::vec2 posn,GLuint VAOs,GLuint VBOs) {
    shader.UseShader();
    glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, text2D::UIdrawProjValPtr());
    glUniform1f(glGetUniformLocation(shader.GetShaderID(), "minAlpha"), 0.1f);

    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(1.0f));
    glUniformMatrix4fv(shader.GetModelLocation(), 1, GL_FALSE, glm::value_ptr(model));

    //glActiveTexture(GL_TEXTURE3);

    glBindVertexArray(VAOs);
    float xpos = posn.x;
    float ypos = posn.y;

    float w = 40;
    float h = 40;
    // update VBO for quad
    float vertices[6][4] = {
        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos,     ypos,       0.0f, 1.0f },
        { xpos + w, ypos,       1.0f, 1.0f },

        { xpos,     ypos + h,   0.0f, 0.0f },
        { xpos + w, ypos,       1.0f, 1.0f },
        { xpos + w, ypos + h,   1.0f, 0.0f }
    };
    // render texture over quad
    //glBindTexture(GL_TEXTURE_2D, textureIDs);
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
template<typename Typ>
void clamp(Typ& val, Typ high, Typ low) {
    val < low ? (val = low) : 0;
    val > high ? (val = high) : 0;
}