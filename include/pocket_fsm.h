/*!
 *  @file pocket_fsm.h
 *  @author Electronicks
 *  @date 2021-02-05
 *
 *  The pocket_fsm single header framework.
 */

#pragma once

#include <functional> // std::function
#include <memory>     // std::unique_ptr
#if defined (UNIX)
#include <cassert>    // assert
#endif

namespace pocket_fsm
{

/************************************************************************************
						M A C R O   D E F I N I T I O N S
-------------------------------------------------------------------------------------

BASE_STATE(BASENAME) : Put at the top of your base state class header.
CONCRETE_STATE(NAME) : Put at the top of all concrete states in source file.
INITIAL_STATE(NAME) : Put in the concrete state that will serve as initial state.
REACT(EVENT) : Function signature for react functions. Event parameter is e.
NESTED_REACT(EVENT) : React implementation for nested state machines


*************************************************************************************
						C L A S S   D E F I N I T I O N S
-------------------------------------------------------------------------------------

PimplBase : The base class for the optional implementation class of the state machine
StateIF : The core for a state machine state that has no pimpl
StatePimplIF<Pimpl> : A state IF that also has a parameterized pimpl
FiniteStateMachine<Base> : The core fsm processing of states of the parameterized type
NestedStateMachine<Nest, Base> : FSM varaint nested inside a concrete state


*************************************************************************************
								  U S A G E
-------------------------------------------------------------------------------------
0. It is highly recommended to start with a state machine diagram, listing the number
and names of states, what stimuli the system responds to and what actions the system
takes in each state, on transitions, on entry and on exit.

1. Start by creating a header file, importing the Pocket FSM header.
	1. (Optional) The forward declaration of the Implementation Class if you use
		one and all the concrete states.
	2. A definition for all the input Events as any type. They may contain fields that
		sevre as inout parameters.
	3. A declaration of a Base State class inheriting from either StatePimplIF
		parameterized with your implementation class or StateIF and using the
		BASE_STATE macro. It also declares a react function for each event you defined
		earlier using the REACT macro plus the internal OnEntry and OnExit events.
	4. Declaration of your state machine class itself inheriting from
		FiniteStateMachine parameterized with your base state.

2. Create the cpp file and define the following:
	1. Give a full definition to your implementation class if you have one and derive
		from PimplBase. It should contains all data fields and methods to perform the
		state machine outputs.
	2. If you haven't forward declared your concrete classes in the header you need to
		do it here. This is required to change to a state not declared yet.
	3. Then define all your states using the CONCRETE_STATE() macro and overriding all
		necessary virtual functions. If you have an implementation class, use need to
		use the INITIAL_STATE  macro in the state you desire to start in.
	4. Define your top level state machine constructor, calling the parent's
		initialize() with an **new** instance of the initial state and a new
		instance of the implementation class. The State Machine takes ownership of
		those pointers.


************************************************************************************/

/*!
 *  Call this macro in your base state class deriving from StateIF or StatePimplIF<>.
 *  It defines the changeState<NextState>() function for your concrete classes.
 *  Members are public after this call
 *  @param BASENAME This base class
 */
#define BASE_STATE(BASENAME) \
	protected: \
		template<class CONCRETE> \
		void changeState(TransitionFunc onTransit = nullptr) { \
			static_assert(std::is_base_of<BASENAME, CONCRETE>::value, "Parameter of changeState needs to be a descendant of " #BASENAME); \
			pocket_fsm::internal::ASSERT(!_nextState, LR"(You have already called " changeState<...>() " in this react!)"); \
			_onTransition = onTransit; \
			_nextState = new CONCRETE(); \
		} \
	public:

 /*!
  *  Call this macro in a concrete state to set up the stringified name.
  *
  *  @param NAME This concrete class
  */
#define CONCRETE_STATE(NAME) \
public: \
	NAME() { _name=#NAME; }

/*!
*  Use this macro to define the constructor of your initial state.
*  It is given the new pimpl instance and it takes ownership of it.
*  This macro is technically only necessary if you use a pimpl, but it's good labelling.
*  The second constructor is intended for initializing nested state machines
*
*  @param NAME This concrete class
*/
#define INITIAL_STATE(NAME) \
public: \
	NAME(PimplType *newPimpl) \
		: NAME() \
	{ \
		pocket_fsm::internal::ASSERT(newPimpl, L"You need to pass a pimpl instance to the initial state!"); \
		_pimpl.reset(dynamic_cast<pocket_fsm::PimplBase*>(newPimpl)); \
	} \
	NAME(PimplSmartPtr pimpl) \
		: NAME() \
	{ \
		pocket_fsm::internal::ASSERT(pimpl.get(), L"You need to pass a pimpl instance to the initial state!"); \
		_pimpl = pimpl; \
	}

/*!
*  Use this macro to reliaby declare your react functions with the proper signature.
*  The event parameter is simply named e ans is a non const reference.
*  Call this macro in a concrete state.
*
*  @param EVENT The type of the parameter of the react function
*/
#define REACT(EVENT) \
	virtual void react(EVENT &e)

/*!
*  Use this macro in a nested state machine to properly set up the event forwarding.
*  This macro needs to be used for all events handled by the nested state machine
*
*  @param EVENT The type of the parameter of the react function
*/
#define NESTED_REACT(EVENT) \
	virtual void react(EVENT &e) override \
	{ \
		sendEvent(e); \
	}

/*!
	*  This namespace includes all things to be obfuscated from users of the header and only relate to the inner workings of pocket_fsm
	*/
namespace internal
{
/*!
 *  Cross platform assert used for DEBUG runtime check
 *
 *      @param [in] expr An expression that has to be true.
 *      @param [in] msg  The message to show when expr is false
 */
constexpr void ASSERT(bool expr, const wchar_t *msg) {
#if defined(WIN32)
	_ASSERT_EXPR(expr, msg);
#elif defined (UNIX)
	assert(expr && msg);
#endif
}

/*!
 *  OnEntry and OnExit are internal events that are only raised by the State Machine itself.
 */
struct OnEntry { };
struct OnExit { };
}

/*!
 *  A pimpl needs to derive from this class to expose a virtual destructor to the smart pointer
 */
class PimplBase
{
public:
	virtual ~PimplBase() {};
};

/*!
 * The State interface is the base class for any state object. It holds transition logic and
 *  pure virtual reaction function to internal events.
 *  Derive your base class from this if you do not need a pimpl.
 */
class StateIF
{
public:
	/*!
	 *  Useful typedefs for internal events
	 */
	using OnEntry = pocket_fsm::internal::OnEntry;
	using OnExit = pocket_fsm::internal::OnExit;

	/*!
	 *  Constructor. All states should be created clean : no copying allowed!
	 */
	StateIF() = default;
	StateIF(StateIF &s) = delete;

	/*!
	 *  Destructor. Perform transition function during the state handoff.
	 */
	virtual ~StateIF()
	{
		if (_onTransition)
		{
			_onTransition();
		}
	}

	/*!
	 *  These functions are run once when the state becomes active
	 *  and the other once as well when the state becomes inactive
	 *
	 *      @param [in,out] e The internal events are empty
	 */
	REACT(OnEntry) = 0;
	REACT(OnExit) = 0;

	/*!
	 *  Returns the state's next state.
	 *  The next state is registered by a call to changeState<NextState>()
	 *
	 *      @return The next state.
	 */
	inline StateIF *getNextState(bool handoff = false)
	{
		if (handoff)
		{
			StateIF* backup = _nextState;
			_nextState = nullptr;
			return backup;
		}
		return _nextState;
	}

	/*!
	 *  Stringified name of the concrete class
	 */
	const char *_name = nullptr;

protected:
	/*!
	 *  Beautifiers
	 */
	using PimplType = void;
	using TransitionFunc = std::function<void()>; 	// Signature for a function to be used during transition

	/*!
	 *  The next state registered for transition
	 */
	StateIF *_nextState = nullptr;

	/*!
	 *  Function to be run on transition (i.e. between the exit and entry calls)
	 */
	TransitionFunc _onTransition = nullptr;
};

/*!
 *  This variant of StateIF additionally holds a Pointer to IMPLementation,
 *  which is an object containing data and functions carried over across states.
 *  The pimpl object is handed off during transition and thus stays in memory
 *  until the state machine itself is deleted.
 *  The pimpl can be accessed under the proper type via the getter pimpl()
 *
 *  @tparam Pimpl The forward declared name of the implementation class deriving from PimplBase.
 */
template<typename Pimpl>
class StatePimplIF : public StateIF
{
public:
	/*!
	 *  Constructor. All states should be created clean : no copying allowed!W
	 */
	StatePimplIF() = default;
	StatePimplIF(StatePimplIF &s) = delete;

	/*!
	 *  Destructor. Call transition and hand over the pimpl to the next state
	 */
	virtual ~StatePimplIF()
	{
		// Transit needs to occur before handing off the pimpl
		if (_onTransition)
		{
			_onTransition();
			_onTransition = nullptr; // Base destructor will be called So clearing this after use is important.
		}

		if (_nextState)
		{
			// upcast is safe because of the assert in changeState method 
			// guarantees nextState to be of the same base class.
			static_cast<StatePimplIF<PimplType>*>(_nextState)->_pimpl = _pimpl;
		}
	}

protected:
	/*!
	 *  Beautifiers
	 */
	using PimplSmartPtr = std::shared_ptr<PimplBase>; // Pointer is only shared with nested state machine current states.
	using PimplType = Pimpl;

	/*!
	 *  Access the Implementation class as its proper type
	 *
	 *      @return
	 */
	inline PimplType * pimpl()
	{
		static_assert(std::is_base_of<PimplBase, Pimpl>::value, "The pimpl class needs to have pocket_fsm::PimplBase as a base");
		return static_cast<PimplType*>(_pimpl.get());
	}

	/*!
	 *  Pointer to implementation.
	 *  This object will contain all fields and output methods. It is unknown outside of the concrete states.
	 */
	PimplSmartPtr _pimpl = { nullptr };
};

/*!
 * The State Machine handles sending events to the current state and operates state transitions. Derive from this class
 *  with your base state class as parameters and set up a constructor that initializes the initial state.
 *      @tparam BASE The name of a base state of your state machine: it should derive from either StateIF or StatePimplIF
 *      and be derived by all concrete classes.
 */
template<class BASE>
class FiniteStateMachine
{
protected:
	static_assert(std::is_base_of<StateIF, BASE>::value, "The parameter of FiniteStateMachine needs to be a descendant of StateIF");
	/*!
	 *  Useful typedefs for internal events
	 */
	using OnEntry = internal::OnEntry;
	using OnExit = internal::OnExit;

public:
	/*!
	 *  Constructor.
	 */
	FiniteStateMachine() = default;

	/*!
	 *  Destructor. Call exit event before deletion.
	 */
	virtual ~FiniteStateMachine()
	{
		setCurrentState(nullptr); // Call exit on current state
	}

	/*!
	 *  Send an external event to the state machine.
	 *  You cannot call internal events such as OnEntry and OnExit externally!
	 *
	 *      @tparam E The type of the event the current state needs to react to.
	 *
	 *      @param [in,out] evt The user defined object the state machine will handle
	 *
	 *      @return the input parameter reference
	 */
	template<typename E>
	E &sendEvent(E &evt)
	{
		static_assert(!std::is_same<E, OnEntry>::value && !std::is_same<E, OnExit>::value, "Cannot send an internal event");
		internal::ASSERT(_currentState.get(), L"You did not call \"initialize(new MyInitialState(...));\" in your constructor!");
		lock();
		_currentState->react(evt);					// Call concrete state's react function
		while (_currentState->getNextState())
		{
			// This cast is safe because of the static assert at the top of this class
			setCurrentState(static_cast<BASE*>(_currentState->getNextState()));
		}
		unlock();
		return evt;
	}

	/*!
	 *  Returns the finite state machine's current state stringified name.
	 *
	 *      @return The current state name, or an empty string if uninitializeed
	 */
	inline const char * getCurrentStateName() const
	{
		return _currentState ? _currentState->_name : "";
	}

protected:
	/*!
	 *  Descendants call this in their constructor typically to set the initial state.
	 *  The state machine takes ownership of the pointer.
	 *  Can be called subsequently to reinitialize the state machine : any pimpl is destroyed
	 *
	 *      @param [in,out] initialState
	 */
	void initialize(BASE *newInitialState)
	{
		internal::ASSERT(newInitialState, L"Need to pass an initial state to the initialize function.");
		lock();
		// Reinitialize state machine with provided state
		// The pimpl is not handed off because _currentState->_nextState is nullptr at this point
		setCurrentState(newInitialState);   
		while (_currentState->getNextState()) // Entry usually doesn't changeState, but it can.
		{
			// This upcast is safe because of the static assert in this class
			setCurrentState(static_cast<BASE*>(_currentState->getNextState()));
		}
		unlock();
	}

	/*!
	 *  Sets the finite state machine's current state.
	 *  Also perform the state transition and call internal events
	 *
	 *      @param [in,out] nextState Next state to set.
	 */
	void setCurrentState(BASE *nextState)
	{
		if (_currentState)
		{
			OnExit exit;
			_currentState->react(exit);
		}

		if (nextState)
		{
			_currentState.reset(nextState); // old state destructor calls onTransition, hands off pimpl and gets deleted
			OnEntry entry;
			_currentState->react(entry);
		}
		else
		{
			_currentState.reset();
		}
	}

	/*!
	 *  Returns the finite state machine's current state in read only
	 *
	 *      @return The current state pointer
	 */
	inline const BASE* getCurrentState() const
	{
		return _currentState.get();
	}

	/*!
	 * Override these functions using your favorite lock mechanism
	 *  to secure State Machine usage across threads
	 */
	virtual void lock() {};
	virtual void unlock() {};

	/*!
	 *  The current state of the state machine.
	 *  Shared instead of unique to enable copy ctor
	 *  but instance should not get widespread
	 */
	std::shared_ptr<BASE> _currentState = nullptr;
};

/*!
 * A State Machine nested inside a concrete state of another state machine.
 * This class enables hierarchical state machines. To use it properly, you need
 * a class representing the base for your nested state type, deriving from a 
 * core base class. Since you will be reacting to a subset or all of the same events,
 * you have to use the macro NESTED_REACT to implement the reaction forwarding to
 * the nested state. The state machine should be initialized OnEntry.
 * 
 * @tparam BASE_NEST_STATE : Base state type of the nested states
 * @tparam BASE_CORE_STATE : Base state of the parent of BASE_NEST_STATE
 * @tparam BASE_ROOT_STATE : Highest level base state, declaring all react overloads
 */
template<class BASE_NEST_STATE, class BASE_CORE_STATE, class BASE_ROOT_STATE = BASE_CORE_STATE>
class NestedStateMachine : public BASE_CORE_STATE, protected FiniteStateMachine<BASE_ROOT_STATE>
{
	static_assert(std::is_base_of<BASE_CORE_STATE, BASE_NEST_STATE>::value, "The first parameter of NestedStateMachine needs to be a descendant of the second parameter");
	static_assert(std::is_base_of<BASE_ROOT_STATE, BASE_NEST_STATE>::value, "The first parameter of NestedStateMachine needs to be a descendant of the third parameter");

protected:
	// Beautifiers
	using OnEntry = pocket_fsm::internal::OnEntry;
	using OnExit = pocket_fsm::internal::OnExit;
	using FSM = FiniteStateMachine<BASE_ROOT_STATE>;

public:
	/*!
	 *  Send an external event to the nested state machine.
	 *  You cannot call internal events such as OnEntry and OnExit externally!
	 *  The nested states can call a transit to a core state or high level
	 *  in which case the next state is adoped by this very state.
	 *
	 *      @tparam E The type of the event the current state needs to react to.
	 *
	 *      @param [in,out] evt The user defined object the state machine will handle
	 *
	 *      @return the input parameter reference
	 */
	template<typename E>
	E &sendEvent(E &evt)
	{
		static_assert(!std::is_same<E, OnEntry>::value && !std::is_same<E, OnExit>::value, "Cannot send an internal event");
		internal::ASSERT(FSM::_currentState.get(), L"You did not call \"initialize(new MyInitialState(...));\" in your constructor!");
		FSM::lock();
		FSM::_currentState.get()->react(evt);					// Call concrete state's react function
		while (FSM::_currentState->getNextState())
		{
			if (auto nestType = dynamic_cast<BASE_NEST_STATE*>(FSM::_currentState->getNextState()))
			{
				// Change of nested state
				FSM::setCurrentState(nestType);
			}
			else // Next state is a concrete core state. We are exiting this nested state machine!
			{
				// Change of core state
				BASE_CORE_STATE::_nextState = FSM::_currentState->getNextState(true);
				break;
			}
		}
		FSM::unlock();
		return evt;
	}
};

} // End of namespace
