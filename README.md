# Pocket FSM

Pocket FSM is a single header lightweight and high performance Finite State Machine Framework. It has minimal dependencies so that it can be used in any system. I believe that FSMs are an effective way to code any kind of software objects that can have states, sequences or simple message handling, and this framework makes it quicker and easier to code those software objects.

## What is a Finite State Machine?

A finite state machine is a system that responds to different inputs in a sequential manner. In other words, the inputs that come in provoke a change of the internal state of the machine, which will lead it to respond to inputs differently. As such, technically speaking, nearly all software controller objects are state machines. Nevertheless, not all objects would be better implemented as state machine ; but as the complexity grows, it becomes more likely that a state machine would be more appropriate.

## Can't I just use a switch-case statement and a state enum?

This is probably the fastest way to implement a state machine. What more? It also leaves the code very easy to read and understand. It may be the best option for the simplest cases. However, it comes with a few tradeoffs. First is that you are likely to get some code repetition, especially on transitions and on entry and exit. Second is that it creates high dependencies among objects that may not or should not depend on each other. Finally, the switch statement becomes way too big way too quick, or there's too many of them, so it is unusable for anything more complex.

## So what does Pocket FSM do? Why should I use it?

Pocket FSM provides an easy and intuitive way to code a complex state machine from a state diagram document and produce good and reliable code that minmizes dependencies, has minimal memory overhead and great performance. It is self documenting and abstracts away the inner workings of the state machine. It is easy to document and share with team members that may not care about how you implemented the machine, as long as it fullfills the state machine diagram.

* The State Machine only ever holds a single state object in it and none others will exist until a change of state is requested.
* Each state has it's own reaction to events but this is completely unknown to the user of the FSM thanks to polymorphism.
* On a state change, the next state object is created and the previous one is deleted : which is a transaction of 64 bytes at most.
* Pocket FSM uses the pImpl pattern to easily handoff an underlying object containing the logic and data that the state machine is controlling to the next state object without making a deep copy, or exposition outside of the state machine source file.
* All memory management is taken care of by smart pointers.
* Pocket FSM also enables easy use of RAII pattern by providing and guaranteing a single onEntry and onExit event call for each state.
* Macros are used to provide quick, consistent and descriptive coding of the custom states
* Compile time and debug-only runtime asserts make sure that appropriate error messages are displayed when a misuse of the framework occurs.
* Stringified concrete state names provide easy logging
* Built-in decoupling sets up for a proper Model-View-Controller triad.

Here's a  depiction of the separation done of the interface and the implementation.
![alt text](https://github.com/Electronicks/Pocket_FSM/tree/master/doc/Decoupling_Plan.jpg "Decoupling Diagram")

On the left you can see the user of the state machine dealing with various structures, containing various fields if any, encompassing all of the state machine inputs. Those are sent to the state machine via the singular sendEvent function. Internally, the call is routed to the proper handler thanks to polymorphism and overloading. This layer can is where things move, objects are created and deleted and moved about as the state of the machine changes after each call. Finally, the rightmost layer contains the data and logic basic state machine 


## Why not use any of the other FSM frameworks

I haven't been able to find one that gave the developper enough simplicity and flexibility at the same time. Some require either having an instance of all states present at all times, or have a strict transition table that is hard to implement and use properly outside of the simplest cases. I just decided to do my own, trying to resolve the issues I've experienced with the existing ones.

## Are there any requirements?

Pocket FSM makes use of C++11 features such as function objects and smart pointers from the STL, and thus requires at least this language level. There are no other dependencies.

## What's in the header

All the content of the header is in the pocket_fsm namespace exept the macros which are always global. You will find the following in the Pocket FSM header:
* A macro ASSERT_X_PLAT in order to perform asserts in debug configuration. This will inform the developer of any misuse of the framework and it has zero impact in release.
* A macro REACT that defines a uniform signature for all react functions
* A functor declaration PimplDeleter, which is to be defined with a macro provided at the end of the file
* A class StateIF to be templated with the forward declaration of your pimpl. It is pure virtual so you need to create your own base state class. There's also a specializer for a version without pimpl.
* A class FiniteStateMachine, templated with your base state class. The state machine you will be using will be a child of this class.
* A set of macros to use in your custom state classes. The macros make use of the stringify feature and help with creating consistent signatures and constructors for your classes. If a macro hinders what you are trying to do, feel free to write its expansion. Take note that all *_STATE() macros make the class member visibility public thereafter.

## How do I use Pocket FSM?

Let's show the step by the use of a simple state machine example: a combination safe. The safe requires you to configure a new combination every time you lock it, and entering a wrong combination puts the safe in lockdown. Once it lockdown, you need a competent authority to reset the safe before trying the lock again. We can start by laying down the basics of our state machine in our header CombinationSafe.h

```c++
// File: CombinationSafe.h
// Author: Electronicks
// Date: January 18th 2021

#pragma once // Header sentinel

// Include the framework to use it.
#include "pocket_fsm.h"
... // Include some more headers as you need them

// I will need a pimpl to hold the combination and possibly more
struct SafeImpl;

// State machine inputs
struct Configure { std::forward_list<int> combination; }; // Our combination is a sequence of integer numbers
struct Number { int digit;  }; // Entering a number is just passing a digit
struct Reset {}; // Reset requires no parameter passing

// Concrete states
class Open; 		// The safe is open and waiting for a configuration
class Locked;		// The safe is locked and processing digits from the user
class Lockdown;		// The safe is in lockdown and requires a reset to use again.
```

At this point we have none of the links between the different pieces, but that's okay because we're in the header, we just do declarations here. Declaring the concrete states is optional in the header, it could be done in the source file instead, but it may be relevant to the user of the state machine since the state names are stringified, and it may be pertinent for that person to know what those values could be. At this point we haven't even used the framework! but this is about to change. Let's declare the base state for our safe, which will be cleverly called SafeState.

```c++
class SafeState : public pocket_fsm::StatePimplIF<SafeImpl>
{
	BASE_STATE(SafeState)
	
	// Override interface / unused
	REACT(OnEntry) override {}
	REACT(OnExit) override {}

	// Default reactions => are unhandled
	REACT(Configure) {}
	REACT(Number) {}
	REACT(Reset) {}
};
```

Oh no, macros! Don't be spooked by those macros: they serve as both a consistent way to declare your states and a way to label your classes with their explicit purpose. The BASE_STATE macro is very important as it declares an  templated function key to the machine, which is the changeState<>() function. Visibility is set to public after the macro so you don't have to think about it. The REACT macros are not really necessary, but simply provides a consistent signature to your react functions. The base class has two internal react functions that require overriding, OnEntry and OnExit events. You then add a react functions for your own inputs you defined earlier. All the react functions can be left pure virtual, defined on the spot, defined empty like in our case, or even labelled final. It's all up to your needs. Take note it's important for the base state to not hold any data members: if you need data to be passed from one state to the next, the pimpl will hold those fields.

This looks like a simple enough class. Now we need the machine itself that will be jugling the different states.

```c++
class CombinationLock : public pocket_fsm::FiniteStateMachine<SafeState>
{
public:
	CombinationLock();
};

// End of CombinationLock.h
```

Well, that is deceptively simple. Is that really it? Well, there are optional lock() and unlock() function that can be overriden to implement a mutex if you plan on sending events from different threads. Also, the machine constructor will be constructing the pimpl, so if it needs any parameters at construction it should be passed here, but we may not know what those are yet. Otherwise, yeah that's it! Of course you can add to it any method and field might seem pertinent, but remember that iteraction with the pimpl should only pass through event reactions.

So that's our header file. At this point it can be shared with coworkers that may desire to start coding it's usage since they have the full interface to the state machine. For us, we need to start coding the implementation itself. So let's start writing our complementary source file. First step here is to define our pimpl that was forward declared in the header. Obviously the implementation can be done in different ways, so the way I choose here is primarily for the purpose of demonstration more than anything else.

```c++
// File: CombinationSafe.cpp
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

	... // Add Helper functions as you need them.
};

PIMPL_DELETER_DEF(SafeImpl);
```

In my implementation I desire to hold the combination and a pointer that will be travelling down as numbers are being entered. I will also need an error flag to indicate if a number was entered wrong at any time since I don't plan on being able to travel backwards, and I don't change state until I have received as many numbers as the combination holds. Typically this should be a different locked state, but I desire to hide this from the user of the CombinationSafe. I also need to define a deleter for the pimpl because of the way the memory management is handled with an opaque pointer. Now onto the fun part defining my actual states and reactions. If I didn't forward declare my concrete classes in the header I would do it here since I need it for changing state.

```c++
class Open : public SafeState
{
	CONCRETE_STATE(Open)

	INITIAL_STATE(Open)

	REACT(Configure) override
	{
		if (!e.combination.empty())
		{
			_pimpl->_combination = e.combination;
			_p = _combination.cbegin();	// Reset function?!
			_error = false;
			changeState<Locked>();
		}
	}
};
```

This reads rather easily doesn't it? Open is a concrete state, what more? the initial state, and it reacts to a configuration by adopting the configuration and changing to a locked state! Let's talk a little more about the macros: both CONCRETE_STATE and INITIAL_STATE define constructors, the first with no arguments and the second with a pimpl pointer as parameter. Once again visibility is handled by the macros so you don't have to bother with it. The REACT macro shows up again, and it is still optional but handy for a consistent signature. Just remember the event parameter it called e and is non-const reference. Changing state is as easy as invoking the templated function. This state is not concerned with other kind of events, so the default reaction will suffice. On to the other states.

```c++
class Locked : public SafeState
{
	CONCRETE_STATE(Locked)

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
		_pimpl->Reset();
	}
};

class Lockdown : public SafeState
{
	CONCRETE_STATE(Lockdown)

	REACT(Reset) override
	{
		_pimpl->Reset();
		changeState<Locked>();
	}
};
```

And here's our main processing. The Reset function was added as I was performing the same operation multiple times. Processing the number could arguably be a function as well, to move the responsibility in the pimpl. We can see that the safe stays in the locked state until enough numbers are entered. Reset enables the user to start over or clear the lockdown. It feels like we should be done, but let's not forget we need the constructor of our CombinationSafe to kick things off!

``` c++
CombinationSafe::CombinationSafe()
{
	initialize(new Open(new SafeImpl()));
}
```

The constructor creates the pimpl instance and the initial state. The function initialize is part of the base class and is required to use the state machine. The initial state will react to an entry event here to honor the RAII pattern. As mentioned previously, any parameters the pimpl needs for construction can be passed along right here. So now that the state machine is coded, how do you use it? Let's see what our coworker Jimmy was working on since we gave him our header.

```c++
// File: main.cpp
// Author: Jimmy
// Date: January 18th 2021

#include "CombinationSafe.h" // Thanks for the header Electronicks

int main()
{
	CombinationSafe safe;
	while (true) // type q to quit
	{
		std::cout << std::endl << "The safe is currently currently " << safe.getCurrentStateName() << std::endl <<
			"What would you like to do?" << std::endl <<
			"1. Configure" << std::endl <<
			"2. Enter a number" << std::endl <<
			"3. Reset" << std::endl <<
			"q. Quit" << std::endl;

		char choice = 0;
		choice = std::cin.get();
		while (std::cin.get() != '\n');

		switch (choice)
		{
		case '1':
		{
			Configure configure;
			std::cout << "Enter your combination of integers, separated by a whitespaces:" << std::endl;
			... // Read, parse, push_back, rinse, repeat
			configure.combination.assign(newCombination.begin(), newCombination.end());
			safe.sendEvent(configure);
		} break;
		case '2':
		{
			Number digit;
			std::cout << "Enter an integer: " << std::endl;
			... // Read and parse
			safe.sendEvent(digit);
		}	break;
		case '3':
		{
			Reset reset;
			safe.sendEvent(reset);
		}	break;
		case 'q':
			std::cout << "Thanks for playing" << std::endl;
			return 0;
		}
	}
}
```

Using the safe seems simple enough. After creation you can just start sending events to it, filling up those event structures with the proper data. Those structures can also contain return values such as error codes, or maybe inout parameters such as other objects. If you want to log what state you are in, you have a handy function to provide you the stringified name of the state. Beyond that you're just feeding events to the machine who will process them in whatever state it happens to be in.

You can find a runnable version of this example, as well as other examples showcasing other features of the framework in the example VS solution provided.

Finally here's a bullet point resume of what you need to do.

If you desire to make your object a finite state machine, you will need the following.
0. It is highly recommended to start with a state machine diagram, listing the number and names of states, what stimuli the system responds to and what actions the system takes in each state, on transitions, on entry and on exit.
1. Start by creating a header file, importing the Pocket FSM header. The header file must contain the following:
    1. The forward declaration of the **Implementation Class** if you need one and optionally all the concrete states. The states have their name stringified, so the header is a good reference of what those strings can be.
    2. A definition for all the input **Events** as structures. They may contain fields.
    3. A declaration of a **Base State** class inheriting from eith StatePimplIF parameterized with your implementation class or StateIF and using the BASE_STATE macro. It also declares a react function for each event you defined earlier using the REACT macro plus the internal OnEntry and OnExit events.
	4. Declaration of **your state machine** class itself inheriting from FiniteStateMachine parameterized with your base state.
2. Create the cpp file and define the following:
    1. Give a full definition to your implementation class if you have one. It should have a function for each output action of the state machine.
    2. Define the deleter of your implementation class using the macro if you have one.
    3. If you haven't forward declared your concrete classes in the header you need to do it here. This is required to change to a state not declared yet.
    4. Then define all your states using the CONCRETE_STATE() macro and overriding all necessary virtual functions. If you have an implementation class, use the Initial state constructor macro in the state you desire your state machine to start in.
    5. Define your top level state machine constructor, calling the parent's initialize() with an **new** instance of the initial state and a **new** instance of the implementation class. Deletion is handled by the State Machine

To change the state of the machine, simply call changeState\<NewState\>() and after returning from the react function the changing sequence will occur, calling the relevent onExit and onEntry functions.

You can also have a function to be run during the transition, after onExit but before onEntry.

It's easy to use lambdas or bind non-static methods to the transition thanks to the use of standard function objects.
