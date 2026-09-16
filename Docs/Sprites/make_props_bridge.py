"""Собирает props_bridge.png из Kenney Racing Pack с переносом палитры.

Кадр у каждого пропа своей формы, а не квадрат: квадратный кадр, растянутый
в прямоугольный бокс, сплющивал машины по вертикали почти вдвое.

Размеры взяты от сетки: клетка 64 - это примерно два метра, поэтому легковая
занимает две клетки в длину и одну в ширину, как машина относительно человека.
"""

import io
import os
import zipfile

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
PACK = os.path.join(HERE, 'vendor', 'kenney_racing.zip')
TARGET = os.path.join(TEXTURES, 'props_bridge.png')

BOX_PART = 0.94

RAMPS = {
    'rust': [(0.10, (46, 26, 18)), (0.45, (128, 66, 38)), (0.80, (188, 116, 70))],
    'signal': [(0.10, (74, 36, 12)), (0.45, (198, 106, 34)), (0.80, (236, 214, 190))],
    'rubber': [(0.10, (10, 10, 12)), (0.45, (30, 30, 34)), (0.80, (58, 58, 64))],
    'carbody': [(0.10, (18, 20, 26)), (0.45, (62, 68, 78)), (0.80, (104, 112, 124))],
    'carrust': [(0.10, (30, 18, 14)), (0.45, (104, 52, 38)), (0.80, (156, 92, 62))],
    'paint': [(0.10, (20, 34, 48)), (0.45, (52, 94, 132)), (0.80, (96, 148, 190))],
    'slick': [(0.10, (10, 10, 14)), (0.45, (28, 28, 36)), (0.80, (54, 54, 66))],
}

# имя, файл в паке, палитра, ширина кадра, высота кадра, поворот на бок
FRAMES = [
    ('car_sedan', 'PNG/Cars/car_black_1.png', 'carbody', 128, 64, True),
    ('car_van', 'PNG/Cars/car_blue_4.png', 'paint', 136, 68, True),
    ('car_small', 'PNG/Cars/car_black_small_1.png', 'carrust', 104, 56, True),
    ('car_wreck', 'burnt', None, 128, 64, False),
    ('road_barrier', 'PNG/Objects/barrier_red.png', 'signal', 128, 40, False),
    ('skid_mark', 'PNG/Objects/skidmark_long_1.png', 'slick', 128, 32, True),
    ('road_cone', 'PNG/Objects/cone_straight.png', 'signal', 40, 40, False),
    ('fuel_barrel', 'PNG/Objects/barrel_red.png', 'rust', 52, 52, False),
    ('tire_stack', 'PNG/Objects/tires_red.png', 'rubber', 64, 64, False),
    ('oil_slick', 'PNG/Objects/oil.png', 'slick', 96, 72, False),
]

WRECK_SOURCE = 'PNG/Cars/car_black_1.png'
SOOT = (26, 24, 24)


def Luminance(pixels):
    return (0.299 * pixels[..., 0] + 0.587 * pixels[..., 1] + 0.114 * pixels[..., 2]) / 255.0


def Recolour(tile, ramp):
    level = Luminance(tile)
    out = tile.copy()
    stops = sorted(RAMPS[ramp])
    for channel in range(3):
        out[..., channel] = np.interp(level, [stop for stop, _ in stops], [colour[channel] for _, colour in stops])

    return out


def Read(pack, name):
    return Image.open(io.BytesIO(pack.read(name))).convert('RGBA')


def Burnt(pack):
    image = Read(pack, WRECK_SOURCE).rotate(-90, expand=True)
    tile = Recolour(np.asarray(image).astype(float), 'slick')

    rng = np.random.default_rng(31)
    height, width = tile.shape[0], tile.shape[1]
    for _ in range(26):
        row = int(rng.integers(0, height))
        column = int(rng.integers(0, width))
        radius = int(rng.integers(2, 6))
        top, left = max(0, row - radius), max(0, column - radius)
        patch = tile[top:row + radius, left:column + radius]
        patch[..., :3] = np.array(SOOT, dtype=float) + rng.normal(0.0, 6.0, 3)

    tile[..., :3] *= 0.92

    return Image.fromarray(np.clip(tile, 0, 255).astype(np.uint8), 'RGBA')


def Fit(image, width, height, isSideways):
    if isSideways:
        image = image.rotate(-90, expand=True)

    # Вписываем без искажения: форма кадра уже повторяет форму объекта.
    image = image.copy()
    image.thumbnail((width, height), Image.LANCZOS)

    tile = Image.new('RGBA', (width, height), (0, 0, 0, 0))
    tile.paste(image, ((width - image.width) // 2, (height - image.height) // 2), image)

    return tile


def Build():
    width = sum(frame[3] for frame in FRAMES)
    height = max(frame[4] for frame in FRAMES)
    sheet = Image.new('RGBA', (width, height), (0, 0, 0, 0))

    left = 0
    places = []
    with zipfile.ZipFile(PACK) as pack:
        for name, source, ramp, frameWidth, frameHeight, isSideways in FRAMES:
            image = Burnt(pack) if source == 'burnt' else Read(pack, source)

            if ramp is not None:
                pixels = Recolour(np.asarray(image).astype(float), ramp)
                image = Image.fromarray(np.clip(pixels, 0, 255).astype(np.uint8), 'RGBA')

            sheet.paste(Fit(image, frameWidth, frameHeight, isSideways), (left, 0))
            places.append((name, left, frameWidth, frameHeight))
            left += frameWidth

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)
    print()
    # Бокс - кадр, ужатый пропорционально: вычитать одинаково у длины и ширины
    # нельзя, у узких кадров это заметно меняет форму.
    print('frame Resources/Textures/props_bridge.png <x> 0 <w> <h>')
    for name, x, frameWidth, frameHeight in places:
        print('%-14s frame %4d 0 %3d %3d   size %3d   height %3d'
              % (name, x, frameWidth, frameHeight, round(frameWidth * BOX_PART), round(frameHeight * BOX_PART)))


if __name__ == '__main__':
    Build()
