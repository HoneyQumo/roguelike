#include "pch.h"
#include "UiPanel.h"
#include "UiProgressBar.h"
#include "UiWidget.h"

using XYZEngine::UiAnchor;
using XYZEngine::UiPanel;
using XYZEngine::UiProgressBar;
using XYZEngine::UiWidget;

namespace
{
	sf::FloatRect Screen(float width, float height)
	{
		return {0.f, 0.f, width, height};
	}
}

TEST(UiLayoutTests, BottomLeftAnchorKeepsMarginsOnTwoResolutions)
{
	UiWidget root;
	UiWidget* block = root.AddChild<UiWidget>();
	block->SetAnchor(UiAnchor::BottomLeft);
	block->SetPivot(UiAnchor::BottomLeft);
	block->SetOffset({26.f, -22.f});
	block->SetSize({260.f, 65.f});

	root.Layout(Screen(1280.f, 720.f));
	EXPECT_FLOAT_EQ(block->GetBounds().left, 26.f);
	EXPECT_FLOAT_EQ(block->GetBounds().top, 720.f - 22.f - 65.f);

	root.Layout(Screen(800.f, 600.f));
	EXPECT_FLOAT_EQ(block->GetBounds().left, 26.f);
	EXPECT_FLOAT_EQ(block->GetBounds().top, 600.f - 22.f - 65.f);
	EXPECT_FLOAT_EQ(block->GetBounds().width, 260.f);
}

TEST(UiLayoutTests, StretchCoversWholeParent)
{
	UiWidget root;
	UiPanel* panel = root.AddChild<UiPanel>();
	panel->SetStretch(true, true);

	root.Layout(Screen(1280.f, 720.f));

	EXPECT_FLOAT_EQ(panel->GetBounds().left, 0.f);
	EXPECT_FLOAT_EQ(panel->GetBounds().top, 0.f);
	EXPECT_FLOAT_EQ(panel->GetBounds().width, 1280.f);
	EXPECT_FLOAT_EQ(panel->GetBounds().height, 720.f);
}

TEST(UiLayoutTests, StretchWithOffsetWorksAsInset)
{
	UiWidget root;
	UiPanel* panel = root.AddChild<UiPanel>();
	panel->SetStretch(true, true);
	panel->SetOffset({10.f, 20.f});

	root.Layout(Screen(400.f, 300.f));

	EXPECT_FLOAT_EQ(panel->GetBounds().left, 10.f);
	EXPECT_FLOAT_EQ(panel->GetBounds().top, 20.f);
	EXPECT_FLOAT_EQ(panel->GetBounds().width, 380.f);
	EXPECT_FLOAT_EQ(panel->GetBounds().height, 260.f);
}

TEST(UiLayoutTests, CenterAnchorPlacesWidgetAroundScreenCenter)
{
	UiWidget root;
	UiWidget* title = root.AddChild<UiWidget>();
	title->SetAnchor(UiAnchor::Center);
	title->SetPivot(UiAnchor::Center);
	title->SetOffset({0.f, -36.f});
	title->SetSize({720.f, 76.f});

	root.Layout(Screen(1280.f, 720.f));

	const sf::FloatRect& bounds = title->GetBounds();
	EXPECT_FLOAT_EQ(bounds.left + 0.5f * bounds.width, 640.f);
	EXPECT_FLOAT_EQ(bounds.top + 0.5f * bounds.height, 360.f - 36.f);
}

TEST(UiLayoutTests, PivotShiftsPositionButKeepsSize)
{
	UiWidget root;
	UiWidget* topLeft = root.AddChild<UiWidget>();
	topLeft->SetAnchor(UiAnchor::Center);
	topLeft->SetPivot(UiAnchor::TopLeft);
	topLeft->SetSize({100.f, 40.f});

	UiWidget* bottomRight = root.AddChild<UiWidget>();
	bottomRight->SetAnchor(UiAnchor::Center);
	bottomRight->SetPivot(UiAnchor::BottomRight);
	bottomRight->SetSize({100.f, 40.f});

	root.Layout(Screen(1000.f, 500.f));

	EXPECT_FLOAT_EQ(topLeft->GetBounds().left - bottomRight->GetBounds().left, 100.f);
	EXPECT_FLOAT_EQ(topLeft->GetBounds().top - bottomRight->GetBounds().top, 40.f);
	EXPECT_FLOAT_EQ(topLeft->GetBounds().width, bottomRight->GetBounds().width);
}

TEST(UiLayoutTests, ChildIsAnchoredToParentNotScreen)
{
	UiWidget root;
	UiWidget* panel = root.AddChild<UiWidget>();
	panel->SetAnchor(UiAnchor::Center);
	panel->SetPivot(UiAnchor::Center);
	panel->SetSize({400.f, 200.f});

	UiWidget* child = panel->AddChild<UiWidget>();
	child->SetAnchor(UiAnchor::TopLeft);
	child->SetPivot(UiAnchor::TopLeft);
	child->SetOffset({8.f, 8.f});
	child->SetSize({50.f, 20.f});

	root.Layout(Screen(1280.f, 720.f));

	EXPECT_FLOAT_EQ(child->GetBounds().left, panel->GetBounds().left + 8.f);
	EXPECT_FLOAT_EQ(child->GetBounds().top, panel->GetBounds().top + 8.f);
}

TEST(UiLayoutTests, HitTestIncludesTopLeftExcludesBottomRight)
{
	UiWidget root;
	UiWidget* widget = root.AddChild<UiWidget>();
	widget->SetAnchor(UiAnchor::TopLeft);
	widget->SetPivot(UiAnchor::TopLeft);
	widget->SetOffset({100.f, 100.f});
	widget->SetSize({200.f, 50.f});

	root.Layout(Screen(1280.f, 720.f));

	EXPECT_TRUE(widget->HitTest({100.f, 100.f}));
	EXPECT_TRUE(widget->HitTest({299.f, 149.f}));
	EXPECT_FALSE(widget->HitTest({300.f, 150.f}));
	EXPECT_FALSE(widget->HitTest({99.f, 120.f}));
}

TEST(UiLayoutTests, HiddenWidgetIsNotHitButKeepsLayout)
{
	UiWidget root;
	UiWidget* widget = root.AddChild<UiWidget>();
	widget->SetSize({100.f, 100.f});
	widget->SetVisible(false);

	root.Layout(Screen(640.f, 480.f));

	EXPECT_FALSE(widget->HitTest({10.f, 10.f}));
	EXPECT_FLOAT_EQ(widget->GetBounds().width, 100.f);
}

TEST(UiLayoutTests, ProgressBarFillScalesWithValue)
{
	UiWidget root;
	UiProgressBar* bar = root.AddChild<UiProgressBar>();
	bar->SetAnchor(UiAnchor::TopLeft);
	bar->SetPivot(UiAnchor::TopLeft);
	bar->SetSize({200.f, 10.f});

	root.Layout(Screen(640.f, 480.f));
	bar->SetValue(0.5f);

	EXPECT_FLOAT_EQ(bar->GetFillBounds().width, 100.f);
	EXPECT_FLOAT_EQ(bar->GetFillBounds().height, 10.f);
	EXPECT_FLOAT_EQ(bar->GetFillBounds().left, bar->GetBounds().left);

	bar->SetValue(-1.f);
	EXPECT_FLOAT_EQ(bar->GetValue(), 0.f);

	bar->SetValue(5.f);
	EXPECT_FLOAT_EQ(bar->GetValue(), 1.f);
	EXPECT_FLOAT_EQ(bar->GetFillBounds().width, 200.f);
}
