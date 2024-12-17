/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2024 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#ifndef NATIVEJS_LINUX_KEYS_H
#define NATIVEJS_LINUX_KEYS_H

#include <stdint.h>
#include <string>

struct JavaScriptKeyDetails
{
  JavaScriptKeyDetails(): type(""), key(""), code(""), shiftKey(false), ctrlKey(false), altKey(false), metaKey(false), repeat(false), keyCode(0) {}
  std::string type;
  std::string key;
  std::string code;
  bool shiftKey;
  bool ctrlKey;
  bool altKey;
  bool metaKey;
  bool repeat;
  uint32_t keyCode;
};

#define WAYLAND_KEY_RESERVED            0
#define WAYLAND_KEY_ESC                 1
#define WAYLAND_KEY_1                   2
#define WAYLAND_KEY_2                   3
#define WAYLAND_KEY_3                   4
#define WAYLAND_KEY_4                   5
#define WAYLAND_KEY_5                   6
#define WAYLAND_KEY_6                   7
#define WAYLAND_KEY_7                   8
#define WAYLAND_KEY_8                   9
#define WAYLAND_KEY_9                   10
#define WAYLAND_KEY_0                   11
#define WAYLAND_KEY_MINUS               12
#define WAYLAND_KEY_EQUAL               13
#define WAYLAND_KEY_BACKSPACE           14
#define WAYLAND_KEY_TAB                 15
#define WAYLAND_KEY_Q                   16
#define WAYLAND_KEY_W                   17
#define WAYLAND_KEY_E                   18
#define WAYLAND_KEY_R                   19
#define WAYLAND_KEY_T                   20
#define WAYLAND_KEY_Y                   21
#define WAYLAND_KEY_U                   22
#define WAYLAND_KEY_I                   23
#define WAYLAND_KEY_O                   24
#define WAYLAND_KEY_P                   25
#define WAYLAND_KEY_LEFTBRACE           26
#define WAYLAND_KEY_RIGHTBRACE          27
#define WAYLAND_KEY_ENTER               28
#define WAYLAND_KEY_LEFTCTRL            29
#define WAYLAND_KEY_A                   30
#define WAYLAND_KEY_S                   31
#define WAYLAND_KEY_D                   32
#define WAYLAND_KEY_F                   33
#define WAYLAND_KEY_G                   34
#define WAYLAND_KEY_H                   35
#define WAYLAND_KEY_J                   36
#define WAYLAND_KEY_K                   37
#define WAYLAND_KEY_L                   38
#define WAYLAND_KEY_SEMICOLON           39
#define WAYLAND_KEY_APOSTROPHE          40
#define WAYLAND_KEY_GRAVE               41
#define WAYLAND_KEY_LEFTSHIFT           42
#define WAYLAND_KEY_BACKSLASH           43
#define WAYLAND_KEY_Z                   44
#define WAYLAND_KEY_X                   45
#define WAYLAND_KEY_C                   46
#define WAYLAND_KEY_V                   47
#define WAYLAND_KEY_B                   48
#define WAYLAND_KEY_N                   49
#define WAYLAND_KEY_M                   50
#define WAYLAND_KEY_COMMA               51
#define WAYLAND_KEY_DOT                 52
#define WAYLAND_KEY_SLASH               53
#define WAYLAND_KEY_RIGHTSHIFT          54
#define WAYLAND_KEY_KPASTERISK          55
#define WAYLAND_KEY_LEFTALT             56
#define WAYLAND_KEY_SPACE               57
#define WAYLAND_KEY_CAPSLOCK            58
#define WAYLAND_KEY_F1                  59
#define WAYLAND_KEY_F2                  60
#define WAYLAND_KEY_F3                  61
#define WAYLAND_KEY_F4                  62
#define WAYLAND_KEY_F5                  63
#define WAYLAND_KEY_F6                  64
#define WAYLAND_KEY_F7                  65
#define WAYLAND_KEY_F8                  66
#define WAYLAND_KEY_F9                  67
#define WAYLAND_KEY_F10                 68
#define WAYLAND_KEY_NUMLOCK             69
#define WAYLAND_KEY_SCROLLLOCK          70
#define WAYLAND_KEY_KP7                 71
#define WAYLAND_KEY_KP8                 72
#define WAYLAND_KEY_KP9                 73
#define WAYLAND_KEY_KPMINUS             74
#define WAYLAND_KEY_KP4                 75
#define WAYLAND_KEY_KP5                 76
#define WAYLAND_KEY_KP6                 77
#define WAYLAND_KEY_KPPLUS              78
#define WAYLAND_KEY_KP1                 79
#define WAYLAND_KEY_KP2                 80
#define WAYLAND_KEY_KP3                 81
#define WAYLAND_KEY_KP0                 82
#define WAYLAND_KEY_KPDOT               83
#define WAYLAND_KEY_102ND               86
#define WAYLAND_KEY_F11                 87
#define WAYLAND_KEY_F12                 88
#define WAYLAND_KEY_KPENTER             96
#define WAYLAND_KEY_RIGHTCTRL           97
#define WAYLAND_KEY_KPSLASH             98
#define WAYLAND_KEY_RIGHTALT            100
#define WAYLAND_KEY_HOME                102
#define WAYLAND_KEY_UP                  103
#define WAYLAND_KEY_PAGEUP              104
#define WAYLAND_KEY_LEFT                105
#define WAYLAND_KEY_RIGHT               106
#define WAYLAND_KEY_END                 107
#define WAYLAND_KEY_DOWN                108
#define WAYLAND_KEY_PAGEDOWN            109
#define WAYLAND_KEY_INSERT              110
#define WAYLAND_KEY_DELETE              111
#define WAYLAND_KEY_MUTE                113
#define WAYLAND_KEY_VOLUME_DOWN         114
#define WAYLAND_KEY_VOLUME_UP           115
#define WAYLAND_KEY_KPEQUAL             117
#define WAYLAND_KEY_KPPLUSMINUS         118
#define WAYLAND_KEY_PAUSE               119
#define WAYLAND_KEY_KPCOMMA             121
#define WAYLAND_KEY_LEFTMETA            125
#define WAYLAND_KEY_RIGHTMETA           126
#ifndef KEY_YELLOW
#define WAYLAND_KEY_YELLOW              0x18e
#endif
#ifndef KEY_BLUE
#define WAYLAND_KEY_BLUE                0x18f
#endif
#define WAYLAND_KEY_PLAYPAUSE           164
#define WAYLAND_KEY_REWIND              168
#ifndef KEY_RED
#define WAYLAND_KEY_RED                 0x190
#endif
#ifndef KEY_GREEN
#define WAYLAND_KEY_GREEN               0x191
#endif
#define WAYLAND_KEY_PLAY                207
#define WAYLAND_KEY_FASTFORWARD         208
#define WAYLAND_KEY_PRINT               210     /* AC Print */
#define WAYLAND_KEY_BACK                158
#define WAYLAND_KEY_MENU                139
#define WAYLAND_KEY_HOMEPAGE            172


#define NATIVEJS_FLAGS_SHIFT        8
#define NATIVEJS_FLAGS_CONTROL      16
#define NATIVEJS_FLAGS_ALT          32
#define NATIVEJS_FLAGS_COMMAND      64


#define NATIVEJS_KEY_BACKSPACE "Backspace"
#define NATIVEJS_KEY_TAB 	"Tab"
#define NATIVEJS_KEY_ENTER 	"Enter"
#define NATIVEJS_KEY_SHIFT	"Shift"
#define NATIVEJS_KEY_SHIFT_RIGHT "ShiftRight"
#define NATIVEJS_KEY_SHIFT_LEFT	"ShiftLeft"
#define NATIVEJS_KEY_SHIFT_RIGHT "ShiftRight"
#define NATIVEJS_KEY_CTRL 	"Control"
#define NATIVEJS_KEY_CTRL_LEFT 	"ControlLeft"
#define NATIVEJS_KEY_CTRL_RIGHT	"ControlRight"
#define NATIVEJS_KEY_ALT 	"Alt"
#define NATIVEJS_KEY_ALT_LEFT 	"AltLeft"
#define NATIVEJS_KEY_ALT_RIGHT 	"AltRight"
#define NATIVEJS_KEY_PAUSE 	"Pause" //TODOCHECK
#define NATIVEJS_KEY_CAPSLOCK 	"CapsLock"
#define NATIVEJS_KEY_ESCAPE 	"Escape"
#define NATIVEJS_KEY_SPACE 	" "
#define NATIVEJS_KEY_PAGEUP 	"PageUp"
#define NATIVEJS_KEY_PAGEDOWN 	"PageDown"
#define NATIVEJS_KEY_END 	"End"
#define NATIVEJS_KEY_HOME 	"Home"
#define NATIVEJS_KEY_LEFT 	"ArrowLeft"
#define NATIVEJS_KEY_UP 	"ArrowUp"
#define NATIVEJS_KEY_RIGHT 	"ArrowRight"
#define NATIVEJS_KEY_DOWN 	"ArrowDown"
#define NATIVEJS_KEY_INSERT 	"Insert"
#define NATIVEJS_KEY_DELETE 	"Delete"
#define NATIVEJS_KEY_ZERO 	"Digit0"
#define NATIVEJS_KEY_ONE 	"Digit1"
#define NATIVEJS_KEY_TWO 	"Digit2"
#define NATIVEJS_KEY_THREE 	"Digit3"
#define NATIVEJS_KEY_FOUR 	"Digit4"
#define NATIVEJS_KEY_FIVE 	"Digit5"
#define NATIVEJS_KEY_SIX 	"Digit6"
#define NATIVEJS_KEY_SEVEN 	"Digit7"
#define NATIVEJS_KEY_EIGHT 	"Digit8"
#define NATIVEJS_KEY_NINE 	"Digit9"
#define NATIVEJS_KEY_A 	"KeyA"
#define NATIVEJS_KEY_B  "KeyB" 	
#define NATIVEJS_KEY_C 	"KeyC"
#define NATIVEJS_KEY_D  "KeyD"	
#define NATIVEJS_KEY_E  "KeyE"	
#define NATIVEJS_KEY_F  "KeyF"	
#define NATIVEJS_KEY_G  "KeyG"	
#define NATIVEJS_KEY_H  "KeyH"	
#define NATIVEJS_KEY_I  "KeyI"	
#define NATIVEJS_KEY_J  "KeyJ"	
#define NATIVEJS_KEY_K  "KeyK"	
#define NATIVEJS_KEY_L  "KeyL"	
#define NATIVEJS_KEY_M  "KeyM"	
#define NATIVEJS_KEY_N  "KeyN"	
#define NATIVEJS_KEY_O  "KeyO"	
#define NATIVEJS_KEY_P  "KeyP"	
#define NATIVEJS_KEY_Q  "KeyQ"	
#define NATIVEJS_KEY_R  "KeyR"	
#define NATIVEJS_KEY_S  "KeyS"	
#define NATIVEJS_KEY_T  "KeyT"	
#define NATIVEJS_KEY_U  "KeyU"	
#define NATIVEJS_KEY_V  "KeyV"	
#define NATIVEJS_KEY_W  "KeyW"	
#define NATIVEJS_KEY_X  "KeyX"	
#define NATIVEJS_KEY_Y  "KeyY"	
#define NATIVEJS_KEY_Z  "KeyZ"	
#define NATIVEJS_KEY_WINDOWKEY_LEFT	"MetaLeft"
#define NATIVEJS_KEY_WINDOWKEY_RIGHT 	"MetaRight"
#define NATIVEJS_KEY_SELECT 	"Select" //TODOCHECK
#define NATIVEJS_KEY_NUMPAD0 "NumPad0"	
#define NATIVEJS_KEY_NUMPAD1 "NumPad1"	
#define NATIVEJS_KEY_NUMPAD2 "NumPad2"	
#define NATIVEJS_KEY_NUMPAD3 "NumPad3"	
#define NATIVEJS_KEY_NUMPAD4 "NumPad4"	
#define NATIVEJS_KEY_NUMPAD5 "NumPad5"	
#define NATIVEJS_KEY_NUMPAD6 "NumPad6"	
#define NATIVEJS_KEY_NUMPAD7 "NumPad7"	
#define NATIVEJS_KEY_NUMPAD8 "NumPad8"	
#define NATIVEJS_KEY_NUMPAD9 "NumPad9"	
#define NATIVEJS_KEY_MULTIPLY 	"NumpadMultiply"
#define NATIVEJS_KEY_ADD 	"NumpadAdd"
#define NATIVEJS_KEY_SUBTRACT 	"NumpadSubtract"
#define NATIVEJS_KEY_DECIMAL 	"NumpadDecimal"
#define NATIVEJS_KEY_DIVIDE 	"NumpadDivide"
#define NATIVEJS_KEY_F1 "F1"	
#define NATIVEJS_KEY_F2 "F2"	
#define NATIVEJS_KEY_F3 "F3"	
#define NATIVEJS_KEY_F4 "F4"	
#define NATIVEJS_KEY_F5 "F5"	
#define NATIVEJS_KEY_F6 "F6"	
#define NATIVEJS_KEY_F7 "F7"	
#define NATIVEJS_KEY_F8 "F8"	
#define NATIVEJS_KEY_F9 "F9"	
#define NATIVEJS_KEY_F10 "F10" 	
#define NATIVEJS_KEY_F11 "F11" 	
#define NATIVEJS_KEY_F12 "F12" 	
#define NATIVEJS_KEY_NUMLOCK 	"NumLock"
#define NATIVEJS_KEY_SCROLLLOCK 	"ScrollLock"
#define NATIVEJS_KEY_SEMICOLON  "Semicolon"
#define NATIVEJS_KEY_EQUALS 	"Equal"
#define NATIVEJS_KEY_COMMA 	","
#define NATIVEJS_KEY_DASH 	"Dash" // TODOCHECK
#define NATIVEJS_KEY_PERIOD 	"Period"
#define NATIVEJS_KEY_FORWARDSLASH 	"Slash"
#define NATIVEJS_KEY_GRAVEACCENT 	"GraveAccent" //TODOCHECK
#define NATIVEJS_KEY_OPENBRACKET 	"BracketLeft"
#define NATIVEJS_KEY_BACKSLASH 	"Backslash"
#define NATIVEJS_KEY_CLOSEBRACKET 	"BracketRight"
#define NATIVEJS_KEY_SINGLEQUOTE 	"Quote"
#define NATIVEJS_KEY_PRINTSCREEN 	"PrintScreen"
#define NATIVEJS_KEY_FASTFORWARD 	"FastForward" //TODOCHECK
#define NATIVEJS_KEY_REWIND 	"Rewind" //TODOCHECK
#define NATIVEJS_KEY_PLAY 	"Play" //TODOCHECK
#define NATIVEJS_KEY_PLAYPAUSE 	"Pause" //TODOCHECK

#define NATIVEJS_LEFTBUTTON       "LeftButton" //TODOCHECK
#define NATIVEJS_MIDDLEBUTTON     "MiddleButton" //TODOCHECK
#define NATIVEJS_RIGHTBUTTON      "RightButton" //TODOCHECK

#define NATIVEJS_MOD_SHIFT        "Shift"
#define NATIVEJS_MOD_CONTROL      "Control"
#define NATIVEJS_MOD_ALT          "Alt"
#define NATIVEJS_MOD_COMMAND      "Meta"

#define NATIVEJS_KEYDOWN_REPEAT   "KeyDownRepeat" //TODOCHECK

#define NATIVEJS_KEY_YELLOW      "Yellow"
#define NATIVEJS_KEY_BLUE        "Blue"
#define NATIVEJS_KEY_RED         "Red"
#define NATIVEJS_KEY_GREEN       "Green"

#define NATIVEJS_KEY_BACK         "Back"
#define NATIVEJS_KEY_MENU         "Menu"
#define NATIVEJS_KEY_HOMEPAGE     "Home"

#define NATIVEJS_KEY_MUTE         "AudioVolumeMute" //TODOCHECK
#define NATIVEJS_KEY_VOLUME_DOWN  "AudioVolumeDown" //TODOCHECK
#define NATIVEJS_KEY_VOLUME_UP    "AudioVolumeUp" //TODOCHECK

// key codes
#define NATIVEJS_KEYCODE_FLAGS_SHIFT        8
#define NATIVEJS_KEYCODE_FLAGS_CONTROL      16
#define NATIVEJS_KEYCODE_FLAGS_ALT          32
#define NATIVEJS_KEYCODE_FLAGS_COMMAND      64


#define NATIVEJS_KEYCODE_BACKSPACE 8
#define NATIVEJS_KEYCODE_TAB 	9
#define NATIVEJS_KEYCODE_ENTER 	13
#define NATIVEJS_KEYCODE_SHIFT 	16
#define NATIVEJS_KEYCODE_CTRL 	17
#define NATIVEJS_KEYCODE_ALT 	18
#define NATIVEJS_KEYCODE_PAUSE 	19
#define NATIVEJS_KEYCODE_CAPSLOCK 	20
#define NATIVEJS_KEYCODE_ESCAPE 	27
#define NATIVEJS_KEYCODE_SPACE 	32
#define NATIVEJS_KEYCODE_PAGEUP 	33
#define NATIVEJS_KEYCODE_PAGEDOWN 	34
#define NATIVEJS_KEYCODE_END 	35
#define NATIVEJS_KEYCODE_HOME 	36
#define NATIVEJS_KEYCODE_LEFT 	37
#define NATIVEJS_KEYCODE_UP 	38
#define NATIVEJS_KEYCODE_RIGHT 	39
#define NATIVEJS_KEYCODE_DOWN 	40
#define NATIVEJS_KEYCODE_INSERT 	45
#define NATIVEJS_KEYCODE_DELETE 	46
#define NATIVEJS_KEYCODE_ZERO 	48
#define NATIVEJS_KEYCODE_ONE 	49
#define NATIVEJS_KEYCODE_TWO 	50
#define NATIVEJS_KEYCODE_THREE 	51
#define NATIVEJS_KEYCODE_FOUR 	52
#define NATIVEJS_KEYCODE_FIVE 	53
#define NATIVEJS_KEYCODE_SIX 	54
#define NATIVEJS_KEYCODE_SEVEN 	55
#define NATIVEJS_KEYCODE_EIGHT 	56
#define NATIVEJS_KEYCODE_NINE 	57
#define NATIVEJS_KEYCODE_A 	65
#define NATIVEJS_KEYCODE_B 	66
#define NATIVEJS_KEYCODE_C 	67
#define NATIVEJS_KEYCODE_D 	68
#define NATIVEJS_KEYCODE_E 	69
#define NATIVEJS_KEYCODE_F 	70
#define NATIVEJS_KEYCODE_G 	71
#define NATIVEJS_KEYCODE_H 	72
#define NATIVEJS_KEYCODE_I 	73
#define NATIVEJS_KEYCODE_J 	74
#define NATIVEJS_KEYCODE_K 	75
#define NATIVEJS_KEYCODE_L 	76
#define NATIVEJS_KEYCODE_M 	77
#define NATIVEJS_KEYCODE_N 	78
#define NATIVEJS_KEYCODE_O 	79
#define NATIVEJS_KEYCODE_P 	80
#define NATIVEJS_KEYCODE_Q 	81
#define NATIVEJS_KEYCODE_R 	82
#define NATIVEJS_KEYCODE_S 	83
#define NATIVEJS_KEYCODE_T 	84
#define NATIVEJS_KEYCODE_U 	85
#define NATIVEJS_KEYCODE_V 	86
#define NATIVEJS_KEYCODE_W 	87
#define NATIVEJS_KEYCODE_X 	88
#define NATIVEJS_KEYCODE_Y 	89
#define NATIVEJS_KEYCODE_Z 	90
#define NATIVEJS_KEYCODE_WINDOWKEY_LEFT	91
#define NATIVEJS_KEYCODE_WINDOWKEY_RIGHT 	92
#define NATIVEJS_KEYCODE_SELECT 	93
#define NATIVEJS_KEYCODE_NUMPAD0 	96
#define NATIVEJS_KEYCODE_NUMPAD1 	97
#define NATIVEJS_KEYCODE_NUMPAD2 	98
#define NATIVEJS_KEYCODE_NUMPAD3 	99
#define NATIVEJS_KEYCODE_NUMPAD4 	100
#define NATIVEJS_KEYCODE_NUMPAD5 	101
#define NATIVEJS_KEYCODE_NUMPAD6 	102
#define NATIVEJS_KEYCODE_NUMPAD7 	103
#define NATIVEJS_KEYCODE_NUMPAD8 	104
#define NATIVEJS_KEYCODE_NUMPAD9 	105
#define NATIVEJS_KEYCODE_MULTIPLY 	106
#define NATIVEJS_KEYCODE_ADD 	107
#define NATIVEJS_KEYCODE_SUBTRACT 	109
#define NATIVEJS_KEYCODE_DECIMAL 	110
#define NATIVEJS_KEYCODE_DIVIDE 	111
#define NATIVEJS_KEYCODE_F1 	112
#define NATIVEJS_KEYCODE_F2 	113
#define NATIVEJS_KEYCODE_F3 	114
#define NATIVEJS_KEYCODE_F4 	115
#define NATIVEJS_KEYCODE_F5 	116
#define NATIVEJS_KEYCODE_F6 	117
#define NATIVEJS_KEYCODE_F7 	118
#define NATIVEJS_KEYCODE_F8 	119
#define NATIVEJS_KEYCODE_F9 	120
#define NATIVEJS_KEYCODE_F10 	121
#define NATIVEJS_KEYCODE_F11 	122
#define NATIVEJS_KEYCODE_F12 	123
#define NATIVEJS_KEYCODE_NUMLOCK 	144
#define NATIVEJS_KEYCODE_SCROLLLOCK 	145
#define NATIVEJS_KEYCODE_SEMICOLON 	186
#define NATIVEJS_KEYCODE_EQUALS 	187
#define NATIVEJS_KEYCODE_COMMA 	188
#define NATIVEJS_KEYCODE_DASH 	189
#define NATIVEJS_KEYCODE_PERIOD 	190
#define NATIVEJS_KEYCODE_FORWARDSLASH 	191
#define NATIVEJS_KEYCODE_GRAVEACCENT 	192
#define NATIVEJS_KEYCODE_OPENBRACKET 	219
#define NATIVEJS_KEYCODE_BACKSLASH 	220
#define NATIVEJS_KEYCODE_CLOSEBRACKET 	221
#define NATIVEJS_KEYCODE_SINGLEQUOTE 	222
#define NATIVEJS_KEYCODE_PRINTSCREEN 	44
#define NATIVEJS_KEYCODE_FASTFORWARD 	223
#define NATIVEJS_KEYCODE_REWIND 	224
#define NATIVEJS_KEYCODE_PLAY 	226
#define NATIVEJS_KEYCODE_PLAYPAUSE 	227

#define NATIVEJS_KEYCODE_LEFTBUTTON       1
#define NATIVEJS_KEYCODE_MIDDLEBUTTON     2
#define NATIVEJS_KEYCODE_RIGHTBUTTON      4

#define NATIVEJS_KEYCODE_MOD_SHIFT        8
#define NATIVEJS_KEYCODE_MOD_CONTROL      16
#define NATIVEJS_KEYCODE_MOD_ALT          32
#define NATIVEJS_KEYCODE_MOD_COMMAND      64

#define NATIVEJS_KEYCODE_YELLOW       403
#define NATIVEJS_KEYCODE_BLUE         404
#define NATIVEJS_KEYCODE_RED          405
#define NATIVEJS_KEYCODE_GREEN        406

#define NATIVEJS_KEYCODE_BACK         407
#define NATIVEJS_KEYCODE_MENU         408
#define NATIVEJS_KEYCODE_HOMEPAGE     409

#define NATIVEJS_KEYCODE_MUTE         173
#define NATIVEJS_KEYCODE_VOLUME_DOWN  174
#define NATIVEJS_KEYCODE_VOLUME_UP    175

bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, struct JavaScriptKeyDetails& keyDetails);
#endif
