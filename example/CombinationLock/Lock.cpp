#include "Lock.h"

struct LockImpl
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
};

PIMPL_DELETER_DEF(LockImpl);

class Open : public LockState
{
	CONCRETE_STATE(Open)

	INITIAL_STATE(Open)

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

class Locked : public LockState
{
	CONCRETE_STATE(Locked)

	REACT(Digit) override
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
		_pimpl->Reset();
	}
};

class Lockdown : public LockState
{
	CONCRETE_STATE(Lockdown)

	REACT(Reset) override
	{
		_pimpl->Reset();
		changeState<Locked>();
	}
};

CombinationLock::CombinationLock()
{
	initialize(new Open(new LockImpl()));
}
