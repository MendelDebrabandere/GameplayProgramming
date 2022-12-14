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

#define FLEE_RADIUS 20
#define ATTACK_RADIUS 40

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

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 FleeTargetPos;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		if (!pBlackboard->GetData("FleeTarget", FleeTargetPos))
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToFlee(FleeTargetPos);
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

	bool IsSmallerEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pAgentsVec;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		if (!pBlackboard->GetData("AgentsVec", pAgentsVec) || pAgentsVec == nullptr)
		{
			return false;
		}

		const float attackRadius{ pAgent->GetRadius() + ATTACK_RADIUS };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, attackRadius, { 0.0f, 1.0f, 1.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

		AgarioAgent* closestSmallerEnemy{ nullptr };
		float closestDistSq{ attackRadius * attackRadius };

		for (AgarioAgent* pOtherAgent : *pAgentsVec)
		{
			if (pOtherAgent == nullptr)
				continue;
			else if (pOtherAgent == pAgent)
				continue;

			const float distSquared{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

			if (distSquared > closestDistSq)
				continue;

			if (pAgent->GetRadius() - pOtherAgent->GetRadius() > 1.5f)
			{
				closestSmallerEnemy = pOtherAgent;
				closestDistSq = distSquared;
			}
		}

		if (closestSmallerEnemy == nullptr)
			return false;

		pBlackboard->ChangeData("Target", closestSmallerEnemy->GetPosition());
		return true;
	}

	bool IsBiggerEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pAgentVector;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		if (!pBlackboard->GetData("AgentsVec", pAgentVector) || pAgentVector == nullptr)
		{
			return false;
		}

		const float fleeRadius{ pAgent->GetRadius() + FLEE_RADIUS };
		const Elite::Vector2 agentPos{ pAgent->GetPosition() };

		DEBUGRENDERER2D->DrawCircle(agentPos, fleeRadius, { 1.0f, 0.0f, 0.0f, 1.0f }, DEBUGRENDERER2D->NextDepthSlice());

		AgarioAgent* closestBiggerEnemy{ nullptr };
		float closestDistSq{ fleeRadius * fleeRadius };

		for (AgarioAgent* pOtherAgent : *pAgentVector)
		{
			if (pOtherAgent == nullptr)
				continue;
			if (pOtherAgent == pAgent)
				continue;

			const float distSquared{ pOtherAgent->GetPosition().DistanceSquared(agentPos) };

			if (distSquared > closestDistSq)
				continue;

			if (pOtherAgent->GetRadius() - pAgent->GetRadius() > 1.5f)
			{
				closestBiggerEnemy = pOtherAgent;
				closestDistSq = distSquared;
			}
		}

		if (closestBiggerEnemy == nullptr) return false;

		pBlackboard->ChangeData("FleeTarget", closestBiggerEnemy);
		return true;
	}
}










#endif