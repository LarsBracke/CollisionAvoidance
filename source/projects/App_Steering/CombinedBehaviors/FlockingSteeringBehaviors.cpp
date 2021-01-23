#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "TheFlock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"

//*********************
//SEPARATION (FLOCKING)
Separation::Separation(Flock* flock)
	: m_Flock{ flock }
{ }

SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	Elite::Vector2 toNeighbor;
	steering.LinearVelocity = { 0.f,0.f };
	float distance;
	float totalDistance{ 0.f };
	for (int index = 0; index < m_Flock->GetNrOfNeighbors(); ++index)
	{
		totalDistance +=
		Elite::Distance(pAgent->GetPosition(), m_Flock->GetNeighbors()[index]->GetPosition());
	}
	for (int index = 0; index < m_Flock->GetNrOfNeighbors(); ++index)
	{
		distance = 
		Elite::Distance(pAgent->GetPosition(), m_Flock->GetNeighbors()[index]->GetPosition());
		toNeighbor = { m_Flock->GetNeighbors()[index]->GetPosition() - pAgent->GetPosition() };
		steering.LinearVelocity +=
		toNeighbor * (distance / totalDistance);

	}
	steering.LinearVelocity = -(steering.LinearVelocity);
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	//std::cout << m_Flock->GetNrOfNeighbors() << std::endl;
	//std::cout << steering.LinearVelocity.x << ' ' << steering.LinearVelocity.y << std::endl;
	//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0, 1, 0, 0.5f }, 0.4f);

	return steering;
}

//*******************
//COHESION (FLOCKING)
Cohesion::Cohesion(Flock* flock)
	: m_Flock{ flock }
{ }

SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	Elite::Vector2 neighborhoodCenter{ m_Flock->GetAverageNeighborPos() };
	m_Target = neighborhoodCenter;
	steering = Seek::CalculateSteering(deltaT, pAgent);
	//std::cout << neighborhoodCenter.x << ' ' << neighborhoodCenter.y << std::endl;
	//std::cout << steering.LinearVelocity.x << ' ' << steering.LinearVelocity.y << std::endl;
	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)

Allignment::Allignment(Flock* flock)
	: m_Flock{ flock }
{ }

SteeringOutput Allignment::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering;
	steering.LinearVelocity = m_Flock->GetAverageNeighborVelocity();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	return steering;
}