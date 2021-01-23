#pragma once
#include "SpacePartitioning.h"
#include "../SteeringHelpers.h"
#include "FlockingSteeringBehaviors.h"
#include "projects/App_Steering/SteeringBehaviors.h";

class ISteeringBehavior;
class SteeringAgent;
class BlendedSteering;
class PrioritySteering;

class Flock
{
public:
	Flock(
		int flockSize = 2000,
		float worldSize = 200.f, 
		SteeringAgent* pAgentToEvade = nullptr, 
		bool trimWorld = false);

	~Flock();

	void Update(float deltaT);
	void UpdateAndRenderUI();
	void Render(float deltaT);

	void RegisterNeighbors(SteeringAgent* pAgent);
	int GetNrOfNeighbors() const;
	const vector<SteeringAgent*>& GetNeighbors() const;

	Elite::Vector2 GetAverageNeighborPos() const;
	Elite::Vector2 GetAverageNeighborVelocity() const;

	void UpdateMouseTarget(const Elite::Vector2& position);

private:
	// flock agents
	int m_FlockSize = 50;
	vector<SteeringAgent*> m_Agents;
	bool m_RenderAgents;

	// neighborhood agents
	vector<SteeringAgent*> m_Neighbors;
	float m_NeighborhoodRadius = 10.f;
	int m_NrOfNeighbors = 0;

	// evade target
	SteeringAgent* m_pAgentToEvade = nullptr;

	// mouse target
	Elite::Vector2 m_MouseTarget;

	// world info
	bool m_TrimWorld = false;
	float m_WorldSize = 0.f;
	
	// steering Behaviors
	BlendedSteering* m_pBlendedSteering = nullptr;
	PrioritySteering* m_pPrioritySteering = nullptr;
	Cohesion* m_pCohesion = nullptr;
	Separation* m_pSeparation = nullptr;
	Allignment* m_pAllignment = nullptr;
	Seek* m_pSeek = nullptr;
	Wander* m_pWander = nullptr;
	Evade* m_pEvade = nullptr;

	// cellspace
	CellSpace m_CellSpace;
	bool m_UsePartitioning;
	std::vector<Elite::Vector2> m_OldPositions;
	
	// private functions
	float* GetWeight(ISteeringBehavior* pBehaviour);
	void RegisterOldPositions();

private:
	Flock(const Flock& other);
	Flock& operator=(const Flock& other);
};