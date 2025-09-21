#include "WndProc.h"
#include "glad/glad.h"
#include "pixelFormat.h"
#include <stdio.h>

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static HGLRC RenderingContext;
    static GLuint VAO, VBO;

    switch (msg)
    {

    case WM_CREATE:
        hdc = GetDC(hWnd);
        pixelFormat(hdc);
        RenderingContext = wglCreateContext(hdc);
        wglMakeCurrent(hdc, RenderingContext);

        // Loading Shader

        FILE *frag = fopen("./shaders/mandelbrot.frag", "r");
        if (!frag)
        {
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

        const char *vertexShaderSource = "#version 330 core\nvoid main()\n{\ngl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n}\n)";

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, shaderSrc, shaderSize);
        glCompileShader(fragShader);
        free(shaderSrc);

        GLint success;
        glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
        }

        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertShader);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertShader);
        glAttachShader(shaderProgram, fragShader);
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        // Full Screen triangles

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        float vertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f};

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        break;

    case WM_PAINT:
        break;

    case WM_CLOSE:
        wglDeleteContext(RenderingContext);
        DeleteDC(hdc);
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