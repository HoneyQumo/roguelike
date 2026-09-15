#pragma once

#include <filesystem>
#include "gtest/gtest.h"

namespace ProjectFiles
{
	constexpr int SEARCH_DEPTH = 6;

	class Test : public ::testing::Test
	{
	protected:
		void SetUp() override
		{
			previous = std::filesystem::current_path();

			std::filesystem::path root = previous;
			for (int step = 0; step < SEARCH_DEPTH; step++)
			{
				if (std::filesystem::exists(root / "Roguelike" / "Resources" / "Levels" / "levels.config"))
				{
					std::filesystem::current_path(root / "Roguelike");
					isFound = true;
					return;
				}

				root = root.parent_path();
			}
		}

		void TearDown() override
		{
			std::filesystem::current_path(previous);
		}

		std::filesystem::path previous;
		bool isFound = false;
	};
}
