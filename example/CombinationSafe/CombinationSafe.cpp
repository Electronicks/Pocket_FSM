// File: CombinationSafe.h
// Author: Electronicks
// Date: January 18th 2021

#include "CombinationSafe.h"

struct SafeImpl : public pocket_fsm::PimplBase
{
	std::forward_list<int> _combination;
	std::forward_list<int>::const_iterator _p;
	// This state flag could be a different state instead, 
	// but it keeps the error status invisible to the user of the lock
	bool _error = false;

	void Reset()
	{
		_p = _combination.cbegin();
		_error = false;
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
			pimpl()->_combination = e.combination;
			pimpl()->Reset();
			changeState<Locked>();
		}
	}
};

class Locked : public SafeState
{
	CONCRETE_STATE(Locked)

	REACT(OnEntry) override
	{
		pimpl()->Close();
	}

	REACT(Number) override
	{
		pimpl()->_error |= e != *pimpl()->_p;

		pimpl()->_p++;

		if (pimpl()->_p == pimpl()->_combination.cend())
		{
			if (pimpl()->_error)
				changeState<Lockdown>();
			else
				changeState<Open>();
		}
	}

	REACT(Reset) override
	{
		pimpl()->Clear();
	}
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
