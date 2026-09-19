import os
import wave

import numpy as np
import soundfile as sf

HERE = os.path.dirname(os.path.abspath(__file__))
SOURCE = os.path.join(HERE, 'Source')
AUDIO = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Audio'))

RATE = 48000
SILENCE = 0.02


def Read(name):
    data, rate = sf.read(os.path.join(SOURCE, name + '.ogg'))
    mono = data.mean(axis=1) if data.ndim > 1 else data
    if rate != RATE:
        raise ValueError(name + ' is not ' + str(RATE) + ' Hz')

    return mono


def Trim(samples, lead=0.01, tail=0.06):
    loud = np.abs(samples) > SILENCE
    if not loud.any():
        return samples

    first = max(0, int(np.argmax(loud)) - int(RATE * lead))
    last = min(len(samples), len(samples) - int(np.argmax(loud[::-1])) + int(RATE * tail))

    return samples[first:last]


def Fade(samples, attack=0.004, release=0.03):
    shaped = samples.copy()
    rise = min(int(RATE * attack), len(shaped))
    fall = min(int(RATE * release), len(shaped))
    if rise > 0:
        shaped[:rise] *= np.linspace(0.0, 1.0, rise)
    if fall > 0:
        shaped[-fall:] *= np.linspace(1.0, 0.0, fall)

    return shaped


def Level(samples, loudness, ceiling=0.85):
    current = np.sqrt((samples ** 2).mean())
    if current <= 0.0:
        return samples

    scaled = samples * (loudness / current)
    peak = np.abs(scaled).max()

    return scaled * (ceiling / peak) if peak > ceiling else scaled


def Place(track, samples, at):
    start = int(RATE * at)
    end = min(len(track), start + len(samples))
    track[start:end] += samples[:end - start]


def Write(name, samples, loudness):
    ready = np.clip(Level(samples, loudness), -1.0, 1.0)
    frames = (ready * 32000.0).astype(np.int16)
    path = os.path.join(AUDIO, name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(frames.tobytes())

    print('built %s  %.2f sec  rms %.4f  peak %.3f' % (
        name, len(frames) / float(RATE), np.sqrt((ready ** 2).mean()), np.abs(ready).max()))


def Step(seed, bodyHz, length=0.16, snap=0.012, decay=0.045):
    """Шаг по твёрдому полу: короткий щелчок каблука и глухое тело удара.

    Записи не берём - шаг звучит три раза в секунду, и любая характерная
    запись за минуту игры превращается в стук дятла. Синтез даёт ровно
    столько, сколько нужно: слышно, что кто-то идёт, и ничего сверх того.
    """
    rng = np.random.default_rng(seed)
    count = int(RATE * length)
    noise = rng.normal(0.0, 1.0, count)
    time = np.arange(count) / float(RATE)

    heel = noise * np.exp(-time / snap)
    body = Resonate(noise, bodyHz, bodyHz * 0.85, 90.0) * np.exp(-time / decay)

    return Fade(heel * 0.35 + body, 0.001, 0.05)


def Fold(name, source):
    """Стерео в моно. Громкость и частоту не трогаем: звук обязан остаться тем же,
    меняется только то, из-за чего SFML отказывается его пространствить."""
    data, rate = sf.read(os.path.join(SOURCE, source), dtype='int16', always_2d=True)
    mono = data.mean(axis=1).round().astype(np.int16)

    path = os.path.join(AUDIO, name)
    with wave.open(path, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(rate)
        out.writeframes(mono.tobytes())

    print('folded %s  %d ch -> 1  %d Hz  %.2f sec  peak %d' % (
        name, data.shape[1], rate, len(mono) / float(rate), int(np.abs(mono.astype(np.int32)).max())))


def Seam(samples, seconds=0.03):
    """Стык петли: хвост подмешивается в начало, иначе на повторе слышен щелчок."""
    blend = min(int(RATE * seconds), len(samples) // 4)
    if blend <= 0:
        return samples

    out = samples[:-blend].copy()
    ramp = np.linspace(0.0, 1.0, blend)
    out[:blend] = out[:blend] * ramp + samples[-blend:] * (1.0 - ramp)

    return out


def Resonate(samples, fromHz, toHz, sharpness, passes=2):
    """Двухполюсный резонатор с уползающей частотой - из шума получается визг.

    Один проход оставляет вокруг полосы слишком много шипения, поэтому по
    умолчанию фильтр проходит дважды: склоны становятся вдвое круче.
    """
    track = np.linspace(fromHz, toHz, len(samples))
    radius = np.exp(-np.pi * sharpness / RATE)
    turn = 2.0 * radius * np.cos(2.0 * np.pi * track / RATE)

    out = samples
    for _ in range(passes):
        source = out
        out = np.zeros(len(samples))
        back1 = 0.0
        back2 = 0.0
        for index in range(len(samples)):
            value = source[index] + turn[index] * back1 - radius * radius * back2
            out[index] = value
            back2 = back1
            back1 = value

        out = out * (1.0 - radius)

    return out


def Door():
    return Fade(Trim(Read('door_open_01')))


def Lever():
    return Fade(Trim(Read('lock_open_01')))


def Hatch():
    start = Fade(Trim(Read('metal_open_01')))
    slide = Fade(Trim(Read('metal_sheet_06'))) * 0.75
    lid = Fade(Trim(Read('metal_slam_01'))) * 0.9

    track = np.zeros(int(RATE * 1.15))
    Place(track, start, 0.0)
    Place(track, slide, 0.28)
    Place(track, lid, 0.80)

    return Fade(track, 0.004, 0.08)


def Engine():
    return Seam(Read('car_engine_loop'))


def Skid():
    """Визг покрышек синтезируется: записи заноса под CC0 в открытом доступе не нашлось.

    Резина на асфальте - это узкая резонансная полоса в шуме, которая ползёт вниз
    вместе со скоростью, и частая дрожь поверх: колесо то цепляется, то срывается.
    """
    rng = np.random.default_rng(7)
    length = int(RATE * 1.05)
    noise = rng.normal(0.0, 1.0, length)

    squeal = Resonate(noise, 1650.0, 620.0, 55.0)
    upper = Resonate(noise, 3300.0, 1240.0, 120.0) * 0.35
    road = Resonate(noise, 220.0, 150.0, 600.0, 1) * 0.18

    at = np.arange(length) / float(RATE)
    chirp = 1.0 + 0.22 * np.sin(2.0 * np.pi * 27.0 * at)

    # Срыв резкий, а затихает визг вместе с машиной - хвост длиннее атаки.
    body = np.clip(at / 0.05, 0.0, 1.0) * np.clip(1.0 - (at - 0.45) / 0.60, 0.0, 1.0)

    return Fade((squeal + upper) * chirp * body + road * body, 0.01, 0.12)


if __name__ == '__main__':
    Write('door_open.wav', Door(), 0.070)
    Write('lever.wav', Lever(), 0.085)
    Write('hatch_open.wav', Hatch(), 0.095)
    Write('car_engine.wav', Engine(), 0.075)
    Write('car_skid.wav', Skid(), 0.085)
    Fold('hurt.wav', 'hurt_stereo.wav')

    # Разные варианты, чтобы подряд идущие шаги не читались петлёй.
    for index, (seed, bodyHz) in enumerate([(11, 190.0), (23, 165.0), (37, 210.0), (51, 150.0)], start=1):
        Write(os.path.join('Steps', 'step_%d.wav' % index), Step(seed, bodyHz), 0.035)
