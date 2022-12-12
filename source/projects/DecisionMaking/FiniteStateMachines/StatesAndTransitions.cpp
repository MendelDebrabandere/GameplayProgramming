#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

#define FOOD_SEARCH_RADIUS 25
#define FLEE_RADUIUS 8
#define ENEMY_SEARCH_RADIUS 35

void WanderState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	const bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	pAgent->SetToWander();
}

void SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };

	if (isValid == false || pAgent == nullptr) 
		return;

	AgarioFood* pNearestFood;
	isValid = pBlackboard->GetData("FoodNearBy", pNearestFood);

	if (isValid == false || pNearestFood == nullptr)
		return;

	pAgent->SetToSeek(pNearestFood->GetPosition());
}

void EvadeBiggerAgentState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
		return;

	AgarioAgent* pTarget;
	isValid = pBlackboard->GetData("Target", pTarget);
	if (isValid == false || pTarget == nullptr)
		return;

	pAgent->SetToFlee(pTarget->GetPosition());
}

void PursueSmallerAgentState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
		return;

	AgarioAgent* pTarget;
	isValid = pBlackboard->GetData("Target", pTarget);
	if (isValid == false || pTarget == nullptr)
		return;

	pAgent->SetToSeek(pTarget->GetPosition());
}

void MoveAwayFromBorderState::Update(Elite::Blackboard* pBlackboard, float deltaTime)
{
	AgarioAgent* pAgent;
	bool isValid{ pBlackboard->GetData("Agent", pAgent) };

	if (isValid == false || pAgent == nullptr)
		return;

	float worldSize;
	isValid = pBlackboard->GetData("WorldSize", worldSize);
	if (isValid == false)
		return;

	const float agentRadius{ pAgent->GetRadius() };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	const Vector2 fleeTarget{ 50,50 };
	
	pAgent->SetToSeek(fleeTarget);
}






//=======================================
//CONDITIONS
//=======================================
bool SeekFoodCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };

	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	isValid = pBlackboard->GetData("FoodVec", pFoodVec);

	if (isValid == false || pFoodVec == nullptr)
	{
		return false;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };

	AgarioFood* pClosestFood{ nullptr };
	float closestFoodDistanceSquared{ FLT_MAX };

	for (AgarioFood* pFood : *pFoodVec)
	{
		// The squared's don't fully work here and the math is kinda off, but it is not that bad (actually kinda good).
		// The math is bad in the way that, as the agent gets bigger, its food search gets smaller.
		// (more likely to search enemies which is good if agent is big)
		if (agentPos.DistanceSquared(pFood->GetPosition()) - pAgent->GetRadius() * pAgent->GetRadius() < closestFoodDistanceSquared)
		{
			closestFoodDistanceSquared = agentPos.DistanceSquared(pFood->GetPosition());
		}
	}

	if (closestFoodDistanceSquared <= FOOD_SEARCH_RADIUS * FOOD_SEARCH_RADIUS)
	{
		pBlackboard->ChangeData("FoodNearBy", pClosestFood);
		return true;
	}

	return false;
}

bool EvadeBiggerAgentCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
		return false;

	std::vector<AgarioAgent*>* pAgentVector;
	isValid = pBlackboard->GetData("AgentVector", pAgentVector);
	if (isValid == false || pAgentVector == nullptr)
		return false;

	const float radius{ pAgent->GetRadius() + FLEE_RADUIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, 0.9f);

	AgarioAgent* closestBiggerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distanceSquared{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distanceSquared > (radius + pOtherAgent->GetRadius()) * (radius + pOtherAgent->GetRadius()))
			continue;

		if (distanceSquared > closestEnemyDistance)
			continue;

		if (pOtherAgent->GetRadius() - pAgent->GetRadius() <= 0.5f)
			continue;

		closestEnemyDistance = distanceSquared;
		closestBiggerEnemy = pOtherAgent;
	}

	if (closestBiggerEnemy == nullptr)
		return false;

	pBlackboard->ChangeData("Target", closestBiggerEnemy);
	return true;
}

bool PursueSmallerAgentCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
		return false;

	std::vector<AgarioAgent*>* pAgentVector;
	isValid = pBlackboard->GetData("AgentVector", pAgentVector);
	if (isValid == false || pAgentVector == nullptr)
		return false;

	const float radius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPos{ pAgent->GetPosition() };

	DEBUGRENDERER2D->DrawCircle(agentPos, radius, Color{ 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

	AgarioAgent* closestSmallerEnemy{};
	float closestEnemyDistance{ FLT_MAX };

	for (AgarioAgent* pOtherAgent : *pAgentVector)
	{
		const float distanceSquared{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

		if (distanceSquared > (radius + pOtherAgent->GetRadius()) * (radius + pOtherAgent->GetRadius()))
			continue;

		if (distanceSquared > closestEnemyDistance)
			continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) < 2.0f)
			continue;

		if (pOtherAgent->GetRadius() > pAgent->GetRadius())
			continue;

		closestSmallerEnemy = pOtherAgent;
		closestEnemyDistance = distanceSquared;
	}

	if (closestSmallerEnemy ==  nullptr)
		return false;

	pBlackboard->ChangeData("Target", closestSmallerEnemy);
	return true;
}

bool MoveAwayFromBorderCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	if (isValid == false || pAgent == nullptr)
		return false;

	float worldSize;
	isValid = pBlackboard->GetData("WorldSize", worldSize);
	if (isValid == false)
		return false;

	const float agentRadius{ pAgent->GetRadius() - 3 };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	bool isAgentNearWall{ agentPosition.x <= agentRadius };
	isAgentNearWall = isAgentNearWall || (agentPosition.y <= agentRadius);
	isAgentNearWall = isAgentNearWall || (agentPosition.x >= worldSize - agentRadius);
	isAgentNearWall = isAgentNearWall || (agentPosition.y >= worldSize - agentRadius);

	return isAgentNearWall;
}

bool FSMConditions::WanderCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	// THIS IS THE SAME CODE AS SEEKFOOD BUT THE TRUE'S ARE NOW FALSE AND VICE VERSA
	// THIS IS THE SAME CODE AS SEEKFOOD BUT THE TRUE'S ARE NOW FALSE AND VICE VERSA
	// THIS IS THE SAME CODE AS SEEKFOOD BUT THE TRUE'S ARE NOW FALSE AND VICE VERSA
	//==============================================================================

	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };

	if (isValid == false || pAgent == nullptr)
	{
		return false;
	}

	isValid = pBlackboard->GetData("FoodVec", pFoodVec);

	if (isValid == false || pFoodVec == nullptr)
	{
		return false;
	}

	const Vector2 agentPos{ pAgent->GetPosition() };

	AgarioFood* pClosestFood{ nullptr };
	float closestFoodDistanceSquared{ FLT_MAX };

	for (AgarioFood* pFood : *pFoodVec)
	{
		// The squared's don't fully work here and the math is kinda off, but it is not that bad (actually kinda good).
		// The math is bad in the way that, as the agent gets bigger, its food search gets smaller.
		// (more likely to search enemies which is good if agent is big)
		if (agentPos.DistanceSquared(pFood->GetPosition()) - pAgent->GetRadius() * pAgent->GetRadius() < closestFoodDistanceSquared)
		{
			closestFoodDistanceSquared = agentPos.DistanceSquared(pFood->GetPosition());
		}
	}

	if (closestFoodDistanceSquared <= FOOD_SEARCH_RADIUS * FOOD_SEARCH_RADIUS)
	{
		return false;
	}

	return true;
}
