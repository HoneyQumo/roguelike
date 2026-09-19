# Инструменты

Здесь лежат генераторы: текстуры, звуки и карта моста не рисуются руками, а собираются скриптами. Готовые файлы лежат в `Roguelike/Resources` и отслеживаются git - скрипты нужны, чтобы их можно было собрать заново или поправить.

Каждый скрипт считает путь к ресурсам от своего расположения, на два уровня вверх. Если переносить папку - вложенность `Tools/<область>/<скрипт>` должна сохраниться.

## Что нужно

```bash
pip install numpy pillow soundfile
```

`make_fixtures.py` обходится одним Pillow, `make_bridge.py` - только стандартной библиотекой.

Двум генераторам пропов нужны бесплатные паки Kenney. Ссылки и имена файлов - в `Sprites/vendor/README.md`, сами архивы в git не лежат.

## Спрайты

Запускать из корня репозитория.

| скрипт | что собирает | что нужно на вход |
|---|---|---|
| `Sprites/make_tileset.py` | `tiles_prison.png`, `tiles_street.png`, `tiles_bridge.png`, `tiles_catacombs.png`, `tiles_ruins.png` | `Resources/Textures/tiles_ruins.png` |
| `Sprites/make_props.py` | `props_act1.png` - 14 кадров обстановки | `Sprites/vendor/kenney_tds.zip` |
| `Sprites/make_props_bridge.py` | `props_bridge.png` - машины и дорожное | `Sprites/vendor/kenney_racing.zip` |
| `Sprites/make_fire.py` | `fire.png` - 8 больших кадров пламени и 8 мелких | ничего, рисуется кодом |
| `Sprites/make_smoke.py` | `smoke.png` - лента из 6 кадров | ничего, рисуется кодом |
| `Sprites/make_fixtures.py` | `fixtures.png` - 5 кадров люка и 2 рычага | ничего, рисуется кодом |

```bash
python Tools/Sprites/make_tileset.py street
```

Без аргумента `make_tileset.py` пересобирает все пять наборов. Набор `ruins` служит исходником для остальных и пересобирается из самого себя - результат при этом не меняется.

## Раскладки

`Sprites/character.json`, `Sprites/weapons.json` и `Sprites/boss_puppeteer.json` ничем не читаются - это описание того, что лежит в атласах. С них сняты константы в `SpriteAtlas.h`, `WeaponCatalog.h` и `BossSpriteAtlas.h`. Меняется атлас - меняется и раскладка, иначе код разойдётся с картинкой.

## Звук

```bash
python Tools/Audio/make_fixture_sfx.py
```

Собирает `door_open.wav`, `lever.wav`, `hatch_open.wav`, `car_engine.wav` и `car_skid.wav` из записей в `Audio/Source`. Каждый файл приводится к заданной громкости с потолком по пику, чтобы механизмы не перекрикивали остальной звук.

Он же сводит `hurt.wav` из стерео-оригинала в моно. Громкость и частота при этом не меняются: сведение нужно не ради звука, а потому что стерео SFML не пространствляет - позиция и затухание к такому буферу не применяются. По той же причине все звуки в игре обязаны быть моно, и за этим следит `ShippedResourcesTest`.

## Карта моста

```bash
python Tools/Maps/make_bridge.py
```

Собирает 14 комнат `act1_bridge_*.config` и файл акта. Генератор детерминированный - при одном и том же сиде выдаёт те же карты.

Длина разгона в начале карты, `ARRIVAL_RUNWAY`, связана с `ARRIVAL_ENTRY_OFFSET` из `GameSettings.h`: машине нужно место, чтобы доехать до точки высадки. Менять одно, не глядя на второе, нельзя.
