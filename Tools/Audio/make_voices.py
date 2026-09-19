"""Озвучивает реплики из каталога.

Тексты берутся из Roguelike/Resources/Speech/speech.config - того же файла,
который читает игра. Так субтитр и голос не могут разойтись: они из одного
источника, а не набраны дважды.

Модель Silero в репозиторий не кладём, как и паки Kenney: ссылка и имя файла
в vendor/README.md.
"""

import os
import re
import wave

import numpy as np
import torch

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
SPEECH = os.path.join(ROOT, 'Roguelike', 'Resources', 'Speech', 'speech.config')
VOICE = os.path.join(ROOT, 'Roguelike', 'Resources', 'Audio', 'Voice')
MODEL = os.path.join(HERE, 'vendor', 'v4_ru.pt')

RATE = 48000
LOUDNESS = 0.075
CEILING = 0.85

# Голос выбирается по имени звука: voice_<кто>_<номер>.
# Разные тембры, а не один, растянутый просодией, - иначе враги на слух одинаковы.
VOICES = {
    'guard': 'aidar',
    'heavy': 'eugene',
    'radio': 'baya',
}


def ReadLines(path):
    """Достаёт из каталога пары «звук - текст». Разбор тот же, что в игре."""
    lines = []
    sound = None
    text = None

    with open(path, encoding='utf-8-sig') as file:
        for raw in file:
            line = raw.strip()
            if not line or line.startswith(';'):
                continue

            if line.startswith('[line ') or line.startswith('[set '):
                if sound and text:
                    lines.append((sound, text))
                sound = None
                text = None
                continue

            key, _, value = line.partition(' ')
            if key == 'sound':
                sound = value.strip()
            elif key == 'text':
                text = value.strip()

    if sound and text:
        lines.append((sound, text))

    return lines


def VoiceOf(sound):
    match = re.match(r'voice_([a-z]+)_\d+$', sound)
    if not match or match.group(1) not in VOICES:
        raise ValueError('не знаю, каким голосом говорить: ' + sound)

    return VOICES[match.group(1)]


def Level(samples):
    current = np.sqrt((samples ** 2).mean())
    if current <= 0.0:
        return samples

    scaled = samples * (LOUDNESS / current)
    peak = np.abs(scaled).max()

    return scaled * (CEILING / peak) if peak > CEILING else scaled


def Write(path, samples):
    frames = (np.clip(Level(samples), -1.0, 1.0) * 32000.0).astype(np.int16)

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(frames.tobytes())

    return len(frames) / float(RATE)


if __name__ == '__main__':
    if not os.path.exists(MODEL):
        raise SystemExit('нет модели ' + MODEL + ' - см. vendor/README.md')

    model = torch.package.PackageImporter(MODEL).load_pickle('tts_models', 'model')
    model.to(torch.device('cpu'))

    for sound, text in ReadLines(SPEECH):
        voice = VoiceOf(sound)
        said = model.apply_tts(text=text, speaker=voice, sample_rate=RATE).numpy()

        seconds = Write(os.path.join(VOICE, sound + '.wav'), said)
        print('voiced %-16s %-8s %.2f sec  %s' % (sound, voice, seconds, text))
