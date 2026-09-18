import io
import os
import re
import zipfile

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
TEXTURES = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Textures'))
PACK = os.path.join(HERE, 'vendor', 'kenney_tds.zip')
TARGET = os.path.join(TEXTURES, 'props_act1.png')

TILE = 64

RAMPS = {
    'steel': [(0.10, (46, 50, 56)), (0.45, (116, 122, 130)), (0.80, (170, 176, 184))],
    'sand': [(0.10, (96, 78, 48)), (0.45, (186, 162, 112)), (0.80, (226, 208, 166))],
    'concrete': [(0.10, (62, 64, 66)), (0.45, (140, 142, 144)), (0.80, (192, 194, 196))],
    'iron': [(0.10, (38, 44, 40)), (0.45, (76, 96, 80)), (0.80, (120, 140, 122))],
    'wood': [(0.10, (52, 36, 22)), (0.45, (128, 92, 56)), (0.80, (176, 136, 92))],
    'lamp': [(0.10, (88, 78, 52)), (0.45, (208, 190, 130)), (0.80, (252, 244, 206))],
}

FRAMES = [
    ('bunk', 431, None),
    ('bars', 'bars', None),
    ('guard_desk', 454, 'wood'),
    ('guard_desk_spent', 292, 'wood'),
    ('locker', 531, 'steel'),
    ('bush', 183, None),
    ('shop_glass', 436, None),
    ('shop_glass_spent', 'shards', None),
    ('bin', 530, 'iron'),
    ('bin_spent', 292, 'iron'),
    ('sandbags', 186, 'sand'),
    ('concrete_block', 237, 'concrete'),
    ('cable_spool', 206, 'steel'),
    ('lamp', 215, 'lamp'),
]

BAR_COLUMNS = (10, 22, 34, 46, 58)
BAR_WIDTH = 4
BAR_LIGHT = (156, 162, 170)
BAR_DARK = (64, 70, 78)
SHARD_COLOUR = (172, 200, 208)


def Luminance(pixels):
    return (0.299 * pixels[..., 0] + 0.587 * pixels[..., 1] + 0.114 * pixels[..., 2]) / 255.0


def Recolour(tile, ramp):
    level = Luminance(tile)
    out = tile.copy()
    stops = sorted(RAMPS[ramp])
    for channel in range(3):
        out[..., channel] = np.interp(level, [stop for stop, _ in stops], [colour[channel] for _, colour in stops])

    return out


def Bars():
    tile = np.zeros((TILE, TILE, 4))
    for column in BAR_COLUMNS:
        for offset in range(BAR_WIDTH):
            x = column + offset
            shade = np.array(BAR_LIGHT if offset < 2 else BAR_DARK, dtype=float)
            tile[2:TILE - 2, x, :3] = shade
            tile[2:TILE - 2, x, 3] = 255

    for y in (2, 3, TILE - 4, TILE - 3):
        tile[y, 6:TILE - 2, :3] = np.array(BAR_LIGHT, dtype=float)
        tile[y, 6:TILE - 2, 3] = 255

    return tile


def Shards():
    rng = np.random.default_rng(7)
    tile = np.zeros((TILE, TILE, 4))
    for piece in range(14):
        top = int(rng.integers(6, TILE - 12))
        left = int(rng.integers(6, TILE - 12))
        height = int(rng.integers(3, 8))
        width = int(rng.integers(3, 8))
        shade = 0.7 + 0.5 * rng.random()
        tile[top:top + height, left:left + width, :3] = np.array(SHARD_COLOUR, dtype=float) * shade
        tile[top:top + height, left:left + width, 3] = 210

    return tile


def Load(pack, number):
    names = [n for n in pack.namelist() if n.startswith('PNG/Tiles/') and n.endswith('.png')]
    wanted = [n for n in names if int(re.search(r'(\d+)', n.rsplit('/', 1)[1]).group(1)) == number]
    if not wanted:
        raise KeyError(number)

    return np.asarray(Image.open(io.BytesIO(pack.read(wanted[0]))).convert('RGBA')).astype(float)


def Build():
    sheet = np.zeros((TILE, TILE * len(FRAMES), 4))
    with zipfile.ZipFile(PACK) as pack:
        for index, (name, source, ramp) in enumerate(FRAMES):
            if source == 'bars':
                tile = Bars()
            elif source == 'shards':
                tile = Shards()
            else:
                tile = Load(pack, source)
                if ramp is not None:
                    tile = Recolour(tile, ramp)

            sheet[:, index * TILE:(index + 1) * TILE] = tile
            print(index * TILE, name)

    Image.fromarray(np.clip(sheet, 0, 255).astype(np.uint8), 'RGBA').save(TARGET)
    print('built', TARGET, sheet.shape)


if __name__ == '__main__':
    Build()
