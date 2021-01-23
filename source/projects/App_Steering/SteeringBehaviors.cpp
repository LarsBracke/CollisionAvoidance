//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringAgent.h"
#include "CombinedBehaviors/TheFlock.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();	// desired velocity
	steering.LinearVelocity.Normalize();	// normalize the lineair velocity
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();	// rescaling to max speed

	//DEBUG RENDERING
	if (pAgent->CanRenderBehavior()) 
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0, 0.5f }, 0.4f);
	}

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);
	SteeringOutput steering{};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();	// desired velocity
	steering.LinearVelocity = -(steering.LinearVelocity);	// flipping the vector (flee)
	steering.LinearVelocity.Normalize(); 
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed(); // rescaling to max speed

	return steering;
}

///////////////////////////////////////
//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(true);
	SteeringOutput steering{};

	float slowRadius{ 10.f };
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	float distance{ pAgent->GetPosition().Distance(m_Target.Position) };
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	
	if (distance < slowRadius)
	{
		float slowdown = distance / slowRadius;
		steering.LinearVelocity *= slowdown;
	}

	return steering;
}

///////////////////////////////////////
//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(false);
	SteeringOutput steering{};

	Elite::Vector2 toTarget = (m_Target.Position - pAgent->GetPosition());	// calc velocity to target
	float angle = atan2(toTarget.y, toTarget.x);	// find angle of this velocity
	float range = 0.15f;
	float angleOffset = M_PI / 2.f;	
	float agentRotation = pAgent->GetRotation();

	if (agentRotation < 0.f) agentRotation = M_PI + (M_PI - abs(agentRotation));	// limit angle range
	if (agentRotation > 2 * M_PI) agentRotation -= 2 * M_PI;
	pAgent->SetRotation(agentRotation);

	angle += angleOffset;	// offset the angle
	if (angle < 0.f) angle = M_PI + (M_PI - abs(angle));	// limit angle range
	if (angle > 2 * M_PI) angle -= 2 * M_PI;

	if (abs(agentRotation - angle) > range)
	{
		if
		(
			(agentRotation < angle )
			&&
			(abs(angle - agentRotation))
			<
			(2*M_PI- abs(angle - agentRotation))

		)	// determine ang velocity based on rotation of agent
		{
			steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
		}
		else
		{
			if (cos(angle) > 0.f && sin(angle) > 0.f)
			{
				steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
			}
			else
			{
				steering.AngularVelocity = -(pAgent->GetMaxAngularSpeed());
			}
		}
	}

	return steering;
}

//WANDER (base> SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	TargetData target;

	float angleMultiplier{ ( (rand() % 201) - 100 ) / 100.f };	// randomize multiplier for new angle
	m_WanderAngle = m_WanderAngle + (angleMultiplier * m_AngleChange);	// getting a new wanderangle

	Elite::Vector2 agentPos{ pAgent->GetPosition() };
	Elite::Vector2 agentVelocity{ pAgent->GetLinearVelocity() };
	Elite::Vector2 velocityChange{ cos(m_WanderAngle), sin(m_WanderAngle) };

	target.Position =	// setting a new target
		agentPos + 
		(agentVelocity.GetNormalized() * m_Offset) + 
		(velocityChange.GetNormalized() * m_Radius);

	if (pAgent->CanRenderBehavior())	// debug rendering
	{
		DEBUGRENDERER2D->DrawCircle
		(agentPos + agentVelocity.GetNormalized() * m_Offset, m_Radius,
			{ 1, 0, 0, 0.5f }, 0.4f);
		DEBUGRENDERER2D->DrawPoint(target.Position, 5.f,
			{ 0, 0, 1, 0.5f }, 0.4f);
	}

	SetTarget(target);

	steering = Seek::CalculateSteering(deltaT, pAgent);
	return steering;
}

void Wander::SetTarget(const TargetData& pTarget)
{
	m_Target = pTarget;
}

//PERSUIT (base> SEEK)
//******
SteeringOutput Persuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;

	Elite::Vector2 targetVelocity = m_Target.LinearVelocity;
	Elite::Vector2 targetPos = m_Target.Position;
	float persuitMultiplier{ 1.25f };
	m_Target = targetPos + persuitMultiplier * targetVelocity;
	
	steering = Seek::CalculateSteering(deltaT, pAgent);
	steering.LinearVelocity = persuitMultiplier * steering.LinearVelocity;
	return steering;
}

//EVADE (base> PERSUIT)
//******
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	Elite::Vector2 toTarget{ pAgent->GetPosition() - m_Target.Position };
	float distance{ toTarget.Magnitude() };

	if (distance < m_EvadeRadius)
	{
		steering = Persuit::CalculateSteering(deltaT, pAgent);
		steering.LinearVelocity = -(steering.LinearVelocity);
	}
	else
	{
		steering.IsValid = false;
	}
	return steering;
}
