#include "DigitalButton.h"
#include <string>
#include <Windows.h>

//constexpr uint16_t VK_SPACE = 32;

PressEvent press{ VK_SPACE };
ReleaseEvent release;
GetKeyCode gkc;
ResetEvt reset;

void iteration()
{

	DigitalButton buttonA("Button #1");
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::NoPress, "Button initialized to the wrong state");

	buttonA.sendEvent<PressEvent>(press);
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::BtnPress, "Button did not transition state");

	buttonA.sendEvent(press);
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::BtnPress, "Button transitioned when it shouldn't");

	buttonA.sendEvent(gkc);
	ASSERT_X_PLAT(gkc.keycode == VK_SPACE, "Button did not capture the right keycode");

	buttonA.sendEvent(release);
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::NoPress, "Button did not transition state");
	ASSERT_X_PLAT(release.result, "Event returned a false result");
	release.result = false;

	buttonA.sendEvent(release);
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::NoPress, "Button transitioned when it shouldn't");
	ASSERT_X_PLAT(release.result == false, "Event returned a true result when it shouldn't");

	buttonA.sendEvent(gkc);
	ASSERT_X_PLAT(gkc.keycode == UINT16_MAX, "Button did not clear the keycode");

	buttonA.sendEvent(press);
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::BtnPress, "Button did not transition state");
	buttonA.sendEvent(reset);
	ASSERT_X_PLAT(buttonA.getState() == E_ButtonState::NoPress, "Button did not transition state");
	ASSERT_X_PLAT(gkc.keycode == UINT16_MAX, "Button did not clear the keycode");

}

int main(void)
{
	// Unit testing the button state machine
	iteration();
	//Sleep(5000);
	for (int i = 0; i < 0; ++i) {
		iteration();
	}
	//Sleep(5000);

	return 0;
}