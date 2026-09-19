"""Рисует ловушки: шипы из пола и сигнальную растяжку.

Кадров у каждой два - взведённая и сработавшая, ровно как у пропа frame
и spentFrame. Взведённая обязана быть заметной, если приглядеться, и
незаметной, если бежишь: в этом вся ловушка.
"""

import math
import os

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
TARGET = os.path.join(TEXTURES, 'traps.png')

TILE = 64

SLOT = (24, 26, 30)
SLOT_EDGE = (58, 62, 68)
GRATE = (74, 78, 86)

SPIKE = (168, 172, 180)
SPIKE_EDGE = (206, 212, 220)
SPIKE_DARK = (92, 96, 104)
BLOOD = (128, 30, 28)

PEG = (78, 72, 62)
PEG_DARK = (44, 40, 34)
WIRE = (198, 202, 208)
WIRE_DIM = (120, 124, 132)
WIRE_GLINT = (255, 246, 210)
TAG = (206, 88, 62)


def Spikes(sprung):
    """Шипы сверху: четыре щели в решётке, из которых выходят зубья."""
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)

    draw.rectangle([9, 9, 54, 54], fill=SLOT_EDGE)
    draw.rectangle([11, 11, 52, 52], fill=GRATE)

    for step in range(4):
        x = 16 + step * 9
        draw.rectangle([x, 16, x + 4, 47], fill=SLOT)

    if not sprung:
        # Взведённые шипы видно только по блику на кромке щели.
        for step in range(4):
            x = 16 + step * 9
            draw.line([x, 16, x + 4, 16], fill=SPIKE_DARK, width=1)
            draw.line([x, 47, x + 4, 47], fill=SPIKE_DARK, width=1)

        return tile

    # Взгляд сверху: зубец - ромб с яркой гранью и точкой на острие.
    for step in range(4):
        x = 18 + step * 9
        for tooth in range(3):
            y = 23 + tooth * 10
            draw.polygon([(x, y - 7), (x + 5, y), (x, y + 7), (x - 5, y)], fill=SPIKE_DARK)
            draw.polygon([(x, y - 5), (x + 3, y), (x, y + 5), (x - 3, y)], fill=SPIKE)
            draw.polygon([(x, y - 4), (x + 2, y - 1), (x, y + 1), (x - 2, y - 1)], fill=SPIKE_EDGE)

    draw.line([20, 20, 24, 27], fill=BLOOD, width=2)
    draw.line([41, 36, 44, 43], fill=BLOOD, width=2)

    return tile


def Tripwire(sprung):
    """Растяжка: два колышка и нитка между ними. Сработала - нитка повисла."""
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)

    for x in (8, 55):
        draw.rectangle([x - 3, 26, x + 3, 44], fill=PEG_DARK)
        draw.rectangle([x - 2, 27, x + 2, 42], fill=PEG)

    if not sprung:
        draw.line([8, 32, 55, 32], fill=WIRE_DIM, width=2)
        draw.line([8, 31, 55, 31], fill=WIRE, width=1)

        # Блик на нитке - единственное, что выдаёт растяжку в темноте.
        for x in (21, 34, 47):
            draw.point((x, 31), fill=WIRE_GLINT)

        draw.rectangle([29, 33, 35, 39], fill=TAG)

        return tile

    for step in range(24):
        part = step / 23.0
        x = 8 + part * 20
        y = 32 + math.sin(part * math.pi) * 9
        draw.point((int(x), int(y)), fill=WIRE_DIM)

    for step in range(24):
        part = step / 23.0
        x = 55 - part * 18
        y = 32 + math.sin(part * math.pi) * 7
        draw.point((int(x), int(y)), fill=WIRE_DIM)

    draw.rectangle([27, 44, 33, 50], fill=TAG)

    return tile


def Build():
    frames = [Spikes(False), Spikes(True), Tripwire(False), Tripwire(True)]
    sheet = Image.new('RGBA', (TILE * len(frames), TILE), (0, 0, 0, 0))
    for index, frame in enumerate(frames):
        sheet.alpha_composite(frame, dest=(index * TILE, 0))

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)


if __name__ == '__main__':
    Build()
