"""Рисует ленту клуба дыма: вид сверху, один прогон от рождения до исчезновения.

Клуб не зациклен, в отличие от огня: каждый кадр он шире и прозрачнее
предыдущего, и последний кадр пустой. Одна затяжка - один объект, который
сам себя убирает, когда лента кончилась.

Дым от покрышек серо-синий, а не белый: белый на сером асфальте не читается.
"""

import os

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
TARGET = os.path.join(TEXTURES, 'smoke.png')

FRAMES = 6
SIZE = 48

CORE = (206, 208, 214)
EDGE = (104, 108, 120)

PUFFS = 5
GROW = 0.55
FADE = 0.92


def Lobes(seed):
    """Комки задаются один раз на всю ленту: иначе клуб не растёт, а кипит."""
    rng = np.random.default_rng(seed)

    return [(rng.uniform(0.0, 2.0 * np.pi), rng.uniform(0.7, 1.2), rng.uniform(0.0, 2.0 * np.pi))
            for _ in range(PUFFS)]


def Puff(phase, lobes):
    tile = np.zeros((SIZE, SIZE, 4))

    middle = (SIZE - 1) / 2.0
    rows = np.arange(SIZE)[:, None] - middle
    columns = np.arange(SIZE)[None, :] - middle

    density = np.zeros((SIZE, SIZE))
    for drift, weight, wobblePhase in lobes:
        reach = (0.06 + 0.30 * phase) * middle

        atRow = rows - reach * np.sin(drift)
        atColumn = columns - reach * np.cos(drift)
        away = np.sqrt(atRow * atRow + atColumn * atColumn) / middle
        angle = np.arctan2(atRow, atColumn)

        # Край комка гуляет по углу - ровный круг читается как мяч, а не как дым.
        wobble = 0.16 * np.sin(3.0 * angle + wobblePhase) + 0.10 * np.sin(5.0 * angle - wobblePhase)
        size = (0.16 + GROW * phase) * weight * (1.0 + wobble)

        density = np.maximum(density, np.clip((size - away) / 0.20, 0.0, 1.0))

    core = np.array(CORE, dtype=float)
    edge = np.array(EDGE, dtype=float)

    # К краю клуб темнее и реже, в середине светлее - и вся масса сереет со временем.
    thick = np.clip(density * (1.3 - 0.7 * phase), 0.0, 1.0)[..., None]
    tile[..., :3] = edge[None, None, :] * (1.0 - thick) + core[None, None, :] * thick

    tile[..., 3] = density * 225.0 * max(0.0, 1.0 - FADE * phase * phase)

    return tile


def Build():
    sheet = Image.new('RGBA', (SIZE * FRAMES, SIZE), (0, 0, 0, 0))

    lobes = Lobes(41)
    for index in range(FRAMES):
        phase = index / float(FRAMES - 1)
        frame = np.clip(Puff(phase, lobes), 0, 255).astype(np.uint8)
        sheet.paste(Image.fromarray(frame, 'RGBA'), (index * SIZE, 0))

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)


if __name__ == '__main__':
    Build()
