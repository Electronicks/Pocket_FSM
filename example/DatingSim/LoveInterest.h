#pragma once

#include "pocket_fsm.h"

// List of interactions or inputs
struct Compliment {};
struct Joke {};
struct Gift {};
struct Flirt {};
struct Insult {};

// List of states
class Indifferent;
class Curious1;
class Curious2;
class Open;
class Delighted;
class Vexed;
class NumberAcquired;

// Generic emotional state
class EmotionalState : public pocket_fsm::StateIF
{
	BASE_STATE(EmotionalState)

	// Interface overrides / unused
	REACT(OnEntry) override {};
	REACT(OnExit) override {};

	// Different for all states
	REACT(Compliment) = 0;
	REACT(Joke) = 0;

	// Different for a few states
	REACT(Flirt);
	REACT(Gift);

	// Same for all states
	REACT(Insult) final;
};

// Basic state machine 
class LoveInterest : public pocket_fsm::FiniteStateMachine<EmotionalState>
{
public:
	LoveInterest();
};