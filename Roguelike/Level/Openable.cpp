#include "Openable.h"
#include "DoorComponent.h"
#include "HatchComponent.h"

namespace RoguelikeGame
{
    Openable OpenableOf(HatchComponent* hatch)
    {
        return {hatch->GetHatchId(), [hatch]() { hatch->Open(); }};
    }

    Openable OpenableOf(DoorComponent* door)
    {
        return {door->GetDoorId(), [door]() { door->Open(); }};
    }
}
