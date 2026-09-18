#pragma once

#include <vector>
#include <Vector.h>

namespace RoguelikeGame
{
    class LevelGrid;

    enum class FogState : unsigned char
    {
        Unseen,
        Known,
        Seen
    };

    /**
    *	Память игрока о карте: что видно сейчас, что уже разведано и что ещё нет.
    *
    *	Радиус ноль - туман выключен, и вся карта считается видимой. Тогда всё,
    *	что спрашивает состояние клетки, ведёт себя ровно как до тумана, и
    *	локацию без тумана не нужно обходить особым случаем в каждом месте.
    */
    class FogOfWar
    {
    public:
        static FogOfWar& Current();
        static void Reset(int width, int height, int radius);

        bool IsEnabled() const;
        int GetWidth() const;
        int GetHeight() const;
        int GetRadius() const;

        // Номер правки: по нему полотно понимает, что пора перекраситься.
        unsigned int GetVersion() const;

        FogState GetState(int column, int row) const;

        // Яркость от 0 до 1 рядом с тремя состояниями, а не вместо них: на состояниях
        // завязана видимость актёров, а яркость нужна только показу.
        float GetLight(int column, int row) const;

        // Яркость угла клетки - среднее по четырём, которые этот угол делят.
        float GetCornerLight(int column, int row) const;
        FogState GetStateAt(const XYZEngine::Vector2Df& position) const;
        int Count(FogState state) const;

        // Открывает клетки вокруг точки. Возвращает true, если карта изменилась.
        bool Reveal(const LevelGrid& grid, const XYZEngine::Vector2Df& from);

    private:
        void LightBlockers(const LevelGrid& grid, int fromColumn, int fromRow);
        void FillLight(int fromColumn, int fromRow);

        static FogOfWar current;

        int width = 0;
        int height = 0;
        int radius = 0;
        unsigned int version = 0u;
        std::vector<FogState> cells;
        std::vector<float> light;
    };
}
