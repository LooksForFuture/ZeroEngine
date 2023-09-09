# ZeroEngine
Zero is a game engine/framework written for c++ with simplicity and functionality in mind.<br />
Zero has been designed to be both user friendly and configurable. The only hard requirements are opengl and glm.<br /><br />
Special thanks to @royvandam for his <a href="https://github.com/royvandam/rtti">rtti</a> library.

## Structure
The engine class controls all entities and their components. And the pipelines manage extra functionality such as rendering, audio, ohysics and etc.<br />
Currently only render and physics pipeline have been implemented.

## Quickstart
I currently use GLAD for loading opengl and GLFW for window management in my examples. But the engine doesn't strictly require them and you can use any other library you want.
```python
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Zero/engine.h>

Engine gameEngine;

namespace Time {
    double scale = 1.0;
    double unscaledDt = 0.0;
    double dt = 0.0;
    double fixedDt = 1.0 / 120.0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main() {

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(800, 500, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true); // tell stb image to load textures normally
    gameEngine.init(); // init the game engine

    Entity* e = gameEngine.createEntity();

    glViewport(0, 0, 800, 500);

    double previousTime = glfwGetTime();
    double previous = previousTime;
    double lag = 0.0f;
    int frameCount = 0;
    while (!glfwWindowShouldClose(window)) {
        // Measure speed
        double currentTime = glfwGetTime();
        frameCount++;
        // If a second has passed.
        if (currentTime - previousTime >= 1.0)
        {
            // Display the frame count here any way you want.
            glfwSetWindowTitle(window, ("FPS: " + std::to_string(frameCount)).c_str());

            frameCount = 0;
            previousTime = currentTime;
        }

        Time::unscaledDt = (currentTime - previous);
        Time::dt = Time::scale * Time::unscaledDt;

        lag += Time::dt;
        // fixed update
        while (lag >= Time::fixedDt) {
            gameEngine.fixedUpdate();
            gameEngine.stepPhysics(Time::fixedDt);
            lag -= Time::fixedDt;
        }

        gameEngine.update();

        gameEngine.lateUpdate();
        gameEngine.clear();

        gameEngine.render();

        glfwSwapBuffers(window);
        glfwPollEvents();

        previous = currentTime;
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
```
