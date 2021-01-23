#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\App_Steering\SteeringAgent.h"

#include <assert.h>

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
	, m_CellWidth{ m_SpaceWidth / cols }
	, m_CellHeight{ m_SpaceHeight / rows }
	, m_pAgentToDebug{ nullptr }
	, m_DebugRenderBehavior{ false, false, false, false, false, false }
	, m_NeighborhoodCells{ maxEntities, Cell{ 0,0,0,0 } }
{
	// info
	std::cout << "Cellspace" << std::endl;
	std::cout << "=========" << std::endl;
	std::cout << "Rows: " << m_NrOfRows << "\t" << "Columns " << m_NrOfCols << std::endl;

	// init cells
	int nrOfCells{ m_NrOfCols * m_NrOfRows };
	Elite::Rect rect{ {0.f, 0.f}, m_CellWidth, m_CellHeight };
	for (int index = 0; index < nrOfCells; ++index)
	{
		if (rect.bottomLeft.x >= m_SpaceWidth)
		{
			rect.bottomLeft.x = 0.f;
			rect.bottomLeft.y += m_CellHeight;
		}
		Cell cell{ rect.bottomLeft.x, rect.bottomLeft.y, rect.width, rect.height };
		m_Cells.push_back(cell);

		rect.bottomLeft.x += m_CellWidth;
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	Elite::Vector2 pos{ agent->GetPosition() };
	int index{ PositionToIndex(agent->GetPosition()) };
	std::list<SteeringAgent*>& agentList{ m_Cells[index].agents };
	agentList.insert(agentList.begin(), agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, const Elite::Vector2& oldPos)
{
	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(agent->GetPosition()) };

	if (oldIndex != newIndex)
	{
		std::list<SteeringAgent*>& agentList{ m_Cells[oldIndex].agents };
		for
		(std::list<SteeringAgent*>::iterator it = agentList.begin(); it != agentList.end(); ++it)
		{
			if (*it == agent)
			{
				agentList.remove(*it);
				AddAgent(agent);
				return;
			}
		}
	}
	else
	{
		return;
	}
}

void CellSpace::RegisterNeighbors(const Elite::Vector2& pos, float queryRadius)
{
	int nrOfCells{ 0 };
	m_NrOfNeighbors = 0;

	Elite::Rect agentRect;
	agentRect.bottomLeft = { pos.x - queryRadius / 2, pos.y - queryRadius / 2 };
	agentRect.width = queryRadius;
	agentRect.height = queryRadius;;

	// check which cells
	// are in the agent neighborhood
	for (const Cell& cell : m_Cells)
	{
		if (Elite::IsOverlapping(agentRect, cell.boundingBox))
		{
			m_NeighborhoodCells[nrOfCells] = cell;
			++nrOfCells;
		}
	}

	// get the agents from those cells
	int neighborIndex{ 0 };
	for (int index = 0; index < nrOfCells; ++index)
	{
		for (SteeringAgent* pAgent : m_NeighborhoodCells[index].agents)
		{	
			// debug rendering
			if (pAgent == m_pAgentToDebug)
			{
				for (int index = 0; index < nrOfCells; ++index)
				{
					if (m_DebugRenderBehavior.renderNeighborhoodCells)
					{
						std::vector<Elite::Vector2> pointVector = m_NeighborhoodCells[index].GetRectPoints();
						DEBUGRENDERER2D->DrawPolygon(&pointVector[0], 4, { 1,1,0,1 }, 0.0f);


					}
					if (m_DebugRenderBehavior.circleNeighbors)
					{
						for (SteeringAgent* pAgent : m_NeighborhoodCells[index].agents)
						{
							DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), 2.f, { 0,1,0 }, 0.1f);
						}
					}
					if (m_DebugRenderBehavior.renderNeighborhoodRadius)
					{
						DEBUGRENDERER2D->DrawCircle(m_pAgentToDebug->GetPosition(), queryRadius, { 1,1,1 }, 0.2f);
						
						std::vector<Elite::Vector2> pointVector;
						pointVector.push_back({ m_pAgentToDebug->GetPosition().x - queryRadius, m_pAgentToDebug->GetPosition().y - queryRadius });
						pointVector.push_back({ m_pAgentToDebug->GetPosition().x + queryRadius, m_pAgentToDebug->GetPosition().y - queryRadius });
						pointVector.push_back({ m_pAgentToDebug->GetPosition().x + queryRadius, m_pAgentToDebug->GetPosition().y + queryRadius });
						pointVector.push_back({ m_pAgentToDebug->GetPosition().x - queryRadius, m_pAgentToDebug->GetPosition().y + queryRadius });
						DEBUGRENDERER2D->DrawPolygon(&pointVector[0], 4, { 0,0,1,1 }, 0.2f);
					}
				}
			}


			if 
				(
					pos != pAgent->GetPosition() &&
					Elite::Distance(pos, pAgent->GetPosition()) <= queryRadius
				)
			{
				m_Neighbors[neighborIndex] = pAgent;
				++m_NrOfNeighbors;
				++neighborIndex;
			}
		}
	}
}

void CellSpace::SetAgentToDebug(SteeringAgent* pAgent)
{
	m_pAgentToDebug = pAgent;
	m_pAgentToDebug->SetBodyColor({ 0,1,0 });
}

DebugRenderBehavior& CellSpace::GetDebugRenderBehavior()
{
	return m_DebugRenderBehavior;
}

void CellSpace::RenderCells() const
{
	int cellIndex{ 0 };
	std::string cellIndexString;
	for (const Cell& cell : m_Cells)
	{
		Elite::Vector2 drawPos{ cell.boundingBox.bottomLeft };

		// draw cell rectangle
		if (m_DebugRenderBehavior.renderCells)
		{
			std::vector<Elite::Vector2> pointVector = cell.GetRectPoints();
			DEBUGRENDERER2D->DrawPolygon(&pointVector[0], 4, { 1,0,0,1 }, 0.4f);
		}

		// draw the cellindex
		if (m_DebugRenderBehavior.displayCellIndex)
		{
			drawPos.x += 1 / 5.f * cell.boundingBox.width;
			drawPos.y += 3 / 5.f * cell.boundingBox.width;
			cellIndexString = std::to_string(cellIndex);
			DEBUGRENDERER2D->DrawString(drawPos, cellIndexString.c_str());
			++cellIndex;
		}

		// draw amount of agent in the cell
		if (m_DebugRenderBehavior.displayAgentCount)
		{
			drawPos.y += 4 / 5.f * cell.boundingBox.height;
			int currentAgentCount{ int(cell.agents.size()) };
			std::string agentCountString{ std::to_string(currentAgentCount) };
			DEBUGRENDERER2D->DrawString(drawPos, agentCountString.c_str());
		}

	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	Elite::Vector2 position{ pos };
	if (position.x >= m_SpaceWidth) position.x = m_SpaceWidth - 1;
	if (position.y >= m_SpaceHeight) position.y = m_SpaceHeight - 1;

	int index;
	int col{ int(position.x / m_CellWidth) };
	int row{ int(position.y / m_CellHeight) };
	index = row * m_NrOfCols + col;

	return index;
}