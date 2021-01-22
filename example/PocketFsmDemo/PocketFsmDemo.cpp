#include "DigitalButton.h"
#include <string>

constexpr uint16_t VK_SPACE = 32;

PressEvent press{ VK_SPACE };
ReleaseEvent release;
GetKeyCode gkc;
ResetEvt reset;

using pocket_fsm::internal::ASSERT;

int main(void)
{
	DigitalButton buttonA("Button #1");
	ASSERT(buttonA.getState() == E_ButtonState::NoPress, L"Button initialized to the wrong state");

	buttonA.sendEvent<PressEvent>(press);
	ASSERT(buttonA.getState() == E_ButtonState::BtnPress, L"Button did not transition state");

	buttonA.sendEvent(press);
	ASSERT(buttonA.getState() == E_ButtonState::BtnPress, L"Button transitioned when it shouldn't");

	buttonA.sendEvent(gkc);
	ASSERT(gkc.keycode == VK_SPACE, L"Button did not capture the right keycode");

	buttonA.sendEvent(release);
	ASSERT(buttonA.getState() == E_ButtonState::NoPress, L"Button did not transition state");
	ASSERT(release.result, L"Event returned a false result");
	release.result = false;

	buttonA.sendEvent(release);
	ASSERT(buttonA.getState() == E_ButtonState::NoPress, L"Button transitioned when it shouldn't");
	ASSERT(release.result == false, L"Event returned a true result when it shouldn't");

	buttonA.sendEvent(gkc);
	ASSERT(gkc.keycode == UINT16_MAX, L"Button did not clear the keycode");

	buttonA.sendEvent(press);
	ASSERT(buttonA.getState() == E_ButtonState::BtnPress, L"Button did not transition state");
	buttonA.sendEvent(reset);
	ASSERT(buttonA.getState() == E_ButtonState::NoPress, L"Button did not transition state");
	ASSERT(gkc.keycode == UINT16_MAX, L"Button did not clear the keycode");

	return 0;
}