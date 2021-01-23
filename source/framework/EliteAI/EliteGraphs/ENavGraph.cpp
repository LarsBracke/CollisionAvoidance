#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	int nodeIndex{ 0 };
	for (Line* pLine : m_pNavMeshPolygon->GetLines())
	{
		const std::vector<const Triangle*>& triangles = m_pNavMeshPolygon->GetTrianglesFromLineIndex(pLine->index);
		if (triangles.size() > 1)
		{
			Elite::Vector2 nodePos{ (pLine->p1.x + pLine->p2.x) / 2.f, (pLine->p1.y + pLine->p2.y) / 2.f };
			NavGraphNode* pNode{ new NavGraphNode(nodeIndex, pLine->index, nodePos) };
			++nodeIndex;
			AddNode(pNode);
		}
	}
	
	//2. Create connections now that every node is created
	for (Triangle* pTriangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<NavGraphNode*> validNodes;
		for (int index : pTriangle->metaData.IndexLines)
		{
			for (NavGraphNode* pNode : m_Nodes)
			{
				if (pNode->GetLineIndex() == index)
				{
					validNodes.push_back(pNode);
				}
			}
			if (validNodes.size() == 2)
			{
				GraphConnection2D* pConnection{ new GraphConnection2D(validNodes[0]->GetIndex(), validNodes[1]->GetIndex()) };
				AddConnection(pConnection);
			}
			else if (validNodes.size() == 3)
			{
				//GraphConnection2D* pConnection{ new GraphConnection2D(validNodes[0]->GetIndex(), validNodes[1]->GetIndex()) };
				//AddConnection(pConnection);
				GraphConnection2D* pConnection = new GraphConnection2D(validNodes[0]->GetIndex(), validNodes[2]->GetIndex());
				AddConnection(pConnection);
				pConnection = new GraphConnection2D(validNodes[1]->GetIndex(), validNodes[2]->GetIndex());
				AddConnection(pConnection);
			}
		}
		validNodes.clear();
	}
	
	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();	
}

