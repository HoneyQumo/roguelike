#pragma once

#include <vector>
#include <UiIcon.h>
#include <UiLabel.h>
#include <UiPanel.h>
#include <UiScreen.h>
#include "InventoryComponent.h"

namespace RoguelikeGame
{
    class InventoryScreen : public XYZEngine::UiScreen
    {
    public:
        InventoryScreen();

        void SetInventory(InventoryComponent* newInventory);

        void Open();
        void Close();
        void Toggle();
        bool IsOpen() const;

        void Update(float deltaTime) override;
        bool HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased) override;

        int GetSelectedSlot() const;
        const XYZEngine::UiLabel& GetSlotName(int index) const;
        const XYZEngine::UiLabel& GetSlotCount(int index) const;
        const XYZEngine::UiPanel& GetSlotPanel(int index) const;

    private:
        struct SlotWidgets
        {
            XYZEngine::UiPanel* panel = nullptr;
            XYZEngine::UiIcon* icon = nullptr;
            XYZEngine::UiLabel* name = nullptr;
            XYZEngine::UiLabel* count = nullptr;
        };

        InventoryComponent* inventory = nullptr;
        XYZEngine::UiPanel* dimmer = nullptr;
        XYZEngine::UiPanel* window = nullptr;
        XYZEngine::UiLabel* title = nullptr;
        std::vector<SlotWidgets> slotWidgets;

        bool isOpen = false;
        float animation = 0.f;
        int selectedSlot = 0;

        void BuildSlots(const sf::Font* font);
        void Refresh();
        void HandleKeyboard();
        void MoveSelection(int columns, int rows);
        void SelectSlot(int index);
        void ApplyAnimation();
    };
}
