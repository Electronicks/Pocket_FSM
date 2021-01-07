#pragma once

#include "pocket_fsm.h"
#include <forward_list>
#include <iostream>

struct LockImpl; // Pimpl can be struct too

struct Configure { std::forward_list<int> combination; };
struct Digit { int digit;  };
struct Reset {};

class Open;
class Locked;
class Lockdown;

class LockState : public pocket_fsm::StateIF<LockImpl>
{
	BASE_STATE(LockState)

	REACT(Configure)
	{
		std::cout << "[LOCK] Cannot configure the lock from state " << _name << std::endl;
	}

	REACT(Digit)
	{
		std::cout << "[LOCK] Cannot enter a digit from state " << _name << std::endl;
	}

	REACT(Reset)
	{
		std::cout << "[LOCK] Cannot reset the lock from state " << _name << std::endl;
	}
	
	void onEntry() {};
	void onExit() {};
};

class CombinationLock : public pocket_fsm::FiniteStateMachine<LockState>
{
public:
	CombinationLock();
};