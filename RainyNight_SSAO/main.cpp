
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

// GLM Mathemtics
#define M_PI 3.14159265358979323846f
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include <SOIL.h>

#include <random> // necessary for generation of random floats (for sample kernel and noise texture)
#include <learnopengl/filesystem.h>

#include "AO.hpp"
#include "SSAmbientOcclusion.hpp"

// Properties
int screenWidth = 900;
int screenHeight = 500;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
void RenderQuad();
void drawModel(Model model, Shader shader, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

void createDepthBuffer();
void createPositionBuffer();
void createNormalBuffer();
void createColorBuffer();
//FBO
GLuint fboHandle;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
glm::mat4 projectionMatrix = glm::mat4();
glm::mat4 viewMatrix = glm::mat4();

// Delta
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Options
GLuint draw_mode = 1;

GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
{
    return a + f * (b - a);
}


// The MAIN function, from here we start our application and run our Game loop
int main()
{
    
    
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RainyNight", nullptr, nullptr); // Windowed
    glfwMakeContextCurrent(window);
    
    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();
    glGetError();
    
    // Define the viewport dimensions
    
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Setup some OpenGL options
    glEnable(GL_DEPTH_TEST);
    
    // Setup and compile our shaders
    Shader shader("shader/ssao_geometry.vs", "shader/ssao_geometry.frag");
    Shader shaderLightingPass("shader/ssao.vs", "shader/ssao_lighting.frag");
    Shader shaderSSAO("shader/ssao.vs", "shader/ssao.frag");
    Shader shaderSSAOBlur("shader/ssao.vs", "shader/ssao_blur.frag");
    Shader shaderTexture("shader/texture.vs","shader/texture.frag");
    
    // Set samplers
    shaderLightingPass.Use();
    glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "gColor"), 2);
    glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "ssao"), 3);
    shaderSSAO.Use();
    glUniform1i(glGetUniformLocation(shaderSSAO.Program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shaderSSAO.Program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shaderSSAO.Program, "texNoise"), 2);
    
    // Objects
    Model house("objects/house/Farmhouse.obj");
    
    // Lights
    glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
    glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);
    
    SSAmbienOcclusion *ssambientocclusion = new SSAmbienOcclusion(screenWidth,screenHeight);
    
    // Set up geometric Buffers
    
    
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    
    // - Position buffer
    ssambientocclusion->createPositionBuffer();
    // - Normal color buffer
    ssambientocclusion->createNormalBuffer();
    // - Albedo color buffer
    ssambientocclusion->createColorBuffer();
    
    // - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    ssambientocclusion->setAttachements();
    // - Create and attach depth buffer (renderbuffer)
    ssambientocclusion->createDepthBuffer();
    
    
    // Also create framebuffer to hold  processing stage
    ssambientocclusion->createSSAOFBO();
    
    ssambientocclusion->createNoiseTexture();
    
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check and call events
        glfwPollEvents();
        Do_Movement();
        
        
        projectionMatrix = glm::perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        viewMatrix = camera.GetViewMatrix();
        
        shader.Use();
        drawModel(house, shader, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.05f, 0.05f, 0.05f));
        
        
        
        // 2. Create SSAO texture
        glBindFramebuffer(GL_FRAMEBUFFER, ssambientocclusion->getSSAOFBO());
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAO.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getPositionBufferHandle());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getNormalBufferHandle());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getNoiseTexture());
        // Send kernel + rotation
        for (GLuint i = 0; i < 64; ++i)
            glUniform3fv(glGetUniformLocation(shaderSSAO.Program, ("samples[" + std::to_string(i) + "]").c_str()), 1, &ssambientocclusion->getSSAOKernel()[i][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderSSAO.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        RenderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        // 3. Blur SSAO texture to remove noise
        glBindFramebuffer(GL_FRAMEBUFFER, ssambientocclusion->getSSAOBlurFBO());
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAOBlur.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getSSAOColorBuffer());
        RenderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        
        // 4. Lighting Pass: traditional deferred Blinn-Phong lighting now with added screen-space ambient occlusion
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getPositionBufferHandle());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getNormalBufferHandle());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getColorBufferHandle());
        glActiveTexture(GL_TEXTURE3); // Add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssambientocclusion->getSSAOColorBufferBlur());
        // Also send light relevant uniforms
        glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(lightPos, 1.0));
        glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "light.Position"), 1, &lightPosView[0]);
        glUniform3fv(glGetUniformLocation(shaderLightingPass.Program, "light.Color"), 1, &lightColor[0]);
        // Update attenuation parameters
        const GLfloat constant = 1.0; // Note that we don't send this to the shader, we assume it is always 1.0 (in our case)
        const GLfloat linear = 0.09;
        const GLfloat quadratic = 0.032;
        glUniform1f(glGetUniformLocation(shaderLightingPass.Program, "light.Linear"), linear);
        glUniform1f(glGetUniformLocation(shaderLightingPass.Program, "light.Quadratic"), quadratic);
        glUniform1i(glGetUniformLocation(shaderLightingPass.Program, "draw_mode"), draw_mode);
        RenderQuad();
        
        
        // Swap the buffers
        glfwSwapBuffers(window);
    }
    
    glfwTerminate();
    return 0;
}


// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // Positions        // Texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

// RenderCube() Renders a 1x1 3D cube in NDC.
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;

bool keys[1024];
bool keysPressed[1024];
// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
    if (keys[GLFW_KEY_1])
        draw_mode = 1;
    if (keys[GLFW_KEY_2])
        draw_mode = 2;
    if (keys[GLFW_KEY_3])
        draw_mode = 3;
    if (keys[GLFW_KEY_4])
        draw_mode = 4;
}

GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    
    if (key >= 0 && key <= 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
        {
            keys[key] = false;
            keysPressed[key] = false;
        }
    }
}

void drawModel(Model model, Shader shader, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glm::mat4 location = glm::mat4();
    location = glm::translate(location, position);
    location = glm::scale(location, scale);
    location = glm::rotate(location, rotation.x * M_PI / 180.0f, glm::vec3(1, 0, 0));
    location = glm::rotate(location, rotation.y * M_PI / 180.0f, glm::vec3(0, 1, 0));
    location = glm::rotate(location, rotation.z * M_PI / 180.0f, glm::vec3(0, 0, 1));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(location));
    model.Draw(shader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
