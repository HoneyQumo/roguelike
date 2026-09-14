#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include "LoggerRegistry.h"

namespace XYZEngine
{
	template <typename TResource>
	class ResourceMap
	{
	public:
		explicit ResourceMap(std::string newTypeName) : typeName(std::move(newTypeName)) {}

		template <typename TLoader>
		bool Load(const std::string& name, const std::string& sourcePath, TLoader loadInto)
		{
			if (Contains(name))
			{
				LOG_WARN(typeName + " is already loaded: " + name);
				return false;
			}

			auto resource = std::make_unique<TResource>();
			if (!loadInto(*resource))
			{
				LOG_ERROR("Can't load " + typeName + ": " + sourcePath);
				return false;
			}

			items.emplace(name, std::move(resource));
			LOG_INFO(typeName + " loaded: " + name + " from " + sourcePath);
			return true;
		}

		TResource* Find(const std::string& name) const
		{
			auto item = items.find(name);
			return item == items.end() ? nullptr : item->second.get();
		}

		TResource* Get(const std::string& name) const
		{
			TResource* found = Find(name);
			if (found == nullptr)
			{
				LOG_ERROR(typeName + " not found: " + name);
			}

			return found;
		}

		bool Contains(const std::string& name) const
		{
			return items.find(name) != items.end();
		}

		void Erase(const std::string& name)
		{
			items.erase(name);
		}

		void Clear()
		{
			items.clear();
		}

		std::size_t GetCount() const
		{
			return items.size();
		}

	private:
		std::string typeName;
		std::map<std::string, std::unique_ptr<TResource>> items;
	};
}
