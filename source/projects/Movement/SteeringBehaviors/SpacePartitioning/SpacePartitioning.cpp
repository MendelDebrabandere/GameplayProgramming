#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_Cells.resize(m_NrOfRows * m_NrOfCols);
	float cellWidth{ width / cols };
	float cellHeight{ height / rows };

	for (int idx{}; idx < m_NrOfRows * m_NrOfCols; ++idx)
	{
		m_Cells.push_back(Cell{ (idx % cols) * cellWidth,
								(idx % rows) * cellHeight,
								cellWidth,
								cellHeight });
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int newIdx{ PositionToIndex(agent->GetPosition()) };
	int oldIdx{ PositionToIndex(oldPos) };

	if (newIdx != oldIdx)
	{
		m_Cells[oldIdx].agents.remove(agent);

		m_Cells[newIdx].agents.push_back(agent);
	}

}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	Elite::Vector2 bottomLeft{ agent->GetPosition().x - queryRadius, agent->GetPosition().y - queryRadius };
	Elite::Rect queryBox{ bottomLeft, queryRadius * 2, queryRadius * 2 };

	
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{

}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int idx{};
	for (int column{}; pos.x < m_Cells[column].boundingBox.bottomLeft.x; ++column)
	{
		idx = column;
	}

	int rowCounter{};
	for (int row{}; pos.y > m_Cells[idx + row * m_NrOfCols].boundingBox.bottomLeft.y; ++row)
	{
		rowCounter = row;
	}

	idx += rowCounter * m_NrOfCols;

	return idx;
}