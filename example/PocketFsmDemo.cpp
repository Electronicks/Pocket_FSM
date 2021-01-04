#include "DigitalButton.h"
#include <string>
#include <Windows.h>

int main(void)
{
	PressEvent press;
	press.keycode = VK_SPACE;
	ReleaseEvent release;
	GetKeyCode gkc;
	ResetEvt reset;
	
	// Unit testing the button state machine

	DigitalButton buttonA("Button #1");
	Sleep(10000);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::NoPress, "Button initialized to the wrong state");

	buttonA.sendEvent<PressEvent>(press);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::BtnPress, "Button did not transition state");

	buttonA.sendEvent(press);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::BtnPress, "Button transitioned when it shouldn't");

	buttonA.sendEvent(gkc);
	X_PLAT_ASSERT(gkc.keycode == VK_SPACE, "Button did not capture the right keycode");

	buttonA.sendEvent(release);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::NoPress, "Button did not transition state");
	X_PLAT_ASSERT(release.result, "Event returned a false result");
	release.result = false;

	buttonA.sendEvent(release);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::NoPress, "Button transitioned when it shouldn't");
	X_PLAT_ASSERT(release.result == false, "Event returned a true result when it shouldn't");

	buttonA.sendEvent(gkc);
	X_PLAT_ASSERT(gkc.keycode == UINT16_MAX, "Button did not clear the keycode");

	buttonA.sendEvent(press);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::BtnPress, "Button did not transition state");
	buttonA.sendEvent(reset);
	X_PLAT_ASSERT(buttonA.getState() == E_ButtonState::NoPress, "Button did not transition state");
	X_PLAT_ASSERT(gkc.keycode == UINT16_MAX, "Button did not clear the keycode");

	Sleep(10000);
	return 0;
}