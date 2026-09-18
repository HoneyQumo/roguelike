#pragma once

#include <algorithm>
#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <Vector.h>
#include "GameSettings.h"
#include "ThreatMarks.h"

namespace RoguelikeGame
{
    struct ThreatMark
    {
        XYZEngine::Vector2Df position;
        ThreatKind kind = ThreatKind::Alerted;
        float strength = 1.f;
    };

    /**
    *	Источник за краем экрана прижимается к самому краю в свою сторону. Иначе про того,
    *	кто обычно и стреляет, игрок не узнал бы ничего: знак уехал бы за кадр вместе с ним.
    *
    *	Отступ внутрь - чтобы знак не срезался краем кадра пополам.
    */
    inline XYZEngine::Vector2Df PinToView(const XYZEngine::Vector2Df& place, const sf::FloatRect& view, float margin)
    {
        float left = view.left + margin;
        float right = view.left + view.width - margin;
        float top = view.top + margin;
        float bottom = view.top + view.height - margin;

        if (right < left)
        {
            left = right = view.left + view.width * 0.5f;
        }

        if (bottom < top)
        {
            top = bottom = view.top + view.height * 0.5f;
        }

        return {std::clamp(place.x, left, right), std::clamp(place.y, top, bottom)};
    }

    inline bool IsInsideView(const XYZEngine::Vector2Df& place, const sf::FloatRect& view)
    {
        return view.contains(place.x, place.y);
    }

    // Ушедший за край показан тусклее: он дальше, и его место названо приблизительно.
    inline std::vector<ThreatMark> BuildThreatMarks(const std::vector<ThreatSource>& sources,
        const XYZEngine::Vector2Df& from, const sf::FloatRect& view)
    {
        std::vector<ThreatMark> marks;

        for (const ThreatSource& source : sources)
        {
            if (source.strength <= 0.f)
            {
                continue;
            }

            XYZEngine::Vector2Df place = from + source.direction;
            place.y += THREAT_MARK_LIFT;

            bool isInside = IsInsideView(place, view);

            ThreatMark mark;
            mark.position = isInside ? place : PinToView(place, view, THREAT_MARK_EDGE_MARGIN);
            mark.kind = source.kind;
            mark.strength = std::min(source.strength, 1.f) * (isInside ? 1.f : THREAT_MARK_EDGE_PART);

            marks.push_back(mark);
        }

        return marks;
    }
}
