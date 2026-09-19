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
ATLAS_ROWS = 6

WALL_NEIGHBOUR_UP = 1
WALL_NEIGHBOUR_RIGHT = 2
WALL_NEIGHBOUR_DOWN = 4
WALL_NEIGHBOUR_LEFT = 8

FLOOR_ROW = 0
WALL_ROW = 1
LINE_ROW = 2
WATER_ROW = 3
OVERLAY_ROW = 4
BREACH_ROW = 5

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


CONCRETE = (116, 114, 108)
CONCRETE_DARK = (58, 56, 52)
REBAR = (122, 82, 56)

BREACH_DEPTH = 13
REBAR_COUNT = 3


def BreachEdge(mask, seed):
    """Обрыв бетона по краю пролома.

    Кадр выбирается по маске соседей: бит стоит там, где к клетке примыкает
    дорога, и именно с той стороны полотно обрывается. Внутри клетки пусто -
    сквозь неё видно нижний слой, то есть воду.
    """
    rng = np.random.default_rng(seed + 900)
    tile = np.zeros((TILE, TILE, 4))

    concrete = np.array(CONCRETE, dtype=float)
    shadow = np.array(CONCRETE_DARK, dtype=float)
    iron = np.array(REBAR, dtype=float)

    def Lay(side):
        # Кромка рваная: глубина гуляет вдоль стороны, ровный срез выглядел бы распилом.
        depth = rng.integers(BREACH_DEPTH - 6, BREACH_DEPTH + 5, size=TILE)
        depth = np.convolve(depth, np.ones(5) / 5.0, mode='same').astype(int)

        for step in range(TILE):
            for into in range(max(1, depth[step])):
                if side == 'up':
                    row, column = into, step
                elif side == 'down':
                    row, column = TILE - 1 - into, step
                elif side == 'left':
                    row, column = step, into
                else:
                    row, column = step, TILE - 1 - into

                # У самого среза плита уходит в тень: там уже обрыв, а не поверхность.
                edge = max(1, depth[step]) - into
                shade = 0.0 if edge > 6 else 1.0 - edge / 6.0

                grain = rng.normal(0.0, 9.0)
                colour = concrete * (1.0 - shade) + shadow * shade + grain

                tile[row, column, :3] = np.clip(colour, 0, 255)
                tile[row, column, 3] = 255

        # Арматура торчит из среза в пустоту.
        for _ in range(REBAR_COUNT):
            at = int(rng.integers(6, TILE - 6))
            out = int(rng.integers(5, 13))
            for into in range(depth[at], depth[at] + out):
                if into >= TILE:
                    break

                if side == 'up':
                    row, column = into, at
                elif side == 'down':
                    row, column = TILE - 1 - into, at
                elif side == 'left':
                    row, column = at, into
                else:
                    row, column = at, TILE - 1 - into

                tile[row, column, :3] = iron
                tile[row, column, 3] = 255

    if mask & WALL_NEIGHBOUR_UP:
        Lay('up')
    if mask & WALL_NEIGHBOUR_DOWN:
        Lay('down')
    if mask & WALL_NEIGHBOUR_LEFT:
        Lay('left')
    if mask & WALL_NEIGHBOUR_RIGHT:
        Lay('right')

    return tile


def OverlayMarking(colour, isVertical, isSolid):
    """Разметка для верхнего слоя: та же линия, но на прозрачном фоне.

    В нижнем слое каждый кадр разметки тащит с собой кусок асфальта, поэтому
    их приходится держать столько же, сколько вариантов покрытия. Накладка
    ложится на любое покрытие и ни от чего не зависит.
    """
    tile = np.zeros((TILE, TILE, 4))
    paint = np.array(colour, dtype=float)
    middle = TILE // 2
    half = LINE_THICKNESS // 2

    for offset in range(-half, half + 1):
        band = middle + offset
        for step in range(TILE):
            if not isSolid and (step % LINE_DASH) >= LINE_STROKE:
                continue

            row, column = (step, band) if isVertical else (band, step)
            tile[row, column, :3] = paint
            tile[row, column, 3] = 255

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


PARAPET_SETS = {'bridge'}

BRICK_HEIGHT = 11
BRICK_LENGTH = 21
BRICK_BASE = (96, 70, 62)
BRICK_DARK = (58, 42, 38)
BRICK_SEAM = (42, 34, 32)

RAIL_TOP_ROW = 6
RAIL_HEIGHT = 13
RAIL_METAL = (150, 156, 164)
RAIL_SHINE = (198, 204, 212)
RAIL_SHADOW = (68, 72, 78)


def Parapet(mask, seed):
    """Кирпичная кладка, а поверх - металлический отбойник.

    Балка рисуется только там, где парапет тянется вдоль: у клетки есть сосед
    слева или справа. На торцах остаётся голая кладка, иначе отбойник
    обрывался бы в воздухе.
    """
    rng = np.random.default_rng(seed + 700)
    tile = np.zeros((TILE, TILE, 4))
    tile[..., 3] = 255

    for row in range(TILE):
        course = row // BRICK_HEIGHT
        shift = (course % 2) * (BRICK_LENGTH // 2)

        for column in range(TILE):
            isSeam = (row % BRICK_HEIGHT) == 0 or ((column + shift) % BRICK_LENGTH) == 0
            shade = 0.86 + 0.1 * ((course * 7 + (column + shift) // BRICK_LENGTH * 3) % 4) / 3.0

            colour = np.array(BRICK_SEAM if isSeam else BRICK_BASE, dtype=float) * (1.0 if isSeam else shade)
            tile[row, column, :3] = colour + rng.normal(0.0, 3.5, 3)

    tile[TILE - 3:, :, :3] = np.array(BRICK_DARK, dtype=float)

    isAlong = (mask & WALL_NEIGHBOUR_LEFT) != 0 or (mask & WALL_NEIGHBOUR_RIGHT) != 0
    if isAlong:
        top = RAIL_TOP_ROW
        tile[top:top + RAIL_HEIGHT, :, :3] = np.array(RAIL_METAL, dtype=float)
        tile[top:top + 3, :, :3] = np.array(RAIL_SHINE, dtype=float)
        tile[top + RAIL_HEIGHT - 3:top + RAIL_HEIGHT, :, :3] = np.array(RAIL_SHADOW, dtype=float)

        # Стойки под балкой: ритм, по которому отбойник читается как отбойник.
        for column in range(6, TILE, 26):
            tile[top + RAIL_HEIGHT:top + RAIL_HEIGHT + 9, column:column + 5, :3] = np.array(RAIL_SHADOW, dtype=float)

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

    if name in PARAPET_SETS:
        for mask in range(WALL_FRAMES):
            out[TILE:2 * TILE, mask * TILE:(mask + 1) * TILE] = Parapet(mask, mask + 1)

    # Разметка и вода идут у каждого тайлсета, чтобы у атласа была одна форма на всех.
    for frame in range(FLOOR_FRAMES):
        floor = out[0:TILE, (frame % FLOOR_FRAMES) * TILE:((frame % FLOOR_FRAMES) + 1) * TILE]
        isVertical = frame % 2 == 1
        isSolid = frame >= 2

        out[LINE_ROW * TILE:(LINE_ROW + 1) * TILE, frame * TILE:(frame + 1) * TILE] = \
            Marking(frame, floor, ramp['line'], isVertical, isSolid)

        out[WATER_ROW * TILE:(WATER_ROW + 1) * TILE, frame * TILE:(frame + 1) * TILE] = \
            Water(frame + 1, ramp['water'])

    # Непрозрачны только нижние слои: накладке прозрачность и нужна.
    out[0:OVERLAY_ROW * TILE, ..., 3] = 255

    # Верхние строки гасятся до прозрачности: иначе кадр, который никто
    # не рисовал, остаётся чёрным квадратом и ждёт первого, кто его запросит.
    out[OVERLAY_ROW * TILE:, ..., 3] = 0

    for frame in range(FLOOR_FRAMES):
        out[OVERLAY_ROW * TILE:(OVERLAY_ROW + 1) * TILE, frame * TILE:(frame + 1) * TILE] = \
            OverlayMarking(ramp['line'], frame % 2 == 1, frame >= 2)

    for mask in range(WALL_FRAMES):
        out[BREACH_ROW * TILE:(BREACH_ROW + 1) * TILE, mask * TILE:(mask + 1) * TILE] = BreachEdge(mask, mask + 1)
    target = os.path.join(TEXTURES, 'tiles_' + name + '.png')
    Image.fromarray(np.clip(out, 0, 255).astype(np.uint8), 'RGBA').save(target)
    print('built', target, out.shape[1], 'x', out.shape[0])


if __name__ == '__main__':
    names = sys.argv[1:] or ['prison', 'street', 'bridge', 'catacombs', 'ruins']
    for name in names:
        Build(name)
