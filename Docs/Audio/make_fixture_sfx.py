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
    with wave.open(path, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(frames.tobytes())

    print('built %s  %.2f sec  rms %.4f  peak %.3f' % (
        name, len(frames) / float(RATE), np.sqrt((ready ** 2).mean()), np.abs(ready).max()))


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


if __name__ == '__main__':
    Write('door_open.wav', Door(), 0.070)
    Write('lever.wav', Lever(), 0.085)
    Write('hatch_open.wav', Hatch(), 0.095)
