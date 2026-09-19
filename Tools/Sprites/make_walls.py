"""Рисует стены, которые можно сломать.

Тайл стены сломать нельзя - он часть полотна. Проп можно: он стоит как стена,
а когда его разбивают, коллайдер уходит в триггер и клетка открывается.
Значит, целый кадр обязан читаться как продолжение кладки, а не как ящик
у стены, иначе игрок пройдёт мимо и пролома не заметит.

Тон взят из палитры тюрьмы в make_tileset.py: холодный серый, не тёплый.
"""

import os
import random

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
TARGET = os.path.join(TEXTURES, 'walls.png')

TILE = 64

DARK = (30, 33, 38)
MID = (96, 100, 106)
LIGHT = (132, 136, 142)
SEAM = (58, 61, 67)
CRACK = (18, 20, 24)
DUST = (150, 152, 156)
REBAR = (122, 82, 56)

BRICK_ROWS = 4
BRICK_HEIGHT = TILE // BRICK_ROWS


def Bricks(draw, shift):
    """Кладка вразбежку: ряд через ряд сдвинут, шов темнее самого кирпича."""
    for row in range(BRICK_ROWS):
        top = row * BRICK_HEIGHT
        offset = (row % 2) * (TILE // 4) + shift

        draw.rectangle([0, top, TILE - 1, top + BRICK_HEIGHT - 1], fill=MID)
        draw.line([0, top, TILE - 1, top], fill=SEAM)

        for step in range(-1, 3):
            x = offset + step * (TILE // 2)
            draw.line([x, top, x, top + BRICK_HEIGHT - 1], fill=SEAM)

        # Верхняя грань кирпича светлее: иначе кладка читается плоским полем.
        draw.line([0, top + 1, TILE - 1, top + 1], fill=LIGHT)


def Crack(draw, rng):
    """Трещина сверху вниз: рваная линия и осыпь по её краям."""
    x = TILE // 2
    points = []
    for y in range(0, TILE, 3):
        x += rng.randint(-3, 3)
        x = max(10, min(TILE - 11, x))
        points.append((x, y))

    for index in range(len(points) - 1):
        draw.line([points[index], points[index + 1]], fill=CRACK, width=3)
        draw.line([(points[index][0] + 2, points[index][1]),
                   (points[index + 1][0] + 2, points[index + 1][1])], fill=DARK, width=1)

    for point in points[::3]:
        draw.point((point[0] - 3, point[1] + 1), fill=DUST)


def Whole():
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    rng = random.Random(7)

    Bricks(draw, 0)
    Crack(draw, rng)

    # Выпавший кирпич: маленькая дыра, по которой видно, что стена не жилец.
    draw.rectangle([38, 33, 52, 45], fill=CRACK)
    draw.rectangle([39, 34, 51, 44], fill=DARK)

    return tile


def Broken():
    """На месте стены - щебень. Через клетку проходят, значит середина пустая
    и сквозь неё виден пол. Пол в тюрьме тёмный, поэтому обломки светлее его,
    а не темнее: иначе пролом читается пятном, а не дырой.
    """
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    rng = random.Random(11)

    # Остатки кладки прижаты к верхнему и нижнему краю: проход идёт посередине.
    for side in (0, 1):
        edge = 0 if side == 0 else TILE
        for step in range(9):
            x = step * 7 + rng.randint(-1, 1)
            depth = rng.randint(5, 14)
            y = 0 if side == 0 else TILE - depth
            draw.rectangle([x, y, x + rng.randint(5, 8), y + depth], fill=MID)
            draw.line([x, y if side == 0 else y, x + 7, y], fill=LIGHT)

        draw.rectangle([0, edge - 4 if side else 0, TILE - 1, edge - 1 if side else 3], fill=MID)

    # Обломки на полу: без них дыра выглядит вырезанной, а не выбитой.
    for _ in range(22):
        x = rng.randint(3, TILE - 9)
        y = rng.randint(16, TILE - 18)
        size = rng.randint(3, 6)
        shade = LIGHT if rng.random() < 0.45 else MID
        draw.rectangle([x, y, x + size, y + size - rng.randint(1, 2)], fill=shade)
        draw.line([x, y, x + size, y], fill=DUST)

    # Прутья торчат из оставшейся кладки, а не висят посреди проёма.
    for x, from_top in ((17, True), (46, False)):
        y = 12 if from_top else TILE - 13
        draw.line([x, y, x + rng.randint(-2, 2), y + (9 if from_top else -9)], fill=REBAR, width=2)

    for _ in range(14):
        draw.point((rng.randint(5, TILE - 5), rng.randint(18, TILE - 18)), fill=DUST)

    return tile


def Build():
    frames = [Whole(), Broken()]
    sheet = Image.new('RGBA', (TILE * len(frames), TILE), (0, 0, 0, 0))
    for index, frame in enumerate(frames):
        sheet.alpha_composite(frame, dest=(index * TILE, 0))

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)


if __name__ == '__main__':
    Build()
