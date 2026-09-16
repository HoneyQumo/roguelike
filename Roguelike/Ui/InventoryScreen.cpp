#include "InventoryScreen.h"
#include "GameSettings.h"
#include "Item.h"
#include "WeaponCatalog.h"
#include <InputSystem.h>
#include <ResourceSystem.h>
#include <TextUtils.h>
#include <UiManager.h>
#include <algorithm>

namespace RoguelikeGame
{
    namespace
    {
        float GridWidth()
        {
            return INVENTORY_GRID_COLUMNS * INVENTORY_SLOT_SIZE + (INVENTORY_GRID_COLUMNS - 1) * INVENTORY_SLOT_GAP;
        }

        float GridHeight()
        {
            return INVENTORY_GRID_ROWS * INVENTORY_SLOT_SIZE + (INVENTORY_GRID_ROWS - 1) * INVENTORY_SLOT_GAP;
        }
    }

    std::string InventoryHint(const ItemDefinition* item)
    {
        if (item == nullptr)
        {
            return {};
        }

        if (item->effect.kind == ItemEffectKind::EquipWeapon)
        {
            WeaponId id = WeaponId::Knife;
            if (!TryGetWeaponId(item->effect.target, id))
            {
                return {};
            }

            return std::string(INVENTORY_EQUIP_HINT) + std::to_string(PreferredWeaponSlot(id) + 1);
        }

        if (item->effect.kind == ItemEffectKind::None)
        {
            return {};
        }

        return INVENTORY_USE_HINT;
    }

    InventoryScreen::InventoryScreen()
    {
        const sf::Font* font = XYZEngine::ResourceSystem::Instance()->GetFont(HUD_FONT);

        dimmer = GetRoot().AddChild<XYZEngine::UiPanel>();
        dimmer->SetStretch(true, true);
        dimmer->SetFillColor(INVENTORY_DIM_COLOR);

        window = GetRoot().AddChild<XYZEngine::UiPanel>();
        window->SetAnchor(XYZEngine::UiAnchor::Center);
        window->SetPivot(XYZEngine::UiAnchor::Center);
        window->SetSize({GridWidth() + 2.f * INVENTORY_WINDOW_PADDING,
            GridHeight() + INVENTORY_TITLE_HEIGHT + INVENTORY_HINT_HEIGHT + 2.f * INVENTORY_WINDOW_PADDING});
        window->SetFillColor(INVENTORY_WINDOW_COLOR);
        window->SetOutline(2.f, INVENTORY_SLOT_OUTLINE_COLOR);

        title = window->AddChild<XYZEngine::UiLabel>();
        title->SetAnchor(XYZEngine::UiAnchor::Top);
        title->SetPivot(XYZEngine::UiAnchor::Top);
        title->SetOffset({0.f, INVENTORY_WINDOW_PADDING * 0.4f});
        title->SetSize({GridWidth(), INVENTORY_TITLE_HEIGHT});
        title->SetAlign(XYZEngine::UiAnchor::Center);
        title->SetCharacterSize(INVENTORY_TITLE_FONT_SIZE);
        title->SetColor(AMMO_HUD_COLOR);
        title->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        title->SetFont(font);
        title->SetUtf8Text(INVENTORY_TITLE);

        BuildSlots(font);

        hint = window->AddChild<XYZEngine::UiLabel>();
        hint->SetAnchor(XYZEngine::UiAnchor::Bottom);
        hint->SetPivot(XYZEngine::UiAnchor::Bottom);
        hint->SetOffset({0.f, -INVENTORY_WINDOW_PADDING * 0.4f});
        hint->SetSize({GridWidth(), INVENTORY_HINT_HEIGHT});
        hint->SetAlign(XYZEngine::UiAnchor::Center);
        hint->SetCharacterSize(INVENTORY_HINT_FONT_SIZE);
        hint->SetColor(AMMO_HUD_COLOR);
        hint->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
        hint->SetFont(font);

        SetVisible(false);
    }

    void InventoryScreen::BuildSlots(const sf::Font* font)
    {
        slotWidgets.resize(INVENTORY_GRID_COLUMNS * INVENTORY_GRID_ROWS);

        float gridTop = INVENTORY_WINDOW_PADDING + INVENTORY_TITLE_HEIGHT;

        for (int index = 0; index < static_cast<int>(slotWidgets.size()); index++)
        {
            int column = index % INVENTORY_GRID_COLUMNS;
            int row = index / INVENTORY_GRID_COLUMNS;

            SlotWidgets& slot = slotWidgets[index];

            slot.panel = window->AddChild<XYZEngine::UiPanel>();
            slot.panel->SetAnchor(XYZEngine::UiAnchor::TopLeft);
            slot.panel->SetPivot(XYZEngine::UiAnchor::TopLeft);
            slot.panel->SetOffset({INVENTORY_WINDOW_PADDING + column * (INVENTORY_SLOT_SIZE + INVENTORY_SLOT_GAP),
                gridTop + row * (INVENTORY_SLOT_SIZE + INVENTORY_SLOT_GAP)});
            slot.panel->SetSize({INVENTORY_SLOT_SIZE, INVENTORY_SLOT_SIZE});
            slot.panel->SetFillColor(INVENTORY_SLOT_EMPTY_COLOR);
            slot.panel->SetOutline(2.f, INVENTORY_SLOT_OUTLINE_COLOR);

            slot.icon = slot.panel->AddChild<XYZEngine::UiIcon>();
            slot.icon->SetAnchor(XYZEngine::UiAnchor::Center);
            slot.icon->SetPivot(XYZEngine::UiAnchor::Center);
            slot.icon->SetOffset({0.f, -8.f});
            slot.icon->SetSize({INVENTORY_SLOT_SIZE * INVENTORY_ICON_PART, INVENTORY_SLOT_SIZE * INVENTORY_ICON_PART});
            slot.icon->SetVisible(false);

            slot.name = slot.panel->AddChild<XYZEngine::UiLabel>();
            slot.name->SetAnchor(XYZEngine::UiAnchor::Bottom);
            slot.name->SetPivot(XYZEngine::UiAnchor::Bottom);
            slot.name->SetOffset({0.f, -4.f});
            slot.name->SetSize({INVENTORY_SLOT_SIZE, INVENTORY_SLOT_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
            slot.name->SetAlign(XYZEngine::UiAnchor::Center);
            slot.name->SetCharacterSize(INVENTORY_SLOT_FONT_SIZE);
            slot.name->SetColor(AMMO_HUD_COLOR);
            slot.name->SetFont(font);

            slot.count = slot.panel->AddChild<XYZEngine::UiLabel>();
            slot.count->SetAnchor(XYZEngine::UiAnchor::TopRight);
            slot.count->SetPivot(XYZEngine::UiAnchor::TopRight);
            slot.count->SetOffset({-6.f, 4.f});
            slot.count->SetSize({40.f, INVENTORY_COUNT_FONT_SIZE * AMMO_HUD_LINE_HEIGHT});
            slot.count->SetAlign(XYZEngine::UiAnchor::Right);
            slot.count->SetCharacterSize(INVENTORY_COUNT_FONT_SIZE);
            slot.count->SetColor(AMMO_HUD_COLOR);
            slot.count->SetOutline(AMMO_HUD_OUTLINE, AMMO_HUD_OUTLINE_COLOR);
            slot.count->SetFont(font);
            slot.count->SetVisible(false);
        }
    }

    void InventoryScreen::SetInventory(InventoryComponent* newInventory)
    {
        inventory = newInventory;

        if (inventory != nullptr)
        {
            inventory->SubscribeChanged([this]() { Refresh(); });
            selectedSlot = inventory->GetSelectedSlot();
        }

        Refresh();
    }

    void InventoryScreen::Open()
    {
        if (isOpen)
        {
            return;
        }

        isOpen = true;
        SetVisible(true);
        Refresh();
    }

    void InventoryScreen::Close()
    {
        isOpen = false;
    }

    void InventoryScreen::Toggle()
    {
        isOpen ? Close() : Open();
    }

    bool InventoryScreen::IsOpen() const
    {
        return isOpen;
    }

    int InventoryScreen::GetSelectedSlot() const
    {
        return selectedSlot;
    }

    void InventoryScreen::Update(float deltaTime)
    {
        auto input = XYZEngine::InputSystem::Instance();

        if (input->WasActionPressed(XYZEngine::InputAction::Inventory))
        {
            Toggle();
            XYZEngine::UiManager::Instance()->CaptureInput();
        }
        else if (isOpen && input->WasActionPressed(XYZEngine::InputAction::Pause))
        {
            Close();
            XYZEngine::UiManager::Instance()->CaptureInput();
        }

        if (isOpen)
        {
            HandleKeyboard();
        }

        float target = isOpen ? 1.f : 0.f;
        float step = INVENTORY_OPEN_TIME > 0.f ? deltaTime / INVENTORY_OPEN_TIME : 1.f;

        if (animation < target)
        {
            animation = std::min(animation + step, target);
        }
        else if (animation > target)
        {
            animation = std::max(animation - step, target);
        }

        ApplyAnimation();
    }

    void InventoryScreen::ApplyAnimation()
    {
        if (animation <= 0.f)
        {
            SetVisible(false);
            return;
        }

        SetVisible(true);

        sf::Color dim = INVENTORY_DIM_COLOR;
        dim.a = static_cast<sf::Uint8>(INVENTORY_DIM_COLOR.a * animation);
        dimmer->SetFillColor(dim);

        window->SetOffset({0.f, INVENTORY_SLIDE_OFFSET * (1.f - animation)});
        GetRoot().Layout(GetRoot().GetBounds());
    }

    bool InventoryScreen::HandlePointer(const sf::Vector2f& point, bool isPressed, bool wasReleased)
    {
        if (!isOpen)
        {
            return false;
        }

        for (int index = 0; index < static_cast<int>(slotWidgets.size()); index++)
        {
            if (!slotWidgets[index].panel->HitTest(point))
            {
                continue;
            }

            if (wasReleased)
            {
                SelectSlot(index);
            }

            return true;
        }

        return window->HitTest(point);
    }

    void InventoryScreen::HandleKeyboard()
    {
        auto input = XYZEngine::InputSystem::Instance();

        if (input->WasActionPressed(XYZEngine::InputAction::NavigateLeft))
        {
            MoveSelection(-1, 0);
        }
        if (input->WasActionPressed(XYZEngine::InputAction::NavigateRight))
        {
            MoveSelection(1, 0);
        }
        if (input->WasActionPressed(XYZEngine::InputAction::NavigateUp))
        {
            MoveSelection(0, -1);
        }
        if (input->WasActionPressed(XYZEngine::InputAction::NavigateDown))
        {
            MoveSelection(0, 1);
        }

        if (input->WasActionPressed(XYZEngine::InputAction::Confirm) && inventory != nullptr)
        {
            inventory->Use(selectedSlot);
        }
    }

    void InventoryScreen::MoveSelection(int columns, int rows)
    {
        int column = selectedSlot % INVENTORY_GRID_COLUMNS + columns;
        int row = selectedSlot / INVENTORY_GRID_COLUMNS + rows;

        column = std::clamp(column, 0, INVENTORY_GRID_COLUMNS - 1);
        row = std::clamp(row, 0, INVENTORY_GRID_ROWS - 1);

        SelectSlot(row * INVENTORY_GRID_COLUMNS + column);
    }

    void InventoryScreen::SelectSlot(int index)
    {
        if (index < 0 || index >= static_cast<int>(slotWidgets.size()))
        {
            return;
        }

        selectedSlot = index;

        if (inventory != nullptr)
        {
            inventory->SelectSlot(index);
        }

        Refresh();
    }

    void InventoryScreen::Refresh()
    {
        for (int index = 0; index < static_cast<int>(slotWidgets.size()); index++)
        {
            SlotWidgets& widgets = slotWidgets[index];
            bool isSelected = index == selectedSlot;

            if (inventory == nullptr || index >= inventory->GetCapacity())
            {
                widgets.icon->SetVisible(false);
                widgets.count->SetVisible(false);
                widgets.name->SetText("");
                widgets.panel->SetFillColor(isSelected ? INVENTORY_SLOT_SELECTED_COLOR : INVENTORY_SLOT_EMPTY_COLOR);
                continue;
            }

            const InventorySlot& slot = inventory->GetSlot(index);

            if (slot.IsEmpty())
            {
                widgets.icon->SetVisible(false);
                widgets.count->SetVisible(false);
                widgets.name->SetText("");
                widgets.panel->SetFillColor(isSelected ? INVENTORY_SLOT_SELECTED_COLOR : INVENTORY_SLOT_EMPTY_COLOR);
                continue;
            }

            const sf::Texture* texture = XYZEngine::ResourceSystem::Instance()->GetTextureShared(ItemTextureName(slot.item.id));
            if (texture != nullptr)
            {
                widgets.icon->SetTexture(texture);
                widgets.icon->SetColor(slot.item.icon.tint);
                widgets.icon->SetVisible(true);
            }

            widgets.name->SetUtf8Text(slot.item.name.c_str());
            widgets.count->SetVisible(slot.count > 1);
            widgets.count->SetText(sf::String(std::to_string(slot.count)));
            widgets.panel->SetFillColor(isSelected ? INVENTORY_SLOT_SELECTED_COLOR : INVENTORY_SLOT_FILLED_COLOR);
        }

        RefreshHint();
    }

    void InventoryScreen::RefreshHint()
    {
        const ItemDefinition* item = nullptr;

        if (inventory != nullptr && selectedSlot >= 0 && selectedSlot < inventory->GetCapacity())
        {
            const InventorySlot& slot = inventory->GetSlot(selectedSlot);
            item = slot.IsEmpty() ? nullptr : &slot.item;
        }

        hint->SetUtf8Text(InventoryHint(item).c_str());
    }

    const XYZEngine::UiLabel& InventoryScreen::GetHint() const
    {
        return *hint;
    }

    const XYZEngine::UiLabel& InventoryScreen::GetSlotName(int index) const
    {
        return *slotWidgets[index].name;
    }

    const XYZEngine::UiLabel& InventoryScreen::GetSlotCount(int index) const
    {
        return *slotWidgets[index].count;
    }

    const XYZEngine::UiPanel& InventoryScreen::GetSlotPanel(int index) const
    {
        return *slotWidgets[index].panel;
    }
}
