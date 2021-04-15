// File: CombinationSafe.h
// Author: Electronicks
// Date: January 18th 2021

#include "CombinationSafe_Nested.h"

class SafeImpl : public pocket_fsm::PimplBase
{
public:
	std::forward_list<int> _combination;
	std::forward_list<int>::const_iterator _p;
	// error flag is replaced with a nested state machine

	void AdoptCombination(const std::forward_list<int> &newCombination)
	{
		_combination = newCombination;
		Reset();
	}

	bool EnterNumber(int number)
	{
		return number == *_p++;
	}

	inline bool isEntryComplete() const
	{
		return _p == _combination.cend();
	}

	void Reset()
	{
		_p = _combination.cbegin();
	}

	void Open()
	{
		std::cout << "[SAFE] *Beep* *Beep* *Click* Ta-da!" << std::endl;
	}

	void Close()
	{
		std::cout << "[Safe] *Click* *Grinck* *Boop* *Boop*" << std::endl;
	}

	void Lockdown()
	{
		std::cout << "[Safe] *WAH* *WAH* *WAH* *WAH*" << std::endl;
	}

	void Clear()
	{
		std::cout << "[Safe] *Beep* *Boop*" << std::endl;
	}
};

// Concrete nested states
class LockedNoError;
class LockedError;

// Nested base state
class BaseLockedState : public SafeState
{
	REACT(Reset) final // Nested states have this evevnt reaction in common
	{
		pimpl()->Clear();
		pimpl()->Reset();
		changeState<LockedNoError>();
	}
};

class LockedNoError : public BaseLockedState
{
	CONCRETE_STATE(LockedNoError)
	INITIAL_STATE(LockedNoError)

	REACT(Number) override
	{
		if (!pimpl()->EnterNumber(e))
		{
			changeState<LockedError>();
		}
		else if (pimpl()->isEntryComplete())
		{
			changeState<Open>();
		}
	}
};

class LockedError : public BaseLockedState
{
	CONCRETE_STATE(LockedError)

	REACT(OnEntry) override
	{
		if (pimpl()->isEntryComplete())
		{
			changeState<Lockdown>();
		}
	}

	REACT(Number) override
	{
		pimpl()->EnterNumber(e);
		if (pimpl()->isEntryComplete())
		{
			changeState<Lockdown>();
		}
	}
};

// Concrete core states

class Open : public SafeState
{
	CONCRETE_STATE(Open)

	INITIAL_STATE(Open)

	REACT(OnEntry) override
	{
		pimpl()->Open();
	}

	REACT(Configure) override
	{
		if (!e.combination.empty())
		{
			pimpl()->AdoptCombination(e.combination);
			changeState<Locked>();
		}
	}
};

// This concrete state is also a state machine itself for BaseLockedStates
class Locked : public pocket_fsm::NestedStateMachine<BaseLockedState, SafeState>
{
	CONCRETE_STATE(Locked)

	REACT(OnEntry) override
	{
		pimpl()->Close();
		initialize(new LockedNoError(_pimpl)); // Init nested FSM on entry, passing the PimplSmartPtr
	}

	// 
	NESTED_REACT(Number)
	NESTED_REACT(Reset)
};

class Lockdown : public SafeState
{
	CONCRETE_STATE(Lockdown)

	REACT(OnEntry)
	{
		pimpl()->Lockdown();
	}

	REACT(Reset) override
	{
		pimpl()->Reset();
		changeState<Locked>();
	}

	REACT(OnExit)
	{
		pimpl()->Reset();
	}
};

CombinationSafe::CombinationSafe()
{
	initialize(new Open(new SafeImpl()));
}
