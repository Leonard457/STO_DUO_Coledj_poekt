#define SDL_MAIN_USE_CALLBACKS 1
#include <iostream>
#include <vector>
#include <string>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

struct GridLine {
    SDL_FPoint start;
    SDL_FPoint end;
    int step;
};

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

static float center_x = 0, center_y = 0;
static float scale = 50.0f;
float dx = 5.0;
float kdx = 0.0;


std::vector<GridLine> x_axis_grid; // Вертикальные линии
std::vector<GridLine> y_axis_grid; // Горизонтальные линии

void DrawLabel(SDL_Renderer* rend, float x, float y, const std::string& text, Uint8 r = 0, Uint8 g = 0, Uint8 b = 0) {
    SDL_SetRenderDrawColor(rend, r, g, b, 255);
    SDL_RenderDebugText(rend, x, y, text.c_str());
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("Minkowski – Interaction", 840, 640, 0, &window, &renderer))
        return SDL_APP_FAILURE;

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    center_x = (float)w / 2.0f;
    center_y = (float)h / 2.0f;

    // Инициализация вертикальных линий (X-сетка)
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    for (int i = -10; i <= 10; ++i) {
        if (i == 0) continue;
        GridLine line;
        line.step = i;
        line.start = { center_x + (i * scale), 0 };
        line.end   = { center_x + (i * scale), (float)h };
        x_axis_grid.push_back(line);
    }

    // Инициализация горизонтальных линий (Y-сетка)
    for (int i = -10; i <= 10; ++i) {
        if (i == 0) continue;
        GridLine line;
        line.step = i;
        line.start = { 0, center_y - (i * scale) };
        line.end   = { (float)w, center_y - (i * scale) };
        y_axis_grid.push_back(line);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        //switch (event->key.key) { }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    int w, h;
    SDL_GetCurrentRenderOutputSize(renderer, &w, &h);

    { // скелет основной оси
        // 1. Отрисовка X-сетки (Вертикальные линии)
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        for (const auto& line : x_axis_grid) {
            // Пример взаимодействия: линии наклоняются или смещаются по X
            float shift = line.step; 
            SDL_RenderLine(renderer, line.start.x + shift, line.start.y, 
                                     line.end.x + shift, line.end.y);
            
            DrawLabel(renderer, line.start.x + shift + 5, center_y + 5, std::to_string(line.step), 150, 150, 150);
        }

        // 2. Отрисовка Y-сетки (Горизонтальные линии)
        for (const auto& line : y_axis_grid) {
            // Линии Y просто рисуются по своим точкам
            SDL_RenderLine(renderer, line.start.x, line.start.y, 
                                     line.end.x, line.end.y);
            
            DrawLabel(renderer, center_x + 5, line.start.y - 12, std::to_string(line.step), 150, 150, 150);
        }
    }

    // 3. Главные оси
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderLine(renderer, center_x, 0, center_x, (float)h);
    SDL_RenderLine(renderer, 0, center_y, (float)w, center_y);

    // Стрелочки
    SDL_RenderLine(renderer, (float)w - 15, center_y - 5, (float)w, center_y);
    SDL_RenderLine(renderer, (float)w - 15, center_y + 5, (float)w, center_y);
    SDL_RenderLine(renderer, center_x - 5, 15, center_x, 0);
    SDL_RenderLine(renderer, center_x + 5, 15, center_x, 0);

    // Метки осей
    DrawLabel(renderer, (float)w - 20, center_y - 20, "X", 255, 0, 0);
    DrawLabel(renderer, center_x + 10, 10, "ct", 255, 0, 0);
    DrawLabel(renderer, center_x - 12, center_y + 8, "0", 0, 0, 0);

    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    SDL_Quit();
}