#pragma once
#include <stack>
#include <vector>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{
		// If the graph is not connected, there can be no Eulerian Trail
		if (IsConnected() == false)
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree 
		int nrOfNodes = m_pGraph->GetNrOfNodes();
		int oddCount = 0;
		for (int index = 0; index < nrOfNodes; ++index)
		{
			if (m_pGraph->IsNodeValid(index) && (m_pGraph->GetNodeConnections(index).size() & 1))
			{
				++oddCount;
			}
		}

		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if (oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (an Euler trail can be made, but only starting and ending in these 2 nodes)
		else if (oddCount == 2 && nrOfNodes != 2)
		{
			return Eulerianity::semiEulerian;
		}

		// A connected graph with no odd nodes is Eulerian
		else
		{
			return Eulerianity::eulerian;
		}

	}

	template<class T_NodeType, class T_ConnectionType>
	inline vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		int nrOfNodes = graphCopy->GetNrOfNodes();
		std::stack<T_NodeType*> stack;
		auto path = vector<T_NodeType*>();

		// algorithm...
		int oddCount{ 0 };
		for (int index = 0; index < nrOfNodes; ++index)
		{
			if (m_pGraph->IsNodeValid(index) && (m_pGraph->GetNodeConnections(index).size() & 1))
			{
				++oddCount;
			}
		}

		T_NodeType* pCurrentNode = nullptr;

		if (oddCount == 0)
		{
			if (graphCopy->IsNodeValid(0))
			{ 
				pCurrentNode = graphCopy->GetNode(0);
			}
		}
		else if (oddCount == 2)
		{
			for (int index = 0; index < nrOfNodes; ++index)
			{
				if (m_pGraph->IsNodeValid(index) && (m_pGraph->GetNodeConnections(index).size() & 1))
				{
					pCurrentNode = m_pGraph->GetNode(index);
					break;
				}
			}
		}
		else
		{
			return path;
		}

		stack.push(pCurrentNode);

		while (stack.size() > 0)
		{
			if (graphCopy->GetNodeConnections(pCurrentNode->GetIndex()).size() == 0)
			{
				pCurrentNode = stack.top();
				stack.pop();
				path.push_back(pCurrentNode);
			}
			else
			{
				std::list<T_ConnectionType*> pConnections = graphCopy->GetNodeConnections(pCurrentNode->GetIndex());
				T_ConnectionType* pConnection = pConnections.front();
				int nextIndex = pConnection->GetTo();
				T_NodeType* pNextNode = m_pGraph->GetNode(nextIndex);
				graphCopy->RemoveConnection(pConnection);
				pCurrentNode = pNextNode;
				stack.push(pCurrentNode);
			}
		}

		eulerianity = IsEulerian();
		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for (T_ConnectionType*pConnection : m_pGraph->GetNodeConnections(startIdx))
		{
			if (m_pGraph->IsNodeValid(pConnection->GetTo()) && !visited[pConnection->GetTo()])
			{
				VisitAllNodesDFS(pConnection->GetTo(), visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		int nrOfNodes = m_pGraph->GetNrOfNodes();
		std::vector<bool> visited(nrOfNodes, false);

		// find a valid starting node that has connections
		int connectedIndex = invalid_node_index;
		for (int index = 0; index < nrOfNodes; ++index)
		{
			if (m_pGraph->IsNodeValid(index) && m_pGraph->GetNodeConnections(index).size() != 0)
			{
				connectedIndex = index;
				break;
			}
		}

		// if no valid node could be found, return false
		if (connectedIndex == invalid_node_index)
		{
			return false;
		}

		// start a depth-first-search traversal from a node that has connections
		VisitAllNodesDFS(connectedIndex, visited);

		// if a node was never visited, this graph is not connected
		for (int index = 0; index < nrOfNodes; ++index)
		{
			if (m_pGraph->IsNodeValid(index) && visited[index] == false)
			{
				return false;
			}
		}

		return true;
	}

}