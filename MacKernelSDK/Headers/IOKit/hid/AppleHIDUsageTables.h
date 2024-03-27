/*
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#ifndef _APPLEHIDUSAGETABLES_H
#define _APPLEHIDUSAGETABLES_H

/* ******************************************************************************************
 * Apple HID Usage Tables
 *
 * The following constants are Apple Vendor specific usages
 * ****************************************************************************************** */


/* Usage Pages */
enum {
    kHIDPage_AppleVendor                        = 0xFF00,
    kHIDPage_AppleVendorKeyboard                = 0xFF01,
    kHIDPage_AppleVendorMouse                   = 0xFF02,
    kHIDPage_AppleVendorAccelerometer           = 0xFF03,
    kHIDPage_AppleVendorAmbientLightSensor      = 0xFF04,
    kHIDPage_AppleVendorTemperatureSensor       = 0xFF05,
    kHIDPage_AppleVendorHeadset                 = 0xFF07,
    kHIDPage_AppleVendorPowerSensor             = 0xFF08,
    kHIDPage_AppleVendorSmartCover              = 0xFF09,
    kHIDPage_AppleVendorPlatinum                = 0xFF0A,
    kHIDPage_AppleVendorHeartRate               = 0xFF0A,
    kHIDPage_AppleVendorLisa                    = 0xFF0B,
    kHIDPage_AppleVendorMotion                  = 0xFF0C,
    kHIDPage_AppleVendorBattery                 = 0xFF0D,
    kHIDPage_AppleVendorIRRemote                = 0xFF0E,
    kHIDPage_AppleVendorDebug                   = 0xFF0F,
    kHIDPage_AppleVendorIRInterface             = 0xFF10,
    kHIDPage_AppleVendorDFR                     = 0xFF11,
    kHIDPage_AppleVendorDFRBrightness           = 0xFF12,
    kHIDPage_AppleVendorSenderID                = 0xFF13,
    kHIDPage_AppleVendorFilteredEvent           = 0xFF50,
    kHIDPage_AppleVendorMultitouch              = 0xFF60,
    kHIDPage_AppleVendorDisplay                 = 0xFF92,
    kHIDPage_AppleVendorTopCase                 = 0x00FF
};


/* AppleVendor Page (0xff00) */
enum {
    kHIDUsage_AppleVendor_TopCase               = 0x0001, /* Application Collection */
    kHIDUsage_AppleVendor_Display               = 0x0002, /* Application Collection */
    kHIDUsage_AppleVendor_Accelerometer         = 0x0003, /* Application Collection */
    kHIDUsage_AppleVendor_AmbientLightSensor    = 0x0004, /* Application Collection */
    kHIDUsage_AppleVendor_TemperatureSensor     = 0x0005, /* Application Collection */
    kHIDUsage_AppleVendor_Keyboard              = 0x0006, /* Application Collection */
    kHIDUsage_AppleVendor_Headset               = 0x0007, /* Application Collection */
    kHIDUsage_AppleVendor_ProximitySensor       = 0x0008, /* Application Collection */
    kHIDUsage_AppleVendor_Gyro                  = 0x0009, /* Application Collection */
    kHIDUsage_AppleVendor_Compass               = 0x000A, /* Application Collection */
    kHIDUsage_AppleVendor_DeviceManagement      = 0x000B, /* Application Collection */
    kHIDUsage_AppleVendor_Trackpad              = 0x000C, /* Application Collection */
    kHIDUsage_AppleVendor_TopCaseReserved       = 0x000D, /* Application Collection */
    kHIDUsage_AppleVendor_Motion                = 0x000E, /* Application Collection */
    kHIDUsage_AppleVendor_KeyboardBacklight     = 0x000F, /* Application Collection */
    kHIDUsage_AppleVendor_DeviceMotionLite      = 0x0010, /* Application Collection */
    kHIDUsage_AppleVendor_Force                 = 0x0011, /* Application Collection */
    kHIDUsage_AppleVendor_BluetoothRadio        = 0x0012, /* Application Collection */
    kHIDUsage_AppleVendor_Orb                   = 0x0013, /* Application Collection */
    kHIDUsage_AppleVendor_AccessoryBattery      = 0x0014, /* Application Collection Use kHIDUsage_PD_PeripheralDevice */
    kHIDUsage_AppleVendor_Humidity              = 0x0015, /* Application Collection */
    kHIDUsage_AppleVendor_DFR                   = 0x0016, /* Application Collection */
    kHIDUsage_AppleVendor_NXEvent               = 0x0017, /* Application Collection */
    kHIDUsage_AppleVendor_NXEvent_Translated    = 0x0018, /* Application Collection */
    kHIDUsage_AppleVendor_NXEvent_Diagnostic    = 0x0019, /* Application Collection */
    kHIDUsage_AppleVendor_Homer                 = 0x0020, /* Application Collection */
    kHIDUsage_AppleVendor_Color                 = 0x0021, /* Dynamic Value */
    kHIDUsage_AppleVendor_Accessibility         = 0x0022, /* Application Collection */
    kHIDUsage_AppleVendor_Message               = 0x0023, /* Application Collection */
    kHIDUsage_AppleVendor_Luna                  = 0x0024, /* Application Collection */
    kHIDUsage_AppleVendor_Payload               = 0x0025, /* Application Collection */
    kHIDUsage_AppleVendor_Perf                  = 0x0026, /* Application Collection */
    kHIDUsage_AppleVendor_Gallium               = 0x0027, /* Application Collection */
    kHIDUsage_AppleVendor_HIDRelay              = 0x0028, /* Application Collection */
    kHIDUsage_AppleVendor_SmartCover            = 0x0029  /* Application Collection */
};


/* AppleVendor Keyboard Page (0xff01) */
enum {
    kHIDUsage_AppleVendorKeyboard_Spotlight             = 0x0001,
    kHIDUsage_AppleVendorKeyboard_Dashboard             = 0x0002,
    kHIDUsage_AppleVendorKeyboard_Function              = 0x0003,
    kHIDUsage_AppleVendorKeyboard_Launchpad             = 0x0004,
    kHIDUsage_AppleVendorKeyboard_Reserved              = 0x000A,
    kHIDUsage_AppleVendorKeyboard_CapsLockDelayEnable   = 0x000B,
    kHIDUsage_AppleVendorKeyboard_PowerState            = 0x000C,
    kHIDUsage_AppleVendorKeyboard_Expose_All            = 0x0010,
    kHIDUsage_AppleVendorKeyboard_Expose_Desktop        = 0x0011,
    kHIDUsage_AppleVendorKeyboard_Brightness_Up         = 0x0020,
    kHIDUsage_AppleVendorKeyboard_Brightness_Down       = 0x0021,
    kHIDUsage_AppleVendorKeyboard_Language              = 0x0030,
    
    /* Synthesized Keyboard Events */
    kHIDUsage_AppleVendorKeyboard_WillReset             = 0x0040,
    kHIDUsage_AppleVendorKeyboard_Reset                 = 0x0041,
    kHIDUsage_AppleVendorKeyboard_WillFactoryReset      = 0x0042,
    kHIDUsage_AppleVendorKeyboard_AccessibilityToggle   = 0x0043,
};

/* AppleVendor Page Headset (0xff07) */
enum
{
    kHIDUsage_AV_Headset_Availability           = 0x0001
};

/* AppleVendor Power Page (0xff08) */
enum {
    kHIDUsage_AppleVendorPowerSensor_Power      = 0x0001,
    kHIDUsage_AppleVendorPowerSensor_Current    = 0x0002,
    kHIDUsage_AppleVendorPowerSensor_Voltage    = 0x0003,
};

/* AppleVendor Smart Cover Page (0xff09) */
enum {
    kHIDUsage_AppleVendorSmartCover_Open        = 0x0001,
    kHIDUsage_AppleVendorSmartCover_Flap1       = 0x0002,
    kHIDUsage_AppleVendorSmartCover_Flap2       = 0x0003,
    kHIDUsage_AppleVendorSmartCover_Flap3       = 0x0004,
    kHIDUsage_AppleVendorSmartCover_Attach      = 0x0010,
    kHIDUsage_AppleVendorSmartCover_StateUnknown= 0x0020,
};

/* AppleVendor Platinum - Heart Rate (0xff0A) */
enum {
    kHIDUsage_AppleVendorPlatinum_Lutetium      = 0x0003,
    kHIDUsage_AppleVendorHeartRate_HR           = 0x0001,
    kHIDUsage_AppleVendorHeartRate_BGHR         = 0x0003,
    kHIDUsage_AppleVendorHeartRate_Erbium       = 0x0010,
};

/* AppleVendor Motion (0xff0C) */
enum {
    kHIDUsage_AppleVendorMotion_Motion          = 0x0001,
    kHIDUsage_AppleVendorMotion_Activity        = 0x0002,
    kHIDUsage_AppleVendorMotion_Gesture         = 0x0003,
    kHIDUsage_AppleVendorMotion_DeviceMotion    = 0x0004,
    kHIDUsage_AppleVendorMotion_DeviceMotion6   = 0x0005,
    kHIDUsage_AppleVendorMotion_Orientation     = 0x0006,
    kHIDUsage_AppleVendorMotion_DeviceMotion3   = 0x0007,
    kHIDUsage_AppleVendorMotion_DeviceMotion10  = 0x0008
};

/* AppleVendor Battery (0xff0D) */
enum {
    kHIDUsage_AppleVendorBattery_RawCapacity            = 0x0001,
    kHIDUsage_AppleVendorBattery_NominalChargeCapacity  = 0x0002,
    kHIDUsage_AppleVendorBattery_CumulativeCurrent      = 0x0003,
};

/* AppleVendor IR Remote (0xff0E) */
enum {
    kHIDUsage_AppleVendorIRRemote_Pair              = 0x0001,
    kHIDUsage_AppleVendorIRRemote_Unpair            = 0x0002,
    kHIDUsage_AppleVendorIRRemote_LowBattery        = 0x0003,
    kHIDUsage_AppleVendorIRRemote_BTLEDiscoveryMode = 0x0004,
};

/* AppleVendor Debug (0xff0F) */
enum {
    kHIDUsage_AppleVendorDebug_Screenshot           = 0x0001,
    kHIDUsage_AppleVendorDebug_Stackshot            = 0x0002,
    kHIDUsage_AppleVendorDebug_SendLogs             = 0x0003,
    kHIDUsage_AppleVendorDebug_BlackScreenRecover   = 0x0004,
};

/* AppleVendor IR Interface (0xff10) */
enum {
    kHIDUsage_AppleVendorIRInterface_IRCommand      = 0x0001,
    kHIDUsage_AppleVendorIRInterface_IRTimings      = 0x0002,
};

/* AppleVendor Multitouch Page (0xff60) */
enum {
    kHIDUsage_AppleVendorMultitouch_PowerOff            = 0x0001,
    kHIDUsage_AppleVendorMultitouch_DeviceReady         = 0x0002,
    kHIDUsage_AppleVendorMultitouch_ExternalMessage     = 0x0003,
    kHIDUsage_AppleVendorMultitouch_WillPowerOn         = 0x0004,
    kHIDUsage_AppleVendorMultitouch_TouchCancel         = 0x0005,
    kHIDUsage_AppleVendorMultitouch_RawFrame            = 0x0006,
};

/* AppleVendor Page Top Case (0x00ff) */
enum
{
    kHIDUsage_AV_TopCase_KeyboardFn            = 0x0003,
    kHIDUsage_AV_TopCase_BrightnessUp          = 0x0004,
    kHIDUsage_AV_TopCase_KeyboardLayoutSwitch  = 0x0004, // use in output report on DFR virtual keyboard
    kHIDUsage_AV_TopCase_BrightnessDown        = 0x0005,
    kHIDUsage_AV_TopCase_VideoMirror           = 0x0006,
    kHIDUsage_AV_TopCase_IlluminationToggle    = 0x0007,
    kHIDUsage_AV_TopCase_IlluminationUp        = 0x0008,
    kHIDUsage_AV_TopCase_IlluminationDown      = 0x0009,
    kHIDUsage_AV_TopCase_ClamshellLatched      = 0x000A,
    kHIDUsage_AV_TopCase_Reserved_MouseData    = 0x00C0
};

/* AppleVendor DFR (0xff11) */
enum {
    kHIDUsage_AppleVendorDFR_TouchTimestamp             = 0x0001,
    kHIDUsage_AppleVendorDFR_TouchGenerationCount       = 0x0002,
    kHIDUsage_AppleVendorDFR_EventRoutingData           = 0x0003
};

/* AppleVendor DFR Brightness (0xff12) */
enum {
    kHIDUsage_AppleVendorDFRBrightness_DFRBrightnes                 = 0x0001,  /* Application Collection */
    
    kHIDUsage_AppleVendorDFRBrightness_Update                       = 0x0010,
    kHIDUsage_AppleVendorDFRBrightness_Absolute                     = 0x0011,
    kHIDUsage_AppleVendorDFRBrightness_Relative                     = 0x0012,
    kHIDUsage_AppleVendorDFRBrightness_RampLength                   = 0x0013,
    
    kHIDUsage_AppleVendorDFRBrightness_DimmingStepOptions           = 0x0020,
    kHIDUsage_AppleVendorDFRBrightness_DimmingStep                  = 0x0021,
    kHIDUsage_AppleVendorDFRBrightness_DimmingStepRampLength        = 0x0022,
    kHIDUsage_AppleVendorDFRBrightness_DimmingStepFactor            = 0x0023,
    
    kHIDUsage_AppleVendorDFRBrightness_DisplayState                 = 0x0031,
    kHIDUsage_AppleVendorDFRBrightness_DisplaySateRampLength        = 0x0032,
    
    kHIDUsage_AppleVendorDFRBrightness_Version                      = 0x0040,
    kHIDUsage_AppleVendorDFRBrightness_AutoBrightness               = 0x0041,
    kHIDUsage_AppleVendorDFRBrightness_MinNits                      = 0x0042,
    kHIDUsage_AppleVendorDFRBrightness_MaxNits                      = 0x0043,
    kHIDUsage_AppleVendorDFRBrightness_CurveType                    = 0x0044,
    kHIDUsage_AppleVendorDFRBrightness_CurvePointCount              = 0x0045,
    kHIDUsage_AppleVendorDFRBrightness_CurveUpdateRampLength        = 0x0046,
    kHIDUsage_AppleVendorDFRBrightness_CurveIlluminanceCoordinates  = 0x0047,
    kHIDUsage_AppleVendorDFRBrightness_CurveLuminanceCoordinates    = 0x0048,
    
    kHIDUsage_AppleVendorDFRBrightness_DFRBurninCounterUpdate       = 0x0050,
    kHIDUsage_AppleVendorDFRBrightness_DFRBurninCounterValue        = 0x0051,
};

/* AppleVendor Sender ID (0xff13) */
enum {
    /* Transports */
    kHIDUsage_AppleVendorSenderID_IR                                = 0x0001,
    kHIDUsage_AppleVendorSenderID_CEC                               = 0x0002,
    
    /* Applications */
    kHIDUsage_AppleVendorSenderID_MediaRemote                       = 0x0010,
};
#endif /* _APPLEHIDUSAGETABLES_H */
