# CollisionAvoidance

## What are we going to do?
We will have a map with different agents each with a random wander-behavior. The agent will predict if there is a collision going to happen a certain time in the future. If an agent detects a collision, the agent will calculate its avoidance force. Using this force the agent will alter it’s velocity to avoid colliding with the other agent.

## What do we need?
First of all we need some agents. Each of these agents has their own steeringbehavior. This behavior calculates the goalvelocity each frame. The goalvelocity is the default velocity without any adjustments. An agent also has a normal velocity parameter, this one is used to update the position of the agent each frame. If an agent predicts that there will be no collision, the velocity is the same as the goalvelocity. Each agent will also need a radius. This radius is the distance where another agent may step into before we consider it a collision. At last each agent has a timehorizon. This is the maximum time for a predicted collision or in other words: how far are we going to look in the future for collisions. 
We will also need a playarea with some boundaries to keep the agents from going too far.

## How does It work?
The first step is to loop over the agents and  register each agent’s neighbors in a predetermined radius. We do this for performance reasons: we don’t want to check all agent on possible collisions, only the ones that are rather near to us.  We will also define a timehorizon and radius for the agents. In my example they have the same values for all the agents, but they could be different. For slower agents it might be interesting to increase the time horizon or you could increase the radius for a bigger agent.
Once we have the neighbors, we can loop over them. For each neighbor we calculate the time to collision (ttc) we can do this by solving the following equation:

||(AgentAPos + AgentAVel * ttc) – (AgentBPos + AgentBVel * ttc)||
 = RadiusA + RadiusB
 
In this equation we are looking for ttc: a time point where the distance between the future positions is equal to the sum of the radiuses. We can rewrite this as a quadratic equation with unknown ttc. 

W = AgentBPos – AgentAPos

V = AgentBVel – AgentAvel

(V * V) * pow(ttc, 2) + 2 * (W * V) * ttc + W * W – pow(RadiusA + RadiusB, 2) = 0

A quadratic equation has 2 solutions:
ttc+ and ttc-
-	If both are negative, there is no collision takes place
-	If one is negative and the other is positive, then they are already colliding
-	If they are both positive, the collision takes place at the lowest ttc value

If ttc is inside the time horizon, we need to calculate the avoidance force. You can look at this as the force that pushes the agent away right before colliding. The force is uses in the end as a velocity: we add the force to the current goal velocity of the agent. The magnitude of the force is also depending on the distance between the 2 agents. If they are very close to each other, the force will be bigger (we scale it with the ttc).

ForceDir = AgentAPos + AgentAVel * ttc – AgentBPos * AgentBVel * ttc

ForceDir  = ForceDir / sqrt(Dot(ForceDir, ForceDir))

ForceMag = (TimeHorizon - ttc) / ttc

Force = ForceDir * ForceMag

## Useful resources

GameAIPro 2, Chapter 19: collision avoidance

GDC talk: https://www.gdcvault.com/play/1021986/Forced-Based-Anticipatory-Collision-Avoidance
