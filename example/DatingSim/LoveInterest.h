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
class EmotionalState : public pocket_fsm::StateIF<void>
{
	BASE_STATE(EmotionalState)

	// Different for all states
	REACT(Compliment) = 0;
	REACT(Joke) = 0;

	// Different for a few states
	REACT(Flirt);
	REACT(Gift);

	// Same for all states
	REACT(Insult) final;

	// unused events
	void onEntry() override {};
	void onExit() override {};
};

// Basic state machine 
class LoveInterest : public pocket_fsm::FiniteStateMachine<EmotionalState>
{
public:
	LoveInterest();
};