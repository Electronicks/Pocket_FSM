// File: CombinationSafe.h
// Author: Electronicks
// Date: January 18th 2021

#pragma once // Header sentinel

// Include the framework to use it.
#include "pocket_fsm.h"
#include <forward_list>
#include <iostream>

// I will need a pimpl to hold the combination and possibly more
struct SafeImpl;

// State machine inputs
struct Configure { std::forward_list<int> combination; };
struct Number { int digit;  };
struct Reset {};

// Concrete states
class Open; 		// The safe is open and waiting for a configuration
class Locked;		// The safe is locked and processing digits from the user
class Lockdown;		// The safe is in lockdown and requires a reset to use again.

class SafeState : public pocket_fsm::StatePimplIF<SafeImpl>
{
	BASE_STATE(SafeState)

	// Override interface / unused
	REACT(OnEntry) override {};
	REACT(OnExit) override {};

	// Default reactions => are unhandled
	REACT(Configure)
	{
		std::cout << "[SAFE] Cannot configure the safe from state " << _name << std::endl;
	}

	REACT(Number)
	{
		std::cout << "[SAFE] Cannot enter a digit from state " << _name << std::endl;
	}

	REACT(Reset)
	{
		std::cout << "[SAFE] Cannot reset the safe from state " << _name << std::endl;
	}
};

class CombinationSafe : public pocket_fsm::FiniteStateMachine<SafeState>
{
public:
	CombinationSafe();
};

// End of CombinationLock.h