#define SDL_MAIN_USE_CALLBACKS 1
#include <iostream>
#include <algorithm>
#include <cmath>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "physics/STO.h"

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;

const float c = 200.0f;
const int grid_half = 300;
const int step = 20;

const float v_max = c * 0.95f;
const float period_sec = 20.0f;

static float slowdown = 0.8f;        // сила замедления в центре (0 = чистый синус, 1 = полная остановка)
static Uint64 start_ticks = 0;
static int center_x = 0, center_y = 0;

// Плавное смешивание без нулевой производной в центре
static float compute_velocity(float raw_sin) {
    const float abs_raw = fabsf(raw_sin);
    const float sign_raw = (raw_sin >= 0.0f) ? 1.0f : -1.0f;

    // Степенной профиль (например квадратичный) для крутых краёв
    const float power = 2.0f;
    const float shaped = sign_raw * powf(abs_raw, power);

    // Насколько сильно замедляем центр: 0..1 плавно нарастает от нуля
    // Но чтобы не было остановки, не даём ему дойти до 1 при raw=0
    const float threshold = 0.3f;
    float t = (abs_raw < threshold) ? (abs_raw / threshold) : 1.0f;
    // Гладкая ступенька smoothstep
    t = t * t * (3.0f - 2.0f * t);
    // map slowdown: при slowdown=0 mix всегда 0 (линейный), при slowdown=1 mix в центре = 0.8 (сильное замедление, но не остановка)
    float mix_center = slowdown * 0.8f;          // максимальная примесь формы в нуле
    float mix = mix_center * (1.0f - t);        // к краям mix падает до 0 -> возвращается к чистому shaped

    // Итоговая скорость
    float v = v_max * (shaped * (1.0f - mix) + raw_sin * mix);
    return v;
}

void DrawMinkowskiGrid(SDL_Renderer* r, float v) {
    SDL_SetRenderDrawColor(r, 80, 120, 200, 255);
    for (int x = -grid_half; x <= grid_half; x += step) {
        for (int t = -grid_half; t < grid_half; t += step) {
            auto p1 = sto::lorentz_transform(t, x, v, c);
            auto p2 = sto::lorentz_transform(t + step, x, v, c);
            SDL_RenderLine(r,
                center_x + (float)p1.x, center_y - (float)p1.t,
                center_x + (float)p2.x, center_y - (float)p2.t);
        }
    }
    SDL_SetRenderDrawColor(r, 200, 80, 80, 255);
    for (int t = -grid_half; t <= grid_half; t += step) {
        for (int x = -grid_half; x < grid_half; x += step) {
            auto p1 = sto::lorentz_transform(t, x, v, c);
            auto p2 = sto::lorentz_transform(t, x + step, v, c);
            SDL_RenderLine(r,
                center_x + (float)p1.x, center_y - (float)p1.t,
                center_x + (float)p2.x, center_y - (float)p2.t);
        }
    }
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderLine(r, center_x - grid_half, center_y, center_x + grid_half, center_y);
    SDL_RenderLine(r, center_x, center_y - grid_half, center_x, center_y + grid_half);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) return SDL_APP_FAILURE;
    if (!SDL_CreateWindowAndRenderer("Minkowski – smooth slowdown", 840, 640, 0, &window, &renderer))
        return SDL_APP_FAILURE;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    center_x = w / 2;
    center_y = h / 2;
    start_ticks = SDL_GetTicks();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.scancode == SDL_SCANCODE_UP) {
            slowdown = std::min(slowdown + 0.1f, 1.0f);
        } else if (event->key.scancode == SDL_SCANCODE_DOWN) {
            slowdown = std::max(slowdown - 0.1f, 0.0f);
        }
        char title[128];
        snprintf(title, sizeof(title), "Slowdown = %.1f (↑↓), 0=fast center, 1=strong pause", slowdown);
        SDL_SetWindowTitle(window, title);
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
    Uint64 now = SDL_GetTicks();
    float elapsed = (now - start_ticks) / 1000.0f;
    float raw_sin = sinf(2.0f * (float)M_PI / period_sec * elapsed);
    float v = compute_velocity(raw_sin);

    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
    SDL_RenderClear(renderer);
    DrawMinkowskiGrid(renderer, v);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
    return SDL_APP_CONTINUE;
}