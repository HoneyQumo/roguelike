#include "pch.h"
#include "ProjectFiles.h"
#include <fstream>
#include <set>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <string>

namespace
{
	// MSBuild собирает по .vcxproj, а дерево в Visual Studio рисуется по .filters.
	// Файл легко добавить в первый и забыть про второй: собирается, но в дереве его нет.
	// Разошлось - запустить Tools/Project/sync_filters.py.
	const std::string PROJECTS[] = {
		"Roguelike.vcxproj",
		"../XYZEngine/XYZEngine.vcxproj",
	};

	std::string Read(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary);
		std::ostringstream text;
		text << file.rdbuf();

		return text.str();
	}

	std::set<std::string> Includes(const std::string& text, const std::string& tag)
	{
		std::set<std::string> found;
		const std::string opening = "<" + tag + " Include=\"";

		for (std::size_t at = text.find(opening); at != std::string::npos; at = text.find(opening, at + 1))
		{
			std::size_t from = at + opening.size();
			std::size_t to = text.find('"', from);
			if (to != std::string::npos)
			{
				found.insert(text.substr(from, to - from));
			}
		}

		return found;
	}

	std::string Listed(const std::set<std::string>& names)
	{
		std::string text;
		for (const std::string& name : names)
		{
			text += "\n  " + name;
		}

		return text;
	}
}

class ProjectTreeTest : public ProjectFiles::Test
{
protected:
	void CheckSameFiles(const std::string& project, const std::string& tag)
	{
		std::string build = Read(project);
		std::string tree = Read(project + ".filters");

		ASSERT_FALSE(build.empty()) << project;
		ASSERT_FALSE(tree.empty()) << project << ".filters";

		std::set<std::string> inBuild = Includes(build, tag);
		std::set<std::string> inTree = Includes(tree, tag);

		std::set<std::string> missing;
		std::set_difference(inBuild.begin(), inBuild.end(), inTree.begin(), inTree.end(),
			std::inserter(missing, missing.end()));

		std::set<std::string> extra;
		std::set_difference(inTree.begin(), inTree.end(), inBuild.begin(), inBuild.end(),
			std::inserter(extra, extra.end()));

		EXPECT_TRUE(missing.empty()) << project << ": собирается, но в дереве нет:" << Listed(missing)
			<< "\nзапустить Tools/Project/sync_filters.py";
		EXPECT_TRUE(extra.empty()) << project << ": в дереве есть, но не собирается:" << Listed(extra)
			<< "\nзапустить Tools/Project/sync_filters.py";
	}
};

TEST_F(ProjectTreeTest, EveryBuiltFileIsInTheVisualStudioTree)
{
	ASSERT_TRUE(isFound) << previous.string();

	for (const std::string& project : PROJECTS)
	{
		CheckSameFiles(project, "ClCompile");
		CheckSameFiles(project, "ClInclude");
	}
}
