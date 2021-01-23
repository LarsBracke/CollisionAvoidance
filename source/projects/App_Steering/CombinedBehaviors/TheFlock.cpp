#include "stdafx.h"
#include "TheFlock.h"

#include "../SteeringAgent.h"
#include "../SteeringBehaviors.h"
#include "CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 5.f }
	, m_NrOfNeighbors{ 0 }
	, m_CellSpace{ m_WorldSize, m_WorldSize, 25, 25, int(m_FlockSize / 4.f) }
	, m_UsePartitioning{ true }
	, m_RenderAgents{ false }
{
	// info
	std::cout << std::endl;
	std::cout << "Flock" << std::endl;
	std::cout << "=====" << std::endl;
	std::cout << "Neighborhood radius: " << '\t' << m_NeighborhoodRadius << std::endl;
	std::cout << "Number of agents: " << '\t' << m_FlockSize << std::endl;
	std::cout << "Worldsize: " << '\t' << '\t' << m_WorldSize << std::endl;

	std::cout << std::endl;
	std::cout << "Use the first checkbox to toggle partitioning on or off." << std::endl;
	std::cout << "The checkboxes below can be used for debug rendering." << std::endl;


	// init behaviors
	m_pCohesion = new Cohesion(this);
	m_pSeparation = new Separation(this);
	m_pAllignment = new Allignment(this);
	m_pSeek = new Seek();
	m_pWander = new Wander();

	m_pBlendedSteering = new BlendedSteering
	(
		{
		{m_pCohesion, 0.25f},
		{m_pSeparation, 0.5f},
		{m_pAllignment, 0.5f},
		{m_pSeek, 0.25f},
		{m_pWander, 0.5f}
		}
	);

	m_pEvade = new Evade();
	m_pPrioritySteering = new PrioritySteering({ m_pEvade, m_pBlendedSteering });

	// init agents
	m_pAgentToEvade = new SteeringAgent();
	m_pAgentToEvade->SetBodyColor({ 1.f,0.f,0.f });
	m_pAgentToEvade->SetSteeringBehavior(m_pWander);
	m_pAgentToEvade->SetMaxLinearSpeed(100.f);

	for (int index = 0; index < m_FlockSize; ++index)
	{
		SteeringAgent* pAgent{ new SteeringAgent() };
		pAgent->SetSteeringBehavior(m_pPrioritySteering);
		Elite::Vector2 pos
		{ float(rand() % int(m_WorldSize)), float(rand() % int(m_WorldSize)) };
		pAgent->SetPosition(pos);
		pAgent->SetMaxLinearSpeed(100.f);

		m_CellSpace.AddAgent(pAgent);

		m_Agents.push_back(pAgent);
		m_Neighbors.push_back(nullptr);
		m_OldPositions.push_back(Elite::Vector2(0, 0));
	}

	// set agent to debug
	m_CellSpace.SetAgentToDebug(m_Agents[0]);
}

Flock::~Flock()
{
	for (SteeringAgent* pAgent : m_Agents)
	{
		SAFE_DELETE(pAgent);
		pAgent = nullptr;
	}

	// agents in m_Neighbors and in the
	// m_Neighbors in m_Cellspace are
	// already deleted

	SAFE_DELETE(m_pAgentToEvade);
	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pCohesion);
	SAFE_DELETE(m_pSeparation);
	SAFE_DELETE(m_pAllignment);
	SAFE_DELETE(m_pSeek);
	SAFE_DELETE(m_pWander);
	SAFE_DELETE(m_pEvade);
}

void Flock::Update(float deltaT)
{
	Elite::Vector2 worldBottomLeft{ 0.f,0.f };
	Elite::Vector2 worldTopRight{ m_WorldSize, m_WorldSize };
	for (int index = 0; index < m_FlockSize; ++index)
	{
		if (m_UsePartitioning)
		{
			Elite::Vector2 lastPos
			{ m_OldPositions[index] };

			m_CellSpace.RegisterNeighbors(m_Agents[index]->GetPosition(), m_NeighborhoodRadius);

			m_Agents[index]->Update(deltaT);
			m_Agents[index]->TrimToWorld(worldBottomLeft, worldTopRight);

			m_CellSpace.UpdateAgentCell(m_Agents[index], lastPos);

			//// = not efficient at all...
			//m_NrOfNeighbors = m_CellSpace.GetNrOfNeighbors();
			//m_Neighbors = m_CellSpace.GetNeighbors();
		}
		else
		{
			RegisterNeighbors(m_Agents[index]);

			m_Agents[index]->Update(deltaT);
			m_Agents[index]->TrimToWorld(worldBottomLeft, worldTopRight);
		}
	}
	m_pAgentToEvade->Update(deltaT);
	m_pAgentToEvade->TrimToWorld(worldBottomLeft, worldTopRight);
	m_pEvade->SetTarget(m_pAgentToEvade->GetPosition());

	if (m_UsePartitioning) RegisterOldPositions();

}

void Flock::Render(float deltaT)
{
	if (m_RenderAgents)
	{
		for (SteeringAgent* pAgent : m_Agents)
		{
			pAgent->Render(deltaT);
		}
	}

	m_pAgentToEvade->Render(deltaT);
	m_CellSpace.RenderCells();
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
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

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// Implement checkboxes and sliders here

	ImGui::Checkbox(" Use Partitioning ", &m_UsePartitioning);
	ImGui::Checkbox(" Render Agents ", &m_RenderAgents);

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("Behavior Weights");
	ImGui::Spacing();
	ImGui::SliderFloat("Cohesion", &m_pBlendedSteering->m_WeightedBehaviors[0].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Separation", &m_pBlendedSteering->m_WeightedBehaviors[1].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Allignment", &m_pBlendedSteering->m_WeightedBehaviors[2].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Seek", &m_pBlendedSteering->m_WeightedBehaviors[3].weight, 0.f, 1.f, "%.2f");
	ImGui::SliderFloat("Wander", &m_pBlendedSteering->m_WeightedBehaviors[4].weight, 0.f, 1.f, "%.2f");

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Text("Debug Rendering");
	ImGui::Spacing();
	ImGui::Checkbox("Render Cells", &(m_CellSpace.GetDebugRenderBehavior().renderCells));
	ImGui::Checkbox("Render Neighborhoodcells", &(m_CellSpace.GetDebugRenderBehavior().renderNeighborhoodCells));
	ImGui::Checkbox("Circle Neighbors", &(m_CellSpace.GetDebugRenderBehavior().circleNeighbors));
	ImGui::Checkbox("Render Neighborhoodradius", &(m_CellSpace.GetDebugRenderBehavior().renderNeighborhoodRadius));
	ImGui::Checkbox("Display Cellindex", &(m_CellSpace.GetDebugRenderBehavior().displayCellIndex));
	ImGui::Checkbox("Display Cell's Agent Count", &(m_CellSpace.GetDebugRenderBehavior().displayAgentCount));


	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	m_NrOfNeighbors = 0;
	for (SteeringAgent* pElement : m_Agents)
	{
		if (pElement != pAgent)
		{
			float distance{ pElement->GetPosition().Distance(pAgent->GetPosition()) };
			if (distance < m_NeighborhoodRadius)
			{
				m_Neighbors[m_NrOfNeighbors] = pElement;
				++m_NrOfNeighbors;
			}
		}
	}
}

int Flock::GetNrOfNeighbors() const
{
	if (m_UsePartitioning)
	{
		return m_CellSpace.GetNrOfNeighbors();
	}
	else
	{
		return m_NrOfNeighbors;
	}
}

const vector<SteeringAgent*>& Flock::GetNeighbors() const
{
	if (m_UsePartitioning)
	{
		return m_CellSpace.GetNeighbors();
	}
	else
	{
		return m_Neighbors;
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	Elite::Vector2 sum{ 0.f, 0.f };
	if (m_UsePartitioning)
	{
		for (int index = 0; index < m_CellSpace.GetNrOfNeighbors(); ++index)
		{
			sum += m_CellSpace.GetNeighbors()[index]->GetPosition();
		}
		sum /= float(m_CellSpace.GetNrOfNeighbors());
	}
	else
	{
		for (int index = 0; index < m_NrOfNeighbors; ++index)
		{
			sum += m_Neighbors[index]->GetPosition();
		}
		sum /= float(m_NrOfNeighbors);
	}
	return sum;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	Elite::Vector2 avgVelocity{ 0.f,0.f };
	if (m_UsePartitioning)
	{
		if (m_CellSpace.GetNrOfNeighbors() == 0)
		{
			return avgVelocity;
		}
		else
		{
			for (int index = 0; index < m_CellSpace.GetNrOfNeighbors(); ++index)
			{
				avgVelocity.x += m_CellSpace.GetNeighbors()[index]->GetLinearVelocity().x;
				avgVelocity.y += m_CellSpace.GetNeighbors()[index]->GetLinearVelocity().y;
			}
			avgVelocity /= float(m_CellSpace.GetNrOfNeighbors());
			//std::cout << avgVelocity.x << ' ' << avgVelocity.y << std::endl;
			return avgVelocity;
		}
	}
	else
	{
		if (m_NrOfNeighbors == 0)
		{
			return avgVelocity;
		}
		else
		{
			for (int index = 0; index < m_NrOfNeighbors; ++index)
			{
				avgVelocity.x += m_Neighbors[index]->GetLinearVelocity().x;
				avgVelocity.y += m_Neighbors[index]->GetLinearVelocity().y;
			}
			avgVelocity /= float(m_NrOfNeighbors);
			return avgVelocity;
		}
	}
}

void Flock::UpdateMouseTarget(const Elite::Vector2& target)
{
	m_MouseTarget = target;
	m_pSeek->SetTarget(m_MouseTarget);
	m_pWander->SetTarget(m_MouseTarget);
}

float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->m_WeightedBehaviors;
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}

void Flock::RegisterOldPositions()
{
	for (int index = 0; index < m_FlockSize; ++index)
	{
		m_OldPositions[index] = m_Agents[index]->GetPosition();
	}
}

