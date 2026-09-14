# Атлас объектов - Resources/Textures/props.png

Полоса из семи кадров 64x64, всего 448x64. Кадры вырезаны из CC0-набора
Kenney "Top-down Shooter" (`Docs/Sprites/vendor/README.md`), без изменений.

| x | Кадр | Исходный тайл | Где используется |
|---|---|---|---|
| 0 | ящик | `PNG/Tiles/tile_129.png` | `crate_wood.frame` |
| 64 | ящик разбит | `PNG/Tiles/tile_156.png` | `crate_wood.spentFrame` |
| 128 | бочка | `PNG/Tiles/tile_132.png` | `barrel_rusty.frame` |
| 192 | склад заперт | `PNG/Tiles/tile_131.png` | `chest_supply.frame` |
| 256 | склад открыт | `PNG/Tiles/tile_180.png` | `chest_supply.spentFrame` |
| 320 | малый ящик | `PNG/Tiles/tile_130.png` | `crate_empty.frame` |
| 384 | малый ящик разбит | `PNG/Tiles/tile_157.png` | `crate_empty.spentFrame` |

Кадр тайла совпадает с `TILE_SIZE`, но объект рисуется своим `size` из
`props.config`, поэтому кадр масштабируется под клетку объекта, а не под тайл.

У бочки нет своего кадра разрушения: `PropVisualComponent` в этом случае
перекрашивает спрайт в `brokenColor`. Тот же путь работает и для объекта
без `frame` вообще - тогда он рисуется прямоугольником, как раньше.
