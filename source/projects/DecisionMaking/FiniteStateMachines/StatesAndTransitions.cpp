#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	AgarioAgent* pAgent;

	bool isValid{ pBlackboard->GetData("Agent", pAgent) };
	
	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	pAgent->SetToWander();
}


void SeekFoodState::OnEnter(Blackboard* pBlackboard)
{
	
}

bool FoodNearByCondition::Evaluate(Blackboard* pBlackboard) const
{
	const float foodRadius{ 50.f };
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
		float dist1 = agentPos.DistanceSquared(pFood1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pFood2->GetPosition());
		return dist1 < dist2;
	};
	auto closestFoodIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), elementdist);

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
