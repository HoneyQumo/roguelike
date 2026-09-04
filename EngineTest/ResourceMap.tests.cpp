#include "pch.h"
#include "ResourceMap.h"

using XYZEngine::ResourceMap;

namespace
{
	auto LoadedAs(const char* content)
	{
		return [content](std::string& item)
		{
			item = content;
			return true;
		};
	}

	bool LoadFails(std::string& item)
	{
		item = "broken";
		return false;
	}
}

TEST(ResourceMapTests, LoadedResourceIsFoundByName)
{
	ResourceMap<std::string> map("Test");

	EXPECT_TRUE(map.Load("first", "first.txt", LoadedAs("hello")));

	ASSERT_NE(map.Find("first"), nullptr);
	EXPECT_EQ(*map.Find("first"), "hello");
	EXPECT_EQ(map.Get("first"), map.Find("first"));
	EXPECT_EQ(map.GetCount(), 1u);
}

TEST(ResourceMapTests, FailedLoadKeepsNothing)
{
	ResourceMap<std::string> map("Test");

	EXPECT_FALSE(map.Load("broken", "broken.txt", LoadFails));

	EXPECT_EQ(map.Find("broken"), nullptr);
	EXPECT_EQ(map.GetCount(), 0u);
	EXPECT_FALSE(map.Contains("broken"));
}

TEST(ResourceMapTests, SecondLoadKeepsTheFirstResource)
{
	ResourceMap<std::string> map("Test");
	map.Load("same", "first.txt", LoadedAs("first"));

	EXPECT_FALSE(map.Load("same", "second.txt", LoadedAs("second")));

	EXPECT_EQ(*map.Find("same"), "first");
	EXPECT_EQ(map.GetCount(), 1u);
}

TEST(ResourceMapTests, MissingResourceIsNull)
{
	ResourceMap<std::string> map("Test");

	EXPECT_EQ(map.Find("missing"), nullptr);
	EXPECT_EQ(map.Get("missing"), nullptr);
}

TEST(ResourceMapTests, EraseOfUnknownNameChangesNothing)
{
	ResourceMap<std::string> map("Test");
	map.Load("kept", "kept.txt", LoadedAs("kept"));

	map.Erase("missing");

	EXPECT_EQ(map.GetCount(), 1u);
	EXPECT_NE(map.Find("kept"), nullptr);

	map.Erase("kept");

	EXPECT_EQ(map.GetCount(), 0u);
	EXPECT_EQ(map.Find("kept"), nullptr);
}

TEST(ResourceMapTests, ClearRemovesEveryResource)
{
	ResourceMap<std::string> map("Test");
	map.Load("first", "first.txt", LoadedAs("first"));
	map.Load("second", "second.txt", LoadedAs("second"));

	map.Clear();

	EXPECT_EQ(map.GetCount(), 0u);
	EXPECT_EQ(map.Find("first"), nullptr);
}
