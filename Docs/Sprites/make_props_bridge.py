import io
import os
import zipfile

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
PACK = os.path.join(HERE, 'vendor', 'kenney_racing.zip')
TARGET = os.path.join(TEXTURES, 'props_bridge.png')

FRAME = 128

RAMPS = {
    'rust': [(0.10, (46, 26, 18)), (0.45, (128, 66, 38)), (0.80, (188, 116, 70)),],
    'steel': [(0.10, (34, 38, 44)), (0.45, (98, 106, 116)), (0.80, (158, 166, 176))],
    'paint': [(0.10, (20, 34, 48)), (0.45, (52, 94, 132)), (0.80, (96, 148, 190))],
    'signal': [(0.10, (74, 36, 12)), (0.45, (198, 106, 34)), (0.80, (236, 214, 190))],
    'rubber': [(0.10, (10, 10, 12)), (0.45, (30, 30, 34)), (0.80, (58, 58, 64))],
    'carbody': [(0.10, (18, 20, 26)), (0.45, (62, 68, 78)), (0.80, (104, 112, 124))],
    'carrust': [(0.10, (30, 18, 14)), (0.45, (104, 52, 38)), (0.80, (156, 92, 62))],
    'slick': [(0.10, (10, 10, 14)), (0.45, (28, 28, 36)), (0.80, (54, 54, 66))],
}

# имя кадра, файл в паке, палитра, доля кадра по длинной стороне, поворот
FRAMES = [
    ('car_sedan', 'PNG/Cars/car_black_1.png', 'carbody', 0.94, True),
    ('car_van', 'PNG/Cars/car_blue_4.png', 'paint', 0.98, True),
    ('car_small', 'PNG/Cars/car_black_small_1.png', 'carrust', 0.80, True),
    ('car_wreck', 'burnt', None, 0.94, False),
    ('road_barrier', 'PNG/Objects/barrier_red.png', 'signal', 0.92, False),
    ('road_cone', 'PNG/Objects/cone_straight.png', 'signal', 0.44, False),
    ('fuel_barrel', 'PNG/Objects/barrel_red.png', 'rust', 0.50, False),
    ('tire_stack', 'PNG/Objects/tires_red.png', 'rubber', 0.62, False),
    ('oil_slick', 'PNG/Objects/oil.png', 'slick', 0.78, False),
    ('skid_mark', 'PNG/Objects/skidmark_long_1.png', 'slick', 0.90, True),
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
    tile = np.asarray(image).astype(float)

    tile = Recolour(tile, 'slick')

    rng = np.random.default_rng(31)
    height, width = tile.shape[0], tile.shape[1]
    for _ in range(26):
        row = int(rng.integers(0, height))
        column = int(rng.integers(0, width))
        radius = int(rng.integers(2, 6))
        top, left = max(0, row - radius), max(0, column - radius)
        patch = tile[top:row + radius, left:column + radius]
        patch[..., :3] = np.array(SOOT, dtype=float) + rng.normal(0.0, 6.0, 3)

    # Стёкол нет: выбитые окна темнее кузова.
    tile[..., :3] *= 0.92

    return Image.fromarray(np.clip(tile, 0, 255).astype(np.uint8), 'RGBA')


def Fit(image, part, isSideways):
    if isSideways:
        image = image.rotate(-90, expand=True)

    wanted = int(FRAME * part)
    image = image.copy()
    image.thumbnail((wanted, wanted), Image.LANCZOS)

    tile = Image.new('RGBA', (FRAME, FRAME), (0, 0, 0, 0))
    tile.paste(image, ((FRAME - image.width) // 2, (FRAME - image.height) // 2), image)

    return tile


def Build():
    sheet = Image.new('RGBA', (FRAME * len(FRAMES), FRAME), (0, 0, 0, 0))

    with zipfile.ZipFile(PACK) as pack:
        for index, (name, source, ramp, part, isSideways) in enumerate(FRAMES):
            image = Burnt(pack) if source == 'burnt' else Read(pack, source)

            if ramp is not None:
                pixels = Recolour(np.asarray(image).astype(float), ramp)
                image = Image.fromarray(np.clip(pixels, 0, 255).astype(np.uint8), 'RGBA')

            sheet.paste(Fit(image, part, isSideways), (index * FRAME, 0))
            print(index * FRAME, name)

    sheet.save(TARGET)
    print('built', TARGET, sheet.size)


if __name__ == '__main__':
    Build()
