"""Рисует мелочи, которые находят, если свернуть с прямой.

Вид сверху: всё лежит на полу и читается сверху, поэтому доска нарядов -
планшет на столе, а не табличка на стене, и счёт дней нацарапан у нар на
полу, а не на кладке. Ничего яркого: тюрьма серая, и пасхалка не должна
светиться, иначе её найдут все и она перестанет быть находкой.
"""

import math
import os
import random

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
TARGET = os.path.join(TEXTURES, 'eggs.png')

TILE = 64

SCRATCH = (128, 132, 138)
SCRATCH_DEEP = (168, 172, 178)

EARTH = (64, 52, 40)
EARTH_DARK = (34, 27, 21)
EARTH_LIGHT = (96, 80, 62)
SPOON = (172, 176, 182)

CLOTH = (122, 104, 86)
CLOTH_DARK = (78, 64, 50)
PORCELAIN = (226, 220, 206)
THREAD = (196, 192, 180)

BOARD = (92, 74, 52)
BOARD_DARK = (56, 44, 30)
PAPER = (206, 202, 190)
INK = (58, 58, 62)
CLIP = (150, 154, 160)


def Tally():
    """Счёт дней: четыре палки и пятая наискось. Три полных пятка и остаток."""
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    rng = random.Random(3)

    groups = [5, 5, 5, 3]
    x = 10
    for count in groups:
        for step in range(min(count, 4)):
            line = x + step * 4
            draw.line([line, 22 + rng.randint(-1, 1), line + rng.randint(-1, 1), 42 + rng.randint(-1, 1)],
                      fill=SCRATCH, width=1)

        if count == 5:
            draw.line([x - 2, 40, x + 14, 24], fill=SCRATCH_DEEP, width=1)

        x += 14

    return tile


def Hole():
    """Подкоп: яма, выброшенная земля и ложка, которой копали."""
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    rng = random.Random(5)

    # Насыпь вокруг ямы: без неё яма читается лужей.
    for _ in range(90):
        angle = rng.random() * math.tau
        reach = 15 + rng.random() * 11
        x = 30 + math.cos(angle) * reach
        y = 34 + math.sin(angle) * reach * 0.8
        size = rng.randint(1, 3)
        shade = EARTH if rng.random() < 0.7 else EARTH_LIGHT
        draw.rectangle([x, y, x + size, y + size], fill=shade)

    draw.ellipse([17, 24, 43, 42], fill=EARTH_DARK)
    draw.ellipse([20, 27, 40, 39], fill=(18, 14, 11))

    # Ложка: единственное, чем копали, и единственное светлое пятно.
    draw.line([44, 44, 52, 36], fill=SPOON, width=2)
    draw.ellipse([50, 32, 56, 38], fill=SPOON)

    return tile


def Puppet():
    """Марионетка с кулак: тряпичное тело, фарфоровая голова, обрывки нитей."""
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)

    draw.line([24, 22, 12, 12], fill=THREAD, width=1)
    draw.line([40, 22, 54, 14], fill=THREAD, width=1)
    draw.line([32, 20, 34, 8], fill=THREAD, width=1)

    draw.rectangle([27, 28, 37, 44], fill=CLOTH_DARK)
    draw.rectangle([28, 29, 36, 43], fill=CLOTH)

    for x, y in ((24, 30), (40, 30)):
        draw.line([32, 31, x, y + 6], fill=CLOTH_DARK, width=2)
    for x in (29, 35):
        draw.line([x, 44, x + (x - 32) // 2, 52], fill=CLOTH_DARK, width=2)

    draw.ellipse([26, 18, 38, 30], fill=(196, 190, 178))
    draw.ellipse([27, 19, 37, 29], fill=PORCELAIN)
    draw.line([32, 19, 31, 29], fill=(150, 144, 132), width=1)
    draw.point((30, 23), fill=INK)
    draw.point((34, 23), fill=INK)

    return tile


def Board():
    """Доска нарядов: планшет с листом, на листе - смены и фамилии."""
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)

    draw.rectangle([13, 9, 51, 55], fill=BOARD_DARK)
    draw.rectangle([15, 11, 49, 53], fill=BOARD)
    draw.rectangle([17, 15, 47, 51], fill=PAPER)

    draw.rectangle([26, 7, 38, 13], fill=CLIP)
    draw.rectangle([28, 8, 36, 12], fill=(112, 116, 122))

    draw.line([19, 21, 45, 21], fill=INK, width=1)
    for row in range(5):
        y = 26 + row * 5
        draw.line([19, y, 30 + (row % 3) * 4, y], fill=INK, width=1)
        draw.line([38, y, 45, y], fill=INK, width=1)

    return tile


ASH = (58, 54, 50)
ASH_DARK = (32, 30, 28)
CHAR = (26, 22, 20)
LOG = (86, 62, 42)
LOG_DARK = (52, 38, 26)
STONE = (104, 102, 98)
CLOTH_ROLL = (92, 86, 70)
CLOTH_ROLL_DARK = (58, 54, 44)

PAINT = (198, 74, 58)
PAINT_DARK = (132, 44, 34)


def Campfire():
    """Остывшее кострище и лежанка: здесь кто-то держался и ушёл.

    Угли чёрные, а не красные: огонь погас давно, и в этом вся мысль.
    """
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    rng = random.Random(23)

    for step in range(9):
        angle = step * math.tau / 9.0
        x = 26 + math.cos(angle) * 13
        y = 30 + math.sin(angle) * 11
        draw.ellipse([x - 4, y - 3, x + 4, y + 3], fill=STONE)
        draw.ellipse([x - 3, y - 3, x + 2, y], fill=(126, 124, 120))

    draw.ellipse([15, 22, 37, 38], fill=ASH_DARK)
    draw.ellipse([17, 24, 35, 36], fill=ASH)

    for _ in range(18):
        x = rng.randint(18, 34)
        y = rng.randint(25, 35)
        draw.rectangle([x, y, x + rng.randint(1, 2), y + 1], fill=CHAR)

    for start, end in (((19, 26), (33, 34)), ((21, 35), (34, 25))):
        draw.line([start, end], fill=LOG_DARK, width=4)
        draw.line([start, end], fill=LOG, width=2)

    # Лежанка рядом: кострище без неё - просто мусор.
    draw.rectangle([44, 20, 56, 46], fill=CLOTH_ROLL_DARK)
    draw.rectangle([45, 21, 55, 45], fill=CLOTH_ROLL)
    for y in range(24, 45, 5):
        draw.line([45, y, 55, y], fill=CLOTH_ROLL_DARK)

    return tile


def Paint():
    """Чей-то знак на асфальте: стрелка и три полосы.

    Куда она ведёт и кто её ставил, не сказано: знак не для игрока.
    """
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    rng = random.Random(29)

    draw.line([14, 40, 34, 20], fill=PAINT_DARK, width=6)
    draw.line([14, 40, 34, 20], fill=PAINT, width=4)
    draw.polygon([(38, 16), (26, 22), (32, 28)], fill=PAINT_DARK)
    draw.polygon([(37, 18), (28, 22), (31, 26)], fill=PAINT)

    for step in range(3):
        y = 44 + step * 5
        draw.line([16 + step * 3, y, 38 - step * 2, y], fill=PAINT_DARK, width=3)
        draw.line([16 + step * 3, y, 38 - step * 2, y], fill=PAINT, width=1)

    # Краска брызнула: без брызг знак выглядит напечатанным.
    for _ in range(22):
        x = rng.randint(10, 44)
        y = rng.randint(14, 56)
        draw.point((x, y), fill=PAINT_DARK if rng.random() < 0.6 else PAINT)

    return tile


def Build():
    frames = [Tally(), Hole(), Puppet(), Board(), Campfire(), Paint()]
    sheet = Image.new('RGBA', (TILE * len(frames), TILE), (0, 0, 0, 0))
    for index, frame in enumerate(frames):
        sheet.alpha_composite(frame, dest=(index * TILE, 0))

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)


if __name__ == '__main__':
    Build()
