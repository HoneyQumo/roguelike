"""Синтезирует звуки ловушек. Исходников не нужно - всё считается на месте.

Шипы - железный лязг: короткий удар и несколько призвуков, которые гаснут
с разной скоростью. Растяжка - щелчок и звон лопнувшей струны.
"""

import os
import wave

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
AUDIO = os.path.normpath(os.path.join(HERE, '..', '..', 'Roguelike', 'Resources', 'Audio'))

RATE = 48000
LOUDNESS = 0.11
CEILING = 0.88


def Time(seconds):
    return np.arange(int(RATE * seconds)) / float(RATE)


def Ring(seconds, frequency, decay, noise=0.0):
    """Затухающий тон. Шум подмешивается в атаку - от него звук становится железным."""
    time = Time(seconds)
    tone = np.sin(2.0 * np.pi * frequency * time) * np.exp(-time * decay)

    if noise > 0.0:
        burst = np.random.default_rng(int(frequency)).normal(0.0, 1.0, len(time))
        tone += burst * np.exp(-time * decay * 6.0) * noise

    return tone


def Mix(parts):
    longest = max(len(part) for part in parts)
    track = np.zeros(longest)
    for part in parts:
        track[:len(part)] += part

    return track


def Level(samples):
    current = np.sqrt((samples ** 2).mean())
    if current <= 0.0:
        return samples

    scaled = samples * (LOUDNESS / current)
    peak = np.abs(scaled).max()

    return scaled * (CEILING / peak) if peak > CEILING else scaled


def Write(name, samples):
    shaped = samples.copy()
    fall = min(int(RATE * 0.02), len(shaped))
    shaped[-fall:] *= np.linspace(1.0, 0.0, fall)

    frames = (np.clip(Level(shaped), -1.0, 1.0) * 32000.0).astype(np.int16)
    path = os.path.join(AUDIO, name + '.wav')
    with wave.open(path, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(frames.tobytes())

    print('made %-16s %.2f sec' % (name, len(frames) / float(RATE)))


def Spikes():
    # Низкий удар держит вес, верхние призвуки дают железо.
    return Mix([
        Ring(0.35, 118.0, 26.0, noise=0.9),
        Ring(0.30, 287.0, 19.0, noise=0.4),
        Ring(0.26, 611.0, 24.0),
        Ring(0.18, 1490.0, 38.0),
    ])


def Wire():
    # Щелчок колышка, потом звон отпущенной струны.
    click = Ring(0.06, 900.0, 90.0, noise=1.2)
    snap = Mix([
        Ring(0.45, 1760.0, 13.0),
        Ring(0.40, 2640.0, 17.0),
        Ring(0.30, 3510.0, 24.0),
    ])

    track = np.zeros(len(snap) + int(RATE * 0.02))
    track[:len(click)] += click
    track[int(RATE * 0.02):int(RATE * 0.02) + len(snap)] += snap * 0.8

    return track


if __name__ == '__main__':
    Write('trap_spikes', Spikes())
    Write('trap_wire', Wire())
