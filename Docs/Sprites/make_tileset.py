import os
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
SOURCE = os.path.join(TEXTURES, 'tiles_ruins.png')

TILE = 64
FLOOR_FRAMES = 4
WALL_FRAMES = 16
ATLAS_COLUMNS = 16
ATLAS_ROWS = 4

FLOOR_ROW = 0
WALL_ROW = 1
LINE_ROW = 2
WATER_ROW = 3

RAMPS = {
    'catacombs': {
        'floor': [(0.00, (0, 0, 0)), (0.19, (28, 39, 47))],
        'wall': [(0.10, (20, 30, 37)), (0.34, (54, 74, 84)), (0.45, (70, 94, 106))],
        'line': (150, 156, 150),
        'water': [(18, 30, 34), (44, 72, 78)],
    },
    'prison': {
        'floor': [(0.00, (18, 20, 24)), (0.19, (88, 94, 104))],
        'wall': [(0.10, (30, 33, 38)), (0.34, (96, 100, 106)), (0.45, (132, 136, 142))],
        'line': (186, 190, 196),
        'water': [(20, 26, 34), (48, 62, 78)],
    },
    'street': {
        'floor': [(0.00, (16, 14, 12)), (0.19, (74, 68, 62))],
        'wall': [(0.10, (42, 26, 22)), (0.34, (120, 70, 54)), (0.45, (154, 100, 78))],
        'line': (198, 186, 120),
        'water': [(22, 26, 30), (52, 64, 72)],
    },
    'ruins': {
        'plain': True,
        'line': (170, 170, 164),
        'water': [(20, 28, 32), (50, 70, 80)],
    },
    'bridge': {
        'floor': [(0.00, (0, 0, 0)), (0.19, (78, 56, 36))],
        'wall': [(0.10, (46, 48, 52)), (0.34, (118, 122, 128)), (0.45, (162, 166, 172))],
        'line': (214, 212, 198),
        'water': [(16, 30, 48), (58, 104, 142)],
    },
}

ASPHALT_SETS = {'bridge'}
ASPHALT_BASE = (56, 58, 64)
ASPHALT_GRAIN = 7.0
ASPHALT_PATCH = (44, 46, 52)

LINE_THICKNESS = 5
# Штрих обязан укладываться в клетку целое число раз, иначе пунктир рвётся на стыке.
LINE_DASH = TILE
LINE_STROKE = 40


def Luminance(pixels):
    return (0.299 * pixels[..., 0] + 0.587 * pixels[..., 1] + 0.114 * pixels[..., 2]) / 255.0


def Along(ramp, level):
    stops = sorted(ramp)
    out = np.zeros(level.shape + (3,))
    for channel in range(3):
        out[..., channel] = np.interp(level, [stop for stop, _ in stops], [colour[channel] for _, colour in stops])

    return out


def Recolour(region, ramp):
    level = Luminance(region)
    out = region.copy()
    out[..., :3] = Along(ramp, level)

    return out


def Asphalt(seed):
    rng = np.random.default_rng(seed + 900)
    tile = np.zeros((TILE, TILE, 4))
    tile[..., 3] = 255

    grain = rng.normal(0.0, ASPHALT_GRAIN, (TILE, TILE, 1))
    tile[..., :3] = np.array(ASPHALT_BASE, dtype=float)[None, None, :] + grain

    for _ in range(rng.integers(2, 5)):
        centre = rng.integers(8, TILE - 8, 2)
        radius = rng.integers(4, 9)
        for row in range(-radius, radius + 1):
            for column in range(-radius, radius + 1):
                if row * row + column * column <= radius * radius:
                    tile[centre[0] + row, centre[1] + column, :3] = np.array(ASPHALT_PATCH, dtype=float) + rng.normal(0.0, 4.0, 3)

    if seed % 2 == 1:
        crack = rng.integers(6, TILE - 6)
        drift = 0
        for row in range(TILE):
            drift += rng.integers(-1, 2)
            column = int(np.clip(crack + drift, 1, TILE - 2))
            tile[row, column, :3] *= 0.72

    return np.clip(tile, 0, 255)


def Marking(seed, floor, colour, isVertical, isSolid):
    tile = floor.copy()
    paint = np.array(colour, dtype=float)
    middle = TILE // 2
    half = LINE_THICKNESS // 2

    for offset in range(-half, half + 1):
        band = middle + offset
        for step in range(TILE):
            if not isSolid and (step % LINE_DASH) >= LINE_STROKE:
                continue

            if isVertical:
                tile[step, band, :3] = paint
            else:
                tile[band, step, :3] = paint

    return tile


def Water(seed, shades):
    rng = np.random.default_rng(seed + 500)
    deep = np.array(shades[0], dtype=float)
    shine = np.array(shades[1], dtype=float)

    rows = np.arange(TILE)[:, None]
    columns = np.arange(TILE)[None, :]
    wave = 0.5 + 0.5 * np.sin(rows / 7.0 + seed) * np.cos(columns / 11.0 - seed)

    tile = np.zeros((TILE, TILE, 4))
    tile[..., 3] = 255
    tile[..., :3] = deep[None, None, :] + (shine - deep)[None, None, :] * (0.18 + 0.22 * wave[..., None])
    tile[..., :3] += rng.normal(0.0, 3.0, (TILE, TILE, 1))

    for _ in range(rng.integers(3, 7)):
        row = rng.integers(4, TILE - 4)
        column = rng.integers(4, TILE - 14)
        length = rng.integers(6, 13)
        tile[row, column:column + length, :3] = shine + rng.normal(0.0, 6.0, 3)

    return np.clip(tile, 0, 255)


def Build(name):
    ramp = RAMPS[name]
    source = np.asarray(Image.open(SOURCE).convert('RGBA')).astype(float)

    out = np.zeros((ATLAS_ROWS * TILE, ATLAS_COLUMNS * TILE, 4))
    out[..., 3] = 255
    out[0:2 * TILE, 0:source.shape[1]] = source[0:2 * TILE]

    # Исходник сам себе тайлсет: его пол и стены остаются как нарисованы.
    if not ramp.get('plain', False):
        out[0:TILE] = Recolour(out[0:TILE], ramp['floor'])
        out[TILE:2 * TILE] = Recolour(out[TILE:2 * TILE], ramp['wall'])

    if name in ASPHALT_SETS:
        for frame in range(FLOOR_FRAMES):
            out[0:TILE, frame * TILE:(frame + 1) * TILE] = Asphalt(frame + 1)

    # Разметка и вода идут у каждого тайлсета, чтобы у атласа была одна форма на всех.
    for frame in range(FLOOR_FRAMES):
        floor = out[0:TILE, (frame % FLOOR_FRAMES) * TILE:((frame % FLOOR_FRAMES) + 1) * TILE]
        isVertical = frame % 2 == 1
        isSolid = frame >= 2

        out[LINE_ROW * TILE:(LINE_ROW + 1) * TILE, frame * TILE:(frame + 1) * TILE] = \
            Marking(frame, floor, ramp['line'], isVertical, isSolid)

        out[WATER_ROW * TILE:(WATER_ROW + 1) * TILE, frame * TILE:(frame + 1) * TILE] = \
            Water(frame + 1, ramp['water'])

    out[..., 3] = 255
    target = os.path.join(TEXTURES, 'tiles_' + name + '.png')
    Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), 'RGBA').save(target)
    print('built', target, out.shape[1], 'x', out.shape[0])


if __name__ == '__main__':
    names = sys.argv[1:] or ['prison', 'street', 'bridge', 'catacombs', 'ruins']
    for name in names:
        Build(name)
