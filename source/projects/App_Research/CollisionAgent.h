#pragma once

#include "projects/App_Steering/SteeringAgent.h"
#include "projects/App_Steering/SteeringBehaviors.h"

class CollisionAgent final
{
public:
	CollisionAgent();
	~CollisionAgent();

	void Update(float dt);
	void Render(float dt);

	float GetRadius() const;
	float GetTimeHorizon() const;
	Elite::Vector2 GetPosition() const;
	SteeringAgent* GetSteeringAgent() const;
	Elite::Vector2 GetGoalVelocity() const;
	Elite::Vector2 GetVelocity() const;

	void SetRadius(float newRadius);
	void SetBehavior(ISteeringBehavior* pNewBehavior);
	void SetVelocity(Elite::Vector2 newVelocity);
	void SetPosition(Elite::Vector2 newPosition);

private:
	SteeringAgent* m_pSteeringAgent = nullptr;
	ISteeringBehavior* m_pBehavior = nullptr;

	float m_Radius = 2.5f;

	Elite::Vector2 m_Velocity = { 0.f, 0.f };
	Elite::Vector2 m_GoalVelocity = { 0.f, 0.f };

	float m_TimeHorizon = 2.5f;
};

