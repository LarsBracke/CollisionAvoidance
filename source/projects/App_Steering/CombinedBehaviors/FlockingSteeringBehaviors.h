#pragma once
#include "../SteeringBehaviors.h"

class Flock;

//SEPARATION - FLOCKING
//*********************
class Separation : public ISteeringBehavior
{
public:
	Separation(Flock* flock);
	virtual ~Separation() = default;
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_Flock;
};

//COHESION - FLOCKING
//*******************
class Cohesion : public Seek
{
public:
	Cohesion(Flock* flock);
	virtual ~Cohesion() = default;
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_Flock;
};


//VELOCITY MATCH - FLOCKING
//************************
class Allignment : public ISteeringBehavior
{
public:
	Allignment(Flock* flock);
	virtual ~Allignment() = default;
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	Flock* m_Flock;
};
