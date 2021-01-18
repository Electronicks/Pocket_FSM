#pragma once

#include <functional> // std::function
#include <memory>     // std::unique_ptr

namespace pocket_fsm
{

	/************************************************************************************
	                     M A C R O   D E F I N I T I O N S
    -------------------------------------------------------------------------------------
	BASE_STATE(BASENAME) : Put at the top of your base sate class header.
	REACT(EVENT) : Function signature for react functions. Event parameter is e.
	PIMPL_DELETER_DEF(PIMPL) : Define deleter function after the pimpl in source file.
	CONCRETE_STATE(NAME) : Put at the top of all concrete states in source file.
	INITIAL_STATE(NAME) : Put in the concrete state that will serve as initial state.
	************************************************************************************/

// Call this macro in your base state class deriving from StateIF or StatePimplIF<>.
// It defines the changeState<NextState>() function for your concrete classes.
#define BASE_STATE(BASENAME) \
	protected: \
		template<class S> \
		void changeState(TransitionFunc onTransit = nullptr) { \
			static_assert(std::is_base_of<BASENAME, S>::value, "Parameter of changeState needs to be a descendant of " #BASENAME); \
			ASSERT_X_PLAT(!_nextState, LR"(You have already called " changeState<...>() " in this react!)"); \
			_onTransition = onTransit; \
			_nextState = new S(); \
		} \
	public:

// Use this macro to reliaby declare your react functions with the proper signature.
// The event parameter is simply named e ans is a non const reference.
#define REACT(EVENT) \
	virtual void react(EVENT &e)

// Because the pimpl is only forward declared, the deleter needs to be defined where the pimpl is also defined.
#define PIMPL_DELETER_DEF(PIMPL) \
template<> \
void pocket_fsm::PimplDeleter<PIMPL>::operator()(PIMPL *p) \
{ delete p; }

// Call this macro in a concrete state where NAME is the name of the class
#define CONCRETE_STATE(NAME) \
public: \
	NAME() { _name=#NAME; }

// Use this macro to define the constructor of your initial state.
// It is given the new pimpl instance and it takes ownership of it
// This macro is technically only necessary if you use a pimpl, but it's good labelling.
#define INITIAL_STATE(NAME) \
public: \
	NAME(PimplType *newPimpl) \
		: NAME() \
	{ \
		_pimpl = PimplSmartPtr(newPimpl); \
	}


// Cross platform assert used for DEBUG runtime check
#if defined(WIN32)
#define ASSERT_X_PLAT(expr, msg) _ASSERT_EXPR(expr, msg)
#elif defined (UNIX)
#include <cassert>
#define X_PLAT_ASSERT(expr, msg) assert(expr && msg)
#endif

// This declaration requires to be specialized for every instance by using the PIMPL_DELETER_DEF macro
template<class Pimpl>
struct PimplDeleter
{
	void operator()(Pimpl *);
};

// OnEntry and OnExit are internal events that cannot be raised outside of the State Machine
struct OnEntry { };
struct OnExit { };

// The State interface holds the transition call logic.
class StateIF
{
public:
	// Useful typedefs
	using OnEntry = pocket_fsm::OnEntry;
	using OnExit = pocket_fsm::OnExit;

	// All states should be created clean : no copying allowed!
	StateIF() = default;
	StateIF(StateIF &s) = delete;
	virtual ~StateIF()
	{
		if (_onTransition)
		{
			_onTransition();
		}
	}
	
	// These functions are run when the state becomes active or inactive
	// The function run on transition occur between these events
	REACT(OnEntry) = 0;
	REACT(OnExit) = 0;

	// The next state is registered by a call to changeState<>()
	inline StateIF *getNextState()
	{
		return _nextState;
	}

	// Stringified name of the concrete class
	const char *_name = nullptr;

protected:
	// Beautifier
	using PimplType = void;
	using TransitionFunc = std::function<void()>; 	// Signature for a function to be used during transition

	// The next state registered for transition
	StateIF *_nextState = nullptr;

	// Function to be run on transition (i.e. between the exit and entry call.
	TransitionFunc _onTransition = nullptr;
};


// This version of StateIF holds a Pointer to IMPLementation 
// that is passed to the next state on destruction.
template<typename Pimpl>
class StatePimplIF : public StateIF
{
public:
	StatePimplIF() = default;
	StatePimplIF(StateIF &s) = delete;

	// Call transition and hand over the pimpl to the next state
	virtual ~StatePimplIF()
	{
		// Transit needs to occur before handing off the pimpl
		if (_onTransition)
		{
			_onTransition();
			_onTransition = nullptr; // So it is cleared after use
		}

		if (_nextState)
		{
			// assert in change state guarantees to only change between to children of the same base class.
			_pimpl.swap(static_cast<StatePimplIF<PimplType>*>(_nextState)->_pimpl);
		}
	}
protected:
	// Beautifiers
	using PimplSmartPtr = std::unique_ptr<Pimpl, PimplDeleter<Pimpl>>;
	using PimplType = Pimpl;

	// Pointer to implementation. This object will contain all internal logic and is unknown outside of the states.
	PimplSmartPtr _pimpl = { nullptr };
};

// The State Machine handles sending events to the current state and manages state transitions. Derive from this class
// with your state base class and Implementation class as parameters and set up a constructor with your initial state.
// Parameter of FiniteStateMachine needs to be a descendant of StateIF or StatePimplIF<>
template<class S>
class FiniteStateMachine
{
public:
	// Basic constructor / destructor
	FiniteStateMachine() = default;
	virtual ~FiniteStateMachine() 
	{
		OnExit exit;
		_currentState->react(exit); // Cleanup
	}

	// Call this method with an object that S can react to.
	// You cannot call internal events such as OnEntry and OnExit externally!
	template<typename E, typename = typename std::enable_if<
		!std::is_same<E, OnEntry>::value && !std::is_same<E, OnExit>::value>::type >
	void sendEvent(E &evt)
	{
		ASSERT_X_PLAT(_currentState, L"You did not call \"initialize(new MyInitialState(...));\" in your constructor!");
		lock();
		_currentState->react(evt);
		while (_currentState->getNextState())
		{
			// This cast is safe because of the static assert in this class
			changeState(static_cast<S*>(_currentState->getNextState()));
		}
		// else, event did not raise a change of state, moving along
		unlock();
	}

	// Get the stringified name of the concrete class
	inline const char * getCurrentStateName() const
	{
		ASSERT_X_PLAT(_currentState, L"You did not call \"initialize(new MyInitialState(...));\" in your constructor!");
		return _currentState->_name;
	}

protected:
	// Descendants call this in their constructor to set the initial state. The state machine takes ownership of the pointer.
	void initialize(S *initialState) 
	{
		ASSERT_X_PLAT(!_currentState, L"You already initialized me!");
		_currentState.reset(initialState);
		OnEntry entry;
		_currentState->react(entry);
		while (_currentState->getNextState()) // Entry usually doesn't changeState, but it can.
		{
			// This cast is safe because of the static assert in this class
			changeState(static_cast<S*>(_currentState->getNextState()));
		}
	}

	// Const accessor to the current state
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
		OnExit exit;
		OnEntry entry;
		_currentState->react(exit);
		_currentState.reset(nextState); // old state destructor calls onTransition, hands off pimpl and gets deleted
		_currentState->react(entry);
	}

	// The current state of the machine.
	std::unique_ptr<S> _currentState = nullptr;
};

} // End of namespace