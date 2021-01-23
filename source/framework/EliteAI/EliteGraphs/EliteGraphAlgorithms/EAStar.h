#pragma once
#include <algorithm>

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;
		NodeRecord currentRecord;
		
		// kickstart the loop
		NodeRecord startRecord;
		startRecord.pNode = pStartNode;
		startRecord.pConnection = nullptr;
		startRecord.costSoFar = 0.f;
		startRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);
		openList.push_back(startRecord);

		// loop
		while (!openList.empty())
		{
			// select best record from openList
			std::vector<NodeRecord>::iterator result = std::min_element(openList.begin(), openList.end());
			currentRecord = *result;

			// check if the node is the endnode
			if (currentRecord.pNode == pGoalNode) { break; }
			else
			{
				const std::list<T_ConnectionType*>& pConnections = m_pGraph->GetNodeConnections(currentRecord.pNode->GetIndex());
				for (T_ConnectionType* pConnection : pConnections)
				{
					// calculating total g-cost
					float gCost{ currentRecord.costSoFar + pConnection->GetCost() };

					// check if connections lead to node in closedlist
					std::vector<NodeRecord>::iterator closedIt = 
						std::find_if(closedList.begin(), closedList.end(), [this, pConnection](NodeRecord record) 
							{ return (m_pGraph->GetNode(pConnection->GetTo()) == record.pNode); } );

					if (closedIt != closedList.end()) 
					{
						if (gCost >= (*closedIt).costSoFar)
						{
							continue;
						}
						else
						{
							closedList.erase(closedIt);
						}
					}
					else
					{
						// check if connections lead to node in openList
						std::vector<NodeRecord>::iterator openIt =
							std::find_if(openList.begin(), openList.end(), [this, pConnection](NodeRecord record)
								{ return (m_pGraph->GetNode(pConnection->GetTo()) == record.pNode); });

						if (openIt != openList.end())
						{
							if (gCost >= (*openIt).costSoFar)
							{
								continue;
							}
							else
							{
								openList.erase(openIt);
							}
						}
					}

					// create new noderecord and add it to openList
					NodeRecord newRecord;
					newRecord.pNode = m_pGraph->GetNode(pConnection->GetTo());
					newRecord.pConnection = pConnection;
					newRecord.costSoFar = gCost;
					newRecord.estimatedTotalCost = GetHeuristicCost(newRecord.pNode, pGoalNode) + gCost;
					openList.push_back(newRecord);
				}
			}

			// remove currentrecord from openList and add to closedList
			auto it = std::find(openList.begin(), openList.end(), currentRecord);
			openList.erase(it);
			closedList.push_back(currentRecord);
		}	

		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);
			for (const NodeRecord& record : closedList)
			{
				if (m_pGraph->GetNode(currentRecord.pConnection->GetFrom()) == record.pNode)
				{
					currentRecord = record;
					break;
				}
				else
				{
					currentRecord.pNode = pStartNode;
				}
			}
		}
		path.push_back(pStartNode);

		std::reverse(path.begin(), path.end());
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}