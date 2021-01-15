#pragma once

#include "pocket_fsm.h"
#include <forward_list>
#include <iostream>

// My pimpl
struct SafeImpl;

// State inputs
struct Configure { std::forward_list<int> combination; };
struct Number { int digit;  };
struct Reset {};

// Concrete states
class Open;
class Locked;
class Lockdown;

class SafeState : public pocket_fsm::StatePimplIF<SafeImpl>
{
	BASE_STATE(SafeState)

	// Default reactions => are unhandled
	REACT(Configure)
	{
		std::cout << "[LOCK] Cannot configure the lock from state " << _name << std::endl;
	}

	REACT(Number)
	{
		std::cout << "[LOCK] Cannot enter a digit from state " << _name << std::endl;
	}

	REACT(Reset)
	{
		std::cout << "[LOCK] Cannot reset the lock from state " << _name << std::endl;
	}
	
	REACT(OnEntry) override {};
	REACT(OnExit) override {};
};

class CombinationSafe : public pocket_fsm::FiniteStateMachine<SafeState>
{
public:
	CombinationSafe();
};