#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <X11/Xlib.h>
#include <cmath>
#include <iostream>

Display* display;
Window root_window;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* background_texture;
SDL_Texture* left_eye_texture;
SDL_Texture* right_eye_texture;

int screen_width = 2048;
int screen_height = 1142;
int left_eye_x = 550 + 100, left_eye_y = 340 + 70;  // Сдвиг на 70 пикселей вниз
int right_eye_x = 1310 + 100, right_eye_y = 350 + 70;  // Сдвиг на 70 пикселей вниз
int eye_radius = 40;
float smoothing_factor = 0.08f;  // Плавное движение

void get_cursor_position(int& x, int& y) {
    XEvent ev;
    XQueryPointer(display, root_window, &ev.xbutton.root, &ev.xbutton.subwindow,
                  &x, &y, &ev.xbutton.x, &ev.xbutton.y, &ev.xbutton.state);
}

SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path); // Используем IMG_Load для PNG
    if (!surface) {
        std::cerr << "Failed to load texture: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

// Функция для вычисления угла, на который нужно повернуть глаз
float calculate_angle(int eye_x, int eye_y, int cursor_x, int cursor_y) {
    float dx = cursor_x - eye_x;
    float dy = cursor_y - eye_y;
    return atan2(dy, dx);
}

// Функция для вычисления позиции глаз с учетом угла
void calculate_eye_position(int& eye_x, int& eye_y, int eye_center_x, int eye_center_y, float angle) {
    eye_x = eye_center_x + eye_radius * cos(angle);
    eye_y = eye_center_y + eye_radius * sin(angle);
}

// Функция для плавного перемещения глаз
void smooth_eye_movement(int& eye_x, int& eye_y, int target_x, int target_y, float smoothing_factor) {
    eye_x += (target_x - eye_x) * smoothing_factor;
    eye_y += (target_y - eye_y) * smoothing_factor;
}

int main(int argc, char* argv[]) {
    // Инициализация SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        return -1;
    }

    window = SDL_CreateWindow("Следящее животное", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              screen_width, screen_height, SDL_WINDOW_BORDERLESS | SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Загрузка изображений (PNG)
    background_texture = load_texture("EyelessNootNoot.png");
    left_eye_texture = load_texture("NootNootsEye.png");
    right_eye_texture = load_texture("NootNootsEye.png");

    // Получение дисплея X11 для получения позиции мыши
    display = XOpenDisplay(NULL);
    if (!display) {
        std::cerr << "Failed to open X display." << std::endl;
        return -1;
    }
    root_window = DefaultRootWindow(display);

    bool running = true;
    SDL_Event event;

    while (running) {
        int cursor_x, cursor_y;
        get_cursor_position(cursor_x, cursor_y);

        // Вычисление углов для глаз
        float left_eye_angle = calculate_angle(left_eye_x, left_eye_y, cursor_x, cursor_y);
        float right_eye_angle = calculate_angle(right_eye_x, right_eye_y, cursor_x, cursor_y);

        // Целевые позиции глаз с учетом углов
        int target_left_eye_x = 550 + 100 + eye_radius * cos(left_eye_angle);
        int target_left_eye_y = 340 + 70 + eye_radius * sin(left_eye_angle);

        int target_right_eye_x = 1310 + 100 + eye_radius * cos(right_eye_angle);
        int target_right_eye_y = 350 + 70 + eye_radius * sin(right_eye_angle);

        // Плавное движение глаз
        smooth_eye_movement(left_eye_x, left_eye_y, target_left_eye_x, target_left_eye_y, smoothing_factor);
        smooth_eye_movement(right_eye_x, right_eye_y, target_right_eye_x, target_right_eye_y, smoothing_factor);

        // Отрисовка
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background_texture, NULL, NULL);

        // Увеличим размеры глаз до 191x198 пикселей
        SDL_Rect left_eye_rect = { left_eye_x - 95, left_eye_y - 99, 191, 198 }; // Размер глаз 191x198
        SDL_Rect right_eye_rect = { right_eye_x - 95, right_eye_y - 99, 191, 198 }; // Размер глаз 191x198

        SDL_RenderCopy(renderer, left_eye_texture, NULL, &left_eye_rect);
        SDL_RenderCopy(renderer, right_eye_texture, NULL, &right_eye_rect);

        SDL_RenderPresent(renderer);

        // Обработка событий
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_Delay(16);  // Примерный FPS 60
    }

    // Очистка ресурсов
    SDL_DestroyTexture(background_texture);
    SDL_DestroyTexture(left_eye_texture);
    SDL_DestroyTexture(right_eye_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();
    XCloseDisplay(display);

    return 0;
}
