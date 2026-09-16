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
    ('*', 'WaveSpawn'),
    ('E', 'Escape:city_car'),
    ('R', 'Prop:bridge_rubble'),
    ('U', 'Prop:bridge_rubble_wide'),
    ('C', 'Prop:car_sedan@72'),
    ('V', 'Prop:car_van@104'),
    ('S', 'Prop:car_small@58'),
    ('W', 'Prop:car_wreck@84'),
]

# Край пролома: символ один, а каким куском бетона он обернётся, решает движок по соседям.
BREACH = 'X'

# Точки маршрута для охраны блокпоста: 1 - пост, 2 - дальний конец обхода.
PATROL_SYMBOLS = ('1', '2')

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
            PutWide(rows, column, lane, rng.choice('cvswcvsCVS'))
            column += rng.randint(4, 8)

    for _ in range(rng.randint(1, 3)):
        Put(rows, rng.randint(3, WIDTH - 5), rng.randint(ROAD_TOP, ROAD_BOTTOM), 'l')


def Checkpoint(rows, rng):
    """Блокпост: стена барьеров с проходом по центру и охрана при нём.

    Проход намеренно оставлен: заслон должен тормозить бегущего, а не запирать.
    """
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

    Garrison(rows, column)


def Garrison(rows, column):
    """Охрана блокпоста: щитовик в проходе, стрелки за барьером, патруль вдоль него.

    Патруль ходит по своей стороне барьера - тот, кто держит заслон,
    не должен стоять истуканом, пока игрок обходит его по краю.
    """
    Put(rows, column + 1, LANE_LINE, 'h')

    for row in (LANE_LINE - 4, LANE_LINE + 4):
        Put(rows, column + 2, row, 'r')

    # Две точки на разных концах барьера: между ними и ходит караул.
    Put(rows, column + 3, ROAD_TOP + 1, PATROL_SYMBOLS[0])
    Put(rows, column + 3, ROAD_BOTTOM - 1, PATROL_SYMBOLS[1])
    Put(rows, column + 4, ROAD_TOP + 1, 'g')


def Wreck(rows, rng):
    """ДТП: машины сбились в кучу и стоят поперёк, а не по линейке."""
    heart = rng.randint(8, WIDTH - 12)

    PutWide(rows, heart, LANE_LINE - 1, 'W')
    PutWide(rows, heart + 2, LANE_LINE + 1, 'C')
    PutWide(rows, heart - 3, LANE_LINE, 'V')
    PutWide(rows, heart + 4, LANE_LINE - 2, 's')
    PutWide(rows, heart - 5, LANE_LINE + 2, 'S')

    for _ in range(5):
        Put(rows, heart + rng.randint(-6, 7), rng.randint(ROAD_TOP, ROAD_BOTTOM), 'l')

    for _ in range(3):
        Put(rows, heart + rng.randint(-8, 8), rng.choice([ROAD_TOP, ROAD_BOTTOM]), 'k')

    for _ in range(2):
        Put(rows, heart + rng.randint(-7, 8), rng.randint(ROAD_TOP, ROAD_BOTTOM), 'o')


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
    """Пролом: полотно обрывается рваным краем, под ним вода.

    Граница нарочно неровная - ровный прямоугольник читался бы как вырез
    ножницами, а не как обвалившийся пролёт. Проход вдоль отбойника остаётся
    всегда: пролом должен пугать и тормозить, а не запирать мост.
    """
    left = rng.randint(8, WIDTH - 16)
    width = rng.randint(7, 10)
    isTop = rng.random() < 0.5

    top = ROAD_TOP if isTop else LANE_LINE + 1
    bottom = LANE_LINE if isTop else ROAD_BOTTOM

    # Сколько рядов съедено в каждой колонке. Глубина гуляет вокруг середины,
    # а не копится шаг за шагом: от накопления край выходил ровной лестницей.
    span = bottom - top + 1
    middle = max(2, int(span * 0.6))

    # Уступы редкие и крупные: если менять глубину каждую колонку,
    # край превращается в мелкую корону вместо обломанной плиты.
    marks = []
    column = 0
    while column < width:
        marks.append((column, middle + rng.randint(-2, 1)))
        column += rng.randint(2, 4)
    marks.append((width - 1, middle + rng.randint(-2, 1)))

    eaten = []
    for column in range(width):
        before = max(mark for mark in marks if mark[0] <= column)
        after = min((mark for mark in marks if mark[0] >= column), default=before)

        depth = before[1] if column - before[0] <= after[0] - column else after[1]

        # Края пролома мельче середины: так он читается как провалившийся кусок.
        toEdge = min(column, width - 1 - column)
        depth = depth if toEdge >= 2 else int(round(depth * (0.45 + 0.3 * toEdge)))

        eaten.append(max(2, min(span, depth)))

    for step, column in enumerate(range(left, left + width)):
        for into in range(eaten[step]):
            row = top + into if isTop else bottom - into
            rows[row][column] = WATER

    # Куски плиты, выброшенные на уцелевшее полотно: обвал не бывает аккуратным.
    for _ in range(rng.randint(4, 7)):
        column = rng.randint(max(2, left - 3), min(WIDTH - 3, left + width + 3))
        row = rng.randint(ROAD_TOP, ROAD_BOTTOM)
        if rows[row][column] == ROAD:
            Put(rows, column, row, rng.choice('RRU'))

    for _ in range(3):
        Put(rows, left - 2, rng.randint(ROAD_TOP, ROAD_BOTTOM), 'o')

    PutWide(rows, left + width + 1, LANE_LINE + 2 if isTop else LANE_LINE - 2, 'W')

    return Breach(rows, left - 1, left + width, top, bottom)


def Breach(rows, fromColumn, toColumn, top, bottom):
    """Обводит воду краем пролома в верхнем слое.

    Символ ставится на каждую клетку воды, у которой есть сосед-дорога;
    какой именно кусок бетона нарисовать, решает движок по маске соседей.
    """
    edges = []

    for row in range(max(0, top - 1), min(HEIGHT, bottom + 2)):
        for column in range(max(0, fromColumn), min(WIDTH, toColumn + 1)):
            if rows[row][column] != WATER:
                continue

            isEdge = False
            for nearRow, nearColumn in ((row - 1, column), (row + 1, column), (row, column - 1), (row, column + 1)):
                if 0 <= nearRow < HEIGHT and 0 <= nearColumn < WIDTH:
                    isEdge = isEdge or rows[nearRow][nearColumn] in (ROAD, LINE)

            if isEdge:
                edges.append((column, row))

    return edges


def WavePoints(rows, index):
    for column in (6, 16, 26):
        for row in (ROAD_TOP, ROAD_BOTTOM):
            if rows[row][column] in (ROAD, LINE):
                rows[row][column] = '*'


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

    # Пролом возвращает клетки своего края, остальные шаблоны ничего не возвращают.
    edges = SHAPES[index](rows, rng)
    edges = edges if edges is not None else []
    WavePoints(rows, index)

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

        # Машина ждёт у самого конца, выход прячется под ней: уехать можно только на ней.
        rows[LANE_LINE][WIDTH - 4] = 'E'
        rows[LANE_LINE][WIDTH - 3] = '>'
    else:
        share = index / float(SECTIONS - 1)
        count = 1 + int(share * 4)
        kinds = 'gg' if share < 0.3 else ('ggrh' if share < 0.7 else 'grhed')
        Enemies(rows, rng, count, kinds)

    return rows, edges


def SplitOverlay(rows):
    """Уводит разметку в верхний слой.

    В нижнем слое каждый кадр разметки несёт свой кусок асфальта, поэтому их
    надо держать столько же, сколько вариантов покрытия. Наверху это просто
    белая полоса на прозрачном, и ложится она на любой асфальт.
    """
    overlay = [[GAP] * WIDTH for _ in range(HEIGHT)]

    for row in range(HEIGHT):
        for column in range(WIDTH):
            if rows[row][column] == LINE:
                rows[row][column] = ROAD
                overlay[row][column] = LINE

    return overlay


def Write(index, rows, edges):
    name = 'act1_bridge_%02d' % (index + 1)
    lines = ['[level]', 'kind %s' % name, 'tileset bridge', '', '[legend]']
    lines += ['%s %s' % (symbol, meaning) for symbol, meaning in LEGEND]

    # Маршрут именуется по секции: одно имя на весь мост склеило бы все посты в один обход.
    if any(symbol in ''.join(''.join(row) for row in rows) for symbol in PATROL_SYMBOLS):
        route = 'block%02d' % (index + 1)
        lines += ['%s Patrol:%s' % (symbol, route) for symbol in PATROL_SYMBOLS]

    if edges:
        lines += ['%s Breach' % BREACH]
    overlay = SplitOverlay(rows)

    for column, row in edges:
        overlay[row][column] = BREACH

    lines += ['', '[map]']
    lines += [''.join(row) for row in rows]

    if any(LINE in row or BREACH in row for row in overlay):
        lines += ['', '[overlay]']
        lines += [''.join(row) for row in overlay]

    path = os.path.join(ROOMS, name + '.config')
    with open(path, 'w', encoding='utf-8-sig', newline='\r\n') as out:
        out.write('\n'.join(lines) + '\n')

    return name


def Build(seed=20260916):
    rng = random.Random(seed)

    names = []
    for index in range(SECTIONS):
        rows, edges = Section(index, rng)
        names.append(Write(index, rows, edges))

    lines = ['[act]', 'title Мост', 'next street', 'tileset bridge', 'music march', '', '[library]']
    lines += ['%s Resources/Rooms/%s.config' % (name, name) for name in names]
    lines += ['', '[pursuit]']
    lines += [
        '; keep - сколько держится на хвосте, grow - до скольки растёт, если стоять на месте',
        '; respawn - через сколько приходит замена убитому',
        'keep 6',
        'grow 12',
        'respawn 2.5',
        '',
        '; from <доля пути> <символ врага><вес>: кто выбегает на этом отрезке моста',
        'from 0.0 m4 g3',
        'from 0.25 m3 a3 g2',
        'from 0.5 a4 s2 m2',
        'from 0.75 a3 s3 h2 r1',
    ]
    lines += ['', '[rooms]']
    lines += ['%s %d 0' % (name, index * WIDTH) for index, name in enumerate(names)]

    path = os.path.join(ACTS, 'act1_bridge.config')
    with open(path, 'w', encoding='utf-8-sig', newline='\r\n') as out:
        out.write('\n'.join(lines) + '\n')

    print('built %d sections, %dx%d' % (SECTIONS, SECTIONS * WIDTH, HEIGHT))


if __name__ == '__main__':
    Build()
