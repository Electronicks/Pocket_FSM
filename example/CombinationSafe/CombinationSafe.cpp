// File: CombinationSafe.h
// Author: Electronicks
// Date: January 18th 2021

#include "CombinationSafe.h"

struct SafeImpl
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

PIMPL_DELETER_DEF(SafeImpl);

class Open : public SafeState
{
	CONCRETE_STATE(Open)

	INITIAL_STATE(Open)

	REACT(OnEntry) override
	{
		_pimpl->Open();
	}

	REACT(Configure) override
	{
		if (!e.combination.empty())
		{
			_pimpl->_combination = e.combination;
			_pimpl->Reset();
			changeState<Locked>();
		}
	}
};

class Locked : public SafeState
{
	CONCRETE_STATE(Locked)

	REACT(OnEntry) override
	{
		_pimpl->Close();
	}

	REACT(Number) override
	{
		_pimpl->_error |= e.digit != *_pimpl->_p;

		_pimpl->_p++;

		if (_pimpl->_p == _pimpl->_combination.cend())
		{
			if (_pimpl->_error)
				changeState<Lockdown>();
			else
				changeState<Open>();
		}
	}

	REACT(Reset) override
	{
		_pimpl->Clear();
	}
};

class Lockdown : public SafeState
{
	CONCRETE_STATE(Lockdown)

	REACT(OnEntry)
	{
		_pimpl->Lockdown();
	}

	REACT(Reset) override
	{
		_pimpl->Reset();
		changeState<Locked>();
	}

	REACT(OnExit)
	{
		_pimpl->Reset();
	}
};

CombinationSafe::CombinationSafe()
{
	initialize(new Open(new SafeImpl()));
}
