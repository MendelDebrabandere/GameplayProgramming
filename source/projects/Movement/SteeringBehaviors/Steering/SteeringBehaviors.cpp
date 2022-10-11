//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{	
	SteeringOutput steering = {};

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	if ((m_Target.Position - pAgent->GetPosition()).Magnitude() <= 1)
	{
		steering.LinearVelocity = Elite::Vector2{0,0};
		return steering;
	}


	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if ((m_Target.Position - pAgent->GetPosition()).Magnitude() <= 10)
	{
		steering.LinearVelocity *= (m_Target.Position - pAgent->GetPosition()).Magnitude() / 20.f;
	}

	if (pAgent->CanRenderBehavior())
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), {0,1,0});

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(false);

	SteeringOutput steering = {};

	float agentAngle{ pAgent->GetRotation() };

	float diagonal{ sqrtf(pow(m_Target.Position.y - pAgent->GetPosition().y,2) + pow(m_Target.Position.x - pAgent->GetPosition().x, 2))};

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	float angleFromSinToTarget{ asin((m_Target.Position.y - agentPos.y) / diagonal)   };
	float angleFromCosToTarget{ acos((m_Target.Position.x - agentPos.x) / diagonal)   };

	float angleToTarget{};

	if (angleFromCosToTarget <= 3.1415f / 2)
	{
		angleToTarget = angleFromSinToTarget;
	}
	else
	{
		if (angleFromSinToTarget >= 0)
		{
			angleToTarget = 3.1415f - angleFromSinToTarget;
		}
		else
		{
			angleToTarget = -3.1315f - angleFromSinToTarget;
		}
	}
	
	float angleToDo{ angleToTarget  - agentAngle };

	if (abs(angleToDo) > 3.1415f)
	{
		if (angleToDo < -3.1415f)
			angleToDo += 2 * 3.1415f;
		else
			angleToDo -= 2 * 3.1415f;
	}

	steering.AngularVelocity = angleToDo;
	

	return steering;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	float circleRadius{ 5 };
	Elite::Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetDirection().GetNormalized() * (circleRadius + 2) };

	float randAngle = { (rand() % 360) / 180.f * 3.1415f };

	Elite::Vector2 randomPoint{ circleCenter.x + cosf(randAngle) * circleRadius , circleCenter.y + sinf(randAngle) * circleRadius };


	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawCircle(circleCenter, circleRadius, { 0,0,1 }, 0.9f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
		DEBUGRENDERER2D->DrawPoint(randomPoint, 3, { 1,0,0 });
	}

	steering.LinearVelocity = randomPoint - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	

	return steering;
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 distance{ m_Target.Position - pAgent->GetPosition() };
	float T{ distance.Magnitude() / pAgent->GetMaxLinearSpeed()};
	Elite::Vector2 direction{ m_Target.Position + m_Target.LinearVelocity * T};

	direction -= pAgent->GetPosition();

	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
	}

	return steering;
}

//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 toTarget = pAgent->GetPosition() - (m_Target).Position;
	float distanceSquared = toTarget.MagnitudeSquared();

	if (distanceSquared > m_EvadeRadius * m_EvadeRadius)
	{
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_EvadeRadius, {0,0,1}, 0.9f);
		steering.IsValid = false;
		return steering;

	}


	Elite::Vector2 distance{ m_Target.Position - pAgent->GetPosition() };
	float T{ distance.Magnitude() / pAgent->GetMaxLinearSpeed() };
	Elite::Vector2 direction{ m_Target.Position + m_Target.LinearVelocity * T };

	direction -= pAgent->GetPosition();

	steering.LinearVelocity = -direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
	}

	return steering;
}

