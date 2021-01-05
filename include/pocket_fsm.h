#pragma once

#include <functional> // std::function
#include <memory>     // std::unique_ptr

// Cross platform assert to check for DEBUG runtime check
#if defined(WIN32)
#define ASSERT_X_PLAT(expr, msg) _ASSERT_EXPR(expr, L#msg)
#elif defined (UNIX)
#include <cassert>
#define X_PLAT_ASSERT(expr, msg) assert(expr && msg)
#endif

namespace pocket_fsm
{

// The State interface holds transition call logic. It hold a Pointer to IMPLementation that is passed
// to the next state on destruction.
template<class Pimpl>
class StateIF
{
public:
	// Signature for a function to be used during transition
	using TransitionFunc = std::function<void()>;

	using PimplDeleter = void(*)(Pimpl*);

	// Constructor used by initial state
	StateIF() = default;

	// Constructor used by all other states: do not copy any StateIF data when chaining constructors
	// Pimpl is not passed until after the transition function is called
	StateIF(StateIF &s) = delete;

	// Call transition and hand over the pimpl to the next state
	virtual ~StateIF() 
	{
		if (_onTransition) 
			_onTransition();

		if (_nextState)
			_pimpl.swap(_nextState->_pimpl);
	}

	// These functions are run when the state becomes active or inactive
	// The function run on transition occur between these events
	virtual void onEntry() = 0;
	virtual void onExit() = 0;

	// If a state transition was called, this object will be the next state, otherwise it's just nullptr.
	inline StateIF *getNextState()
	{
		return _nextState;
	}

	const char *_name = nullptr; // Stringified name of the concrete class

protected:
	// Typedef beautifier
	using PimplSmartPtr = std::unique_ptr<Pimpl, PimplDeleter>;

	// The next state that the state machine must transition to
	StateIF *_nextState = nullptr;

	// Action to perform on changing of state after exit before entry
	TransitionFunc _onTransition = nullptr;

	// Pointer to implementation. This object will contain all internal logic and is unknown outside of the states.
	// unique_ptr doesn't work here because it causes compilation issues regarding deleting forward declared classes.
	PimplSmartPtr _pimpl = { nullptr, nullptr };
};


// The State Machine handles sending events to the current state and manages state transitions. Derive from this class
// with your state base class and Implementation class as parameters and set up a constructor with your initial state.
// Parameter of FiniteStateMachine needs to be a descendant of StateIF
template<class S>
class FiniteStateMachine
{
public:
	// Basic constructor / destructor
	FiniteStateMachine() = default;
	virtual ~FiniteStateMachine() 
	{
		_currentState->onExit(); // Cleanup
	}

	// Call this method with an object that S can react to!
	template<class E>
	void sendEvent(E &evt)
	{
		ASSERT_X_PLAT(_currentState, L"You did not call \"initialize(new MyInitialState(...));\" in your constructor!");
		lock();
		_currentState->react(evt);
		if (_currentState->getNextState())
		{
			// This cast is safe because of the static assert in this class
			changeState(static_cast<S*>(_currentState->getNextState()));
		}
		// else, event did not raise a change of state, moving along
		unlock();
	}

	inline const char * getCurrentStateName() const
	{
		ASSERT_X_PLAT(_currentState, L"You did not call \"initialize(new MyInitialState(...));\" in your constructor!");
		return _currentState->_name;
	}

protected:
	// Descendants call this in their constructor to set the initial state 
	void initialize(S *initialState) 
	{
		ASSERT_X_PLAT(!_currentState, L"You already initialized me!");
		_currentState.reset(initialState);
		_currentState->onEntry();
	}

	inline const S* getCurrentState() const
	{
		return _currentState.get();
	}

	// Override these functions using your favorite lock mechanism 
	// to secure State Machine usage across threads
	virtual void lock() {};
	virtual void unlock() {};

private:
	// This function performs the state transition and calls state events
	void changeState(S *nextState)
	{
		_currentState->onExit();
		_currentState.reset(nextState); // old state destructor calls onTransition, hands off pimpl and gets deleted
		_currentState->onEntry();
	}

	// The current state machine state.
	std::unique_ptr<S> _currentState = nullptr;
};

// Call this macro in your base state deriving from StateIF that will serve as the base class for your concrete states
#define BASE_STATE(BASENAME) \
protected: \
	template<class S> \
	void changeState(TransitionFunc onTransit = nullptr) { \
		static_assert(std::is_base_of<BASENAME, S>::value, "Parameter of changeState needs to be a descendant of " #BASENAME); \
		ASSERT_X_PLAT(!_nextState, "You have already called \"changeState<...>(...)\ in this react!"); \
		_onTransition = onTransit; \
		_nextState = new S(); \
	} \
public:

// Use this macro to reliaby declare your react functions with the proper signature.
// You are free to follow up with =0, final, {} or whatever suits your needs.
// The event parameter is simply named e
#define REACT(EVENT) \
	virtual void react(EVENT &e)

// Call this macro in a concrete state where NAME is the name of your State Class
// derived from BASE, and BASE is derived from StateIF. This macro sets up the chain.
#define CONCRETE_STATE(NAME) \
public: \
	NAME() { _name=#NAME; }


// Use this macro in the custom constructor of your initial state to initialize it
// The state takes ownership of the PIMPL instance
#define INITIAL_STATE_CTOR(NAME, PIMPL, PIMPLDEL) \
	_name = #NAME; \
	ASSERT_X_PLAT(((PIMPL == nullptr) ^ (PIMPLDEL == nullptr)) == false, \
		"If you give a pimpl, you also need a deleter : just a static function that calls delete on your pimpl should do."); \
    _pimpl = PimplSmartPtr(PIMPL, PIMPLDEL);

} // End of namespace