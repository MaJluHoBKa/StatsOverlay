import cv2
import numpy as np
import easyocr
from mss import mss
from time import sleep

def resource_path(relative_path):
    import sys, os
    if hasattr(sys, '_MEIPASS'):
        return os.path.join(sys._MEIPASS, relative_path)
    return os.path.join(os.path.abspath("."), relative_path)

# Загружаем шаблон цветной
template = cv2.imread(resource_path('src/template/default_damage_template.png'), cv2.IMREAD_COLOR)
if template is None:
    raise FileNotFoundError("Не удалось загрузить шаблон damage_template.png")
w, h = template.shape[1], template.shape[0]  # ширина, высота

monitor = {"top": 230, "left": 60, "width": 120, "height": 60}

# Инициализируем EasyOCR reader (указать 'en' достаточно для цифр)
reader = easyocr.Reader(['en'], gpu=False)

with mss() as sct:
    frame_id = 0
    while True:
        screenshot = np.array(sct.grab(monitor))             # BGRA
        screenshot_bgr = cv2.cvtColor(screenshot, cv2.COLOR_BGRA2BGR)

        # Цветовое шаблонное сопоставление
        res = cv2.matchTemplate(screenshot_bgr, template, cv2.TM_CCOEFF_NORMED)
        min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(res)
        print(f"[{frame_id}] max match value: {max_val:.3f} at {max_loc}")
        threshold = 0.5
        loc = np.where(res >= threshold)

        found = False
        for pt in zip(*loc[::-1]):
            found = True
            x, y = pt

            # ROI — цифры под нашивкой (40 пикселей вниз)
            roi = screenshot_bgr[y + h:y + h + 40, x:x + w]
            if roi.size == 0:
                continue

            # Преобразуем roi в RGB, т.к. EasyOCR ожидает RGB, а у нас BGR
            roi_rgb = cv2.cvtColor(roi, cv2.COLOR_BGR2RGB)

            # Распознаём текст в ROI
            result = reader.readtext(roi_rgb, detail=0, paragraph=False)

            # Фильтруем только цифры (опционально)
            digits = ''.join(filter(str.isdigit, ''.join(result)))

            print(f"[{frame_id}] Нашивка в ({x},{y}) — цифры: '{digits}'")

        if not found:
            print(f"[{frame_id}] Нашивка не найдена")

        frame_id += 1
        sleep(1)
