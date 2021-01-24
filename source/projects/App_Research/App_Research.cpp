//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_Research.h"
#include "projects/App_Steering/SteeringAgent.h"
#include "projects/App_Steering/Obstacle.h"

//Destructor
App_Research::~App_Research()
{
	for (CollisionAgent* pAgent : m_pAgents)
	{
		SAFE_DELETE(pAgent);
	}
	m_pAgents.clear();

	for (auto& o : m_pObstacles)
		SAFE_DELETE(o);
	m_pObstacles.clear();
}

//Functions
void App_Research::Start()
{
	// create the agents
	for (int index = 0; index < m_AgentAmount; ++index)
	{
		m_pAgents.push_back(new CollisionAgent());
	}

	m_IsInitialized = true;
}

void App_Research::Update(float deltaTime)
{
	////INPUT
	//if(INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft) && m_VisualizeTarget)
	//{
	//	auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
	//	m_Target.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
	//	for (auto& a : m_AgentVec)
	//	{
	//		UpdateTarget(a);
	//	}
	//}

	// debug rendering
	for (CollisionAgent* pAgent : m_pAgents)
	{
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), pAgent->GetRadius(), Color(1, 0, 0), 0.4f);
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), m_NeighborhoodRadius, Color(1, 1, 1), 0.4f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetGoalVelocity(), pAgent->GetGoalVelocity().Magnitude(), Color(0, 1, 0), 0.4f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetVelocity(), pAgent->GetVelocity ().Magnitude(), Color(0, 0, 1), 0.4f);
	}

	// update the agents
	for (CollisionAgent* pAgent : m_pAgents)
	{

		// register the neighbors
		RegisterNeighbors(pAgent);

		// check collisions with other agent
		for (CollisionAgent* pNeighbor : m_pNeighbors)
		{
			float timeToCollision =
				CalculateTimeToCollision(pAgent, pNeighbor);

			if (timeToCollision < pAgent->GetTimeHorizon() && timeToCollision > 0.f)
			{
				std::cout << "collision going to happen" << endl;

				// adjust velocity
				Vector2 avoidanceForce = 
					CalculateAvoidanceForce(pAgent, pNeighbor, timeToCollision);

				Vector2 newVelocity = 
					pAgent->GetGoalVelocity() + avoidanceForce;
				
				newVelocity = 
					newVelocity.GetNormalized() * pAgent->GetSteeringAgent()->GetMaxLinearSpeed();

				pAgent->SetVelocity(newVelocity);

				pAgent->SetPosition(pAgent->GetPosition() + (newVelocity * deltaTime));
			}
			else
			{
				//std::cout << "no collision detected" << endl;

				// update the agent
				pAgent->SetVelocity(Vector2{ 0,0 });
				pAgent->Update(deltaTime);
			}
		}

		// trim to world
		if (pAgent->GetSteeringAgent())
		{
			if (m_TrimWorld)
				pAgent->GetSteeringAgent()->TrimToWorld(m_TrimWorldSize);

			//UpdateTarget(a);
		}
		

	}

#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int const menuWidth = 235;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
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

		ImGui::Text("Steering Behaviors");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Trim World", &m_TrimWorld);
		if (m_TrimWorld)
		{
			ImGui::SliderFloat("Trim Size", &m_TrimWorldSize, 0.f, 200.f, "%.1");
		}
		ImGui::Spacing();

		if (ImGui::Button("Add Obstacle"))
			AddObstacle();

		ImGui::Spacing();
		ImGui::Separator();

		//for (UINT i = 0; i < m_AgentVec.size(); ++i)
		//{
		//	ImGui::PushID(i);
		//	char headerName[100];
		//	snprintf(headerName, sizeof(headerName), "ACTOR %i", i);

		//	auto& a = m_AgentVec[i];

		//	if (ImGui::CollapsingHeader(headerName))
		//	{
		//		ImGui::Indent();
		//		//Actor Props
		//		if (ImGui::CollapsingHeader("Properties"))
		//		{
		//			auto v = a.pAgent->GetMaxLinearSpeed();
		//			if (ImGui::SliderFloat("Lin", &v, 0.f, 20.f, "%.2f"))
		//				a.pAgent->SetMaxLinearSpeed(v);

		//			v = a.pAgent->GetMaxAngularSpeed();
		//			if (ImGui::SliderFloat("Ang", &v, 0.f, 20.f, "%.2f"))
		//				a.pAgent->SetMaxAngularSpeed(v);

		//			v = a.pAgent->GetMass();
		//			if (ImGui::SliderFloat("Mass ", &v, 0.f, 20.f, "%.2f"))
		//				a.pAgent->SetMass(v);
		//		}


		//		bool behaviourModified = false;

		//		ImGui::Spacing();

		//		ImGui::PushID(i + 50);
		//		ImGui::AlignFirstTextHeightToWidgets();
		//		ImGui::Text(" Behavior: ");
		//		ImGui::SameLine();
		//		ImGui::PushItemWidth(100);
		//		if (ImGui::Combo("", &a.SelectedBehavior, "Seek\0Wander\0Flee\0Arrive\0Face\0Evade\0Pursuit\0Hide\0AvoidObstacle\0Align\0FacedArrive\0SlowClap", 4))
		//		{
		//			behaviourModified = true;
		//		}
		//		ImGui::PopItemWidth();
		//		ImGui::PopID();

		//		ImGui::Spacing();
		//		ImGui::PushID(i + 100);
		//		ImGui::AlignFirstTextHeightToWidgets();
		//		ImGui::Text(" Target: ");
		//		ImGui::SameLine();
		//		int itemSelected2 = 0;
		//		ImGui::PushItemWidth(100);
		//		auto selectedTargetOffset = a.SelectedTarget + 1;
		//		if (ImGui::Combo("", &selectedTargetOffset, [](void* vec, int idx, const char** out_text)
		//		{
		//			std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);

		//			if (idx < 0 || idx >= (int)vector->size())
		//				return false;

		//			*out_text = vector->at(idx).c_str();
		//			return true;
		//		}, reinterpret_cast<void*>(&m_TargetLabelsVec), m_TargetLabelsVec.size()))
		//		{
		//			a.SelectedTarget = selectedTargetOffset - 1;
		//			behaviourModified = true;
		//		}
		//		ImGui::PopItemWidth();
		//		ImGui::PopID();
		//		ImGui::Spacing();
		//		ImGui::Spacing();

		//		if (behaviourModified)
		//			SetAgentBehavior(a);

		//		if (ImGui::Button("x"))
		//		{
		//			m_AgentToRemove = i;
		//		}

		//		ImGui::SameLine(0, 20);

		//		bool isChecked = a.pAgent->CanRenderBehavior();
		//		ImGui::Checkbox("Render Debug", &isChecked);
		//		a.pAgent->SetRenderBehavior(isChecked);

		//		ImGui::Unindent();
		//	}

		//	ImGui::PopID();
		//}

		//if (m_AgentToRemove >= 0)
		//{
		//	RemoveAgent(m_AgentToRemove);
		//	m_AgentToRemove = -1;
		//}

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif


}

void App_Research::Render(float deltaTime) const
{
	for (CollisionAgent* pAgent : m_pAgents)
	{
		pAgent->Render(deltaTime);
	}

	if (m_TrimWorld)
	{
		vector<Elite::Vector2> points =
		{
			{ -m_TrimWorldSize,m_TrimWorldSize },
			{ m_TrimWorldSize,m_TrimWorldSize },
			{ m_TrimWorldSize,-m_TrimWorldSize },
			{-m_TrimWorldSize,-m_TrimWorldSize }
		};
		DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
	}

	//Render Target
	if (m_VisualizeTarget)
		DEBUGRENDERER2D->DrawSolidCircle(m_Target.Position, 0.3f, { 0.f,0.f }, { 1.f,0.f,0.f }, -0.8f);
}

void App_Research::RegisterNeighbors(CollisionAgent* pAgent)
{
	m_pNeighbors.clear();
	for (CollisionAgent* pOtherAgent : m_pAgents)
	{
		if (pAgent != pOtherAgent)
		{
			Vector2 agentPos = pAgent->GetSteeringAgent()->GetPosition();
			Vector2 otherAgentPos = pOtherAgent->GetSteeringAgent()->GetPosition();

			float distance = Distance(agentPos, otherAgentPos);

			if (distance < m_NeighborhoodRadius)
			{
				m_pNeighbors.push_back(pOtherAgent);
			}
		}
	}
}

float App_Research::CalculateTimeToCollision(CollisionAgent* pFirst, CollisionAgent* pSecond)
{
	float totalRadius = pFirst->GetRadius() + pSecond->GetRadius();
	Vector2 toSecondPos = pSecond->GetPosition() - pFirst->GetPosition();

	float c = Elite::Dot(toSecondPos, toSecondPos) - (totalRadius * totalRadius);
	if (c < 0) return 0.f; // the agents are already colliding

	Vector2 toFirstVelocity = pFirst->GetGoalVelocity() + pSecond->GetGoalVelocity();
	float a = Elite::Dot(toFirstVelocity, toFirstVelocity);
	float b = Elite::Dot(toSecondPos, toFirstVelocity);

	float discriminant = b * b - a * c;
	if (discriminant <= 0.f)
		return FLT_MAX;
	
	float tau = (b - sqrt(discriminant)) / a;
	if (tau < 0.f)
		return FLT_MAX;
	return tau;
}

Elite::Vector2 App_Research::CalculateAvoidanceForce(CollisionAgent* pFirst, CollisionAgent* pSecond, float timeToCollision)
{
	Elite::Vector2 forceDirection
		= pFirst->GetPosition() + pFirst->GetGoalVelocity() * timeToCollision - 
		pSecond->GetPosition() * pSecond->GetGoalVelocity() * timeToCollision;

	Elite::Vector2 force{ };
	float magnitude{ };
	if (forceDirection.x != 0 && forceDirection.y != 0)
	{
		force = forceDirection / sqrt(Dot(forceDirection, forceDirection));
		magnitude = (pFirst->GetTimeHorizon() - timeToCollision) / (timeToCollision);
		force *= magnitude;
	}

	return force;
}

//void App_Research::UpdateTarget(CollisionAgent& a)
//{
//
//	bool useMouseAsTarget = a.SelectedTarget < 0;
//	if (useMouseAsTarget)
//		a.pBehavior->SetTarget(m_Target);
//	else
//	{
//		auto pAgent = m_AgentVec[a.SelectedTarget].pAgent;
//		auto target = TargetData{};
//		target.Position = pAgent->GetPosition();
//		target.Orientation = pAgent->GetOrientation();
//		target.LinearVelocity = pAgent->GetLinearVelocity();
//		target.AngularVelocity = pAgent->GetAngularVelocity();
//		a.pBehavior->SetTarget(target);
//	}
//}

void App_Research::UpdateTargetLabel()
{
	m_TargetLabelsVec.clear();

	stringstream ss;
	m_TargetLabelsVec.push_back("Mouse");
	for (UINT i = 0; i < m_pAgents.size(); ++i)
	{
		ss << "Agent " << i;
		m_TargetLabelsVec.push_back(ss.str());
		ss.str("");
	}
}

void App_Research::AddObstacle()
{
	auto radius = randomFloat(m_MinObstacleRadius, m_MaxObstacleRadius);
	bool positionFound = false;
	auto pos = GetRandomObstaclePosition(radius, positionFound);

	if (positionFound)
		m_pObstacles.push_back(new Obstacle(pos, radius));
}

Elite::Vector2 App_Research::GetRandomObstaclePosition(float newRadius, bool& positionFound)
{
	positionFound = false;
	Elite::Vector2 pos;
	int tries = 0;
	int maxTries = 200;

	while (positionFound == false && tries < maxTries)
	{
		positionFound = true;
		pos = randomVector2(m_TrimWorldSize);

		for (const auto& obstacle : m_pObstacles)
		{
			auto radiusSum = newRadius + obstacle->GetRadius();
			auto distance = Distance(pos, obstacle->GetCenter());

			if (distance < radiusSum + m_MinObstacleDistance)
			{
				positionFound = false;
				break;
			}
		}
		++tries;
	}

	return pos;
}
