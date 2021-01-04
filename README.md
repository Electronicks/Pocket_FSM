# Pocket FSM

Pocket FSM is a single header lightweight and high performance Finite State Machine Framework to be used in any system. I believe that FSMs are an effective way to code any kind of software objects that can have states, sequences or simple message handling, and this framework makes it quicker and easier to code in such a way.

## What is a Finite State Machine?

A finite state machine is a system that responds to input in a sequential manner. In other words, the inputs that come in provoke a change of the internal state of the machine, which will lead it to respond to inputs differently to the same input as its state changes. As such, technically speaking, all software objects that holds some fields and has methods that contains a condition on those fields are state machines. Nevertheless, not all objects have the need of using this framework, but it should help with those more complicated classes.

## Why not just use a switch case statement and a state enum?

This is probably the simplest, easiest and fastest way to implement a state machine. What more? It also leaves the code very easy to read and understand. It may be the best option for the simplest cases. However, it comes with a few tradeoffs. First is that you are likely to get some code repetition, especially on transitions and on entry and exit. Second is that it creates high dependencies among objects that may not or should not depend on each other.

## So what does Pocket FSM do? Why should I use it?

Using Pocket FSM, you can easily code a complex state machine from a state diagram document and produce good and reliable code that minmizes dependencies, has minimal memory overhead and good performance. It is easy to document and share with team members that may not care about how you implemented the machine, as long as it fullfills the state machine diagram.

## How does Pocket FSM does it?

Pocket FSM uses typical Object-Oriented features and patterns to offer a powerful and effective framework. The State Machine only ever holds a single state object in it. Each state has it's own reaction to events thanks to polymorphism, but this is completely unknown to the user of the FSM. On a state change, the next state object is created and the previous one is deleted : the new state is as low as 104 bytes big. Pocket FSM uses the pImpl pattern to easily handoff the underlying object containing the logic and data the state machine is controlling to the next state object without making a deep copy. All memory management is taken care of by unique pointers, you only need to provide a Deleter static function for the pimpl. Pocket FSM also enables easy use of RAII by providing and guaranteing a single onEntry and onExit function call for each state.

## How do I use Pocket FSM?

If you desire to make your object a finite state machine, you will need the following.
0. It is highly recommended to start with a state machine diagram, listing the number and names of states, what events the system responds to and what actions the system takes in each state.
1. Start by creating a header file, importing the framework header and forward declare the implementation class.
2. Define all system input events as structure. They may contain fields.
3. Define a base state class inheriting from StateIF parameterized with your implementation class and using the BaseState macro. Also declare a react function for each event you defined earlier.
4. Finally, declare the top level class inheriting from FiniteStateMachine parameterized with your base state.
5. Create the cpp file. Give a definition to your implementation class.
6. Forward declare all your concrete states. Then you can define them using the concrete state macro and overriding all pure virtual functions.
7. Define your top level state machine constructor, initializing with the initial state and creating the implementation instance.

To change the state of the machine, your a react function simply needs to call changeState\<NewState\>() and after returning the changing sequence will occur, calling the relevent onExit and onEntry functions.

You can also have a function to be run during the transition, after onExit but before onEntry.

It's easy to use lambdas or bind non-static methods to the transition thanks to the use of standard function pointer.