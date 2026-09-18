#pragma once

#include <cmath>
#include <vector>

namespace RoguelikeGame
{
    struct SightOrigin
    {
        int column = 0;
        int row = 0;
        int radius = 0;
    };

    namespace ShadowcastDetail
    {
        // Четыре четверти: глубина всегда идёт от наблюдателя, поперечная ось - вправо по ней.
        inline void ToCell(int quarter, const SightOrigin& from, int depth, int across, int& column, int& row)
        {
            switch (quarter)
            {
            case 0: column = from.column + across; row = from.row - depth; return;
            case 1: column = from.column + across; row = from.row + depth; return;
            case 2: column = from.column + depth; row = from.row + across; return;
            default: column = from.column - depth; row = from.row + across; return;
            }
        }

        // Наклон берётся до середины грани клетки, а не до её центра: иначе соседние
        // клетки на одном расстоянии получают разный вердикт из-за округления.
        inline float Slope(int depth, int across)
        {
            return depth > 0 ? (2.f * across - 1.f) / (2.f * depth) : 0.f;
        }

        inline int FirstAcross(int depth, float slope)
        {
            return static_cast<int>(std::floor(depth * slope + 0.5f));
        }

        inline int LastAcross(int depth, float slope)
        {
            return static_cast<int>(std::ceil(depth * slope - 0.5f));
        }

        struct Wedge
        {
            int depth = 1;
            float start = -1.f;
            float end = 1.f;
        };
    }

    /**
    *	Симметричный обзор по четвертям: клетка посещается один раз, и видимость
    *	честна в обе стороны - если игрок видит клетку, то с этой клетки виден игрок.
    *
    *	Луч до центра каждой клетки этого не давал: путь туда и обратно шёл по разным
    *	клеткам, край выходил рваным из-за округления, а стены заслоняли друг друга.
    *	Здесь стена внутри клина помечается всегда - подсветка стен встроена в правило.
    *
    *	isBlocking(column, row) - закрывает ли клетка обзор, mark(column, row) - пометить видимой.
    */
    template <typename TBlocking, typename TMark>
    void Shadowcast(const SightOrigin& from, TBlocking isBlocking, TMark mark)
    {
        if (from.radius <= 0)
        {
            return;
        }

        mark(from.column, from.row);

        std::vector<ShadowcastDetail::Wedge> pending;

        for (int quarter = 0; quarter < 4; quarter++)
        {
            pending.clear();
            pending.push_back({});

            while (!pending.empty())
            {
                ShadowcastDetail::Wedge wedge = pending.back();
                pending.pop_back();

                if (wedge.depth > from.radius)
                {
                    continue;
                }

                int first = ShadowcastDetail::FirstAcross(wedge.depth, wedge.start);
                int last = ShadowcastDetail::LastAcross(wedge.depth, wedge.end);

                bool hasPrevious = false;
                bool wasBlocking = false;

                // Начало клина ползёт по ходу разбора строки: за стеной открывается
                // новый просвет, и всё, что правее, меряется уже от него.
                float start = wedge.start;

                for (int across = first; across <= last; across++)
                {
                    int column = 0;
                    int row = 0;
                    ShadowcastDetail::ToCell(quarter, from, wedge.depth, across, column, row);

                    bool blocks = isBlocking(column, row);
                    bool isWithinWedge = across >= wedge.depth * start && across <= wedge.depth * wedge.end;

                    if ((blocks || isWithinWedge)
                        && wedge.depth * wedge.depth + across * across <= from.radius * from.radius)
                    {
                        mark(column, row);
                    }

                    if (hasPrevious && wasBlocking && !blocks)
                    {
                        start = ShadowcastDetail::Slope(wedge.depth, across);
                    }

                    if (hasPrevious && !wasBlocking && blocks)
                    {
                        pending.push_back({wedge.depth + 1, start, ShadowcastDetail::Slope(wedge.depth, across)});
                    }

                    hasPrevious = true;
                    wasBlocking = blocks;
                }

                if (hasPrevious && !wasBlocking)
                {
                    pending.push_back({wedge.depth + 1, start, wedge.end});
                }
            }
        }
    }
}
