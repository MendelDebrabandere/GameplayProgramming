/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

class Blackboard;

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	private:
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	private:
	};

	class EvadeBiggerAgentState : public Elite::FSMState
	{
	public:
		EvadeBiggerAgentState() : FSMState() {};
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;	private:
	private:
	};

	class PursueSmallerAgentState : public Elite::FSMState
	{
	public:
		PursueSmallerAgentState() : FSMState() {};
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	private:
	};

	class MoveAwayFromBorderState : public Elite::FSMState
	{
	public:
		MoveAwayFromBorderState() : FSMState() {};
		virtual void Update(Elite::Blackboard* pBlackboard, float deltaTime) override;
	private:
	};
}


//-----------------
//---TRANSITIONS---
//-----------------

namespace FSMConditions
{
	class SeekFoodCondition : public Elite::FSMCondition
	{
	public:
		SeekFoodCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class EvadeBiggerAgentCondition : public Elite::FSMCondition
	{
	public:
		EvadeBiggerAgentCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class PursueSmallerAgentCondition : public Elite::FSMCondition
	{
	public:
		PursueSmallerAgentCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};

	class MoveAwayFromBorderCondition : public Elite::FSMCondition
	{
	public:
		MoveAwayFromBorderCondition() : FSMCondition() {};
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	private:
	};
}

#endif