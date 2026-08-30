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
// R перезарядка, F удар, 1-9 смена ствола, K убить себя.
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
constexpr int   WPN_W   = 160, WPN_H = 64;    // кадр оружия
constexpr float WPN_PX  = 80.f, WPN_PY = 32.f;// пивот оружия — та же точка мира
constexpr float PI      = 3.14159265f;
constexpr int   SCALE_UP = 3;                 // целый множитель на экран

float deg(float rad) { return rad * 180.f / PI; }

float wrapPi(float a) {
    while (a >  PI) a -= 2.f * PI;
    while (a < -PI) a += 2.f * PI;
    return a;
}

// --- раскладка атласа тела: строка = анимация, столбец = кадр -------------
enum State { IDLE, WALK, RUN, SHOOT, RELOAD, MELEE, HURT, DEATH, ROLL, NSTATE };

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
    /* ROLL   */ {8, 10,  50, false},   // row здесь — row0, дальше +направление
};
constexpr int ROLL_DIRS = 8;

// weapon_offset: сдвиг слоя оружия в пикселях кадра, по кадрам анимации.
const sf::Vector2f RUN_OFF[8]  = {{0,3},{0,4},{0,3},{0,2},{0,3},{0,4},{0,3},{0,2}};
const sf::Vector2f WALK_OFF[8] = {{0,0},{0,1},{0,0},{0,-1},{0,0},{0,1},{0,0},{0,-1}};
const sf::Vector2f SHOOT_OFF[4]= {{-3,0},{-2,0},{-1,0},{0,0}};
const sf::Vector2f ROLL_OFF[10]= {{0,1},{0,0},{0,0},{0,0},{0,0},
                                  {0,0},{0,0},{0,0},{-1,1},{0,0}};

// Кадры, на которых слой оружия НЕ рисуется.
const bool ROLL_HIDE [10] = {false,true,true,true,true,true,true,true,false,false};
const bool DEATH_HIDE[8]  = {false,false,true,true,true,true,true,true};

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
    const char* id; int row; float muzX, muzY; float recoil, flashScale;
};
const WeaponDef WEAPONS[] = {
    {"ak47",                0,  58.56f, 12.16f, 1.00f, 1.00f},
    {"m16",                 1,  60.80f, 12.16f, 0.85f, 0.90f},
    {"shotgun_double",      2,  53.60f, 12.16f, 1.50f, 1.35f},
    {"shotgun_pump",        3,  55.20f, 11.52f, 1.35f, 1.25f},
    {"smg_suppressed",      4,  57.60f, 12.16f, 0.60f, 0.35f},
    {"glock",               5,  32.00f, 11.68f, 0.55f, 0.65f},
    {"deagle",              6,  38.88f, 12.00f, 1.40f, 1.20f},
    {"pistol_suppressed",   7,  56.00f, 12.16f, 0.50f, 0.30f},
    {"knife",               8,  42.40f, 15.36f, 0.00f, 0.00f},
    {"bat",                 9,  55.20f, 12.32f, 0.00f, 0.00f},
    {"rpg",                10,  76.00f, 12.16f, 1.80f, 1.60f},
};
constexpr int NWEAPON = int(sizeof(WEAPONS) / sizeof(WEAPONS[0]));

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
    float   deathAng = 0.f;      // угол, на котором труп замер
    bool    dead     = false;
    int     weapon   = 0;
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

    void update(float dt) {
        flash = std::max(0.f, flash - dt / 0.12f);
        if (poolT >= 0.f) poolT += dt;

        timer += dt * 1000.f;
        const AnimDef& a = def();
        while (timer >= a.ms) {
            timer -= a.ms;
            if (frame + 1 < a.frames)      ++frame;
            else if (a.loop)               frame = 0;
            else if (state == DEATH)       break;          // труп остаётся лежать
            else                           { play(IDLE); break; }
        }
        if (state == ROLL)
            pos += rollVec * 260.f * ROLL_SPEED[frame] * dt;
        if (state == DEATH && frame == 3 && poolT < 0.f)
            poolT = 0.f;                                    // blood_pool_start_frame
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
        return false;
    }

    int weaponVariant() const {
        return state == RELOAD ? RELOAD_VARIANT[frame] : 0;
    }

    sf::Vector2f weaponOffset() const {
        switch (state) {
            case WALK:  return WALK_OFF[frame];
            case RUN:   return RUN_OFF[frame];
            case ROLL:  return ROLL_OFF[frame];
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
    sf::Texture body, enemy[6], wpn, fx;
    sf::Shader  hit;
    bool        hasShader = false;

    bool load(const std::string& dir) {
        const char* types[6] = {"grunt","assault","shield","heavy","radio","boss"};
        bool ok = body.loadFromFile(dir + "/player.png")
               && wpn.loadFromFile(dir + "/weapons.png")
               && fx.loadFromFile(dir + "/fx/fx.png");
        for (int i = 0; i < 6; ++i)
            ok = ok && enemy[i].loadFromFile(dir + "/enemy_" + types[i] + ".png");
        // КРИТИЧНО: без этого поворот превратит пиксель-арт в кашу.
        body.setSmooth(false); wpn.setSmooth(false); fx.setSmooth(false);
        for (int i = 0; i < 6; ++i) enemy[i].setSmooth(false);
        if (sf::Shader::isAvailable())
            hasShader = hit.loadFromFile(dir + "/fx/hit_flash.frag",
                                         sf::Shader::Fragment);
        return ok;
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

        // 2. тело
        sf::Sprite b(tex, c.bodyRect());
        b.setOrigin(PIVOT, PIVOT);
        b.setPosition(c.pos);
        b.setRotation(ang);

        // 3. оружие: свой пивот, та же позиция и тот же угол
        sf::Sprite w(wpn);
        bool showWeapon = !c.weaponHidden();
        if (showWeapon) {
            const WeaponDef& wd = WEAPONS[c.weapon];
            w.setTextureRect(sf::IntRect(c.weaponVariant() * WPN_W,
                                         wd.row * WPN_H, WPN_W, WPN_H));
            w.setOrigin(WPN_PX, WPN_PY);
            w.setPosition(c.pos + rotated(c.weaponOffset(), rad));
            w.setRotation(ang);
        }

        if (c.flash > 0.f && hasShader) {
            hit.setUniform("texture", sf::Shader::CurrentTexture);
            hit.setUniform("amount", c.flash);
            rt.draw(b, &hit);
            if (showWeapon) rt.draw(w, &hit);
        } else {
            rt.draw(b);
            if (showWeapon) rt.draw(w);
        }

        // 4. магазин в руке на перезарядке
        if (c.state == RELOAD && RELOAD_ITEM[c.frame])
            drawFx(rt, FX_ITEM, 0, c.pos + rotated(RELOAD_ANCHOR[c.frame], rad), ang);

        // 5. дульная вспышка поверх оружия
        if (c.state == SHOOT && c.frame < 3 && WEAPONS[c.weapon].flashScale > 0.f) {
            const WeaponDef& wd = WEAPONS[c.weapon];
            sf::Vector2f m = rotated({wd.muzX, wd.muzY}, rad);
            drawFx(rt, FX_MUZZLE, c.frame, c.pos + m, ang, wd.flashScale);
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

    sf::Clock clock;
    while (win.isOpen()) {
        sf::Event ev;
        while (win.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) win.close();
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::R) player.play(RELOAD);
                if (ev.key.code == sf::Keyboard::F) player.play(MELEE);
                if (ev.key.code == sf::Keyboard::K) player.kill();
                if (ev.key.code >= sf::Keyboard::Num1 &&
                    ev.key.code <= sf::Keyboard::Num9)
                    player.weapon = std::min(NWEAPON - 1,
                                             ev.key.code - sf::Keyboard::Num1);
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

        if (!player.dead && player.state != ROLL) {
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

        scene.clear(sf::Color(0x3A, 0x3F, 0x4A));
        for (std::size_t i = 0; i < foes.size(); ++i)
            R.draw(scene, foes[i], R.enemy[i]);
        R.draw(scene, player, R.body);
        scene.display();

        sf::Sprite out(scene.getTexture());
        out.setScale(float(SCALE_UP), float(SCALE_UP));
        win.clear();
        win.draw(out);
        win.display();
    }
    return 0;
}
