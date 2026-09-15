import os
import sys

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
SOURCE = os.path.join(TEXTURES, 'tiles_ruins.png')

TILE = 64
FLOOR_FRAMES = 4

RAMPS = {
    'catacombs': {
        'floor': [(0.00, (0, 0, 0)), (0.19, (28, 39, 47))],
        'wall': [(0.10, (20, 30, 37)), (0.34, (54, 74, 84)), (0.45, (70, 94, 106))],
    },
    'prison': {
        'floor': [(0.00, (18, 20, 24)), (0.19, (88, 94, 104))],
        'wall': [(0.10, (30, 33, 38)), (0.34, (96, 100, 106)), (0.45, (132, 136, 142))],
    },
    'street': {
        'floor': [(0.00, (16, 14, 12)), (0.19, (74, 68, 62))],
        'wall': [(0.10, (42, 26, 22)), (0.34, (120, 70, 54)), (0.45, (154, 100, 78))],
    },
    'bridge': {
        'floor': [(0.00, (0, 0, 0)), (0.19, (78, 56, 36))],
        'wall': [(0.10, (38, 28, 24)), (0.34, (112, 76, 54)), (0.45, (148, 106, 76))],
    },
}

PLANK_SETS = {'bridge'}
PLANK_HEIGHT = 16
PLANK_BASE = (96, 70, 46)
PLANK_SEAM = (44, 30, 20)
PLANK_EDGE = (124, 94, 64)


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


def Planks(seed):
    rng = np.random.default_rng(seed)
    tile = np.zeros((TILE, TILE, 4))
    tile[..., 3] = 255

    for row in range(TILE):
        board = row % PLANK_HEIGHT
        shade = 0.92 + 0.16 * ((row // PLANK_HEIGHT) % 3) / 2.0
        base = np.array(PLANK_BASE, dtype=float) * shade

        if board == 0:
            colour = np.array(PLANK_SEAM, dtype=float)
        elif board == 1:
            colour = np.array(PLANK_EDGE, dtype=float)
        else:
            colour = base

        grain = rng.normal(0.0, 4.0, TILE)
        tile[row, :, :3] = np.clip(colour[None, :] + grain[:, None], 0, 255)

    if seed % 2 == 1:
        knot = rng.integers(10, TILE - 10, 2)
        for offset in range(-3, 4):
            for side in range(-2, 3):
                if offset * offset + side * side * 2 <= 9:
                    tile[int(knot[0]) + offset, int(knot[1]) + side, :3] *= 0.78

    return tile


def Build(name):
    ramp = RAMPS[name]
    source = np.asarray(Image.open(SOURCE).convert('RGBA')).astype(float)

    out = source.copy()
    out[0:TILE] = Recolour(source[0:TILE], ramp['floor'])
    out[TILE:2 * TILE] = Recolour(source[TILE:2 * TILE], ramp['wall'])

    if name in PLANK_SETS:
        for frame in range(FLOOR_FRAMES):
            out[0:TILE, frame * TILE:(frame + 1) * TILE] = Planks(frame + 1)

    out[..., 3] = source[..., 3]
    target = os.path.join(TEXTURES, 'tiles_' + name + '.png')
    Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), 'RGBA').save(target)
    print('built', target)


if __name__ == '__main__':
    names = sys.argv[1:] or ['prison', 'street', 'bridge']
    for name in names:
        Build(name)
