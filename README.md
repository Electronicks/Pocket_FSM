# Pocket FSM

Pocket FSM is a single header lightweight and high performance Finite State Machine Framework to be used in any system. I believe that FSMs are an effective way to code any kind of software objects that can have states, sequences or simple message handling, and this framework makes it quicker and easier to code in such a way.

## What is a Finite State Machine?

A finite state machine is a system that responds to input in a sequential manner. In other words, the inputs that come in provoke a change of the internal state of the machine, which will lead it to respond to inputs differently to the same input as its state changes. As such, technically speaking, all software objects that holds some fields and has methods that contains a condition on those fields are state machines. Nevertheless, not all objects have the need of using this framework, but it should help with those more complicated classes.

## Why not just use a switch-case statement and a state enum?

This is probably the simplest, easiest and fastest way to implement a state machine. What more? It also leaves the code very easy to read and understand. It may be the best option for the simplest cases. However, it comes with a few tradeoffs. First is that you are likely to get some code repetition, especially on transitions and on entry and exit. Second is that it creates high dependencies among objects that may not or should not depend on each other. Finally, the switch statement becomes way too big way too quick, so it is unusable for anything slightly complex.

## So what does Pocket FSM do? Why should I use it?

Using Pocket FSM, you can easily code a complex state machine from a state diagram document and produce good and reliable code that minmizes dependencies, has minimal memory overhead and good performance. It is easy to document and share with team members that may not care about how you implemented the machine, as long as it fullfills the state machine diagram.

Pocket FSM uses typical Object-Oriented features and patterns to offer a powerful and effective framework. The State Machine only ever holds a single state object in it and none others will exist until a change of state is requested. Each state has it's own reaction to events but this is completely unknown to the user of the FSM thanks to polymorphism. On a state change, the next state object is created and the previous one is deleted : which is a transaction of 104 bytes only. Pocket FSM uses the pImpl pattern to easily handoff the underlying object containing the logic and data that the state machine is controlling to the next state object without making a deep copy. All memory management is taken care of by unique pointers, so you only need to provide a Deleter static function for the pimpl. Pocket FSM also enables easy use of RAII pattern by providing and guaranteing a single onEntry and onExit function call for each state.

## Why not use any of the other FSM frameworks

I haven't been able to find one that gave the developper enough flexibility and simplicity at the same time. Some require either having an instance of all states present at all times, or have a strict transition table that is hard to implement and use properly outside of the simplest cases. I just decided to do my own, trying to resolve the issues I've experienced with the existing ones.

## Are there any requirements?

Pocket FSM makes use of C++11 features such as lamdas and smart pointers, and thus requires at least this language level. There are no other dependencies.

## What's in the header

All the content of the header is in the pocket_fsm namespace exept the macros which are always global. You will find the following in the Pocket FSM header:
* A macro ASSERT_X_PLAT in order to perform asserts in debug configuration. This protects the code from misuse by the developer and it has zero impact in release.
* A class StateIF to be templated with the forward declaration of your pimpl. It is pure virtual so you need to create your own base state class.
* A class FiniteStateMachine, also templated with your custom base class. The state machine you will be using will be a child of this class.
* A set of macros to use in your custom state classes. The macros make use of the stringify feature and help with creating consistent signatures and constructors for your classes. If a macro hinders what youa re trying to do, feel free to write it out. Take note that all *_STATE() macros make the class member visibility public thereafter.

## How do I use Pocket FSM?

If you desire to make your object a finite state machine, you will need the following.
0. It is highly recommended to start with a state machine diagram, listing the number and names of states, what stimuli the system responds to and what actions the system takes in each state, on transitions, on entry and on exit.
1. Start by creating a header file, importing the Pocket FSM header. The header file must contain the following:
    a. The forward declaration of the **Implementation Class** and optionally all the concrete states. The states have their name stringified, so the header is a good reference of what those strings can be.
    b. A definition for all the input **Events** as structures. They may contain fields.
    c. A declaration of a **Base State** class inheriting from StateIF parameterized with your implementation class and using the BASE_STATE macro. It also declares a react function for each event you defined earlier using the REACT macro.
	d. Declaration of **your state machine** class itself inheriting from FiniteStateMachine parameterized with your base state.
2. Create the cpp file and define the following:
    a. Give a full definition to your implementation class. It should have a function for each output action of the state machine.
    b. If you haven't forward declared your concrete classes in the header you need to do it here. This is required to change to a state not declared yet.
    c. Then define all your states using the CONCRETE_STATE() macro and overriding all necessary virtual functions.
    d. Define your top level state machine constructor, calling the parent's initialize() with an **new** instance of the initial state and a **new** instance of the implementation class. Deletion is handled by the State Machine

To change the state of the machine, your a react function simply needs to call changeState\<NewState\>() and after returning the changing sequence will occur, calling the relevent onExit and onEntry functions.

You can also have a function to be run during the transition, after onExit but before onEntry.

It's easy to use lambdas or bind non-static methods to the transition thanks to the use of standard function pointer.
