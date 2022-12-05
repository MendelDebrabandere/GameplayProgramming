/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToWander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		if (!pBlackboard->GetData("Target", targetPos))
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToSeek(targetPos);
		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioFood*>* pFoodVec;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}		
		if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		{
			return false;
		}

		const float searchRadius{ 50.f + pAgent->GetRadius()};
		AgarioFood* pClosestFood = nullptr;
		float closestDistSq{ searchRadius * searchRadius };
		Elite::Vector2 agentPos = pAgent->GetPosition();

		//TODO: Debug rendering

		for (auto& pFood : *pFoodVec)
		{
			float distSq = pFood->GetPosition().DistanceSquared(agentPos);
			if (distSq < closestDistSq)
			{
				pClosestFood = pFood;
				closestDistSq = distSq;
			}
		}

		if (pClosestFood != nullptr)
		{
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}
		return false;
	}
}










#endif