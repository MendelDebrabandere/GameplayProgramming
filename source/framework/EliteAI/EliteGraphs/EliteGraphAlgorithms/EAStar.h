#pragma once

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

		NodeRecord curRecord{ pStartNode, nullptr, 0.0f, GetHeuristicCost(pStartNode, pGoalNode) };
		openList.push_back(curRecord);

		// Keep searching for a connection that leads to the end node
		while (!openList.empty())
		{
			// Get connection with lowest F score
			curRecord = *std::min_element(openList.begin(), openList.end());

			// Check if that connection leads to the end node
			if (curRecord.pNode == pGoalNode) break;

			// For every neighbor
			for (auto& connection : m_pGraph->GetNodeConnections(curRecord.pNode))
			{
				// Get the current node/neighbor
				T_NodeType* pCurNode{ m_pGraph->GetNode(connection->GetTo()) };

				// Calculate the total cost so far
				const float curGCost{ curRecord.costSoFar + connection->GetCost() };

				// A variable to check if we should skip this connection (if a cheaper connection already exists)
				bool cheaperConnectionFound{};

				// Check the closed list for an already existing connection to the current node
				for (const NodeRecord& existingRecord : closedList)
				{
					if (existingRecord.pNode == pCurNode)
					{
						// Check if the already existing connection is cheaper
						if (existingRecord.costSoFar < curGCost)
						{
							// If so, continue to the next connection
							cheaperConnectionFound = true;
							break;
						}

						// Else remove it from the closedList
						closedList.erase(std::remove(closedList.begin(), closedList.end(), existingRecord));
						break;
					}
				}

				// If an already existing connection is cheaper, continue to the next connection
				if (cheaperConnectionFound) continue;

				// Check the open list for a connection to the current node
				for (const NodeRecord& upcomingRecord : openList)
				{
					if (upcomingRecord.pNode == pCurNode)
					{
						// Check if the already existing connection is cheaper
						if (upcomingRecord.costSoFar < curGCost)
						{
							// If so, continue to the next connection
							cheaperConnectionFound = true;
							break;
						}

						// Else remove it from the openList
						openList.erase(std::remove(openList.begin(), openList.end(), upcomingRecord));
						break;
					}
				}

				// If an already existing connection is cheaper, continue to the next connection
				if (cheaperConnectionFound) continue;

				// At this point any expensive connection to the current node is removed (if it existed)
				//		We create a new nodeRecord and add it to the openList
				openList.push_back(NodeRecord{ pCurNode, connection, curGCost, curGCost + GetHeuristicCost(pCurNode, pGoalNode) });
			}

			// Remove the current record from the openList and add it to the closestList
			openList.erase(std::remove(openList.begin(), openList.end(), curRecord));
			closedList.push_back(curRecord);
		}

		// Loop over the open list for all non-checked records
		for (const NodeRecord& openRecord : openList)
		{
			// Variable to hold wether the non-checked record is already in the closedList
			bool isRecordInClosedList{};

			// Check the closed list for an already existing connection to the current node
			for (NodeRecord& existingRecord : closedList)
			{
				if (openRecord.pNode == existingRecord.pNode)
				{
					isRecordInClosedList = true;

					// Check if the non-checked record is cheaper then the existing record
					if (openRecord.costSoFar < existingRecord.costSoFar)
					{
						// If so, replace the connection in the existing record
						existingRecord.pConnection = openRecord.pConnection;
						existingRecord.costSoFar = openRecord.costSoFar;
					}

					break;
				}
			}

			// If the non-checked record is not yet in the closedList, this is the cheapest route to the current node
			// (Every route is cheaper then no route)
			// So we add the record it to the closedList
			if (!isRecordInClosedList)
			{
				closedList.push_back(openRecord);
			}
		}

		// If the result of the our search didn't end up at the goal node, find the node closest to the 
		if (curRecord.pNode != pGoalNode)
		{
			// Go over all existing records to find the closest node to the goal
			for (const NodeRecord& existingRecord : closedList)
			{
				if (curRecord.estimatedTotalCost - curRecord.costSoFar > existingRecord.estimatedTotalCost - existingRecord.costSoFar)
				{
					curRecord = existingRecord;
				}
			}
		}

		// Reconstruct path from last connection to start node
		// Track back until the node of the record is the start node
		while (curRecord.pNode != pStartNode)
		{
			// Add the current node to the path
			path.push_back(curRecord.pNode);

			// Look in the closedList for the cheapest route to the previous node in the connection
			for (const NodeRecord& existingRecord : closedList)
			{
				if (existingRecord.pNode->GetIndex() == curRecord.pConnection->GetFrom())
				{
					// Set the currentRecord to the found record
					curRecord = existingRecord;
					break;
				}
			}
		}

		// Add the startnode's position to the path
		path.push_back(pStartNode);

		// Reverse the path
		std::reverse(path.begin(), path.end());

		// Return the path
		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}