"""Рисует атлас огня: вид сверху, зацикленная анимация.

Готовые наборы огня рисуют пламя сбоку - в виде сверху такой спрайт выглядит
приклеенным к полу. Здесь огонь строится как пятно с языками по краю: ядро
светлое, края тёмно-красные и прозрачные, языки колышутся от кадра к кадру.
"""

import os

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
TARGET = os.path.join(TEXTURES, 'fire.png')

FRAMES = 8

BIG = 64
SMALL = 32

CORE = (255, 244, 176)
MIDDLE = (252, 150, 34)
EDGE = (190, 48, 14)
SMOKE = (58, 44, 40)

TONGUES = 7


def Flame(size, phase, seed, isSmall):
    rng = np.random.default_rng(seed)
    tile = np.zeros((size, size, 4))

    middle = (size - 1) / 2.0
    rows = np.arange(size)[:, None] - middle
    columns = np.arange(size)[None, :] - middle

    away = np.sqrt(rows * rows + columns * columns) / middle
    angle = np.arctan2(rows, columns)

    # Языки: радиус пятна гуляет по углу, а фаза сдвигает их по кругу.
    wobble = np.zeros_like(angle)
    for tongue in range(1, TONGUES):
        weight = 0.2 / tongue
        wobble += weight * np.sin(tongue * angle + phase * 2.0 * np.pi + tongue * seed)

    reach = (0.62 if isSmall else 0.74) + wobble + 0.05 * np.sin(phase * 2.0 * np.pi)
    body = np.clip((reach - away) / 0.34, 0.0, 1.0)

    heat = np.clip(body * (1.15 - 0.55 * away), 0.0, 1.0)

    core = np.array(CORE, dtype=float)
    mid = np.array(MIDDLE, dtype=float)
    edge = np.array(EDGE, dtype=float)

    colour = np.zeros((size, size, 3))
    hot = np.clip((heat - 0.55) / 0.45, 0.0, 1.0)[..., None]
    warm = np.clip(heat / 0.55, 0.0, 1.0)[..., None]

    colour += edge[None, None, :] * (1.0 - warm)
    colour += mid[None, None, :] * warm * (1.0 - hot)
    colour += core[None, None, :] * hot

    flicker = rng.normal(0.0, 5.0, (size, size, 1))
    tile[..., :3] = np.clip(colour + flicker, 0, 255)

    # Ядро непрозрачно: иначе сквозь огонь просвечивает асфальт и пламя сереет.
    solid = np.clip(body * 1.6, 0.0, 1.0)
    tile[..., 3] = np.clip(solid * 255.0, 0, 255)

    # Копоть по внешней кромке: без неё огонь выглядит наклейкой.
    rim = np.clip((away - reach + 0.22) / 0.22, 0.0, 1.0) * np.clip(body * 4.0, 0.0, 1.0)
    tile[..., :3] = tile[..., :3] * (1.0 - rim[..., None] * 0.55) + np.array(SMOKE, dtype=float)[None, None, :] * rim[..., None] * 0.55

    return np.clip(tile, 0, 255)


def Build():
    sheet = np.zeros((BIG + SMALL, BIG * FRAMES, 4))

    for frame in range(FRAMES):
        phase = frame / float(FRAMES)
        sheet[0:BIG, frame * BIG:(frame + 1) * BIG] = Flame(BIG, phase, frame + 1, False)

        small = Flame(SMALL, phase, frame + 11, True)
        sheet[BIG:BIG + SMALL, frame * SMALL:(frame + 1) * SMALL] = small

    Image.fromarray(np.clip(sheet, 0, 255).astype(np.uint8), 'RGBA').save(TARGET)
    print('built', TARGET, sheet.shape[1], 'x', sheet.shape[0])
    print('big   x 0 y 0  %dx%d, %d frames' % (BIG, BIG, FRAMES))
    print('small x 0 y %d %dx%d, %d frames' % (BIG, SMALL, SMALL, FRAMES))


if __name__ == '__main__':
    Build()
