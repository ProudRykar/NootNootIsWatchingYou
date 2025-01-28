import sys
import pygame
import math

if sys.platform == "linux":
    import Xlib.display

# Инициализация Pygame и окна
pygame.init()
display = pygame.display.set_mode((2048, 1142), pygame.NOFRAME)
pygame.display.set_caption('Следящее животное')

# Загрузка изображений глаз
background_img = pygame.image.load('EyelessNootNoot.png')
left_eye_img = pygame.image.load('NootNootsEye.png')
right_eye_img = pygame.image.load('NootNootsEye.png')

# Параметры глаз
left_eye_center = (550, 340)
right_eye_center = (1310, 350)
eye_radius = 40
left_eye_pos = left_eye_center
right_eye_pos = right_eye_center
smoothing_factor = 0.1

# Функции для расчета позиции глаза
def calculate_smooth_eye_position(current_pos, target_pos, factor):
    new_x = current_pos[0] + (target_pos[0] - current_pos[0]) * factor
    new_y = current_pos[1] + (target_pos[1] - current_pos[1]) * factor
    return int(new_x), int(new_y)

def calculate_eye_target_position(eye_center, cursor_pos, max_radius):
    dx = cursor_pos[0] - eye_center[0]
    dy = cursor_pos[1] - eye_center[1]
    distance = math.hypot(dx, dy)
    if distance > max_radius:
        scale = max_radius / distance
        dx *= scale
        dy *= scale
    return int(eye_center[0] + dx), int(eye_center[1] + dy)

# Платформозависимый метод получения позиции курсора
if sys.platform == "linux":
    display_x = Xlib.display.Display()

    def get_cursor_position():
        data = display_x.screen().root.query_pointer()._data
        return data["root_x"], data["root_y"]

elif sys.platform == "win32":
    import win32api

    def get_cursor_position():
        return win32api.GetCursorPos()

else:
    raise NotImplementedError("Эта платформа не поддерживается")

# Главный цикл
running = True
while running:
    cursor_pos = get_cursor_position()
    left_eye_target = calculate_eye_target_position(left_eye_center, cursor_pos, eye_radius)
    right_eye_target = calculate_eye_target_position(right_eye_center, cursor_pos, eye_radius)
    left_eye_pos = calculate_smooth_eye_position(left_eye_pos, left_eye_target, smoothing_factor)
    right_eye_pos = calculate_smooth_eye_position(right_eye_pos, right_eye_target, smoothing_factor)

    display.blit(background_img, (0, 0))
    display.blit(left_eye_img, left_eye_pos)
    display.blit(right_eye_img, right_eye_pos)
    pygame.display.flip()

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

pygame.quit()
