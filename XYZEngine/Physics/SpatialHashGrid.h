#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <vector>

namespace XYZEngine
{
	constexpr float DEFAULT_GRID_CELL_SIZE = 128.f;
	constexpr int GRID_CELL_LIMIT = 10000000;

	struct GridRange
	{
		int minColumn = 0;
		int minRow = 0;
		int maxColumn = 0;
		int maxRow = 0;

		bool IsEmpty() const
		{
			return maxColumn < minColumn || maxRow < minRow;
		}
		std::size_t GetCellCount() const
		{
			if (IsEmpty())
			{
				return 0u;
			}

			return static_cast<std::size_t>(maxColumn - minColumn + 1) * static_cast<std::size_t>(maxRow - minRow + 1);
		}

		bool operator==(const GridRange& other) const
		{
			return minColumn == other.minColumn && minRow == other.minRow
				&& maxColumn == other.maxColumn && maxRow == other.maxRow;
		}
		bool operator!=(const GridRange& other) const
		{
			return !(*this == other);
		}
	};

	template <typename TItem>
	class SpatialHashGrid
	{
	public:
		void SetCellSize(float newCellSize);
		float GetCellSize() const;

		GridRange RangeOf(const sf::FloatRect& area) const;

		void Insert(const TItem& item, const sf::FloatRect& bounds);
		void Remove(const TItem& item, const sf::FloatRect& bounds);
		void Move(const TItem& item, const sf::FloatRect& from, const sf::FloatRect& to);
		void Clear();

		void Query(const sf::FloatRect& area, std::vector<TItem>& found) const;

		std::size_t GetCellCount() const;
		std::size_t GetEntryCount() const;

	private:
		using CellKey = std::int64_t;

		static CellKey KeyOf(int column, int row);
		int IndexOf(float value) const;

		float cellSize = DEFAULT_GRID_CELL_SIZE;
		std::unordered_map<CellKey, std::vector<TItem>> cells;
	};

	template <typename TItem>
	void SpatialHashGrid<TItem>::SetCellSize(float newCellSize)
	{
		if (newCellSize <= 0.f)
		{
			return;
		}

		cellSize = newCellSize;
		Clear();
	}

	template <typename TItem>
	float SpatialHashGrid<TItem>::GetCellSize() const
	{
		return cellSize;
	}

	template <typename TItem>
	typename SpatialHashGrid<TItem>::CellKey SpatialHashGrid<TItem>::KeyOf(int column, int row)
	{
		std::uint64_t packed = (static_cast<std::uint64_t>(static_cast<std::uint32_t>(column)) << 32)
			| static_cast<std::uint64_t>(static_cast<std::uint32_t>(row));

		return static_cast<CellKey>(packed);
	}

	template <typename TItem>
	int SpatialHashGrid<TItem>::IndexOf(float value) const
	{
		float cell = std::floor(value / cellSize);

		if (!(cell > static_cast<float>(-GRID_CELL_LIMIT)))
		{
			return -GRID_CELL_LIMIT;
		}
		if (cell > static_cast<float>(GRID_CELL_LIMIT))
		{
			return GRID_CELL_LIMIT;
		}

		return static_cast<int>(cell);
	}

	template <typename TItem>
	GridRange SpatialHashGrid<TItem>::RangeOf(const sf::FloatRect& area) const
	{
		float right = area.left + area.width;
		float bottom = area.top + area.height;

		GridRange range;
		range.minColumn = IndexOf(std::min(area.left, right));
		range.minRow = IndexOf(std::min(area.top, bottom));
		range.maxColumn = IndexOf(std::max(area.left, right));
		range.maxRow = IndexOf(std::max(area.top, bottom));

		return range;
	}

	template <typename TItem>
	void SpatialHashGrid<TItem>::Insert(const TItem& item, const sf::FloatRect& bounds)
	{
		GridRange range = RangeOf(bounds);
		if (range.IsEmpty())
		{
			return;
		}

		for (int row = range.minRow; row <= range.maxRow; row++)
		{
			for (int column = range.minColumn; column <= range.maxColumn; column++)
			{
				cells[KeyOf(column, row)].push_back(item);
			}
		}
	}

	template <typename TItem>
	void SpatialHashGrid<TItem>::Remove(const TItem& item, const sf::FloatRect& bounds)
	{
		GridRange range = RangeOf(bounds);
		if (range.IsEmpty())
		{
			return;
		}

		for (int row = range.minRow; row <= range.maxRow; row++)
		{
			for (int column = range.minColumn; column <= range.maxColumn; column++)
			{
				auto cell = cells.find(KeyOf(column, row));
				if (cell == cells.end())
				{
					continue;
				}

				std::vector<TItem>& items = cell->second;
				auto found = std::find(items.begin(), items.end(), item);
				if (found != items.end())
				{
					*found = items.back();
					items.pop_back();
				}

				if (items.empty())
				{
					cells.erase(cell);
				}
			}
		}
	}

	template <typename TItem>
	void SpatialHashGrid<TItem>::Move(const TItem& item, const sf::FloatRect& from, const sf::FloatRect& to)
	{
		if (RangeOf(from) == RangeOf(to))
		{
			return;
		}

		Remove(item, from);
		Insert(item, to);
	}

	template <typename TItem>
	void SpatialHashGrid<TItem>::Clear()
	{
		cells.clear();
	}

	template <typename TItem>
	void SpatialHashGrid<TItem>::Query(const sf::FloatRect& area, std::vector<TItem>& found) const
	{
		found.clear();

		GridRange range = RangeOf(area);
		if (cells.empty() || range.IsEmpty())
		{
			return;
		}

		if (range.GetCellCount() > cells.size())
		{
			for (const auto& cell : cells)
			{
				found.insert(found.end(), cell.second.begin(), cell.second.end());
			}
		}
		else
		{
			for (int row = range.minRow; row <= range.maxRow; row++)
			{
				for (int column = range.minColumn; column <= range.maxColumn; column++)
				{
					auto cell = cells.find(KeyOf(column, row));
					if (cell != cells.end())
					{
						found.insert(found.end(), cell->second.begin(), cell->second.end());
					}
				}
			}
		}

		std::sort(found.begin(), found.end());
		found.erase(std::unique(found.begin(), found.end()), found.end());
	}

	template <typename TItem>
	std::size_t SpatialHashGrid<TItem>::GetCellCount() const
	{
		return cells.size();
	}

	template <typename TItem>
	std::size_t SpatialHashGrid<TItem>::GetEntryCount() const
	{
		std::size_t total = 0u;
		for (const auto& cell : cells)
		{
			total += cell.second.size();
		}

		return total;
	}
}
