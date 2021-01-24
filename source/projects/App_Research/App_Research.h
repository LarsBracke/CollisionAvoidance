#ifndef STEERINGBEHAVIORS_APPLICATION_H
#define STEERINGBEHAVIORS_APPLICATION_H
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteInterfaces/EIApp.h"
#include "projects/App_Steering/SteeringBehaviors.h"

#include "CollisionAgent.h"

class SteeringAgent;
class Obstacle;

//-----------------------------------------------------------------
// Application
//-----------------------------------------------------------------
class App_Research final : public IApp
{
public:
	//Constructor & Destructor
	App_Research() = default;
	virtual ~App_Research();

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	//Datamembers
	int m_AgentAmount = 5;
	std::vector<CollisionAgent*> m_pAgents = {};
	std::vector<CollisionAgent*> m_pNeighbors = {};

	float m_NeighborhoodRadius = 10.f;
	void RegisterNeighbors(CollisionAgent* pAgent);
	float CalculateTimeToCollision(CollisionAgent* pFirst, CollisionAgent* pSecond);
	Elite::Vector2 CalculateAvoidanceForce(CollisionAgent* pFirst, CollisionAgent* pSecond, float timeToCollision);

private:

	//Datamembers
	std::vector<string> m_TargetLabelsVec = {};
	TargetData m_Target = {};

	bool m_VisualizeTarget = true;
	bool m_IsInitialized = false;
	bool m_TrimWorld = true;
	float m_TrimWorldSize = 50.f;
	int m_AgentToRemove = -1;

	std::vector<Obstacle*> m_pObstacles;
	const float m_MaxObstacleRadius = 5.f;
	const float m_MinObstacleRadius = 1.f;
	const float m_MinObstacleDistance = 10.f;

	//Interface Functions
	//void UpdateTarget(CollisionAgent& a);
	void UpdateTargetLabel();

	void AddObstacle();
	Elite::Vector2 GetRandomObstaclePosition(float obstacleRadius, bool& positionFound);

	//C++ make the class non-copyable
	App_Research(const App_Research&) = delete;
	App_Research& operator=(const App_Research&) = delete;
};
#endif