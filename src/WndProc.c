#include "WndProc.h"
#include "glad/glad.h"
#include "pixelFormat.h"
#include <stdio.h>
#include <math.h>

#define PI 3.1415

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static HGLRC RenderingContext;
    static GLuint VAO, VBO;
    // Equilateral triangle centered at origin, height = 1.0
    static float triangle[] = {
        // Position      // Color (R, G, B)
        0.0f, 0.5774f, 1.0f, 0.0f, 0.0f,   // Top vertex: Red
        -0.5f, -0.2887f, 0.0f, 1.0f, 0.0f, // Bottom left: Green
        0.5f, -0.2887f, 0.0f, 0.0f, 1.0f   // Bottom right: Blue
    };
    static GLuint shaderProgram;
    switch (msg)
    {

    case WM_CREATE:
        hdc = GetDC(hWnd);
        pixelFormat(hdc);
        RenderingContext = wglCreateContext(hdc);
        wglMakeCurrent(hdc, RenderingContext);

        // Loading Shader
        if (!gladLoadGL())
        {
            MessageBoxA(hWnd, "Failed to initialize OpenGL (glad)", "Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(1);
            return -1;
        }

        FILE *frag = fopen("../shaders/fragment.frag", "r");
        FILE *vert = fopen("../shaders/vertex.vert", "r");

        if (!frag || !vert)
        {
            MessageBoxA(hWnd, "Unable to find shader source", "Error", MB_OK | MB_ICONERROR);
            PostQuitMessage(1);
            return -1;
        }

        fseek(frag, 0, SEEK_END);
        long shaderSize = ftell(frag);
        fseek(frag, 0, SEEK_SET);
        char *shaderSrc = (char *)malloc(sizeof(char) * (shaderSize + 1));
        fread(shaderSrc, 1, shaderSize, frag);
        shaderSrc[shaderSize] = '\0';
        fclose(frag);

        // printf("%s\n", shaderSrc);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &shaderSrc, NULL);
        glCompileShader(fragShader);
        free(shaderSrc);

        GLint success;
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
            printf("Failed Fragment shader compile\n%s\n", infoLog);
        }

        fseek(vert, 0, SEEK_END);
        shaderSize = ftell(vert);
        fseek(vert, 0, SEEK_SET);
        char *vertexShaderSource = (char *)malloc(sizeof(char) * (shaderSize + 1));
        fread(vertexShaderSource, 1, shaderSize, vert);
        vertexShaderSource[shaderSize] = '\0';
        fclose(vert);

        // printf("%s\n", vertexShaderSource);

        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertShader);

        glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);

        free(vertexShaderSource);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(vertShader, 512, NULL, infoLog);
            printf("Failed Vertex shader compile\n%s\n", infoLog);
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertShader);
        glAttachShader(shaderProgram, fragShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        // Full Screen triangles

        // init
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Add data to VBo
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

        // add data to vao
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        break;

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        glViewport(0, 0, width, height);
    }

    break;

    case WM_PAINT:
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SwapBuffers(hdc);

        break;

    case WM_CLOSE:
        wglDeleteContext(RenderingContext);
        ReleaseDC(hWnd, hdc);
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}