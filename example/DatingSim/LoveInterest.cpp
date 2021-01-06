#include "LoveInterest.h"

class Indifferent : public EmotionalState
{
	CONCRETE_STATE(Indifferent)

	REACT(Compliment) override
	{
		printf("Thank you! That's very kind. :)\n");
		changeState<Curious1>();
	}

	REACT(Joke) override
	{
		printf("Hahahaha! :)\n");
		changeState<Curious2>();
	}
};

class Curious1 : public EmotionalState
{
	CONCRETE_STATE(Curious1)

	REACT(Compliment) override
	{
		printf("You are quite the smooth talker.\n");
	}

	REACT(Joke) override
	{
		printf("Hahahaha! :)\n");
		changeState<Open>();
	}
};

class Curious2 : public EmotionalState
{
	CONCRETE_STATE(Curious2)

	REACT(Compliment) override
	{
		printf("Thank you! That's very kind. :)\n");
		changeState<Open>();
	}

	REACT(Joke) override
	{
		printf("You are quite the joker.\n");
	}
};

class Open : public EmotionalState
{
	CONCRETE_STATE(Open)

	REACT(Compliment) override
	{
		printf("Ah yeah, you told me that already. :)\n");
		changeState<Curious1>();
	}

	REACT(Joke) override
	{
		printf("Was funny the first time. :)\n");
		changeState<Curious2>();
	}

	REACT(Gift) override
	{
		printf("You are quite the gentleman! :D\n");
		changeState<Delighted>();
	}
};

class Delighted : public EmotionalState
{
	CONCRETE_STATE(Delighted)

	REACT(Compliment) override
	{
		printf("Aw thanks. I gotta go..\n");
		changeState<Vexed>();
	}

	REACT(Flirt) override
	{
		printf("I'll give you my number.\n");
		changeState<NumberAcquired>();
	}

	REACT(Joke) override
	{
		printf("ha haha... Hold on a minute, my boyfriend is calling.\n");
		changeState<Vexed>();
	}

	REACT(Gift)override
	{
		printf("Stop. You're spoiling me.\n");
		changeState<Indifferent>();
	}
};

class Vexed : public EmotionalState
{
	CONCRETE_STATE(Vexed)

	REACT(Compliment) override {}
	REACT(Joke) override {}
};

class NumberAcquired : public EmotionalState
{
	CONCRETE_STATE(NumberAcquired)

	REACT(Compliment) override
	{
		printf("Call me. ;-)\n");
	};

	REACT(Joke) override
	{
		printf("Call me. ;-)\n");
	};

	REACT(Gift) override
	{
		printf("Call me. ;-)\n");
	};

	REACT(Flirt) override
	{
		printf("Call me. ;-)\n");
	};
};

void EmotionalState::react(Insult &e)
{
	printf("How rude. Nevermind.\n");
	changeState<Vexed>();
}

void EmotionalState::react(Flirt &e)
{
	printf("I'm sorry, I don't feel like talking right now.\n");
	changeState<Vexed>();
};

void EmotionalState::react(Gift &e)
{
	printf("No thanks\n");
}

LoveInterest::LoveInterest()
{
	initialize(new Indifferent());
}
