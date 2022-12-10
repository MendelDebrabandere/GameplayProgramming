#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

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
	isValid = pBlackboard->GetData("NearestFood", pNearestFood);

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

	const float wallSpacing{ 10 };
	const float agentRadius{ pAgent->GetRadius() + wallSpacing };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	Vector2 fleeTarget{ agentPosition };

	if (agentPosition.x < agentRadius)
	{
		fleeTarget.x = worldSize;
	}
	else if (agentPosition.x + agentRadius > worldSize)
	{
		fleeTarget.x = 0.0f;
	}

	if (agentPosition.y < agentRadius)
	{
		fleeTarget.y = worldSize;
	}
	else if (agentPosition.y + agentRadius > worldSize)
	{
		fleeTarget.y = 0.0f;
	}
	
	pAgent->SetToFlee(fleeTarget);
}





#define FOOD_SEARCH_RADIUS 50
#define ENEMY_SEARCH_RADIUS 20

//=======================================
//CONDITIONS
//=======================================
bool SeekFoodCondition::Evaluate(Elite::Blackboard* pBlackboard) const
{
	const float foodRadius{ 40.f };
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

	Vector2 agentPos{ pAgent->GetPosition() };

	auto elementdist = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2) 
	{ 
		const float dist1 = agentPos.DistanceSquared(pFood1->GetPosition());
		const float dist2 = agentPos.DistanceSquared(pFood2->GetPosition());
		return dist1 < dist2;
	};
	const auto closestFoodIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), elementdist);

	if (closestFoodIt != pFoodVec->end())
	{
		AgarioFood* pFood = *closestFoodIt;
		
		if (agentPos.DistanceSquared(pFood->GetPosition()) < foodRadius * foodRadius)
		{
			pBlackboard->ChangeData("FoodNearBy", pFood);
			return true;
		}
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

	const float radius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
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

		if (pOtherAgent->GetRadius() < pAgent->GetRadius())
			continue;

		if (abs(pOtherAgent->GetRadius() - pAgent->GetRadius()) <= 1.1f)
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

	const float radius{ pAgent->GetRadius() + FOOD_SEARCH_RADIUS };
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

	const float agentRadius{ pAgent->GetRadius() + ENEMY_SEARCH_RADIUS };
	const Vector2 agentPosition{ pAgent->GetPosition() };

	bool isAgentNearWall{ agentPosition.x <= agentRadius };
	isAgentNearWall = isAgentNearWall || (agentPosition.y <= agentRadius);
	isAgentNearWall = isAgentNearWall || (agentPosition.x >= worldSize - agentRadius);
	isAgentNearWall = isAgentNearWall || (agentPosition.y >= worldSize - agentRadius);

	return isAgentNearWall;
}