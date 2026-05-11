#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h> // MUY IMPORTANTE EN SDL3
#include <iostream>
#include <cmath>
#include <string>

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

// ============================================================
// FUENTE PIXEL 3x5 - Para dibujar letras con cuadrados
// Cada glifo se define como 15 caracteres ('#' encendido, '.' vacio)
// ============================================================
const char* GetGlyph(char c) {
    switch (c) {
    case 'A': return "###" "#.#" "###" "#.#" "#.#";
    case 'B': return "##." "#.#" "##." "#.#" "##.";
    case 'C': return "###" "#.." "#.." "#.." "###";
    case 'D': return "##." "#.#" "#.#" "#.#" "##.";
    case 'E': return "###" "#.." "##." "#.." "###";
    case 'F': return "###" "#.." "##." "#.." "#..";
    case 'G': return "###" "#.." "#.#" "#.#" "###";
    case 'H': return "#.#" "#.#" "###" "#.#" "#.#";
    case 'I': return "###" ".#." ".#." ".#." "###";
    case 'J': return "###" "..#" "..#" "#.#" "###";
    case 'K': return "#.#" "#.#" "##." "#.#" "#.#";
    case 'L': return "#.." "#.." "#.." "#.." "###";
    case 'M': return "#.#" "###" "###" "#.#" "#.#";
    case 'N': return "#.#" "###" "###" "###" "#.#";
    case 'O': return "###" "#.#" "#.#" "#.#" "###";
    case 'P': return "###" "#.#" "###" "#.." "#..";
    case 'Q': return "###" "#.#" "#.#" "###" "..#";
    case 'R': return "###" "#.#" "##." "#.#" "#.#";
    case 'S': return "###" "#.." "###" "..#" "###";
    case 'T': return "###" ".#." ".#." ".#." ".#.";
    case 'U': return "#.#" "#.#" "#.#" "#.#" "###";
    case 'V': return "#.#" "#.#" "#.#" "#.#" ".#.";
    case 'W': return "#.#" "#.#" "###" "###" "#.#";
    case 'X': return "#.#" "#.#" ".#." "#.#" "#.#";
    case 'Y': return "#.#" "#.#" ".#." ".#." ".#.";
    case 'Z': return "###" "..#" ".#." "#.." "###";
    case '0': return "###" "#.#" "#.#" "#.#" "###";
    case '1': return ".#." "##." ".#." ".#." "###";
    case '2': return "###" "..#" "###" "#.." "###";
    case '3': return "###" "..#" "###" "..#" "###";
    case '4': return "#.#" "#.#" "###" "..#" "..#";
    case '5': return "###" "#.." "###" "..#" "###";
    case '6': return "###" "#.." "###" "#.#" "###";
    case '7': return "###" "..#" "..#" "..#" "..#";
    case '8': return "###" "#.#" "###" "#.#" "###";
    case '9': return "###" "#.#" "###" "..#" "###";
    case '/': return "..#" "..#" ".#." "#.." "#..";
    case ' ': return "..." "..." "..." "..." "...";
    default:  return "###" "###" "###" "###" "###";
    }
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
    GLint locColor = glGetUniformLocation(prog, "color");

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

        // Lambdas para el sistema de texto pixel 3x5
        auto MedirTexto = [](const std::string& s, float px) {
            if (s.empty()) return 0.0f;
            return (4.0f * s.size() - 1.0f) * px;
            };
        auto DrawChar = [&](char c, float cx, float cy, float px,
            float r, float g, float b) {
                const char* gph = GetGlyph(c);
                for (int fila = 0; fila < 5; fila++) {
                    for (int col = 0; col < 3; col++) {
                        if (gph[fila * 3 + col] == '#') {
                            float x = cx + (col - 1) * px;
                            float y = cy + (2 - fila) * px;
                            DrawRect(x, y, px, px, r, g, b);
                        }
                    }
                }
            };
        auto DrawTextLeft = [&](const std::string& s, float xIzq, float y, float px,
            float r, float g, float b) {
                float step = 4.0f * px;
                for (size_t i = 0; i < s.size(); i++) {
                    float cx = xIzq + i * step + px;
                    DrawChar(s[i], cx, y, px, r, g, b);
                }
            };

        // 1. CAJA DE COMBATE (Bordes Blancos)
        float boxY = -0.2f;
        float bxW = 0.6f;
        float bxH = 0.6f;
        float thick = 0.02f; // Grosor
        DrawRect(0.0f, boxY - (bxH / 2.0f), bxW, thick, 1.0f, 1.0f, 1.0f); // Abajo
        DrawRect(0.0f, boxY + (bxH / 2.0f), bxW, thick, 1.0f, 1.0f, 1.0f); // Arriba
        DrawRect(-(bxW / 2.0f), boxY, thick, bxH + thick, 1.0f, 1.0f, 1.0f); // Izquierda
        DrawRect((bxW / 2.0f), boxY, thick, bxH + thick, 1.0f, 1.0f, 1.0f); // Derecha

        float sansX = 0.0f;
        float sansY = 0.55f;

        // 2. SANS (Cabeza base)

        // Cráneo apilado
        DrawRect(sansX, sansY + 0.18f, 0.16f, 0.04f, 1.0f, 1.0f, 1.0f); // top craneo
        DrawRect(sansX, sansY + 0.14f, 0.22f, 0.04f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX, sansY + 0.09f, 0.26f, 0.06f, 1.0f, 1.0f, 1.0f); // cara central
        DrawRect(sansX, sansY + 0.03f, 0.24f, 0.06f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX, sansY - 0.01f, 0.20f, 0.04f, 1.0f, 1.0f, 1.0f); // mandibula

        // Boca negra + dientes blancos
        DrawRect(sansX, sansY + 0.005f, 0.10f, 0.015f, 0.0f, 0.0f, 0.0f);
        DrawRect(sansX - 0.03f, sansY + 0.005f, 0.005f, 0.015f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX, sansY + 0.005f, 0.005f, 0.015f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX + 0.03f, sansY + 0.005f, 0.005f, 0.015f, 1.0f, 1.0f, 1.0f);

        // Ojo Izq Negro
        DrawRect(sansX - 0.06f, sansY + 0.10f, 0.05f, 0.06f, 0.0f, 0.0f, 0.0f);

        // 3. OJO DERECHO (Punto extra Animado)
        float brilloAzul = (sin(tiempo * 5.0f) + 1.0f) / 2.0f;
        DrawRect(sansX + 0.06f, sansY + 0.10f, 0.05f, 0.06f, 0.0f, 0.0f, 0.0f);
        DrawRect(sansX + 0.06f, sansY + 0.10f, 0.025f, 0.03f, 1.0f, brilloAzul, 0.0f);

        // Cuerpo
        DrawRect(sansX, sansY - 0.10f, 0.30f, 0.13f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX, sansY - 0.18f, 0.34f, 0.05f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX, sansY - 0.10f, 0.06f, 0.10f, 0.0f, 0.0f, 0.0f);
        DrawRect(sansX, sansY - 0.10f, 0.04f, 0.10f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX - 0.03f, sansY - 0.06f, 0.01f, 0.04f, 0.0f, 0.0f, 0.0f);
        DrawRect(sansX + 0.03f, sansY - 0.06f, 0.01f, 0.04f, 0.0f, 0.0f, 0.0f);

        // Brazos
        DrawRect(sansX - 0.18f, sansY - 0.10f, 0.05f, 0.14f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX + 0.18f, sansY - 0.10f, 0.05f, 0.14f, 1.0f, 1.0f, 1.0f);

        // Piernas
        DrawRect(sansX - 0.06f, sansY - 0.25f, 0.07f, 0.10f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX + 0.06f, sansY - 0.25f, 0.07f, 0.10f, 1.0f, 1.0f, 1.0f);

        // Pies
        DrawRect(sansX - 0.06f, sansY - 0.32f, 0.09f, 0.04f, 1.0f, 1.0f, 1.0f);
        DrawRect(sansX + 0.06f, sansY - 0.32f, 0.09f, 0.04f, 1.0f, 1.0f, 1.0f);

        // 4. HUESOS (Punto extra Animado)
        float movX = cos(tiempo * 3.0f) * 0.2f;
        DrawRect(movX - 0.1f, boxY, 0.03f, 0.2f, 0.9f, 0.9f, 0.9f);
        DrawRect(movX + 0.1f, boxY, 0.03f, 0.2f, 0.9f, 0.9f, 0.9f);

        // 5. ALMA (Corazón del Jugador)
        float heartY = boxY + (sin(tiempo * 4.0f) * 0.05f);
        {
            const char* heart[6] = {
                ".##.##.",
                "#######",
                "#######",
                ".#####.",
                "..###..",
                "...#..."
            };
            float hpx = 0.013f; // tamaño de cada pixel del corazón
            float hcx = 0.0f;
            for (int fila = 0; fila < 6; fila++) {
                for (int col = 0; col < 7; col++) {
                    if (heart[fila][col] == '#') {
                        float x = hcx + (col - 3) * hpx;
                        float y = heartY + (2.5f - fila) * hpx;
                        DrawRect(x, y, hpx, hpx, 1.0f, 0.0f, 0.0f);
                    }
                }
            }
        }

        // 6. BOTONES BASE (FIGHT / ACT / ITEM / MERCY)
        float btnY = -0.82f;
        float btnW = 0.20f;
        float btnH = 0.10f;
        float espaciado = 0.24f;
        float rN = 1.0f, gN = 0.5f, bN = 0.0f;

        const char* etiquetas[4] = { "FIGHT", "ACT", "ITEM", "MERCY" };
        float bpx = 0.007f;
        float iconAncho = 0.035f;
        float iconGap = 0.012f;

        for (int i = 0; i < 4; i++) {
            float posX = -0.36f + (i * espaciado);

            auto DrawFrame = [&](float x, float y, float w, float h, float th) {
                DrawRect(x, y - h / 2, w, th, rN, gN, bN); // Abajo
                DrawRect(x, y + h / 2, w, th, rN, gN, bN); // Arriba
                DrawRect(x - w / 2, y, th, h, rN, gN, bN); // Izquierda
                DrawRect(x + w / 2, y, th, h, rN, gN, bN); // Derecha
                };

            DrawFrame(posX, btnY, btnW, btnH, 0.01f);

            // Centrado del contenido
            std::string et = etiquetas[i];
            float wTxt = MedirTexto(et, bpx);
            float wContenido = iconAncho + iconGap + wTxt;
            float xIzq = posX - wContenido / 2.0f;
            float iconCX = xIzq + iconAncho / 2.0f;
            float txtIzq = xIzq + iconAncho + iconGap;

            // Espada con triangulo y cruz (FIGHT)
            if (i == 0) {
                DrawRect(iconCX, btnY + 0.005f, 0.012f, 0.045f, rN, gN, bN);
                DrawRect(iconCX, btnY - 0.015f, 0.030f, 0.008f, rN, gN, bN);
            }
            // Bocina (ACT)
            if (i == 1) {
                DrawRect(iconCX - 0.005f, btnY, 0.018f, 0.028f, rN, gN, bN);
                DrawRect(iconCX + 0.010f, btnY + 0.005f, 0.006f, 0.008f, rN, gN, bN);
                DrawRect(iconCX + 0.018f, btnY + 0.012f, 0.006f, 0.008f, rN, gN, bN);
            }
            // Bolsa de item (ITEM)
            if (i == 2) {
                DrawRect(iconCX, btnY - 0.005f, 0.028f, 0.028f, rN, gN, bN);
                DrawRect(iconCX, btnY + 0.018f, 0.010f, 0.010f, rN, gN, bN);
            }
            // X con cruz girada (MERCY)
            if (i == 3) {
                DrawRect(iconCX, btnY, 0.008f, 0.035f, rN, gN, bN);
                DrawRect(iconCX, btnY, 0.035f, 0.008f, rN, gN, bN);
            }

            // Texto del botón usando la fuente pixel
            DrawTextLeft(et, txtIzq, btnY, bpx, rN, gN, bN);
        }

        // 7. HUD (PLAYER  LV 19  HP[barra]  KR 01/92)
        {
            float hudY = -0.62f;
            float tpx = 0.007f;

            std::string s1 = "PLAYER";
            std::string s2 = "LV 19";
            std::string s3 = "HP";
            std::string s4 = "KR";
            std::string s5 = "01/92";

            float gap = 0.04f;
            float barW = 0.16f;
            float barH = 5.0f * tpx;

            float w1 = MedirTexto(s1, tpx);
            float w2 = MedirTexto(s2, tpx);
            float w3 = MedirTexto(s3, tpx);
            float w4 = MedirTexto(s4, tpx);
            float w5 = MedirTexto(s5, tpx);

            float anchoTotal = w1 + gap + w2 + gap + w3 + 0.012f + barW + gap + w4 + gap + w5;
            float xCursor = -anchoTotal / 2.0f;

            DrawTextLeft(s1, xCursor, hudY, tpx, 1, 1, 1);
            xCursor += w1 + gap;
            DrawTextLeft(s2, xCursor, hudY, tpx, 1, 1, 1);
            xCursor += w2 + gap;
            DrawTextLeft(s3, xCursor, hudY, tpx, 1, 1, 1);
            xCursor += w3 + 0.012f;

            // BARRA DE VIDA
            float barCX = xCursor + barW / 2.0f;
            DrawRect(barCX, hudY, barW, barH, 1.0f, 1.0f, 0.0f);
            DrawRect(barCX - barW * 0.15f, hudY, barW * 0.70f, barH, 0.8f, 0.0f, 0.0f);
            xCursor += barW + gap;

            DrawTextLeft(s4, xCursor, hudY, tpx, 1, 1, 1);
            xCursor += w4 + gap;
            DrawTextLeft(s5, xCursor, hudY, tpx, 1, 1, 1);
        }

        SDL_GL_SwapWindow(win);
    }

    // Limpieza
    glDeleteProgram(prog);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    SDL_GL_DestroyContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}