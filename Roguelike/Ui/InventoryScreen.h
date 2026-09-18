#pragma once

#include <vector>
#include <UiIcon.h>
#include <UiLabel.h>
#include <UiPanel.h>
#include <UiScreen.h>
#include "InventoryComponent.h"
#include "ItemDefinition.h"

namespace RoguelikeGame
{
    std::string InventoryHint(const ItemDefinition* item);

    class InventoryScreen : public XYZEngine::UiScreen
    {
    public:
        InventoryScreen();

        void SetInventory(InventoryComponent* newInventory);

        // Экран не знает ни про раскладку, ни про каталог оружия - только про запрос.
        void SetEquipHandler(std::function<bool(int bagSlot, int targetSlot)> newEquipHandler);

        // Те же цифры пояса, но при открытой сумке они вешают, а не применяют.
        void SetBeltHandler(std::function<bool(int bagSlot, int hook)> newBeltHandler);

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
        const XYZEngine::UiLabel& GetHint() const;
        const XYZEngine::UiLabel& GetNotice() const;

        // Причина отказа обязана быть видна там, где нажали: HUD лежит под затемнением окна.
        void ShowNotice(const std::string& text);

    private:
        struct SlotWidgets
        {
            XYZEngine::UiPanel* panel = nullptr;
            XYZEngine::UiIcon* icon = nullptr;
            XYZEngine::UiLabel* name = nullptr;
            XYZEngine::UiLabel* count = nullptr;
        };

        InventoryComponent* inventory = nullptr;
        std::function<bool(int, int)> equipHandler;
        std::function<bool(int, int)> beltHandler;
        XYZEngine::UiPanel* dimmer = nullptr;
        XYZEngine::UiPanel* window = nullptr;
        XYZEngine::UiLabel* title = nullptr;
        XYZEngine::UiLabel* hint = nullptr;
        XYZEngine::UiLabel* notice = nullptr;
        std::vector<SlotWidgets> slotWidgets;

        bool isOpen = false;
        float animation = 0.f;
        float noticeTimeLeft = 0.f;
        int selectedSlot = 0;

        void BuildSlots(const sf::Font* font);
        void Refresh();
        void RefreshHint();
        void HandleKeyboard();
        void TryEquip(int targetSlot);
        void MoveSelection(int columns, int rows);
        void SelectSlot(int index);
        void ApplyAnimation();
    };
}
