#include "stdafx.h"
#include "CollisionAgent.h"

using namespace Elite;

CollisionAgent::CollisionAgent()
	: m_pSteeringAgent{ new SteeringAgent() }
	, m_pBehavior{ new Wander() }
{
	m_pSteeringAgent->SetSteeringBehavior(m_pBehavior);
}

CollisionAgent::~CollisionAgent()
{
	SAFE_DELETE(m_pSteeringAgent);
	SAFE_DELETE(m_pBehavior);
}

void CollisionAgent::Update(float dt)
{
	m_GoalVelocity = m_pSteeringAgent->GetLinearVelocity();

	m_pSteeringAgent->Update(dt);
}

void CollisionAgent::Render(float dt)
{
	m_pSteeringAgent->Render(dt);
}

float CollisionAgent::GetRadius() const
{
	return m_Radius;
}

float CollisionAgent::GetTimeHorizon() const
{
	return m_TimeHorizon;
}

Elite::Vector2 CollisionAgent::GetPosition() const
{
	return m_pSteeringAgent->GetPosition();
}

SteeringAgent* CollisionAgent::GetSteeringAgent() const
{
	return m_pSteeringAgent;
}

Elite::Vector2 CollisionAgent::GetGoalVelocity() const
{
	return m_GoalVelocity;
}

Elite::Vector2 CollisionAgent::GetVelocity() const
{
	return m_Velocity;
}

void CollisionAgent::SetRadius(float newRadius)
{
	m_Radius = newRadius;
}

void CollisionAgent::SetBehavior(ISteeringBehavior* pNewBehavior)
{
	m_pBehavior = pNewBehavior;
}

void CollisionAgent::SetVelocity(Elite::Vector2 newVelocity)
{
	m_Velocity = newVelocity;
}

void CollisionAgent::SetPosition(Elite::Vector2 newPosition)
{
	m_pSteeringAgent->SetPosition(newPosition);
}
