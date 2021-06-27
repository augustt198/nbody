#include "graphics.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shaders.h"

GLFWwindow *window = NULL;

GLuint buf;
GLuint shader_program = 0;
GLint mvp_loc;

void glfw_error_cb(int error, const char *desc) {
    fprintf(stderr, "glfw error: %s\n", desc);
}

void glfw_key_cb(GLFWwindow *win, int key, int keycode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, GLFW_TRUE);
    }
}

bool graphics_init(simulation_setup_t *sim) {
    glfwSetErrorCallback(glfw_error_cb);

    int major, minor, rev;
    glfwGetVersion(&major, &minor, &rev);
    printf("GLFW %d.%d.%d\n", major, minor, rev);

    if (!glfwInit()) {
        fprintf(stderr, "Couldn't init glfw\n");
        return false;
    }
    
    window = glfwCreateWindow(640, 480, "N-Body Simulation", NULL, NULL);
    if (!window) {
        return false;
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "Couldn't init glew: %s\n", glewGetErrorString(err));
        return false;
    }

    glfwSwapInterval(1);
    glfwSetKeyCallback(window, glfw_key_cb);

    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);

    glBufferData(GL_ARRAY_BUFFER, sim->n_points * sizeof(point_mass_t), (void*) &sim->points[0], GL_DYNAMIC_DRAW);

    //
    // Shader stuff
    // 
    GLuint vertex_shader, fragment_shader;
    GLint is_compiled;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {
        GLint len = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &len);
        char strbuf[len];
        glGetShaderInfoLog(vertex_shader, len, &len, (GLchar*) strbuf);
        fprintf(stderr, "Vertex shader: %.*s\n", len, strbuf);
        return false;
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &is_compiled);
    if (is_compiled == GL_FALSE) {
        GLint len = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &len);
        char strbuf[len];
        glGetShaderInfoLog(fragment_shader, len, &len, (GLchar*) strbuf);
        fprintf(stderr, "Fragment shader: %.*s\n", len, strbuf);
        return false;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    GLint pos_loc, clr_loc;
    mvp_loc = glGetUniformLocation(shader_program, "mvp");
    pos_loc = glGetAttribLocation(shader_program, "pos");
    clr_loc = glGetAttribLocation(shader_program, "in_color");

    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,
         sizeof(point_mass_t), (void*) 0);
    //glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE,
    //    6*sizeof(float), (void*) 0);

    glEnableVertexAttribArray(clr_loc);
    glVertexAttribPointer(clr_loc, 3, GL_FLOAT, GL_FALSE,
        sizeof(point_mass_t), (void*) (4 * sizeof(float)));

    //glEnable(GL_POINT_SMOOTH); // no squares thanks!
    glEnable(GL_POINT_SPRITE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    return true;
}

float ss = 0.0;

bool graphics_update(simulation_setup_t *sim) {
    if (glfwWindowShouldClose(window)) return false;

    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sim->n_points * sizeof(point_mass_t), (void*) &sim->points[0]);
    float *thing = (float*) &sim->points[0];

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1, 0.0, 0.1, 1.0);

    glUseProgram(shader_program);
    //glPointSize(10.0);

    float s = 0.1;
    ss += 0.005;
    float si = sinf(ss), co = cosf(ss);

    const GLfloat mvp[16] = {s*co, 0.0, -s*si, 0.0,
                            0.0, s, 0.0, 0.0,
                            s*si, 0.0, s*co, 0.0,
                            0.0, 0.0, 0.0, 1.0};
    glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, mvp);
    glDrawArrays(GL_POINTS, 0, sim->n_points);

    glfwSwapBuffers(window);
    glfwPollEvents();

    return true;
}

void graphics_exit() {
    if (window) {
        glfwDestroyWindow(window);
    }

    glfwTerminate();
}
