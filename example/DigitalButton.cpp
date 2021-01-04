#include "DigitalButton.h"
#include <iostream>
#include <vector>

// Step #5: Implement your pimpl! Hold all fields and declare all functions
// Also define a static deleter function to replace the destructor. This is 
// required by the unique ptr
class ButtonImpl {
public:
	ButtonImpl(const char *name)
	    : _name(name)
	{}

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

	static void Deleter(ButtonImpl *obj)
	{
		using namespace std;
		obj->_downKey = UINT16_MAX;
		cout << "Button is being destroyed: " << obj->_name << endl;
	}
};

// Step #6: Forward declare all your concrete states first, then for each concrete state:
//  - Declare and use CONCRETE_STATE macro to set up constructor with stringified name
//  - Implement all pure virtual functions and override those desired: onEntry/Exit and react(...)
//  - Call changeState to transition after returning
//  - Transition action is optional and occurs between exit and entry. Pimpl is still valid then.
//  - The initial state needs a constructor that calls the provided macro.

class NoPress;		// When the button is not pressed
class BtnPress;		// When the button is indeed pressed

class NoPress : public ButtonState
{
	CONCRETE_STATE(NoPress)

	// Constructor of the initial state also sets up the pimpl with its deleter
	NoPress(ButtonImpl *pimpl)
	{
		INITIAL_STATE_CTOR(NoPress, pimpl, ButtonImpl::Deleter);
	}

	void react(PressEvent &evt) override
	{
		_pimpl->_downKey = evt.keycode;
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

class BtnPress : public ButtonState
{
	CONCRETE_STATE(BtnPress)

	void onEntry() override
	{
		_pimpl->DisplayPress();
	}

	void react(ReleaseEvent &evt) override
	{
		// Or you can use std::bind with a member function. The function is called on destruction.
		changeState<NoPress>(std::bind(&BtnPress::ReleaseTransition, this));
		evt.result = true;
	}

	void ReleaseTransition()
	{
		printf("(%s) press false\n", _name);
	}

	void onExit() override
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
void ButtonState::react(GetKeyCode &evt)
{
	evt.keycode = _pimpl->_downKey;
}

void ButtonState::react(ResetEvt &evt)
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
	initialize(new NoPress(new ButtonImpl(name)));
}
