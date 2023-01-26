#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "filesystem.h"
#include <learnopengl/shader_m.h>
#include  "camera.h"
#include <learnopengl/model.h>
#include <iostream>
#include <irrklang/irrKlang.h>
#include <math.h>
#include "Sphere.h"
#include <ft2build.h>
#include FT_FREETYPE_H  

#define _USE_MATH_DEFINES
#define TAU (M_PI * 2.0)
/* Create Sound Engine to play music */
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll
using namespace irrklang;
ISoundEngine* SoundEngine = createIrrKlangDevice();

struct MenuWindow {
    std::string Health;
    std::string Points;
    std::string Lives;
};
MenuWindow Menu;

struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};

/* Functions */
void GetDesktopResolution(float& horizontal, float& vertical);
void RenderText(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(vector<std::string> faces);

float delta = .1f;
glm::vec3 ScenePositions[2];
glm::vec3 viewPos;
double viewX;
double viewZ;
int Scene = 0;
void ShowInfo(Shader& s);

std::map<GLchar, Character> Characters;
GLuint textVAO, textVBO;

enum GameState {
    GAME_SCENETWO,
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN,
    GAME_LOSE,
    GAME_PAUSE
};

enum Direction {
    MOVEUP,
    MOVERIGHT,
    MOVEDOWN,
    MOVELEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;
GameState State = GAME_ACTIVE;
bool Keys[1024];
bool KeysProcessed[1024];
unsigned int Width, Height;
unsigned int points;
unsigned int lives;
unsigned int level;
float SCR_WIDTH = 1000;
float SCR_HEIGHT = 900;
unsigned int VAO = 0;
unsigned int VBO;
unsigned int nrRows = 10;
unsigned int nrZ = 6;
unsigned int nrColumns = 7;
unsigned int spacing = 3;
float speed = .1f;

GLfloat rotateY = 0.0f;
GLfloat rotateX = 0.0f;
glm::vec3 point = glm::vec3(0.0f, 0.0f, 0.0f);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float pyramidVertices[] = {
     -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
     -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.5f, 0.5f,

     -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.5f, 0.5f,
     0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
     0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.5f, 0.5f,

     0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
     -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.5f, 0.5f,
     -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,
     -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
};

glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

int main()
{
    GetDesktopResolution(SCR_WIDTH, SCR_HEIGHT); // get resolution for create window
    camera.LookAtPos = point;
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

    FT_Face face;
    if (FT_New_Face(ft, "Antonio-Bold.ttf", 0, &face))
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
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
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("VS.vs", "FS.fs");
    Shader skyboxShader("skybox.vs", "skybox.fs");
    Shader textShader("TextShader.vs", "TextShader.fs");

    // for text rendering
    glm::mat4 Text_projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);
    textShader.use();
    glUniformMatrix4fv(glGetUniformLocation(textShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(Text_projection));


    std::vector<glm::vec3> lightPositions;
    lightPositions.push_back(glm::vec3(-.8f, 1.4f, -1.0f)); // green light
    std::vector<glm::vec3> lightColors;

    lightColors.push_back(glm::vec3(0.0f, 5.0f, 0.0f)); // greeen color

    float cubeVertices[] = {
        -0.5f, -2.5f, -0.5f,    0.0f, 0.0f,
        0.5f, -2.5f, -0.5f, 	1.0f, 0.0f,   // top 
        0.5f, -1.5f, -0.5f, 	1.0f, 1.0f,
        0.5f, -1.5f, -0.5f, 	1.0f, 1.0f,
        -0.5f, -1.5f, -0.5f, 	0.0f, 1.0f,
        -0.5f, -2.5f, -0.5f, 	0.0f, 0.0f,

        -0.5f, -2.5f, 0.5f, 	0.0f, 0.0f,
        0.5f, -2.5f, 0.5f, 	    1.0f, 0.0f,
        0.5f, -1.5f, 0.5f, 		1.0f, 1.0f,
        0.5f, -1.5f, 0.5f, 		1.0f, 1.0f, // bottom 
        -0.5f, -1.5f, 0.5f, 	0.0f, 1.0f,
        -0.5f, -2.5f, 0.5f, 	0.0f, 0.0f,

        -0.5f, -1.5f, 0.5f, 	1.0f, 0.0f,
        -0.5f, -1.5f, -0.5f, 	1.0f, 1.0f,
        -0.5f, -2.5f, -0.5f, 	0.0f, 1.0f,
        -0.5f, -2.5f, -0.5f, 	0.0f, 1.0f,  // left
        -0.5f, -2.5f, 0.5f, 	0.0f, 0.0f,
        -0.5f, -1.5f, 0.5f, 	1.0f, 0.0f,

        0.5f, -1.5f, 0.5f, 		1.0f, 0.0f,
        0.5f, -1.5f, -0.5f, 	1.0f, 1.0f,
        0.5f, -2.5f, -0.5f, 	0.0f, 1.0f,
        0.5f, -2.5f, -0.5f, 	0.0f, 1.0f,  // right
        0.5f, -2.5f, 0.5f, 	    0.0f, 0.0f,
        0.5f, -1.5f, 0.5f, 		1.0f, 0.0f,

        -0.5f, -2.5f, -0.5f, 	0.0f, 1.0f,
        0.5f, -2.5f, -0.5f, 	1.0f, 1.0f,
        0.5f, -2.5f, 0.5f, 	    1.0f, 0.0f,
        0.5f, -2.5f, 0.5f, 	    1.0f, 0.0f,
        -0.5f, -2.5f, 0.5f, 	0.0f, 0.0f,  // back 
        -0.5f, -2.5f, -0.5f, 	0.0f, 1.0f,

        -0.5f, -1.5f, -0.5f, 	0.0f, 1.0f,
        0.5f, -1.5f, -0.5f, 	1.0f, 1.0f,
        0.5f, -1.5f, 0.5f, 		1.0f, 0.0f,
        0.5f, -1.5f, 0.5f, 		1.0f, 0.0f,
        -0.5f, -1.5f, 0.5f, 	0.0f, 0.0f, // front
      -0.5f, -1.5f, -0.5f, 		0.0f, 1.0f
    };


    float ufoVertices[] = {

         -0.05f, -0.5f, 0.0f, 0.0f, 0.0f,
        -0.05f, 0.5f, 0.0f, 0.0f, 1.0f,  // bottom
        0.05f, -0.5f, 0.0f, 1.0f, 0.0f,

        0.05f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.05f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.05f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.5f, 0.5f,
        -0.05f, -0.5f, 0.0f, 0.0f, 0.0f,

        0.05f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.5f, 0.5f,
        0.05f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.05f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.5f, 0.5f,

        0.05f, 0.5f, 0.0f, 0.0f, 0.0f,
        -0.05f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.5f, 0.5f,   // right
        -0.05f, 0.5f, 0.0f, 0.0f, 0.0f,
        -0.05f, -0.5f, 0.0f, 1.0f, 0.0f,

    };

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glm::vec3 pointLightPositions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f),
        glm::vec3(-1.7f,  -1.2f,  1.0f),
        glm::vec3(1.3f, -2.3f, -3.0f),
        glm::vec3(-3.0f,  1.0f, -11.0f),
        glm::vec3(0.0f,  -1.0f, -2.0f)

    };
    glm::vec3 ufoPositions[] = {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 0.0f, 0.0f),
        glm::vec3(4.0f, 0.0f, 0.0f),
        glm::vec3(5.0f, 0.0f, 0.0f),
        glm::vec3(6.0f, 0.0f, 0.0f),
        glm::vec3(7.0f, 0.0f, 0.0f),
        glm::vec3(8.0f, 0.0f, 0.0f),
        glm::vec3(9.0f, 0.0f, 0.0f),
        glm::vec3(10.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 3.0f, 0.0f),
        glm::vec3(2.0f, 3.0f, 0.0f),
        glm::vec3(3.0f, 3.0f, 0.0f),
        glm::vec3(4.0f, 3.0f, 0.0f),
        glm::vec3(5.0f, 3.0f, 0.0f),
        glm::vec3(6.0f, 3.0f, 0.0f),
        glm::vec3(7.0f, 3.0f, 0.0f),
        glm::vec3(8.0f, 3.0f, 0.0f),
        glm::vec3(9.0f, 3.0f, 0.0f),
        glm::vec3(10.0f, 3.0f, 0.0f)
    };

    glm::vec3 pyramidPositions[] = {
        glm::vec3(1.0f, 1.0f, 0.0f),
        glm::vec3(2.0f, 1.0f, 0.0f),
        glm::vec3(3.0f, 1.0f, 0.0f),
        glm::vec3(4.0f, 1.0f, 0.0f),
        glm::vec3(5.0f, 1.0f, 0.0f),
        glm::vec3(6.0f, 1.0f, 0.0f),
        glm::vec3(7.0f, 1.0f, 0.0f),
        glm::vec3(8.0f, 1.0f, 0.0f),
        glm::vec3(9.0f, 1.0f, 0.0f),
        glm::vec3(10.0f, 1.0f, 0.0f),
        glm::vec3(1.0f, 4.0f, 0.0f),
        glm::vec3(2.0f, 4.0f, 0.0f),
        glm::vec3(3.0f, 4.0f, 0.0f),
        glm::vec3(4.0f, 4.0f, 0.0f),
        glm::vec3(5.0f, 4.0f, 0.0f),
        glm::vec3(6.0f, 4.0f, 0.0f),
        glm::vec3(7.0f, 4.0f, 0.0f),
        glm::vec3(8.0f, 4.0f, 0.0f),
        glm::vec3(9.0f, 4.0f, 0.0f),
        glm::vec3(10.0f, 4.0f, 0.0f)
    };
    glm::vec3 spherePositions[] = {
        glm::vec3(-10.0f, 0.0f, 2.0f),
        glm::vec3(-8.0f, 0.0f, 2.0f),
        glm::vec3(-6.0f, 0.0f, 2.0f),
        glm::vec3(-4.0f, 0.0f, 2.0f),
        glm::vec3(-2.0f, 0.0f, 2.0f),        
        glm::vec3(0.0f, 0.0f, 2.0f),
        glm::vec3(2.0f, 0.0f, 2.0f),
        glm::vec3(4.0f, 0.0f, 2.0f),
        glm::vec3(6.0f, 0.0f, 2.0f),
        glm::vec3(8.0f, 0.0f, 2.0f),        
        glm::vec3(10.0f, 0.0f, 2.0f),
        glm::vec3(12.0f, 0.0f, 2.0f),
        glm::vec3(14.0f, 0.0f, 2.0f),
        glm::vec3(16.0f, 0.0f, 2.0f),
        glm::vec3(18.0f, 0.0f, 2.0f),
    };
  
    unsigned int cube1VAO, cube1VBO;
    glGenVertexArrays(1, &cube1VAO);
    glGenBuffers(1, &cube1VBO);
    glBindVertexArray(cube1VAO);
    glBindBuffer(GL_ARRAY_BUFFER, cube1VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ufoVertices), &ufoVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    unsigned int ufoVAO, ufoVBO;
    glGenVertexArrays(1, &ufoVAO);
    glGenBuffers(1, &ufoVBO);
    glBindVertexArray(ufoVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ufoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ufoVertices), &ufoVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    unsigned int pyramidVAO, pyramidVBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), &pyramidVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  
    /* TEXT RENDERING VAO-VBO*/
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    /* TEXT RENDERING VAO-VBO*/

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    unsigned int ufoTexture = loadTexture("resources/textures/4.png");
    unsigned int cubeTexture = loadTexture("resources/textures/3.png");
    unsigned int pyramidTexture = loadTexture("resources/textures/4.png");
    unsigned int pyramid2Texture = loadTexture("resources/textures/6.png");
    unsigned int sphereTexture = loadTexture("resources/textures/7.png");

    vector<std::string> faces
    {
        "resources/textures/6.png", // right   or rightright
        "resources/textures/6.png", // right   or rightright
        "resources/textures/6.png", // right   or rightright
        "resources/textures/6.png", // right   or rightright
        "resources/textures/6.png", // right   or rightright
        "resources/textures/6.png", // right   or rightright
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    vector<std::string> faces2
    {
        "resources/textures/5.png", // right   or rightright
        "resources/textures/5.png", // right   or rightright
        "resources/textures/5.png", // right   or rightright
        "resources/textures/5.png", // right   or rightright
        "resources/textures/5.png", // right   or rightright
        "resources/textures/5.png", // right   or rightright
    };
    unsigned int cubemap2Texture = loadCubemap(faces2);

    vector<std::string> faces3
    {
        "resources/textures/2.png", // right   or rightright
        "resources/textures/2.png", // right   or rightright
        "resources/textures/2.png", // right   or rightright
        "resources/textures/2.png", // right   or rightright
        "resources/textures/2.png", // right   or rightright
        "resources/textures/2.png", // right   or rightright
    };
    unsigned int cubemap3Texture = loadCubemap(faces3);
    shader.use();
    shader.setInt("texture1", 0);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    SoundEngine->play2D("breakout.mp3", true);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);   // radians(camera.Zoom) or 40.0f format less than 100.0f

        shader.setMat4("model", model);
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        /* SPHERE GENERATION */
        Sphere sphere(.4f, 36, 18);

        if (State == GAME_MENU)
        {
            double xx = sin(glfwGetTime() * delta) * 100.0f * 2.0f * 1.3f;
            double zz = cos(glfwGetTime() * delta) * 100.0f * 2.0f * 1.3f;
            ScenePositions[0] = glm::vec3(xx, 0.0f, zz);
            float angle = 270.0f;
            float angle2 = 0.0f;
            glBindVertexArray(pyramidVAO);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.01f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
            shader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ufoTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            shader.use();

            // pyramid
            glBindVertexArray(pyramidVAO);
            model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.3f, 1.3f, 1.3f));
            model = glm::rotate(model, glm::radians(angle2), glm::vec3(0.01f, 0.0f, 0.0f));
            shader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ufoTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            shader.use();

            // cube
            glBindVertexArray(cubeVAO);
            model = glm::translate(model, glm::vec3(0.0f, -0.4f, 0.65f));
            model = glm::rotate(model, glm::radians(angle), glm::vec3(0.01f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
            shader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            shader.use();

            float angle3 = 200.0f;
            glBindVertexArray(cube1VAO);
            model = glm::translate(model, glm::vec3(0.0f, -5.0f, 9.5f));
            model = glm::rotate(model, glm::radians(angle3), glm::vec3(1.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(4.9f, 4.9f, 4.9f));
            shader.setMat4("model", model);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ufoTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            shader.use();

            glDepthFunc(GL_LEQUAL); 
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 72);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); 
        }

        if (State == GAME_SCENETWO)
        {

            for (unsigned int i = 0; i < sizeof(spherePositions) / sizeof(spherePositions[0]); ++i)
            {
                double xx = sin(glfwGetTime() * delta) * 100.0f * 3.0f * 1.3f;
                double zz = cos(glfwGetTime() * delta) * 100.0f * 3.0f * 1.3f;
                glm::vec3 newPos = spherePositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0); 
                newPos = spherePositions[i];
                shader.setVec3("spherePositions[" + std::to_string(i) + "]", newPos);
                model = glm::mat4(1.0f);
                model = glm::translate(model, newPos);
                model = glm::scale(model, glm::vec3(01.0f));
                model = glm::translate(model, point);
                model = glm::rotate(model, glm::radians(rotateY), glm::vec3(1.0f, -1.0f, -13.0f));
                model = glm::rotate(model, glm::radians(rotateX), glm::vec3(0.0f, 0.0f, -1.0f));
                ScenePositions[1] = glm::vec3(xx, 0.0f, zz);
                model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.f));
                model = glm::rotate(model, glm::radians(-33.25f), glm::vec3(0.0f, 1.0f, 0.f));
                model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(-33.25f) * 2.0f, glm::vec3(0.0f, 0.0f, 1.f));
                shader.setMat4("model", model);
                float shininess = 128;
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sphereTexture);
                sphere.Draw();
                float angle = 20.0;
                glDrawArrays(GL_TRIANGLES, 0, 36);
                glm::mat4 model = glm::mat4(1.0f);
                shader.setMat4("model", model);
                model = glm::rotate(model, glm::radians(angle), glm::vec3(0.01f, 0.01f, 0.01f));
                model = glm::translate(model, spherePositions[i]);
                model = glm::scale(model, glm::vec3(0.2f));
                glPopMatrix();
            }
            glDepthFunc(GL_LEQUAL);
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap2Texture);
            glDrawArrays(GL_TRIANGLES, 0, 72);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS);
        }

        if (State == GAME_ACTIVE)
        {
            double xx = sin(glfwGetTime() * delta) * 100.0f * 4.0f * 1.3f;
            double zz = cos(glfwGetTime() * delta) * 100.0f * 4.0f * 1.3f;
            ScenePositions[1] = glm::vec3(xx, 0.0f, zz);
            for (unsigned int i = 0; i < lightPositions.size(); i++)
            {
                shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
                shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
            }
            glBindVertexArray(pyramidVAO);
            for (int row = 0; row < nrRows; ++row)
            {
                for (int col = 0; col < nrColumns; ++col)
                {
                    for (int a = 0; a < nrZ; ++a)
                    {
                        shader.setFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3((float)(col - (nrColumns / 2)) * spacing, (float)(row - (nrRows / 2)) * spacing * 2, -2.0f * a + 1));
                        shader.setMat4("model", model);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, pyramid2Texture);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
            glBindVertexArray(ufoVAO);
            for (int row2 = 0; row2 < nrRows; ++row2)
            {
                for (int col2 = 0; col2 < nrColumns; ++col2)
                {
                    for (int b = 0; b < nrZ; ++b)
                    {
                        shader.setFloat("roughness", glm::clamp((float)col2 / (float)nrColumns, 0.05f, 1.0f));
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3((float)(col2 - (nrColumns / 2)) * spacing, (float)(row2 - (nrRows / 2)) * spacing * 2, -2.0f * b));
                        shader.setMat4("model", model);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, pyramid2Texture);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }

            for (int row3 = 0; row3 < nrRows; ++row3)
            {
                for (int col3 = 0; col3 < nrColumns; ++col3)
                {
                    for (int c = 0; c < nrZ; ++c)
                    {
                        spacing = 1;
                        shader.setFloat("roughness", glm::clamp((float)col3 / (float)nrColumns, 0.05f, 1.0f));
                        model = glm::mat4(1.0f);
                        model = glm::translate(model, glm::vec3((float)(col3 - (nrColumns / 2)) * spacing + 1, (float)(row3 - (nrRows / 2)) * spacing * 2, -2.4f * c));
                        shader.setMat4("model", model);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, pyramid2Texture);
                        sphere.Draw();
                    }
                }
            }           

            glDepthFunc(GL_LEQUAL);
            skyboxShader.use();
            view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
            skyboxShader.setMat4("view", view);
            skyboxShader.setMat4("projection", projection);
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap3Texture);
            glDrawArrays(GL_TRIANGLES, 0, 72);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); 
            }
        if (State == GAME_ACTIVE || GAME_MENU || GAME_SCENETWO)
        { 
            switch (Scene)
            {
            case 1: // Menu

                viewX = sin(glfwGetTime() * delta) * 100.0f * 2.0f * 1.3f;
                viewZ = cos(glfwGetTime() * delta) * 100.0f * 2.0f * 1.3f;
                viewPos = glm::vec3(viewX, 50.0f, viewZ);
                view = glm::lookAt(viewPos, ScenePositions[0], glm::vec3(0.0f, 1.0f, 0.0f));
                ShowInfo(textShader);
                break;

            case 2: // AI
                viewX = sin(glfwGetTime() * delta) * 100.0f * 3.0f * 1.3f;
                viewZ = cos(glfwGetTime() * delta) * 100.0f * 3.0f * 1.3f;
                viewPos = glm::vec3(viewX, 50.0f, viewZ);
                view = glm::lookAt(viewPos, ScenePositions[1], glm::vec3(0.0f, 1.0f, 0.0f));
                ShowInfo(textShader);
                break;

            case 3: // GAME_ACTIVE
                viewX = sin(glfwGetTime() * delta) * 100.0f * 4.0f * 1.3f;
                viewZ = cos(glfwGetTime() * delta) * 100.0f * 4.0f * 1.3f;
                viewPos = glm::vec3(viewX, 50.0f, viewZ);
                view = glm::lookAt(viewPos, ScenePositions[2], glm::vec3(0.0f, 1.0f, 0.0f));  
                for (int row3 = 0; row3 < nrRows; ++row3)
                {
                    for (int col3 = 0; col3 < nrColumns; ++col3)
                    {
                        for (int c = 0; c < nrZ; ++c)
                        {
                            spacing = 1;
                            shader.setFloat("roughness", glm::clamp((float)col3 / (float)nrColumns, 0.05f, 1.0f));
                            model = glm::mat4(1.0f);
                            model = glm::translate(model, glm::vec3((float)(col3 - (nrColumns / 2)) * spacing + 1, (float)(row3 - (nrRows / 2)) * spacing * 2, -2.4f * c));
                            shader.setMat4("model", model);
                            glActiveTexture(GL_TEXTURE0);
                            glBindTexture(GL_TEXTURE_2D, pyramid2Texture);
                            sphere.Draw();
                        }
                    }
                }
                ShowInfo(textShader);
                break;
            default:
                RenderText(textShader, "Press WASD to navigate the scenes", SCR_WIDTH / 2.0f - 350.0f, SCR_HEIGHT / 2.0f, 1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
                RenderText(textShader, "Press 1, 2, or 3 to change scenes", SCR_WIDTH / 2.0f - 260.0f, SCR_HEIGHT / 2.0f + 50.0f, 0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            }
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &pyramidVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteBuffers(1, &pyramidVBO);
    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow* window)
{
    Shader shader("6.1.cubemaps.vs", "6.1.cubemaps.fs");
    glm::mat4 model = glm::mat4(1.0f);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if ((glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS))
    {
        State = GAME_PAUSE;
    }
    if ((glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS))
    {
        State = GAME_MENU;
        Scene = 1;
    }
    if ((glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS))
    {
        State = GAME_SCENETWO;
        Scene = 2;
    }
    if ((glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS))
    {
        State = GAME_ACTIVE;
        Scene = 3;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void RenderText(Shader& s, std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    s.use();
    glUniform3f(glGetUniformLocation(s.ID, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale; 
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void ShowInfo(Shader& s)
{
    RenderText(s, "Points:" + Menu.Points, 10.0f, SCR_HEIGHT - 100.0f, 1.0f, glm::vec3(1.7f, 0.0f, 1.11f));
    RenderText(s, "Lives:" + Menu.Lives, 10.0f, SCR_HEIGHT - 50.0f,  0.75f, glm::vec3(1.7f, 0.0f, 1.11f));
}

void GetDesktopResolution(float& horizontal, float& vertical)
{
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
