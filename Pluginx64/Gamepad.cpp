#include "pch.h"
#include"Gamepad.h"

// Default constructor
Gamepad::Gamepad() {}

// Overloaded constructor
Gamepad::Gamepad(int a_iIndex)
{
    // Set gamepad index
    m_iGamepadIndex = a_iIndex - 1;
}

// Return gamepad state
XINPUT_STATE Gamepad::GetState()
{
    // Temporary XINPUT_STATE to return
    XINPUT_STATE GamepadState;

    // Zero memory
    ZeroMemory(&GamepadState, sizeof(XINPUT_STATE));

    // Get the state of the gamepad
    XInputGetState(m_iGamepadIndex, &GamepadState);

    // Return the gamepad state
    return GamepadState;
}

// Return gamepad index
int Gamepad::GetIndex()
{
    return m_iGamepadIndex;
}

// Return true if the gamepad is connected
bool Gamepad::Connected()
{
    // Zero memory
    ZeroMemory(&m_State, sizeof(XINPUT_STATE));

    // Get the state of the gamepad
    DWORD Result = XInputGetState(m_iGamepadIndex, &m_State);

    if (Result == ERROR_SUCCESS)
        return true;  // The gamepad is connected
    else
        return false; // The gamepad is not connected
}

// Update gamepad state
void Gamepad::Update()
{
    m_State = GetState(); // Obtain current gamepad state
}

bool Gamepad::checkButtonPress(WORD press) {
    return (m_State.Gamepad.wButtons & press) != 0;
}

// Deadzone check for Left Thumbstick
bool Gamepad::LStick_InDeadzone()
{
    // Obtain the X & Y axes of the stick
    short sX = m_State.Gamepad.sThumbLX;
    short sY = m_State.Gamepad.sThumbLY;

    // X axis is outside of deadzone
    if (sX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
        sX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        return false;

    // Y axis is outside of deadzone
    if (sY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
        sY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
        return false;

    // One (or both axes) axis is inside of deadzone
    return true;
}

// Deadzone check for Right Thumbstick
bool Gamepad::RStick_InDeadzone()
{
    // Obtain the X & Y axes of the stick
    short sX = m_State.Gamepad.sThumbRX;
    short sY = m_State.Gamepad.sThumbRY;

    // X axis is outside of deadzone
    if (sX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
        sX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        return false;

    // Y axis is outside of deadzone
    if (sY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
        sY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
        return false;

    // One (or both axes) axis is inside of deadzone
    return true;
}

// Return X axis of left stick
float Gamepad::LeftStick_X()
{
    // Obtain X axis of left stick
    short sX = m_State.Gamepad.sThumbLX;

    // Return axis value, converted to a float
    return (static_cast<float>(sX) / 32768.0f);
}

// Return Y axis of left stick
float Gamepad::LeftStick_Y()
{
    // Obtain Y axis of left stick
    short sY = m_State.Gamepad.sThumbLY;

    // Return axis value, converted to a float
    return (static_cast<float>(sY) / 32768.0f);
}

// Return X axis of right stick
float Gamepad::RightStick_X()
{
    // Obtain X axis of right stick
    short sX = m_State.Gamepad.sThumbRX;

    // Return axis value, converted to a float
    return (static_cast<float>(sX) / 32768.0f);
}

// Return Y axis of right stick
float Gamepad::RightStick_Y()
{
    // Obtain the Y axis of the left stick
    short sY = m_State.Gamepad.sThumbRY;

    // Return axis value, converted to a float
    return (static_cast<float>(sY) / 32768.0f);
}

// Return value of left trigger
float Gamepad::LeftTrigger()
{
    // Obtain value of left trigger
    BYTE Trigger = m_State.Gamepad.bLeftTrigger;

    if (Trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
        return Trigger / 255.0f;

    return 0.0f; // Trigger was not pressed
}

// Return value of right trigger
float Gamepad::RightTrigger()
{
    // Obtain value of right trigger
    BYTE Trigger = m_State.Gamepad.bRightTrigger;

    if (Trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
        return Trigger / 255.0f;

    return 0.0f; // Trigger was not pressed
}