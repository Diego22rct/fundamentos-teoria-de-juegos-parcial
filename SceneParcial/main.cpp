#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <vector>

const int WIN_W = 800;
const int WIN_H = 600;

// =============================================================================
// 1. CÓDIGO DE LOS SHADERS (GLSL)
// =============================================================================
// Vertex Shader: Recibe las coordenadas de los vértices
const char* vertexShaderSource = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    void main() {
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    }
)";

// Fragment Shader: Aplica el color que le enviamos desde C++ (Uniform)
const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 u_Color; 
    void main() {
        FragColor = u_Color;
    }
)";

// Función auxiliar para compilar shaders y no ensuciar el main
GLuint CompileShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// =============================================================================
// MAIN
// =============================================================================
int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Escena Parcial", WIN_W, WIN_H, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext glContext = SDL_GL_CreateContext(window);

    glewExperimental = GL_TRUE;
    glewInit();

    // 1. Compilar el programa de Shaders
    GLuint shaderProgram = CompileShaders();

    // Obtener la ubicación del uniform "u_Color" para enviarle colores después
    GLint colorUniformLoc = glGetUniformLocation(shaderProgram, "u_Color");

    // =============================================================================
    // 2. DEFINIR GEOMETRÍA (Vértices)
    // =============================================================================

    // OBJETO 1: Cuerpo base de Pikachu (Un rectángulo compuesto por 2 triángulos)
    // Las coordenadas en OpenGL van de -1.0 a 1.0
    float cuerpoVertices[] = {
        // Primer triángulo
        -0.3f, -0.4f, // Abajo izquierda
         0.3f, -0.4f, // Abajo derecha
        -0.3f,  0.2f, // Arriba izquierda
        // Segundo triángulo
         0.3f, -0.4f, // Abajo derecha
         0.3f,  0.2f, // Arriba derecha
        -0.3f,  0.2f  // Arriba izquierda
    };

    GLuint VAO_Cuerpo, VBO_Cuerpo;
    glGenVertexArrays(1, &VAO_Cuerpo);
    glGenBuffers(1, &VBO_Cuerpo);

    glBindVertexArray(VAO_Cuerpo);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_Cuerpo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cuerpoVertices), cuerpoVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // =============================================================================
    // BUCLE PRINCIPAL
    // =============================================================================
    bool running = true;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.scancode == SDL_SCANCODE_ESCAPE) running = false;
        }

        // Color de fondo (Cielo)
        glClearColor(0.4f, 0.7f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Usar nuestro Shader
        glUseProgram(shaderProgram);

        // --- DIBUJAR OBJETO 1 (Cuerpo) ---
        // Enviamos el color Amarillo por Uniform (R, G, B, Alpha)
        glUniform4f(colorUniformLoc, 0.98f, 0.84f, 0.11f, 1.0f);
        glBindVertexArray(VAO_Cuerpo);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 6 vértices = 2 triángulos

        SDL_GL_SwapWindow(window);
    }

    // Limpieza de memoria
    glDeleteVertexArrays(1, &VAO_Cuerpo);
    glDeleteBuffers(1, &VBO_Cuerpo);
    glDeleteProgram(shaderProgram);
    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}