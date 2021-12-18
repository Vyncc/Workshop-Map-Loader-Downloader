#pragma once

#include <Windows.h>

#include<Xinput.h>
#pragma comment(lib, "Xinput.lib")

//https://lcmccauley.wordpress.com/2014/01/05/gamepad-input-tutorial/


class Gamepad
{
public:
    // Function prototypes
    //---------------------//

    // Constructors
    Gamepad();
    Gamepad(int a_iIndex);

    void Update(); // Update gamepad state

    // Thumbstick functions
     // - Return true if stick is inside deadzone, false if outside
    bool LStick_InDeadzone();
    bool RStick_InDeadzone();

    float LeftStick_X();  // Return X axis of left stick
    float LeftStick_Y();  // Return Y axis of left stick
    float RightStick_X(); // Return X axis of right stick
    float RightStick_Y(); // Return Y axis of right stick

    // Trigger functions
    float LeftTrigger();  // Return value of left trigger
    float RightTrigger(); // Return value of right trigger

    // Utility functions
    XINPUT_STATE GetState(); // Return gamepad state
    int GetIndex();          // Return gamepad index
    bool Connected();        // Return true if gamepad is connected
    bool checkButtonPress(WORD button);

private:
    // Member variables
    //---------------------//

    XINPUT_STATE m_State; // Current gamepad state
    int m_iGamepadIndex;  // Gamepad index (eg. 1,2,3,4)
};
