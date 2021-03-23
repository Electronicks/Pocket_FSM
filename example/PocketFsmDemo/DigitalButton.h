#pragma once
#include "pocket_fsm.h"
#include <shared_mutex> // Lock mechanism for when I use cross sendEvent() from different threads

// Step 0: Print the state machine diagram as referance so you have an idea of what you are doing! ;P

// Step #1.1: Forward declare the implementation class
// Its content is unknown outside of the concrete states
class ButtonImpl;

// Step #1.2 (optional): You may forward declare your concrete states. This is not required, 
// but handy for documentation in the header here since they are stringified.
// Alternatively, identify each state with an enum value, reported via virtual function.
enum class E_ButtonState
{
	NoPress,
	BtnPress,
};

// Step #2: List event structures that the state machine reacts to (i.e.: state machine inputs)
// Include any data that may be relevant for the state to handle. Can be in, out or inout.
struct PressEvent { uint16_t keycode; };
struct ReleaseEvent { bool result = false; };
struct ResetEvt {};

// You can use the event messaging system to define getters to the pimpl
struct GetKeyCode { uint16_t keycode; };

// Step #3.1: Declare your base state for the state machine, deriving from the state interface parameterized with the pimpl
class ButtonStateIF : public pocket_fsm::StatePimplIF<ButtonImpl>
{
	// Step #3.2: Use the provided macro, this sets up the changeState function. Visibility is public thereafter.
	BASE_STATE(ButtonStateIF)

	// Step #3.3: Override base class reactions to OnEntry/OnExit, or leave it to concrete classes.
	REACT(OnEntry) override {};
	REACT(OnExit) override {};

	// Step #3.4: Declare a react function for each event in Step 2. Use macro to ensure consistent signature.
	// Implement default behaviour, leave undefined or mark final.
	REACT(PressEvent) {};
	REACT(ReleaseEvent) 
	{ 
		e.result = false; 
	};

	// You can make sure all event have the same reaction by making the react function final
	REACT(ResetEvt) final;
	REACT(GetKeyCode) final;
	
	// Pure virtual ensures concrete classes have to define this function and use the enum
	virtual E_ButtonState getState() const = 0;
};

// Step #4.1: Define the State Machine object for your custom state
class DigitalButton : public pocket_fsm::FiniteStateMachine<ButtonStateIF>
{
public:
	// Add parameters required to instantiate your pimpl
	DigitalButton(const char *name);
	DigitalButton(const DigitalButton &copy) : FiniteStateMachine(copy){};

	// Getter for custom state identifier. This is arguably better than the stringified name of the class.
	inline E_ButtonState getState()
	{
		return getCurrentState()->getState();
	}

protected:

	// Step #4.2: Optionally declare a lock object field and override lock/unlock() to secure cross thread operation.
	std::shared_mutex _dumbSpinlock;

	void lock() override;

	void unlock() override;
};

