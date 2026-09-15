import math
import os

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
TARGET = os.path.join(TEXTURES, 'fixtures.png')

TILE = 64
HATCH_FRAMES = 5
LEVER_FRAMES = 2

RIM = (92, 96, 104)
RIM_DARK = (54, 58, 64)
LID = (126, 118, 96)
LID_DARK = (86, 80, 64)
LID_LIGHT = (166, 158, 132)
HOLE = (16, 18, 22)
BOLT = (198, 192, 170)

PANEL = (96, 102, 112)
PANEL_DARK = (58, 62, 70)
PANEL_LIGHT = (140, 146, 158)
HANDLE = (206, 88, 62)
HANDLE_DOWN = (96, 150, 98)


def Circle(draw, centre, radius, fill, outline=None):
    x, y = centre
    draw.ellipse([x - radius, y - radius, x + radius, y + radius], fill=fill, outline=outline)


def Hatch(frame):
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)
    centre = (TILE / 2, TILE / 2)

    Circle(draw, centre, 29, RIM_DARK)
    Circle(draw, centre, 26, HOLE)
    Circle(draw, centre, 23, RIM)
    Circle(draw, centre, 21, HOLE)

    part = frame / float(HATCH_FRAMES - 1)
    turn = part * 115.0
    shift = part * 19.0

    lid = Image.new('RGBA', (TILE * 2, TILE * 2), (0, 0, 0, 0))
    lidDraw = ImageDraw.Draw(lid)
    lidCentre = (TILE, TILE)

    Circle(lidDraw, lidCentre, 19, LID_DARK)
    Circle(lidDraw, lidCentre, 17, LID)
    Circle(lidDraw, lidCentre, 9, LID_DARK)
    Circle(lidDraw, lidCentre, 7, LID_LIGHT)

    for step in range(6):
        angle = math.radians(step * 60.0)
        bolt = (lidCentre[0] + math.cos(angle) * 12.5, lidCentre[1] + math.sin(angle) * 12.5)
        Circle(lidDraw, bolt, 2.4, BOLT)

    lidDraw.line([lidCentre[0] - 17, lidCentre[1], lidCentre[0] + 17, lidCentre[1]], fill=LID_DARK, width=2)

    lid = lid.rotate(turn, resample=Image.BICUBIC, center=lidCentre)
    tile.alpha_composite(lid, dest=(0, 0), source=(int(TILE / 2 + shift), int(TILE / 2 - shift * 0.35),
                                                   int(TILE / 2 + shift + TILE), int(TILE / 2 - shift * 0.35 + TILE)))

    return tile


def Lever(frame):
    tile = Image.new('RGBA', (TILE, TILE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(tile)

    draw.rectangle([12, 14, 51, 49], fill=PANEL_DARK)
    draw.rectangle([14, 16, 49, 47], fill=PANEL)
    draw.rectangle([14, 16, 49, 20], fill=PANEL_LIGHT)

    for x in (20, 32, 44):
        Circle(draw, (x, 43), 2.6, PANEL_DARK)

    slot = [28, 24, 36, 39]
    draw.rectangle(slot, fill=PANEL_DARK)

    if frame == 0:
        draw.rectangle([29, 25, 35, 31], fill=HANDLE)
        Circle(draw, (32, 26), 3.4, HANDLE)
    else:
        draw.rectangle([29, 32, 35, 38], fill=HANDLE_DOWN)
        Circle(draw, (32, 37), 3.4, HANDLE_DOWN)

    return tile


def Build():
    frames = [Hatch(index) for index in range(HATCH_FRAMES)] + [Lever(index) for index in range(LEVER_FRAMES)]
    sheet = Image.new('RGBA', (TILE * len(frames), TILE), (0, 0, 0, 0))
    for index, frame in enumerate(frames):
        sheet.alpha_composite(frame, dest=(index * TILE, 0))

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)


if __name__ == '__main__':
    Build()
