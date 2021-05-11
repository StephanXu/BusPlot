
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmrc/cmrc.hpp>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <thread>
#include <chrono>
#include <sstream>

#include "gl.hpp"
#include "series.hpp"
#include "shader.hpp"
#include "chart.hpp"

CMRC_DECLARE(resources);

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "BusPlot", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // NOTE: OpenGL error checks have been omitted for brevity

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    auto resourcesFS = cmrc::resources::get_filesystem();
    auto seriesVertexShaderFile = resourcesFS.open("shaders/vertex.glsl");
    auto fragmentShaderFile = resourcesFS.open("shaders/fragment.glsl");
    auto textVertexShaderFile = resourcesFS.open("shaders/text_vertex.glsl");
    auto textFragmentShaderFile = resourcesFS.open("shaders/text_fragment.glsl");
    auto seriesShader = ShaderLoader()
            .AddShader(GL_VERTEX_SHADER, {seriesVertexShaderFile.begin(), seriesVertexShaderFile.end()})
            .AddShader(GL_FRAGMENT_SHADER, {fragmentShaderFile.begin(), fragmentShaderFile.end()})
            .Done();
    auto textShader = ShaderLoader()
            .AddShader(GL_VERTEX_SHADER, {textVertexShaderFile.begin(), textVertexShaderFile.end()})
            .AddShader(GL_FRAGMENT_SHADER, {textFragmentShaderFile.begin(), textFragmentShaderFile.end()})
            .Done();

    auto chart = Chart::MakeChart(seriesShader,
                                  textShader,
                                  "asset/Roboto-Regular.ttf",
                                  false);

    auto s1 = std::make_shared<Series>();
    s1->SetColor(glm::vec3(0.f, 1.0, 0.f));
    auto s2 = std::make_shared<Series>();
    s2->SetColor(glm::vec3(0.f, 1.f, 1.f));

    chart->AddSeries("series 1", s1);
    chart->AddSeries("series 2", s2);
    chart->SetPosition({200, 200});

    std::thread([&]() {
        while (!glfwWindowShouldClose(window)) {
            auto t = std::chrono::time_point_cast<Duration>(Clock::now());
            auto s = static_cast<double>(t.time_since_epoch().count());
            s1->AddData(Dot{t.time_since_epoch().count(), static_cast<float>(50.f * glm::sin(s / 400000.f) + 10.f)});
            s2->AddData(Dot{t.time_since_epoch().count(), static_cast<float>(10.f * glm::cos(s / 200000.f) + 10.f)});
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }).detach();


    while (!glfwWindowShouldClose(window)) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glm::dmat4 mvp = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), 1.f, -1.f);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        seriesShader->Activate();
        seriesShader->SetMatrix4d("projection", mvp);
        textShader->Activate();
        textShader->SetMatrix4d("projection", mvp);

        chart->SetSize({width - 200 * 2, height - 200 * 2});
        chart->Render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}