// sfml_example.cpp — рабочий референс интеграции спрайт-листов.
//
// Собирается без внешних зависимостей кроме SFML:
//     g++ -std=c++17 sfml_example.cpp -o demo
//        -lsfml-graphics -lsfml-window -lsfml-system
//
// Проверено компиляцией на SFML 2.6.1. Весь используемый API одинаков
// в 2.5.1: setTextureRect, setOrigin, setRotation, Shader::setUniform.
//
// Управление: WASD движение, мышь прицел, ЛКМ огонь, Space кувырок,
// R перезарядка, F быстрый удар, ПКМ держать — мощный заряжаемый
// удар (отпустить = ударить), 1-9 смена ствола (с анимацией),
// K убить себя, E подобрать
// ствол с пола, G выбросить свой.
//
// Раскладка ниже ДУБЛИРУЕТ character.json намеренно: пример не тащит
// json-библиотеку. В боевом коде читайте JSON, а не эти константы.

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace {

constexpr int   FRAME   = 64;                 // кадр тела
constexpr float PIVOT   = 32.f;               // пивот тела и лужи крови
constexpr int   WPN_W   = 160, WPN_H = 64;    // кадр оружия в руках
constexpr float WPN_PX  = 80.f, WPN_PY = 32.f;// пивот оружия — та же точка мира
constexpr int   SIDE_W  = 112, SIDE_H = 32;   // кадр вида сбоку
constexpr float SIDE_PX = 56.f, SIDE_PY = 16.f;
constexpr float PICKUP_R = 22.f;              // радиус подбора
constexpr float PI      = 3.14159265f;
constexpr int   SCALE_UP = 3;                 // целый множитель на экран

float deg(float rad) { return rad * 180.f / PI; }

float wrapPi(float a) {
    while (a >  PI) a -= 2.f * PI;
    while (a < -PI) a += 2.f * PI;
    return a;
}

// --- раскладка атласа тела: строка = анимация, столбец = кадр -------------
enum State { IDLE, WALK, RUN, SHOOT, RELOAD, MELEE, HURT, DEATH, HEAVY, SWAP,
             ROLL, NSTATE };

// ms — одна длительность на строку. У мощной атаки она НЕ используется:
// там покадровый HEAVY_MS, см. frameMs() ниже.
struct AnimDef { int row, frames, ms; bool loop; };

// Порядок строк ровно как в atlas.py / character.json.
const AnimDef ANIM[NSTATE] = {
    /* IDLE   */ {0,  6, 175, true },
    /* WALK   */ {1,  8, 110, true },
    /* RUN    */ {2,  8,  75, true },
    /* SHOOT  */ {3,  4,  60, false},
    /* RELOAD */ {4, 12,  90, false},
    /* MELEE  */ {5,  6,  60, false},
    /* HURT   */ {6,  3,  80, false},
    /* DEATH  */ {7,  8, 110, false},
    /* HEAVY  */ {8, 12,  81, false},   // только у ГГ; ms не использовать
    /* SWAP   */ {9, 10,  55, false},   // только у ГГ; ms не использовать
    /* ROLL   */ {10,10,  50, false},   // row здесь — row0, дальше +направление
};
constexpr int ROLL_DIRS = 8;

// --- мощная атака: заряжаемый удар сверху с шагом -------------------------
// Покадровый тайминг — единственная анимация набора, где он нужен: замах
// по 85-110 мс против удара по 34 мс, и именно этот контраст читается как
// вес. При равномерном тайминге мощный удар выглядит как быстрый, длиннее.
const int HEAVY_MS[12] = {70, 85, 95, 110, 110, 90, 90, 34, 34, 46, 95, 115};
const float HEAVY_MOVE[12] = {0,0,0,0,0,0,0, 0.55f, 1.00f, 0.72f, 0.18f, 0};
const sf::Vector2f HEAVY_OFF[12] = {{0,0},{0,0},{0,0},{0,0},{0,0},{1,-1},
                                    {-1,1},{0,0},{0,0},{0,0},{0,0},{0,0}};
const float HEAVY_ROT[12] = {0, 44, 88, 116, 110, 122, 115, 48, -20, -78, -40, 0};
constexpr int HEAVY_CHARGING[2] = {3, 4};   // петля: набор заряда
constexpr int HEAVY_CHARGED[2]  = {5, 6};   // петля: заряд набран
constexpr int HEAVY_RELEASE     = 7;        // куда прыгать по отпусканию
constexpr int HEAVY_HIT0 = 8, HEAVY_HIT1 = 9;
constexpr float HEAVY_CHARGE_MS = 440.f;    // за столько петля набора
                                            // переходит в «заряд набран»

// --- смена оружия: убрать старый за спину, достать новый ------------------
// SWAP_FRAME — кадр, на котором меняется СТРОКА оружия. Он внутри
// SWAP_HIDE: подмена спрайта на видимом кадре читается как рывок.
// На скрытых кадрах старый ствол рисуется отдельным слоем из столбца
// SIDE_NOGLOVE (без перчаток) по SWAP_STOW / SWAP_SROT.
const int  SWAP_MS[10]   = {55, 60, 65, 70, 70, 60, 55, 45, 35, 30};
const bool SWAP_HIDE[10] = {false,false,true,true,true,true,false,false,
                            false,false};
const int  SWAP_VAR[10]  = {0, 1, 0, 0, 0, 0, 1, 0, 0, 0};
const sf::Vector2f SWAP_STOW[10] = {{0,0},{0,0},{-13,12},{-17,7},{-18,-1},
                                    {-13,-11},{0,0},{0,0},{0,0},{0,0}};
const float SWAP_SROT[10] = {0,0,-96.f,-124.f,-150.f,-170.f,0,0,0,0};
constexpr int SWAP_FRAME = 5;
constexpr int WPN_NOGLOVE = 3;      // столбец weapons.png без перчаток

int frameMs(State s, int frame) {
    if (s == HEAVY) return HEAVY_MS[frame];
    if (s == SWAP)  return SWAP_MS[frame];
    return ANIM[s].ms;
}

// weapon_offset: сдвиг слоя оружия в пикселях кадра, по кадрам анимации.
const sf::Vector2f RUN_OFF[8]  = {{0,3},{0,4},{0,3},{0,2},{0,3},{0,4},{0,3},{0,2}};
const sf::Vector2f WALK_OFF[8] = {{0,0},{0,1},{0,0},{0,-1},{0,0},{0,1},{0,0},{0,-1}};
const sf::Vector2f SHOOT_OFF[4]= {{-3,0},{-2,0},{-1,0},{0,0}};
const sf::Vector2f ROLL_OFF[10]= {{0,1},{0,0},{0,0},{0,0},{0,0},
                                  {0,0},{0,0},{0,0},{-1,1},{0,0}};

// Поворот СЛОЯ ОРУЖИЯ по кадрам, в градусах, прибавляется к углу прицела.
// Слой крутится вокруг своего пивота, а он совпадает с пивотом тела,
// поэтому оружие идёт дугой вокруг корпуса, а руки остаются на рукояти.
const float MELEE_ROT[6] = {34.f, 18.f, -22.f, -52.f, -28.f, 0.f};
constexpr int MELEE_HIT_FRAME = 3;

// Кадры, на которых слой оружия НЕ рисуется.
const bool ROLL_HIDE [10] = {false,true,true,true,true,true,true,true,false,false};
const bool DEATH_HIDE[8]  = {false,false,true,true,true,true,true,true};
// death.drop_frame: с этого кадра ствол исчезает из рук и становится лутом.
constexpr int DEATH_DROP_FRAME = 1;

// Столбец в weapons.png по кадру перезарядки: 0 обычный, 1 без поддерживающей
// перчатки, 2 без перчатки и магазина.
const int RELOAD_VARIANT[12] = {0,1,1,2,2,2,2,2,2,2,1,0};
const bool RELOAD_ITEM  [12] = {false,false,false,true,true,true,
                                false,true,true,true,false,false};
// Точка предмета в руке, смещение от пивота в пикселях кадра.
const sf::Vector2f RELOAD_ANCHOR[12] = {
    {25.2f,11.6f},{21.5f,17.4f},{17.4f,20.6f},{16.4f,23.8f},
    {12.2f,26.6f},{ 8.4f,25.8f},{ 3.9f,19.6f},{ 2.3f,15.1f},
    { 7.8f,20.6f},{14.2f,23.0f},{17.4f,19.4f},{25.2f,11.6f}};

// Доля пиковой скорости рывка по кадрам кувырка. Без неё анимация
// и перемещение разъедутся: персонаж поедет на вставании и зависнет в полёте.
const float ROLL_SPEED[10] = {0.10f,0.55f,1.f,1.f,0.92f,0.66f,0.46f,0.30f,0.10f,0.f};

// --- прямоугольники в fx.png (из character.json -> atlas.fx.rects) --------
struct FxRect { int x, y, w, h, frames; float px, py; };
const FxRect FX_MUZZLE = {  0,   0, 32, 24, 3,  4.f, 12.f};
const FxRect FX_POOL   = {  0,  24, 64, 64,10, 32.f, 32.f};
const FxRect FX_EXPL   = {  0,  88, 64, 64, 7, 32.f, 32.f};
const FxRect FX_BHIT   = {  0, 152, 32, 24, 4,  6.f, 12.f};
const FxRect FX_SPECK  = {  0, 176, 32, 24, 1,  6.f, 12.f};
const FxRect FX_IMPACT = {  0, 200, 24, 24, 4,  4.f, 12.f};
const FxRect FX_PICKUP = {  0, 224, 24, 24, 6, 12.f, 12.f};
const FxRect FX_ITEM   = {  0, 248, 16, 16, 6,  8.f,  8.f};
const FxRect FX_SHELL  = {  0, 264, 12, 12, 4,  6.f,  6.f};
const FxRect FX_ROCKET = {  0, 276, 28, 12, 2,  4.f,  6.f};
const FxRect FX_BULLET = {  0, 288, 24,  8, 3,  2.f,  4.f};

sf::IntRect fxFrame(const FxRect& r, int i) {
    return sf::IntRect(r.x + r.w * i, r.y, r.w, r.h);
}

// --- оружие: строка в weapons.png ----------------------------------------
struct WeaponDef {
    // muzX/muzY — muzzle_offset из weapons.json: смещение дульного
    // среза ОТ ПИВОТА ТЕЛА в пикселях кадра. Ничего вычитать не надо.
    // sb* — side_bbox: занятый прямоугольник внутри кадра 112x32 в
    // weapons_side.png. Нужен, чтобы обрезать иконку под инвентарь.
    const char* id; int row; float muzX, muzY; float recoil, flashScale;
    int sbx, sby, sbw, sbh;
};
const WeaponDef WEAPONS[] = {
    {"ak47",                 0,   58.56f, 12.16f, 1.00f, 1.00f,  17,  4, 78, 24},
    {"m16",                  1,   60.80f, 12.16f, 0.85f, 0.90f,  15,  3, 82, 26},
    {"shotgun_double",       2,   53.60f, 12.16f, 1.50f, 1.35f,  19,  6, 73, 20},
    {"shotgun_pump",         3,   55.20f, 11.52f, 1.35f, 1.25f,  18,  7, 76, 17},
    {"smg_suppressed",       4,   57.60f, 12.16f, 0.60f, 0.35f,  19,  4, 74, 24},
    {"glock",                5,   32.00f, 11.68f, 0.55f, 0.65f,  40,  3, 32, 25},
    {"deagle",               6,   38.88f, 12.00f, 1.40f, 1.20f,  36,  3, 40, 25},
    {"pistol_suppressed",    7,   56.00f, 12.16f, 0.50f, 0.30f,  27,  3, 57, 25},
    {"knife",                8,   42.40f, 15.36f, 0.00f, 0.00f,  32,  9, 47, 14},
    {"bat",                  9,   55.20f, 12.32f, 0.00f, 0.00f,  25, 10, 62, 12},
    {"rpg",                 10,   76.00f, 12.16f, 1.80f, 1.60f,   5,  2,101, 27},
};
constexpr int NWEAPON = int(sizeof(WEAPONS) / sizeof(WEAPONS[0]));

// --- лут на полу ---------------------------------------------------------
// Лежащий ствол берётся из weapons_side.png (вид СБОКУ), а не из
// weapons.png (вид сверху): сверху пистолет — палка, и на полу его не
// отличить от ножа. Строка та же, столбец 1 — подсветка «можно поднять».
// Поворот НЕ применяется: предмет лежит, ему незачем доворачиваться за
// прицелом. Для разнообразия допустимо зеркалить по X — сетка не рвётся.
struct Loot {
    sf::Vector2f pos;
    int  weapon = 0;
    bool flip   = false;
    bool taken  = false;
};

// -------------------------------------------------------------------------
struct Character {
    sf::Vector2f pos;
    float   aim      = 0.f;      // радианы
    State   state    = IDLE;
    int     frame    = 0;
    int     rollDir  = 0;        // 0..7, ОТНОСИТЕЛЬНО прицела
    float   timer    = 0.f;
    float   flash    = 0.f;      // сила белой вспышки урона
    float   poolT    = -1.f;     // таймер лужи крови, <0 — лужи нет
    float   hitT     = -1.f;     // таймер брызг, <0 — брызг нет
    float   hitAng   = 0.f;      // куда летят брызги: по вектору урона
    int     hp       = 3;
    float   deathAng = 0.f;      // угол, на котором труп замер
    bool    dead     = false;
    int     weapon   = 0;
    bool    dropped  = false;    // ствол уже выпал на пол
    bool    holding  = false;    // кнопка мощной атаки зажата
    float   chargeT  = 0.f;      // сколько держим, мс
    bool    charged  = false;    // заряд набран: крутим вторую петлю
    int     pendingWeapon = -1;  // куда меняемся; применится на SWAP_FRAME
    sf::Vector2f rollVec;        // направление рывка

    const AnimDef& def() const { return ANIM[state]; }

    void play(State s) {
        if (state == s && def().loop) return;
        state = s; frame = 0; timer = 0.f;
    }

    // Строка кувырка = угол МЕЖДУ рывком и прицелом. Восемь строк так
    // покрывают все направления мира, и ствол остаётся на цели.
    void startRoll(sf::Vector2f dir) {
        if (dir.x == 0.f && dir.y == 0.f) dir = {std::cos(aim), std::sin(aim)};
        rollVec = dir;
        float rel = wrapPi(std::atan2(dir.y, dir.x) - aim);
        rollDir = ((int)std::lround(rel * 4.f / PI) % ROLL_DIRS + ROLL_DIRS) % ROLL_DIRS;
        play(ROLL);
    }

    // Урон: белая вспышка шейдером + брызги отдельным слоем. Оба живут
    // СВОИМИ таймерами, а не кадрами анимации: попасть могут в перекате,
    // на перезарядке, в любой момент любой анимации.
    void takeHit(float bulletAngleRad) {
        if (dead) return;
        flash  = 1.f;                 // гаснет за 120 мс, decay_ms из JSON
        hitT   = 0.f;
        hitAng = deg(bulletAngleRad); // брызги летят ПО ходу пули, от стрелка
        if (--hp <= 0) kill();
        else           play(HURT);
    }

    void update(float dt) {
        flash = std::max(0.f, flash - dt / 0.12f);
        if (state == HEAVY && holding) {
            chargeT += dt * 1000.f;
            if (chargeT >= HEAVY_CHARGE_MS) charged = true;
        }
        if (poolT >= 0.f) poolT += dt;
        if (hitT  >= 0.f) {
            hitT += dt;
            if (hitT * 1000.f / 45.f >= 4.f) hitT = -1.f;   // 4 кадра по 45 мс
        }

        timer += dt * 1000.f;
        const AnimDef& a = def();
        while (timer >= frameMs(state, frame)) {
            timer -= frameMs(state, frame);
            if (state == HEAVY && advanceHeavy(dt)) continue;
            if (state == SWAP && frame + 1 == SWAP_FRAME && pendingWeapon >= 0) {
                weapon = pendingWeapon;      // подмена строки, кадр скрыт
                pendingWeapon = -1;
            }
            if (frame + 1 < a.frames)      ++frame;
            else if (a.loop)               frame = 0;
            else if (state == DEATH)       break;          // труп остаётся лежать
            else                           { play(IDLE); break; }
        }
        if (state == ROLL)
            pos += rollVec * 260.f * ROLL_SPEED[frame] * dt;
        if (state == HEAVY)
            pos += sf::Vector2f{std::cos(aim), std::sin(aim)}
                   * 150.f * HEAVY_MOVE[frame] * dt;
        if (state == DEATH && frame == 3 && poolT < 0.f)
            poolT = 0.f;                                    // blood_pool_start_frame
    }

    // Разбор двух петель удержания. Возвращает true, если кадр уже выбран
    // здесь и общий счётчик трогать не надо.
    //
    // Удержание НЕ растягивает кадры, а ПОВТОРЯЕТ петлю: растянутый кадр
    // читается как подвисшая игра, повтор — как накопление.
    bool advanceHeavy(float) {
        const bool inCharging = (frame == HEAVY_CHARGING[0] ||
                                 frame == HEAVY_CHARGING[1]);
        const bool inCharged  = (frame == HEAVY_CHARGED[0] ||
                                 frame == HEAVY_CHARGED[1]);
        if (!inCharging && !inCharged) return false;
        if (!holding) { frame = HEAVY_RELEASE; return true; }
        if (inCharging) {
            if (frame == HEAVY_CHARGING[0]) { frame = HEAVY_CHARGING[1]; return true; }
            // конец петли набора: либо по кругу, либо переход в «заряжено»
            frame = charged ? HEAVY_CHARGED[0] : HEAVY_CHARGING[0];
            return true;
        }
        frame = (frame == HEAVY_CHARGED[0]) ? HEAVY_CHARGED[1] : HEAVY_CHARGED[0];
        return true;
    }

    void startHeavy() {
        if (dead || state == ROLL || state == HEAVY) return;
        holding = true; chargeT = 0.f; charged = false;
        play(HEAVY);
    }

    void releaseHeavy() {
        holding = false;
        // Если игрок отпустил во время заноса, петли просто не сыграют:
        // анимация доиграет занос и уйдёт в удар сама.
    }

    void kill() { if (!dead) { dead = true; deathAng = deg(aim); play(DEATH); } }

    float drawAngle() const { return dead ? deathAng : deg(aim); }

    sf::IntRect bodyRect() const {
        int row = (state == ROLL) ? ANIM[ROLL].row + rollDir : def().row;
        return sf::IntRect(frame * FRAME, row * FRAME, FRAME, FRAME);
    }

    bool weaponHidden() const {
        if (state == ROLL)  return ROLL_HIDE[frame];
        if (state == DEATH) return DEATH_HIDE[frame];
        if (state == SWAP)  return SWAP_HIDE[frame];
        return false;
    }

    int weaponVariant() const {
        if (state == RELOAD) return RELOAD_VARIANT[frame];
        if (state == SWAP)   return SWAP_VAR[frame];
        return 0;
    }

    // Смена оружия. Строка меняется РОВНО на SWAP_FRAME, поэтому новый
    // ствол хранится отдельно до этого момента.
    void startSwap(int to) {
        if (dead || state == ROLL || state == SWAP || to == weapon) return;
        pendingWeapon = to;
        play(SWAP);
    }

    // Ствол, который сейчас висит за спиной: до подмены — старый.
    bool stowedWeapon(int& outWeapon, sf::Vector2f& anchor, float& rot) const {
        if (state != SWAP || !SWAP_HIDE[frame]) return false;
        outWeapon = weapon;
        anchor = SWAP_STOW[frame];
        rot = SWAP_SROT[frame];
        return true;
    }

    float weaponRot() const {
        if (state == MELEE) return MELEE_ROT[frame];
        if (state == HEAVY) return HEAVY_ROT[frame];
        return 0.f;
    }

    bool hitFrame() const {
        if (state == MELEE) return frame == MELEE_HIT_FRAME;
        if (state == HEAVY) return frame == HEAVY_HIT0 || frame == HEAVY_HIT1;
        return false;
    }

    // Пульс свечения на заряженной петле: тот же шейдер, что у урона,
    // на малой амплитуде. Новых кадров не требует.
    float chargeGlow() const {
        if (state != HEAVY || !charged) return 0.f;
        if (frame != HEAVY_CHARGED[0] && frame != HEAVY_CHARGED[1]) return 0.f;
        return 0.22f * (0.5f + 0.5f * std::sin(chargeT * 0.035f));
    }

    sf::Vector2f weaponOffset() const {
        switch (state) {
            case WALK:  return WALK_OFF[frame];
            case RUN:   return RUN_OFF[frame];
            case ROLL:  return ROLL_OFF[frame];
            case HEAVY: return HEAVY_OFF[frame];
            case SHOOT: {
                sf::Vector2f o = SHOOT_OFF[frame];
                return {o.x * WEAPONS[weapon].recoil, o.y};
            }
            default:    return {0.f, 0.f};
        }
    }
};

// Поворот вектора на угол персонажа: любой слой, у которого свой пивот,
// смещается в ЛОКАЛЬНЫХ координатах кадра, а потом доворачивается.
sf::Vector2f rotated(sf::Vector2f v, float rad) {
    return {v.x * std::cos(rad) - v.y * std::sin(rad),
            v.x * std::sin(rad) + v.y * std::cos(rad)};
}

struct Renderer {
    sf::Texture body, enemy[6], wpn, side, fx;
    sf::Shader  hit;
    bool        hasShader = false;

    bool load(const std::string& dir) {
        const char* types[6] = {"grunt","assault","shield","heavy","radio","boss"};
        bool ok = body.loadFromFile(dir + "/player.png")
               && wpn.loadFromFile(dir + "/weapons.png")
               && side.loadFromFile(dir + "/weapons_side.png")
               && fx.loadFromFile(dir + "/fx/fx.png");
        for (int i = 0; i < 6; ++i)
            ok = ok && enemy[i].loadFromFile(dir + "/enemy_" + types[i] + ".png");
        // КРИТИЧНО: без этого поворот превратит пиксель-арт в кашу.
        body.setSmooth(false); wpn.setSmooth(false); fx.setSmooth(false);
        side.setSmooth(false);
        for (int i = 0; i < 6; ++i) enemy[i].setSmooth(false);
        if (sf::Shader::isAvailable())
            hasShader = hit.loadFromFile(dir + "/fx/hit_flash.frag",
                                         sf::Shader::Fragment);
        return ok;
    }

    // Лут рисуется ДО тел: игрок проходит поверх лежащего ствола.
    void drawLoot(sf::RenderTarget& rt, const Loot& l, bool lit) {
        sf::Sprite s(side, sf::IntRect(lit ? SIDE_W : 0,
                                       WEAPONS[l.weapon].row * SIDE_H,
                                       SIDE_W, SIDE_H));
        s.setOrigin(SIDE_PX, SIDE_PY);
        s.setPosition(l.pos);
        s.setScale(l.flip ? -1.f : 1.f, 1.f);   // зеркало по X, не поворот
        rt.draw(s);
    }

    // Иконка в HUD — тот же кадр, обрезанный по side_bbox из weapons.json.
    void drawIcon(sf::RenderTarget& rt, int weapon, sf::Vector2f at) {
        const WeaponDef& w = WEAPONS[weapon];
        sf::Sprite s(side, sf::IntRect(w.sbx, w.row * SIDE_H + w.sby,
                                       w.sbw, w.sbh));
        s.setPosition(at);
        rt.draw(s);
    }

    void drawFx(sf::RenderTarget& rt, const FxRect& r, int i,
                sf::Vector2f at, float angleDeg, float scale = 1.f) {
        sf::Sprite s(fx, fxFrame(r, std::min(i, r.frames - 1)));
        s.setOrigin(r.px, r.py);
        s.setPosition(at);
        s.setRotation(angleDeg);
        s.setScale(scale, scale);
        rt.draw(s);
    }

    void draw(sf::RenderTarget& rt, Character& c, const sf::Texture& tex) {
        const float ang = c.drawAngle();
        const float rad = ang * PI / 180.f;

        // 1. лужа крови — ПОД телом, тот же пивот и тот же угол
        if (c.poolT >= 0.f) {
            int pf = std::min(int(c.poolT * 1000.f / 150.f), FX_POOL.frames - 1);
            drawFx(rt, FX_POOL, pf, c.pos, ang);
        }

        // 2. убранный ствол — ПОД телом: он за спиной. Столбец без
        // перчаток, иначе на бесхозном стволе висят тёмные комки.
        int sw; sf::Vector2f sa; float srot;
        if (c.stowedWeapon(sw, sa, srot)) {
            sf::Sprite s(wpn, sf::IntRect(WPN_NOGLOVE * WPN_W,
                                          WEAPONS[sw].row * WPN_H,
                                          WPN_W, WPN_H));
            s.setOrigin(WPN_PX, WPN_PY);
            s.setPosition(c.pos + rotated(sa, rad));
            s.setRotation(ang + srot);
            rt.draw(s);
        }

        // 3. тело
        sf::Sprite b(tex, c.bodyRect());
        b.setOrigin(PIVOT, PIVOT);
        b.setPosition(c.pos);
        b.setRotation(ang);

        // 4. оружие: свой пивот, та же позиция и тот же угол
        sf::Sprite w(wpn);
        bool showWeapon = !c.weaponHidden();
        if (showWeapon) {
            const WeaponDef& wd = WEAPONS[c.weapon];
            w.setTextureRect(sf::IntRect(c.weaponVariant() * WPN_W,
                                         wd.row * WPN_H, WPN_W, WPN_H));
            w.setOrigin(WPN_PX, WPN_PY);
            w.setPosition(c.pos + rotated(c.weaponOffset(), rad));
            // Дуга удара — это поворот СЛОЯ ОРУЖИЯ поверх угла прицела.
            // Пивот слоя совпадает с пивотом тела, поэтому оружие идёт
            // дугой вокруг корпуса, а руки остаются на рукояти сами.
            w.setRotation(ang + c.weaponRot());
        }

        // Свечение заряда идёт тем же шейдером, что белая вспышка урона:
        // отдельных кадров под «заряжено» в атласе нет и не нужно.
        const float glow = std::max(c.flash, c.chargeGlow());
        if (glow > 0.f && hasShader) {
            hit.setUniform("texture", sf::Shader::CurrentTexture);
            hit.setUniform("amount", glow);
            rt.draw(b, &hit);
            if (showWeapon) rt.draw(w, &hit);
        } else {
            rt.draw(b);
            if (showWeapon) rt.draw(w);
        }

        // 5. магазин в руке на перезарядке
        if (c.state == RELOAD && RELOAD_ITEM[c.frame])
            drawFx(rt, FX_ITEM, 0, c.pos + rotated(RELOAD_ANCHOR[c.frame], rad), ang);

        // 6. дульная вспышка поверх оружия
        if (c.state == SHOOT && c.frame < 3 && WEAPONS[c.weapon].flashScale > 0.f) {
            const WeaponDef& wd = WEAPONS[c.weapon];
            sf::Vector2f m = rotated({wd.muzX, wd.muzY}, rad);
            drawFx(rt, FX_MUZZLE, c.frame, c.pos + m, ang, wd.flashScale);
        }

        // 7. брызги крови — ПОСЛЕДНИМИ, поверх всего (draw_order из JSON).
        // Их угол свой, от вектора урона, и с углом прицела не связан:
        // попасть могут когда персонаж смотрит в другую сторону.
        if (c.hitT >= 0.f) {
            int bf = std::min(int(c.hitT * 1000.f / 45.f), FX_BHIT.frames - 1);
            drawFx(rt, FX_BHIT, bf, c.pos, c.hitAng - 14.f);
            drawFx(rt, FX_BHIT, bf, c.pos, c.hitAng + 17.f);
        }
    }
};

} // namespace

int main() {
    const std::string dir = "assets";
    const int W = 480, H = 270;

    sf::RenderWindow win(sf::VideoMode(W * SCALE_UP, H * SCALE_UP),
                         "spriteforge demo");
    win.setFramerateLimit(60);

    sf::RenderTexture scene;
    if (!scene.create(W, H)) return 1;

    Renderer R;
    if (!R.load(dir)) return 2;

    Character player;
    player.pos = {W * 0.5f, H * 0.5f};

    std::vector<Character> foes;
    for (int i = 0; i < 6; ++i) {
        Character e;
        e.pos = {90.f + i * 55.f, 60.f};
        e.aim = PI * 0.5f;
        e.weapon = i % NWEAPON;
        foes.push_back(e);
    }

    // Лут, разложенный по карте: тот же спрайт, что выпадает из трупа.
    std::vector<Loot> loot;
    for (int i = 0; i < NWEAPON; ++i)
        loot.push_back({{62.f + (i % 4) * 118.f, 152.f + (i / 4) * 34.f},
                        i, (i % 2) == 1, false});

    sf::Clock clock;
    bool takeKey = false, dropKey = false;
    while (win.isOpen()) {
        sf::Event ev;
        while (win.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) win.close();
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::R) player.play(RELOAD);
                if (ev.key.code == sf::Keyboard::F) player.play(MELEE);
                if (ev.key.code == sf::Keyboard::K) player.kill();
                if (ev.key.code == sf::Keyboard::E) takeKey = true;
                if (ev.key.code == sf::Keyboard::G) dropKey = true;
                if (ev.key.code >= sf::Keyboard::Num1 &&
                    ev.key.code <= sf::Keyboard::Num9)
                    player.startSwap(std::min(NWEAPON - 1,
                                              ev.key.code - sf::Keyboard::Num1));
            }
        }
        float dt = clock.restart().asSeconds();

        sf::Vector2f mv{0.f, 0.f};
        using K = sf::Keyboard;
        if (K::isKeyPressed(K::A)) mv.x -= 1.f;
        if (K::isKeyPressed(K::D)) mv.x += 1.f;
        if (K::isKeyPressed(K::W)) mv.y -= 1.f;
        if (K::isKeyPressed(K::S)) mv.y += 1.f;
        float len = std::hypot(mv.x, mv.y);
        if (len > 0.f) mv /= len;

        sf::Vector2i mouse = sf::Mouse::getPosition(win);
        if (!player.dead)
            player.aim = std::atan2(float(mouse.y) / SCALE_UP - player.pos.y,
                                    float(mouse.x) / SCALE_UP - player.pos.x);

        // Мощная атака: ПКМ держать — копится заряд, отпустить — удар.
        const bool rmb = sf::Mouse::isButtonPressed(sf::Mouse::Right);
        if (rmb && player.state != HEAVY) player.startHeavy();
        if (!rmb && player.holding)       player.releaseHeavy();

        if (!player.dead && player.state != ROLL &&
            player.state != HEAVY && player.state != SWAP) {
            if (K::isKeyPressed(K::Space))            player.startRoll(mv);
            else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                                                       player.play(SHOOT);
            else if (player.state == IDLE || player.state == WALK ||
                     player.state == RUN) {
                player.play(len > 0.f ? (K::isKeyPressed(K::LShift) ? RUN : WALK)
                                      : IDLE);
                player.pos += mv * (player.state == RUN ? 96.f : 58.f) * dt;
            }
        }

        player.update(dt);
        for (auto& f : foes) f.update(dt);

        // Смерть роняет ствол на пол: drop_frame из character.json.
        auto dropFrom = [&loot](Character& c) {
            if (c.state == DEATH && c.frame >= DEATH_DROP_FRAME && !c.dropped) {
                c.dropped = true;
                loot.push_back({c.pos + sf::Vector2f{6.f, 5.f}, c.weapon,
                                (c.weapon & 1) != 0, false});
            }
        };
        dropFrom(player);
        for (auto& f : foes) dropFrom(f);

        // Ближайший лут в радиусе подбора — он и подсвечивается.
        int near = -1; float best = PICKUP_R;
        for (std::size_t i = 0; i < loot.size(); ++i) {
            if (loot[i].taken) continue;
            float d = std::hypot(loot[i].pos.x - player.pos.x,
                                 loot[i].pos.y - player.pos.y);
            if (d < best) { best = d; near = int(i); }
        }
        if (takeKey && near >= 0 && !player.dead && player.state != SWAP) {
            const int fromGround = loot[near].weapon;
            loot[near].weapon = player.weapon;   // обмен, не подбор
            player.startSwap(fromGround);        // через ту же анимацию
        }
        if (dropKey && !player.dead)
            loot.push_back({player.pos + sf::Vector2f{0.f, 9.f},
                            player.weapon, false, false});
        takeKey = dropKey = false;

        scene.clear(sf::Color(0x3A, 0x3F, 0x4A));
        // Лут — ПОД телами: игрок проходит поверх лежащего ствола.
        for (std::size_t i = 0; i < loot.size(); ++i)
            if (!loot[i].taken) R.drawLoot(scene, loot[i], int(i) == near);
        for (std::size_t i = 0; i < foes.size(); ++i)
            R.draw(scene, foes[i], R.enemy[i]);
        R.draw(scene, player, R.body);
        // HUD: иконка текущего ствола, обрезанная по side_bbox.
        R.drawIcon(scene, player.weapon, {6.f, float(H) - 6.f
                                          - WEAPONS[player.weapon].sbh});
        scene.display();

        sf::Sprite out(scene.getTexture());
        out.setScale(float(SCALE_UP), float(SCALE_UP));
        win.clear();
        win.draw(out);
        win.display();
    }
    return 0;
}
