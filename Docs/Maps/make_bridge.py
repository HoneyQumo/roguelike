"""Раскладывает полотно моста по шаблонам секций.

Секция - 32x24 клетки. По вертикали всегда одно и то же: вода, отбойник,
полоса, разметка, полоса, отбойник, вода. Шаблон рисует поверх этой основы,
но крайние колонки не трогает - иначе не сойдутся швы между секциями.
"""

import os
import random

HERE = os.path.dirname(os.path.abspath(__file__))
ROOMS = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Rooms'))
ACTS = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Acts'))

WIDTH = 32
HEIGHT = 24
SECTIONS = 22


WATER_TOP = 5
RAIL_TOP = 5
ROAD_TOP = 6
LANE_LINE = 11
ROAD_BOTTOM = 16
RAIL_BOTTOM = 17

WATER = '~'
RAIL = '#'
ROAD = '.'
LINE = '-'
GAP = ' '

LEGEND = [
    ('#', 'Wall'),
    ('.', 'Floor'),
    ('~', 'Water'),
    ('-', 'Line'),
    ('@', 'PlayerSpawn'),
    ('<', 'Entrance'),
    ('>', 'Exit'),
    ('c', 'Prop:car_sedan'),
    ('v', 'Prop:car_van'),
    ('s', 'Prop:car_small'),
    ('w', 'Prop:car_wreck'),
    ('b', 'Prop:road_barrier'),
    ('o', 'Prop:road_cone'),
    ('f', 'Prop:fuel_barrel'),
    ('t', 'Prop:tire_stack'),
    ('l', 'Prop:oil_slick'),
    ('k', 'Prop:skid_mark'),
    ('x', 'Prop:crate_ammo'),
    ('m', 'Prop:crate_medical'),
    ('a', 'Prop:crate_armor'),
    ('g', 'MarauderSpawn'),
    ('r', 'AssaultSpawn'),
    ('h', 'ShieldSpawn'),
    ('e', 'HeavySpawn'),
    ('d', 'RadioSpawn'),
]

# Проп шириной в две клетки: ставим его через одну, чтобы машины не слипались.
WIDE = set('cvsw')


def Blank():
    rows = []
    for row in range(HEIGHT):
        if row < WATER_TOP or row > RAIL_BOTTOM:
            rows.append([WATER] * WIDTH)
        elif row == RAIL_TOP or row == RAIL_BOTTOM:
            rows.append([RAIL] * WIDTH)
        elif row == LANE_LINE:
            rows.append([LINE] * WIDTH)
        else:
            rows.append([ROAD] * WIDTH)

    return rows


def IsFree(rows, column, row):
    return rows[row][column] in (ROAD, LINE)


def Put(rows, column, row, symbol):
    if 1 <= column < WIDTH - 1 and ROAD_TOP <= row <= ROAD_BOTTOM and IsFree(rows, column, row):
        rows[row][column] = symbol
        return True

    return False


def PutWide(rows, column, row, symbol):
    if not IsFree(rows, column, row) or column + 1 >= WIDTH - 1 or not IsFree(rows, column + 1, row):
        return False

    return Put(rows, column, row, symbol)


def Plain(rows, rng):
    for _ in range(rng.randint(3, 6)):
        Put(rows, rng.randint(2, WIDTH - 4), rng.choice([ROAD_TOP + 1, ROAD_BOTTOM - 1]), 'k')

    for _ in range(rng.randint(1, 3)):
        Put(rows, rng.randint(3, WIDTH - 5), rng.randint(ROAD_TOP, ROAD_BOTTOM), 'o')

    # Даже на пустом пролёте есть за чем присесть.
    for _ in range(rng.randint(1, 3)):
        PutWide(rows, rng.randint(3, WIDTH - 6), rng.randint(ROAD_TOP, ROAD_BOTTOM), rng.choice('csw'))

    Put(rows, rng.randint(3, WIDTH - 5), rng.randint(ROAD_TOP, ROAD_BOTTOM), 't')


def Jam(rows, rng):
    for lane in (ROAD_TOP + 1, ROAD_BOTTOM - 1, LANE_LINE - 2, LANE_LINE + 2):
        column = rng.randint(2, 6)
        while column < WIDTH - 5:
            PutWide(rows, column, lane, rng.choice('cvsw'))
            column += rng.randint(4, 8)

    for _ in range(rng.randint(1, 3)):
        Put(rows, rng.randint(3, WIDTH - 5), rng.randint(ROAD_TOP, ROAD_BOTTOM), 'l')


def Checkpoint(rows, rng):
    column = rng.randint(10, 18)
    for row in range(ROAD_TOP, ROAD_BOTTOM + 1):
        if row in (LANE_LINE - 1, LANE_LINE, LANE_LINE + 1):
            continue

        Put(rows, column, row, 'b')

    for row in (ROAD_TOP, ROAD_BOTTOM):
        Put(rows, column - 2, row, 't')

    for offset in (-4, 4):
        Put(rows, column + offset, LANE_LINE - 3, 'o')
        Put(rows, column + offset, LANE_LINE + 3, 'o')

    PutWide(rows, column - 6, LANE_LINE - 2, 'v')
    PutWide(rows, column + 3, LANE_LINE + 2, 'c')


def Wreck(rows, rng):
    heart = rng.randint(8, WIDTH - 12)
    PutWide(rows, heart, LANE_LINE - 1, 'w')
    PutWide(rows, heart + 3, LANE_LINE + 1, 'w')
    PutWide(rows, heart - 4, LANE_LINE + 2, 's')

    for _ in range(4):
        Put(rows, heart + rng.randint(-5, 6), rng.randint(ROAD_TOP, ROAD_BOTTOM), 'l')

    for _ in range(3):
        Put(rows, heart + rng.randint(-8, 8), rng.choice([ROAD_TOP, ROAD_BOTTOM]), 'k')


def Fuel(rows, rng):
    column = rng.randint(6, WIDTH - 12)
    for step in range(rng.randint(3, 5)):
        Put(rows, column + step * 2, LANE_LINE - 3, 'f')
        Put(rows, column + step * 2, LANE_LINE + 3, 'f')

    PutWide(rows, column + 2, LANE_LINE - 1, 'v')
    PutWide(rows, column + 5, LANE_LINE + 1, 'c')
    Put(rows, column - 2, LANE_LINE, 't')


def Ramp(rows, rng):
    """Карман за отбойником: лут в стороне от прямого пути."""
    isTop = rng.random() < 0.5
    left = rng.randint(8, WIDTH - 16)
    width = 8
    depth = 4

    if isTop:
        outer = RAIL_TOP - depth
        rows[RAIL_TOP][left:left + width] = [ROAD] * width
        for row in range(outer, RAIL_TOP):
            rows[row][left:left + width] = [ROAD] * width
        rows[outer - 1][left - 1:left + width + 1] = [RAIL] * (width + 2)
        for row in range(outer, RAIL_TOP):
            rows[row][left - 1] = RAIL
            rows[row][left + width] = RAIL
        pocket = outer + 1
    else:
        outer = RAIL_BOTTOM + depth
        rows[RAIL_BOTTOM][left:left + width] = [ROAD] * width
        for row in range(RAIL_BOTTOM + 1, outer + 1):
            rows[row][left:left + width] = [ROAD] * width
        rows[outer + 1][left - 1:left + width + 1] = [RAIL] * (width + 2)
        for row in range(RAIL_BOTTOM + 1, outer + 1):
            rows[row][left - 1] = RAIL
            rows[row][left + width] = RAIL
        pocket = outer - 1

    for index, crate in enumerate(rng.sample(['x', 'm', 'a', 'x'], 3)):
        rows[pocket][left + 1 + index * 2] = crate

    Put(rows, left + 2, LANE_LINE - 2 if isTop else LANE_LINE + 2, 'o')
    Put(rows, left + 5, LANE_LINE - 2 if isTop else LANE_LINE + 2, 'o')


def Collapse(rows, rng):
    """Пролёт обвалился: полотно съедено водой, остаётся полоса вдоль отбойника."""
    left = rng.randint(8, WIDTH - 16)
    width = rng.randint(7, 10)
    isTop = rng.random() < 0.5

    top = ROAD_TOP if isTop else LANE_LINE + 1
    bottom = LANE_LINE if isTop else ROAD_BOTTOM

    for row in range(top, bottom + 1):
        for column in range(left, left + width):
            rows[row][column] = WATER

    for _ in range(3):
        Put(rows, left - 2, rng.randint(ROAD_TOP, ROAD_BOTTOM), 'o')

    PutWide(rows, left + width + 1, LANE_LINE + 2 if isTop else LANE_LINE - 2, 'w')


def Enemies(rows, rng, count, kinds):
    placed = 0
    guard = 0
    while placed < count and guard < 400:
        guard += 1
        column = rng.randint(3, WIDTH - 4)
        row = rng.randint(ROAD_TOP, ROAD_BOTTOM)
        if Put(rows, column, row, rng.choice(kinds)):
            placed += 1


# Ритм задан руками: обрушения и пандусы должны встречаться регулярно,
# а не выпадать как придётся из арифметики по номеру секции.
SHAPES = [
    Plain, Jam, Wreck, Ramp, Fuel, Collapse, Jam, Checkpoint,
    Ramp, Wreck, Collapse, Fuel, Jam, Ramp, Checkpoint, Collapse,
    Wreck, Fuel, Jam, Ramp, Collapse, Plain,
]


def Section(index, rng):
    rows = Blank()

    SHAPES[index](rows, rng)

    # Первые секции щадящие, дальше плотнее.
    # Торцы моста закрыты: за первой секцией и за последней ехать некуда.
    if index == 0:
        for row in range(HEIGHT):
            rows[row][0] = RAIL
        rows[LANE_LINE - 2][2] = '@'
        rows[LANE_LINE][2] = '<'
    elif index == SECTIONS - 1:
        for row in range(HEIGHT):
            rows[row][WIDTH - 1] = RAIL
        rows[LANE_LINE][WIDTH - 3] = '>'
    else:
        share = index / float(SECTIONS - 1)
        count = 1 + int(share * 4)
        kinds = 'gg' if share < 0.3 else ('ggrh' if share < 0.7 else 'grhed')
        Enemies(rows, rng, count, kinds)

    return rows


def Write(index, rows):
    name = 'act1_bridge_%02d' % (index + 1)
    lines = ['[level]', 'kind %s' % name, 'tileset bridge', '', '[legend]']
    lines += ['%s %s' % (symbol, meaning) for symbol, meaning in LEGEND]
    lines += ['', '[map]']
    lines += [''.join(row) for row in rows]

    path = os.path.join(ROOMS, name + '.config')
    with open(path, 'w', encoding='utf-8-sig', newline='\r\n') as out:
        out.write('\n'.join(lines) + '\n')

    return name


def Build(seed=20260916):
    rng = random.Random(seed)

    names = []
    for index in range(SECTIONS):
        names.append(Write(index, Section(index, rng)))

    lines = ['[act]', 'title Мост', 'next street', 'tileset bridge', 'music march', '', '[library]']
    lines += ['%s Resources/Rooms/%s.config' % (name, name) for name in names]
    lines += ['', '[rooms]']
    lines += ['%s %d 0' % (name, index * WIDTH) for index, name in enumerate(names)]

    path = os.path.join(ACTS, 'act1_bridge.config')
    with open(path, 'w', encoding='utf-8-sig', newline='\r\n') as out:
        out.write('\n'.join(lines) + '\n')

    print('built %d sections, %dx%d' % (SECTIONS, SECTIONS * WIDTH, HEIGHT))


if __name__ == '__main__':
    Build()
