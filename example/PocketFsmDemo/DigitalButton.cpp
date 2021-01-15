#include "DigitalButton.h"
#include <iostream>
#include <vector>

// Step #5.1: Implement your pimpl! Hold all fields and declare all functions 
// to perform whatever output your state machine is handling.
class ButtonImpl {
public:
	ButtonImpl(const char *name)
	    : _name(name)
	{}

	~ButtonImpl()
	{
		using namespace std;
		cout << "Button is being destroyed: " << _name << endl;
		_downKey = UINT16_MAX;
	}

	void DisplayPress()
	{
		using namespace std;
		cout << "Key " << _downKey << " is down" << endl;
	}

	void DisplayRelease()
	{
		using namespace std;
		cout << "Key " << _downKey << " is up" << endl;
		_downKey = UINT16_MAX;
	}

	uint16_t _downKey = UINT16_MAX;

	const char * _name;

};

// Step #5.2: Use the macro to define the specialized deleter for your pimpl
PIMPL_DELETER_DEF(ButtonImpl)

// Step #6: Forward declare all your concrete states first, then for each concrete state:
//  - Declare and use CONCRETE_STATE macro to set up constructor with stringified name
//  - Implement all pure virtual functions and override those desired: onEntry/Exit and react(...)
//  - Call changeState to transition after returning
//  - Transition action is optional and occurs between exit and entry. Pimpl is still valid then.
//  - The initial state needs a constructor that calls the provided macro.
//  - Also implement final functions from the base state

class NoPress;		// When the button is not pressed
class BtnPress;		// When the button is indeed pressed

class NoPress : public ButtonStateIF
{
	CONCRETE_STATE(NoPress)

	// Constructor of the initial state also sets up the pimpl with its deleter
	INITIAL_STATE(NoPress)

	REACT(PressEvent) override
	{
		_pimpl->_downKey = e.keycode;
		// You can use lambda for transition function
		changeState<BtnPress>( [this] () {
			printf("(%s) press true\n", _name); 
		});
	}

	E_ButtonState getState() const override
	{
		return E_ButtonState::NoPress;
	}
};

class BtnPress : public ButtonStateIF
{
	CONCRETE_STATE(BtnPress)

	REACT(pocket_fsm::OnEntry) override
	{
		_pimpl->DisplayPress();
	}

	REACT(ReleaseEvent) override
	{
		// Or you can use std::bind with a member function. The function is called on destruction after exit
		changeState<NoPress>(std::bind(&BtnPress::ReleaseTransition, this));
		e.result = true;
	}

	void ReleaseTransition()
	{
		printf("(%s) press false\n", _name);
	}

	REACT(pocket_fsm::OnExit) override
	{
		_pimpl->DisplayRelease();
	}

	E_ButtonState getState() const override
	{
		return E_ButtonState::BtnPress;
	}
};

// The base state's final functions will need a definition here
// These include pimpl getters via events
void ButtonStateIF::react(GetKeyCode &e)
{
	e.keycode = _pimpl->_downKey;
}

void ButtonStateIF::react(ResetEvt &e)
{
	changeState<NoPress>([]()
		{
			printf("Resetting state machine!\n");
		});
}

// Step #7: Define the customized state machine functions and constructor
//  - Constructor calls initialize() with an instance of your initial state and your pimpl. Base class takes ownership of both pointers.
//  - Optionally override lock/unlock functions to secure cross thread operation
DigitalButton::DigitalButton(const char *name)
	: FiniteStateMachine()
{
	// The top level state machine can also become a factory object by making the pimpl a base class for different implementations, 
	// for which the selection of which implementation can be done here.
	initialize(new NoPress(new ButtonImpl(name)));
}

void DigitalButton::lock()
{
	while (!_dumbSpinlock.try_lock());
}

void DigitalButton::unlock()
{
	_dumbSpinlock.unlock();
}