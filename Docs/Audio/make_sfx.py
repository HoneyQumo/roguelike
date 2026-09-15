import os
import struct
import wave

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
AUDIO = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Audio'))

RATE = 44100


def Write(name, samples):
    data = np.clip(samples, -1.0, 1.0)
    frames = (data * 32000.0).astype(np.int16)
    path = os.path.join(AUDIO, name)
    with wave.open(path, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(frames.tobytes())

    print('built', path, round(len(frames) / float(RATE), 3), 'sec')


def Envelope(length, attack, release):
    line = np.ones(length)
    rise = int(RATE * attack)
    fall = int(RATE * release)
    if rise > 0:
        line[:rise] = np.linspace(0.0, 1.0, rise)
    if fall > 0:
        line[-fall:] = np.linspace(1.0, 0.0, fall)

    return line


def Noise(length, seed):
    rng = np.random.default_rng(seed)
    return rng.normal(0.0, 1.0, length)


def LowPass(samples, keep):
    out = np.zeros_like(samples)
    value = 0.0
    for index, sample in enumerate(samples):
        value += (sample - value) * keep
        out[index] = value

    return out


def Door():
    length = int(RATE * 0.55)
    time = np.arange(length) / float(RATE)

    sweep = np.sin(2.0 * np.pi * np.cumsum(np.linspace(150.0, 78.0, length)) / RATE)
    creak = np.sign(np.sin(2.0 * np.pi * np.cumsum(np.linspace(320.0, 210.0, length)) / RATE)) * 0.22
    grain = LowPass(Noise(length, 11), 0.05) * 1.4

    body = sweep * 0.5 + creak + grain * 0.5
    thud = np.sin(2.0 * np.pi * 62.0 * time) * np.exp(-time * 26.0) * 0.5

    return (body * Envelope(length, 0.02, 0.22) + thud) * 0.8


def Lever():
    length = int(RATE * 0.18)
    time = np.arange(length) / float(RATE)

    click = Noise(length, 5) * np.exp(-time * 190.0)
    ping = np.sin(2.0 * np.pi * 1450.0 * time) * np.exp(-time * 42.0) * 0.55
    low = np.sin(2.0 * np.pi * 240.0 * time) * np.exp(-time * 30.0) * 0.4

    return (click * 0.7 + ping + low) * 0.85


def Hatch():
    length = int(RATE * 1.15)
    time = np.arange(length) / float(RATE)

    grind = LowPass(Noise(length, 23), 0.03) * 2.2
    grind *= np.minimum(1.0, time * 6.0) * np.exp(-np.maximum(0.0, time - 0.75) * 7.0)

    scrape = np.sin(2.0 * np.pi * np.cumsum(np.linspace(96.0, 148.0, length)) / RATE) * 0.3
    scrape *= np.exp(-np.maximum(0.0, time - 0.7) * 6.0)

    clangAt = int(RATE * 0.86)
    clang = np.zeros(length)
    tail = np.arange(length - clangAt) / float(RATE)
    clang[clangAt:] = (np.sin(2.0 * np.pi * 520.0 * tail) * 0.5 + np.sin(2.0 * np.pi * 780.0 * tail) * 0.3
                       + np.sin(2.0 * np.pi * 131.0 * tail) * 0.45) * np.exp(-tail * 9.0)

    return (grind * 0.55 + scrape + clang) * 0.8


if __name__ == '__main__':
    Write('door_open.wav', Door())
    Write('lever.wav', Lever())
    Write('hatch_open.wav', Hatch())
