#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // MUY IMPORTANTE EN SDL3
#include <iostream>
#include <cmath>

// Shaders 
const char* vertexSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;

    uniform vec2 offset;   // posición en pantalla
    uniform vec2 escala;   // tamaño del rectángulo

    void main() {
        vec3 pos = aPos;
        pos.x = pos.x * escala.x + offset.x;
        pos.y = pos.y * escala.y + offset.y;
        gl_Position = vec4(pos, 1.0);
    }
)";

const char* fragmentSrc = R"(
    #version 330 core
    out vec4 FragColor;

    uniform vec3 color;    // color del objeto

    void main() {
        FragColor = vec4(color, 1.0);
    }
)";

// Compilar shader 
GLuint compilarShader(GLenum tipo, const char* src) {
    GLuint shader = glCreateShader(tipo);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        SDL_Log("Error shader: %s", log);
    }
    return shader;
}

int main(int argc, char* argv[]) {

    // Init
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* win = SDL_CreateWindow("Escena Parcial - UNDERTALE", 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    SDL_GL_SetSwapInterval(1); // VSync ON

    glewExperimental = GL_TRUE;
    glewInit();

    // Geometría base (Cuadrado 1x1 centrado)
    float verticesBase[] = {
        -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,

         -0.5f, -0.5f, 0.0f,
          0.5f, -0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
    };

    GLuint vbo, vao;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesBase), verticesBase, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Shader program 
    GLuint vert = compilarShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint frag = compilarShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    glDeleteShader(vert);
    glDeleteShader(frag);

    GLint locOffset = glGetUniformLocation(prog, "offset");
    GLint locEscala = glGetUniformLocation(prog, "escala");
    GLint locColor  = glGetUniformLocation(prog, "color");

    //Loop 
    bool running = true;
    SDL_Event ev;

    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
            if (ev.type == SDL_EVENT_KEY_DOWN && ev.key.scancode == SDL_SCANCODE_ESCAPE) running = false;
        }

        // Fondo Negro
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(vao);

        float tiempo = SDL_GetTicks() / 1000.0f;

        // --- SISTEMA DE DIBUJO (Múltiples Draw Calls Reutilizando el Cuadrado) ---
        // Función Lambda local para facilitar dibujar rectángulos rápidamente
        auto DrawRect = [&](float x, float y, float w, float h, float r, float g, float b) {
            glUniform2f(locOffset, x, y);
            glUniform2f(locEscala, w, h);
            glUniform3f(locColor, r, g, b);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        };

        // 1. CAJA DE COMBATE (Bordes Blancos)
        float boxY = -0.2f;
        float bxW = 0.6f;
        float bxH = 0.6f;
        float thick = 0.02f; // Grosor
        DrawRect(0.0f, boxY - (bxH/2.0f), bxW, thick,  1.0f, 1.0f, 1.0f); // Abajo
        DrawRect(0.0f, boxY + (bxH/2.0f), bxW, thick,  1.0f, 1.0f, 1.0f); // Arriba
        DrawRect(-(bxW/2.0f), boxY, thick, bxH + thick, 1.0f, 1.0f, 1.0f); // Izquierda
        DrawRect( (bxW/2.0f), boxY, thick, bxH + thick, 1.0f, 1.0f, 1.0f); // Derecha

        // 2. BARRA DE VIDA
        DrawRect(0.0f, -0.6f, 0.4f, 0.05f,  0.8f, 0.0f, 0.0f);  // Fondo Rojo
        DrawRect(-0.1f, -0.6f, 0.2f, 0.05f, 1.0f, 1.0f, 0.0f);  // Vida Actual Amarilla

        // 3. SANS (Cabeza base)
        DrawRect(0.0f,  0.45f, 0.35f, 0.35f, 1.0f, 1.0f, 1.0f); // Cara Blanca
        DrawRect(0.0f,  0.35f, 0.2f,  0.05f, 0.0f, 0.0f, 0.0f); // Boca Negra
        DrawRect(-0.08f,0.48f, 0.08f, 0.08f, 0.0f, 0.0f, 0.0f); // Ojo Izq Negro

        // 4. OJO DERECHO (Punto extra Animado)
        float brilloAzul = (sin(tiempo * 5.0f) + 1.0f) / 2.0f; 
        DrawRect(0.08f, 0.48f, 0.08f, 0.08f, 0.0f, brilloAzul, 1.0f);

        // 5. HUESOS (Punto extra Animado)
        float movX = cos(tiempo * 3.0f) * 0.2f;
        DrawRect(movX - 0.1f, boxY, 0.03f, 0.2f, 0.9f, 0.9f, 0.9f);
        DrawRect(movX + 0.1f, boxY, 0.03f, 0.2f, 0.9f, 0.9f, 0.9f);

        // 6. ALMA (Corazón del Jugador)
        float heartY = boxY + (sin(tiempo * 4.0f) * 0.05f);
        DrawRect(0.0f,   heartY,        0.1f,  0.08f, 1.0f, 0.0f, 0.0f); // Base
        DrawRect(-0.03f, heartY+0.04f, 0.04f, 0.04f, 1.0f, 0.0f, 0.0f); // Oreja Izq
        DrawRect( 0.03f, heartY+0.04f, 0.04f, 0.04f, 1.0f, 0.0f, 0.0f); // Oreja Der
		// 7. BOTONES BASE
        float btnY = -0.82f;
        float btnW = 0.20f;
        float btnH = 0.10f;
        float espaciado = 0.24f;
        float rN = 1.0f, gN = 0.5f, bN = 0.0f; 

        for (int i = 0; i < 4; i++) {
            float posX = -0.36f + (i * espaciado);

            auto DrawFrame = [&](float x, float y, float w, float h, float th) {
                DrawRect(x, y - h / 2, w, th, rN, gN, bN); // Abajo
                DrawRect(x, y + h / 2, w, th, rN, gN, bN); // Arriba
                DrawRect(x - w / 2, y, th, h, rN, gN, bN); // Izquierda
                DrawRect(x + w / 2, y, th, h, rN, gN, bN); // Derecha
                };

            DrawFrame(posX, btnY, btnW, btnH, 0.01f);

            // Espada con triangulo y cruz
            if (i == 0) {
                DrawRect(posX - 0.05f, btnY, 0.015f, 0.06f, rN, gN, bN);
                DrawRect(posX - 0.05f, btnY - 0.02f, 0.04f, 0.01f, rN, gN, bN);
            }

            // X con cruz girada
            if (i == 3) {
                // Dos rectángulos rotados o cruzados para la X
                DrawRect(posX - 0.05f, btnY, 0.01f, 0.05f, rN, gN, bN);
                DrawRect(posX - 0.05f, btnY, 0.05f, 0.01f, rN, gN, bN);
            }
       
        }
        SDL_GL_SwapWindow(win);
    }

    // Limpieza?
    glDeleteProgram(prog);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    SDL_GL_DestroyContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}