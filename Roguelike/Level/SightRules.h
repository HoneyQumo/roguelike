#pragma once

namespace RoguelikeGame
{
    struct SightStep
    {
        int column = 0;
        int row = 0;
    };

    /**
    *	Шаги от клетки в сторону наблюдателя - те и только те, что уменьшают смещение.
    *	Стена светится от соседа со стороны игрока, а не от того, что лежит за ней:
    *	иначе стена между двумя комнатами загоралась бы из комнаты, куда игрок не смотрит.
    *
    *	Возвращает количество заполненных шагов: их не больше двух, по одному на ось.
    */
    inline int StepsTowardViewer(int fromViewerColumns, int fromViewerRows, SightStep steps[2])
    {
        int count = 0;

        if (fromViewerColumns != 0)
        {
            steps[count++] = {fromViewerColumns > 0 ? -1 : 1, 0};
        }

        if (fromViewerRows != 0)
        {
            steps[count++] = {0, fromViewerRows > 0 ? -1 : 1};
        }

        return count;
    }
}
