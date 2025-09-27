#define STB_IMAGE_IMPLEMENTATION

#include "WndProc.h"
#include "glad/glad.h"
#include "pixelFormat.h"
#include <stdio.h>
#include <math.h>

#include "stb_image.h"

#define PI 3.1415

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HDC hdc;
    static HGLRC RenderingContext;
    static GLuint VAO, VBO, EBO;
    static GLuint texture = 0;

    static char *img = NULL;
    static int width, height, components;

    static float cube[] = {
        // Front face          // Color        // Texture
        -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 0: Front top left
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // 1: Front top right
        -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, // 2: Front bottom left
        0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // 3: Front bottom right
        0.8f, -0.8f, 0.0f, 1.0f, 1.0f, 1.0f, 0.8f, 0.0f,  // 3: Front bottom right
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // 3: Front bottom right
        1.0f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,  // 3: Front bottom right
    };

    static int cubeE[] = {
        // Front face
        0, 1, 2, // a
        1, 3, 2, // b
        4, 5, 6};
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
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        // Add data to VBo
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeE), cubeE, GL_STATIC_DRAW);
        // add data to vao
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Texture generation
        img = stbi_load("../texture/wood.jpg", &width, &height, &components, 0);

        if (!img)
        {
            printf("\x1b[38;2;200;40;0mError: loading texture image\x1b[0m");
            return 1;
        }

        glGenTextures(1, &texture);
        glActiveTexture(texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLenum srcFormat = GL_RGB;
        GLenum internalFormat = GL_RGB;
        if (components == 4)
        {
            srcFormat = GL_RGBA;
            internalFormat = GL_RGBA;
        }
        else if (components == 1)
        {
            srcFormat = GL_RED;
            internalFormat = GL_R8;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, srcFormat, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);

        GLuint tex0Uni = glGetUniformLocation(shaderProgram, "tex0");
        glUniform1i(tex0Uni, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_image_free(img);

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
        glBindTexture(GL_TEXTURE_2D, texture);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
        SwapBuffers(hdc);

        break;

    case WM_CLOSE:
        glDeleteTextures(1, &texture);
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