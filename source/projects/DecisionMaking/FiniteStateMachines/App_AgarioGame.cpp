#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"


//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace FSMStates;
using namespace FSMConditions;
using namespace Elite;


App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pCustomAgent);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pConditions)
	{
		SAFE_DELETE(t);
	}

}

void App_AgarioGame::Start()
{
	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//DEFINE WANDER
	auto* pWanderState = new WanderState();
	m_pStates.push_back(pWanderState);

	//Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);
	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* newAgent = new AgarioAgent(randomPos);

		Elite::Blackboard* pBlackboard = CreateBlackboard(newAgent);


		FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackboard);
		newAgent->SetDecisionMaking(pStateMachine);


		m_pAgentVec.push_back(newAgent);
	}

	

	//-------------------
	//Create Custom Agent
	//-------------------
	const Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
	const Color customColor { 0.0f, 1.0f, 0.0f };
	m_pCustomAgent = new AgarioAgent(randomPos, customColor);

	//1. Create and add the necessary blackboard data
	Elite::Blackboard* pBlackboard = CreateBlackboard(m_pCustomAgent);

	//2. Create the different agent states
	// WANDER IS DEFINED ABOVE TO INITIALIZE AGENT
	auto* pSeekFoodState = new SeekFoodState();
	m_pStates.push_back(pSeekFoodState);

	auto* pEvadeBiggerAgentState = new EvadeBiggerAgentState();
	m_pStates.push_back(pEvadeBiggerAgentState);

	auto* pPursueSmallerAgentState = new PursueSmallerAgentState();
	m_pStates.push_back(pPursueSmallerAgentState);

	auto* pMoveAwayFromBorderState = new MoveAwayFromBorderState();
	m_pStates.push_back(pMoveAwayFromBorderState);


	//3. Create the conditions beetween those states
	auto* pWanderCondition = new WanderCondition();
	m_pConditions.push_back(pWanderCondition);

	auto* pSeekFoodCondition = new SeekFoodCondition();
	m_pConditions.push_back(pSeekFoodCondition);

	auto* pMoveAwayFromBorderCondition = new MoveAwayFromBorderCondition();
	m_pConditions.push_back(pMoveAwayFromBorderCondition);

	auto* pPursueSmallerAgentCondition = new PursueSmallerAgentCondition();
	m_pConditions.push_back(pPursueSmallerAgentCondition);

	auto* pEvadeBiggerAgentCondition = new EvadeBiggerAgentCondition();
	m_pConditions.push_back(pEvadeBiggerAgentCondition);

	//4. Create the finite state machine with a starting state and the blackboard
	auto* pStateMachine = new FiniteStateMachine(pWanderState, pBlackboard);

	//5. Add the transitions for the states to the state machine
	// stateMachine->AddTransition(startState, toState, condition)
	// startState: active state for which the transition will be checked
	// condition: if the Evaluate function returns true => transition will fire and move to the toState
	// toState: end state where the agent will move to if the transition fires

	//Everything that makes you wander
	pStateMachine->AddTransition(pSeekFoodState,			pWanderState, pWanderCondition);

	//Everything that makes you seek food
	pStateMachine->AddTransition(pWanderState,				pSeekFoodState, pSeekFoodCondition);
	pStateMachine->AddTransition(pPursueSmallerAgentState,	pSeekFoodState, pSeekFoodCondition);
	pStateMachine->AddTransition(pEvadeBiggerAgentState,	pSeekFoodState, pSeekFoodCondition);
	pStateMachine->AddTransition(pMoveAwayFromBorderState,	pSeekFoodState, pSeekFoodCondition);

	//Everything that makes you Distance from wall
	pStateMachine->AddTransition(pSeekFoodState,			pMoveAwayFromBorderState, pMoveAwayFromBorderCondition);
	pStateMachine->AddTransition(pWanderState,				pMoveAwayFromBorderState, pMoveAwayFromBorderCondition);

	//Everything that makes you pursue
	pStateMachine->AddTransition(pSeekFoodState,			pPursueSmallerAgentState, pPursueSmallerAgentCondition);
	pStateMachine->AddTransition(pPursueSmallerAgentState,	pPursueSmallerAgentState, pPursueSmallerAgentCondition);

	//Everything that makes you flee
	pStateMachine->AddTransition(pSeekFoodState,			pEvadeBiggerAgentState, pEvadeBiggerAgentCondition);
	pStateMachine->AddTransition(pWanderState,				pEvadeBiggerAgentState, pEvadeBiggerAgentCondition);
	pStateMachine->AddTransition(pPursueSmallerAgentState,	pEvadeBiggerAgentState, pEvadeBiggerAgentCondition);
	pStateMachine->AddTransition(pMoveAwayFromBorderState,	pEvadeBiggerAgentState, pEvadeBiggerAgentCondition);
	pStateMachine->AddTransition(pEvadeBiggerAgentState,	pEvadeBiggerAgentState, pEvadeBiggerAgentCondition);


	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pCustomAgent->SetDecisionMaking(pStateMachine);
}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pCustomAgent->CanBeDestroyed())
	{
		m_GameOver = true;

		//Update the other agents and food
		UpdateAgarioEntities(m_pFoodVec, deltaTime);
		UpdateAgarioEntities(m_pAgentVec, deltaTime);
		return;
	}
	//Update the custom agent
	m_pCustomAgent->Update(deltaTime);
	m_pCustomAgent->TrimToWorld(m_TrimWorldSize, false);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0, m_TrimWorldSize)));
	}
}

void App_AgarioGame::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pCustomAgent->Render(deltaTime);
}

Elite::Blackboard* App_AgarioGame::CreateBlackboard(AgarioAgent* a)
{
	Elite::Blackboard* pBlackboard = new Elite::Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("FoodVec", &m_pFoodVec);
	pBlackboard->AddData("FoodNearBy", static_cast<AgarioFood*>(nullptr));
	pBlackboard->AddData("WorldSize", m_TrimWorldSize);
	pBlackboard->AddData("AgentVector", &m_pAgentVec);
	pBlackboard->AddData("Target", static_cast<AgarioAgent*>(nullptr));

	return pBlackboard;
}

void App_AgarioGame::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pCustomAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
