#ifndef SWITCH_HID_H_
#define SWITCH_HID_H_

#include <stddef.h>
#include "HIDReportData.h"

// HID joystick report
const uint8_t JoystickReport[] = {
    HID_RI_USAGE_PAGE(8,1), /* Generic Desktop */
    HID_RI_USAGE(8,5), /* Joystick */
    HID_RI_COLLECTION(8,1), /* Application */
        // Buttons (2 bytes)
        HID_RI_LOGICAL_MINIMUM(8,0),
        HID_RI_LOGICAL_MAXIMUM(8,1),
        HID_RI_PHYSICAL_MINIMUM(8,0),
        HID_RI_PHYSICAL_MAXIMUM(8,1),
        // The Switch will allow us to expand the original HORI descriptors to a full 16 buttons.
        // The Switch will make use of 14 of those buttons.
        HID_RI_REPORT_SIZE(8,1),
        HID_RI_REPORT_COUNT(8,16),
        HID_RI_USAGE_PAGE(8,9),
        HID_RI_USAGE_MINIMUM(8,1),
        HID_RI_USAGE_MAXIMUM(8,16),
        HID_RI_INPUT(8,2),
        // HAT Switch (1 nibble)
        HID_RI_USAGE_PAGE(8,1),
        HID_RI_LOGICAL_MAXIMUM(8,7),
        HID_RI_PHYSICAL_MAXIMUM(16,315),
        HID_RI_REPORT_SIZE(8,4),
        HID_RI_REPORT_COUNT(8,1),
        HID_RI_UNIT(8,20),
        HID_RI_USAGE(8,57),
        HID_RI_INPUT(8,66),
        // There's an additional nibble here that's utilized as part of the Switch Pro Controller.
        // I believe this -might- be separate U/D/L/R bits on the Switch Pro Controller, as they're utilized as four button descriptors on the Switch Pro Controller.
        HID_RI_UNIT(8,0),
        HID_RI_REPORT_COUNT(8,1),
        HID_RI_INPUT(8,1),
        // Joystick (4 bytes)
        HID_RI_LOGICAL_MAXIMUM(16,255),
        HID_RI_PHYSICAL_MAXIMUM(16,255),
        HID_RI_USAGE(8,48),
        HID_RI_USAGE(8,49),
        HID_RI_USAGE(8,50),
        HID_RI_USAGE(8,53),
        HID_RI_REPORT_SIZE(8,8),
        HID_RI_REPORT_COUNT(8,4),
        HID_RI_INPUT(8,2),
        // ??? Vendor Specific (1 byte)
        // This byte requires additional investigation.
        HID_RI_USAGE_PAGE(16,65280),
        HID_RI_USAGE(8,32),
        HID_RI_REPORT_COUNT(8,1),
        HID_RI_INPUT(8,2),
        // Output (8 bytes)
        // Original observation of this suggests it to be a mirror of the inputs that we sent.
        // The Switch requires us to have these descriptors available.
        HID_RI_USAGE(16,9761),
        HID_RI_REPORT_COUNT(8,8),
        HID_RI_OUTPUT(8,2),
    HID_RI_END_COLLECTION(0),
};

typedef struct {
    uint16_t Button; // 16 buttons; see JoystickButtons_t for bit mapping
    uint8_t  HAT;    // HAT switch; one nibble w/ unused nibble
    uint8_t  LX;     // Left  Stick X
    uint8_t  LY;     // Left  Stick Y
    uint8_t  RX;     // Right Stick X
    uint8_t  RY;     // Right Stick Y
    uint8_t  VendorSpec;
} USB_JoystickReport_Input_t;

// The output is structured as a mirror of the input.
// This is based on initial observations of the Pokken Controller.
typedef struct {
    uint16_t Button; // 16 buttons; see JoystickButtons_t for bit mapping
    uint8_t  HAT;    // HAT switch; one nibble w/ unused nibble
    uint8_t  LX;     // Left  Stick X
    uint8_t  LY;     // Left  Stick Y
    uint8_t  RX;     // Right Stick X
    uint8_t  RY;     // Right Stick Y
} USB_JoystickReport_Output_t;

typedef enum {
    SWITCH_Y       = 0x01,
    SWITCH_B       = 0x02,
    SWITCH_A       = 0x04,
    SWITCH_X       = 0x08,
    SWITCH_L       = 0x10,
    SWITCH_R       = 0x20,
    SWITCH_ZL      = 0x40,
    SWITCH_ZR      = 0x80,
    SWITCH_MINUS   = 0x100,
    SWITCH_PLUS    = 0x200,
    SWITCH_LCLICK  = 0x400,
    SWITCH_RCLICK  = 0x800,
    SWITCH_HOME    = 0x1000,
    SWITCH_CAPTURE = 0x2000,
} JoystickButtons_t;

#define HAT_TOP          0x00
#define HAT_TOP_RIGHT    0x01
#define HAT_RIGHT        0x02
#define HAT_BOTTOM_RIGHT 0x03
#define HAT_BOTTOM       0x04
#define HAT_BOTTOM_LEFT  0x05
#define HAT_LEFT         0x06
#define HAT_TOP_LEFT     0x07
#define HAT_CENTER       0x08

#define STICK_MIN      0
#define STICK_CENTER 128
#define STICK_MAX    255

#endif // SWITCH_HID_H_