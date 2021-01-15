// CombinationSafe.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "CombinationSafe.h"
#include <sstream>
#include <list>

int main()
{
	CombinationSafe lock;
	while (true)
	{
		std::cout << std::endl << "The lock is currently currently " << lock.getCurrentStateName() << std::endl <<
			"What would you like to do?" << std::endl <<
			"1. Configure" << std::endl <<
			"2. Enter a number" << std::endl <<
			"3. Reset" << std::endl <<
			"q. Quit" << std::endl;

		char choice = 0;
		choice = std::cin.get();// >> choice;
		
		while (std::cin.get() != '\n');

		switch (choice)
		{
		case '1':
		{
			int number;
			std::cout << "Enter your combination of integers, separated by a whitespaces:" << std::endl;
			std::string buffer;
			std::getline(std::cin, buffer);
			std::stringstream ss(buffer);
			std::list<int> newCombination;
			for (ss >> number; !ss.fail(); ss >> number)
			{
				newCombination.push_back(number);
			}
			Configure configure;
			configure.combination.assign(newCombination.begin(), newCombination.end());
			lock.sendEvent(configure);
		} break;
		case '2':
		{
			std::cout << "Enter an integer: " << std::endl;
			Number digit;
			std::cin >> digit.digit;
			while (std::cin.get() != '\n');
			lock.sendEvent(digit);
		}	break;
		case '3':
		{
			Reset reset;
			lock.sendEvent(reset);
		}	break;
		case 'q':
			return 0;
		}
	}
	std::cout << "Thanks for playing" << std::endl;
}
