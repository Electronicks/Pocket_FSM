// SimplestStateMachine.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "LoveInterest.h"

#define STR(state) #state

Compliment compliment;
Joke joke;
Gift gift;
Flirt flirt;
Insult insult;

int main()
{
	LoveInterest li;
	while (li.getCurrentStateName() != STR(NumberAcquired))
	{
		std::cout << "The love interest is currently " << li.getCurrentStateName() << std::endl <<
			"What would you like to do?" << std::endl <<
			"1. Compliment" << std::endl <<
			"2. Joke" << std::endl <<
			"3. Gift" << std::endl <<
			"4. Flirt" << std::endl <<
			"5. Insult" << std::endl <<
			"q. Quit" << std::endl;

		char choice = 0;
		scanf_s("%c", &choice, 1);
		switch (choice)
		{
		case '1':
			li.sendEvent(compliment);
			break;
		case '2':
			li.sendEvent(joke);
			break;
		case '3':
			li.sendEvent(gift);
			break;
		case '4':
			li.sendEvent(flirt);
			break;
		case '5':
			li.sendEvent(insult);
			break;
		case 'q':
			return 0;
		}
		do {
			scanf_s("%c", &choice, 1); // Read
		} while (choice != '\n');
	}
	std::cout << "Thanks for playing" << std::endl;
}
