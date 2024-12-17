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

#include "KeyInput.h"
#include <iostream>

#include <map>
#include <string>
#include <vector>

uint32_t getKeyFlag(std::string modifier)
{
  uint32_t flag = 0;
  if (0 == modifier.compare("ctrl"))
  {
    flag = NATIVEJS_FLAGS_CONTROL;
  }
  else if (0 == modifier.compare("shift"))
  {
    flag = NATIVEJS_FLAGS_SHIFT;
  }
  else if (0 == modifier.compare("alt"))
  {
    flag = NATIVEJS_FLAGS_ALT;
  }
  return flag;
}

std::map<uint32_t, std::vector<std::string>> keyMappings = {

	{ WAYLAND_KEY_ENTER, { "Enter", "Enter", "Enter", "Enter"  } },
	{ WAYLAND_KEY_BACKSPACE, { "Backspace", "Backspace", "Backspace", "Backspace"} },
	{ WAYLAND_KEY_TAB, { "Tab", "Tab", "Tab", "Tab"} },
	{ WAYLAND_KEY_RIGHTSHIFT, { "Shift", "Shift", "Shift", "Shift" } },
	{ WAYLAND_KEY_LEFTSHIFT, {  "Shift", "Shift", "Shift", "Shift" } },
	{ WAYLAND_KEY_RIGHTCTRL, { "Control", "Control", "Control", "Control" } },
	{ WAYLAND_KEY_LEFTCTRL, {  "Control", "Control", "Control", "Control" } },
	{ WAYLAND_KEY_RIGHTALT, { "Alt", "Alt", "Alt", "Alt" } },
	{ WAYLAND_KEY_LEFTALT, { "Alt", "Alt", "Alt", "Alt" } },
	{ WAYLAND_KEY_PAUSE, { "Pause", "Pause", "Pause", "Pause" } },
	{ WAYLAND_KEY_CAPSLOCK, { "CapsLock", "CapsLock", "CapsLock", "CapsLock" } },
	{ WAYLAND_KEY_ESC, { "Escape", "Escape", "Escape", "Escape" } },
	{ WAYLAND_KEY_SPACE, { " ", " ", " ", " ", " " } },
	{ WAYLAND_KEY_PAGEUP, { "PageUp", "PageUp", "PageUp", "PageUp" } },
	{ WAYLAND_KEY_PAGEDOWN, { "PageDown", "PageDown", "PageDown", "PageDown" } },
	{ WAYLAND_KEY_END, { "End", "End", "End", "End" } },
	{ WAYLAND_KEY_HOME, {  "Home", "Home", "Home", "Home" } },
	{ WAYLAND_KEY_LEFT, { "ArrowLeft", "ArrowLeft", "ArrowLeft", "ArrowLeft" } },
	{ WAYLAND_KEY_UP, { "ArrowUp", "ArrowUp", "ArrowUp", "ArrowUp" } },
	{ WAYLAND_KEY_RIGHT, { "ArrowRight", "ArrowRight", "ArrowRight", "ArrowRight" } },
	{ WAYLAND_KEY_DOWN, { "ArrowDown", "ArrowDown", "ArrowDown", "ArrowDown" } },
	{ WAYLAND_KEY_COMMA, { ",", "<", ",", ","  } },
	{ WAYLAND_KEY_DOT, { ".", ">", ".", "."} },
	{ WAYLAND_KEY_SLASH, { "/", "?", "/", "/"} },
	{ WAYLAND_KEY_0, {  "0", ")", "0", "0" } },
	{ WAYLAND_KEY_1, { "1", "!", "1", "1" } },
	{ WAYLAND_KEY_2, { "2", "@", "2", "2" } },
	{ WAYLAND_KEY_3, { "3", "#", "3", "3"  } },
	{ WAYLAND_KEY_4, {  "4", "$", "4", "4" } },
	{ WAYLAND_KEY_5, {  "5", "%", "5", "5" } },
	{ WAYLAND_KEY_6, { "6", "^", "6", "6" } },
	{ WAYLAND_KEY_7, {  "7", "&", "7", "7" } },
	{ WAYLAND_KEY_8, {  "8", "*", "8", "8" } },
	{ WAYLAND_KEY_9, {  "9", "(", "9", "9" } },
	{ WAYLAND_KEY_SEMICOLON, {  ";", ":", ";", ";" } },
	{ WAYLAND_KEY_EQUAL, { "=", "+", "=", "=" } },
	{ WAYLAND_KEY_A, { "a", "A", "a", "a" } }, 
	{ WAYLAND_KEY_B, { "b", "B", "b", "b" } },
	{ WAYLAND_KEY_C, { "c", "C", "c", "c"} },
	{ WAYLAND_KEY_D, { "d", "D", "d", "d"} },
	{ WAYLAND_KEY_E, { "e", "E", "e", "e"} },
	{ WAYLAND_KEY_F, { "f", "F", "f", "f"} },
	{ WAYLAND_KEY_G, { "g", "G", "g", "g"} },
	{ WAYLAND_KEY_H, { "h", "H", "h", "h"} },
	{ WAYLAND_KEY_I, { "i", "I", "i", "i"} },
	{ WAYLAND_KEY_J, { "j", "J", "j", "j"} },
	{ WAYLAND_KEY_K, { "k", "K", "k", "k"} },
	{ WAYLAND_KEY_L, { "l", "L", "l", "l"} },
	{ WAYLAND_KEY_M, { "m", "M", "m", "m"} },
	{ WAYLAND_KEY_N, { "n", "N", "n", "n"} },
	{ WAYLAND_KEY_O, { "o", "O", "o", "o"} },
	{ WAYLAND_KEY_P, { "p", "P", "p", "p"} },
	{ WAYLAND_KEY_Q, { "q", "Q", "q", "q"} },
	{ WAYLAND_KEY_R, { "r", "R", "r", "r"} },
	{ WAYLAND_KEY_S, { "s", "S", "s", "s"} },
	{ WAYLAND_KEY_T, { "t", "T", "t", "t"} },
	{ WAYLAND_KEY_U, { "u", "U", "u", "u"} },
	{ WAYLAND_KEY_V, { "v", "V", "v", "v"} },
	{ WAYLAND_KEY_W, { "w", "W", "w", "w"} },
	{ WAYLAND_KEY_X, { "x", "X", "x", "x"} },
	{ WAYLAND_KEY_Y, { "y", "Y", "y", "y"} },
	{ WAYLAND_KEY_Z, { "z", "Z", "z", "z"} },
	{ WAYLAND_KEY_LEFTBRACE, { "{", "{", "[", "[" } },
	{ WAYLAND_KEY_BACKSLASH, {  "/", "?", "/", "/" } },
	{ WAYLAND_KEY_RIGHTBRACE, { "}", "}", "]", "]" } },
	{ WAYLAND_KEY_KP0, { "0", "0", "0", "0" } },  
	{ WAYLAND_KEY_KP1, { "1", "1", "1", "1" } },
	{ WAYLAND_KEY_KP2, { "2", "2", "2", "2" } },
	{ WAYLAND_KEY_KP3, { "3", "3", "3", "3" } },
	{ WAYLAND_KEY_KP4, { "4", "4", "4", "4" } },
	{ WAYLAND_KEY_KP5, { "5", "5", "5", "5" } },
	{ WAYLAND_KEY_KP6, { "6", "6", "6", "6" } },
	{ WAYLAND_KEY_KP7, { "7", "7", "7", "7" } },
	{ WAYLAND_KEY_KP8, { "8", "8", "8", "8" } },
	{ WAYLAND_KEY_KP9, { "9", "9", "9", "9" } },
	{ WAYLAND_KEY_KPASTERISK, {  "*", "*", "*", "*" } },
	{ WAYLAND_KEY_KPPLUS, {  "+", "+", "+", "+" } },
	{ WAYLAND_KEY_KPMINUS, {  "-", "-", "-", "-" } },
	{ WAYLAND_KEY_KPDOT, {  ".", ".", ".", "." } },
	{ WAYLAND_KEY_KPSLASH, { "/", "/", "/", "/" } },
	{ WAYLAND_KEY_F1, { "F1", "F1", "F1", "F1"  } }, 
	{ WAYLAND_KEY_F2, { "F2", "F2", "F2", "F2" } },
	{ WAYLAND_KEY_F3, { "F3", "F3", "F3", "F3" } },
	{ WAYLAND_KEY_F4, { "F4", "F4", "F4", "F4" } },
	{ WAYLAND_KEY_F5, { "F5", "F5", "F5", "F5" } },
	{ WAYLAND_KEY_F6, { "F6", "F6", "F6", "F6" } },
	{ WAYLAND_KEY_F7, { "F7", "F7", "F7", "F7" } },
	{ WAYLAND_KEY_F8, { "F8", "F8", "F8", "F8" } },
	{ WAYLAND_KEY_F9, { "F9", "F9", "F9", "F9" } },
	{ WAYLAND_KEY_F10, { "F10", "F10", "F10", "F10" } },
	{ WAYLAND_KEY_F11, { "F11", "F11", "F11", "F11" } },
	{ WAYLAND_KEY_F12, { "F12", "F12", "F12", "F12" } },
	{ WAYLAND_KEY_DELETE, { "Delete", "Delete", "Delete", "Delete" } },
	{ WAYLAND_KEY_SCROLLLOCK, {  "ScrollLock", "ScrollLock", "ScrollLock", "ScrollLock" } },
	{ WAYLAND_KEY_PRINT, { "Print", "Print", "Print", "Print" } },
	{ WAYLAND_KEY_INSERT, { "Insert", "Insert", "Insert", "Insert" } },
	{ WAYLAND_KEY_MUTE, {  "AudioVolumeMute", "AudioVolumeMute", "AudioVolumeMute", "AudioVolumeMute" } },
	{ WAYLAND_KEY_VOLUME_DOWN, { "AudioVolumeDown", "AudioVolumeDown", "AudioVolumeDown", "AudioVolumeDown" } },
	{ WAYLAND_KEY_VOLUME_UP, { "AudioVolumeUp", "AudioVolumeUp", "AudioVolumeUp", "AudioVolumeUp" }  }
};
    
static void getJavaScriptKeyCode(uint32_t waylandKeyCode, std::string& keyCode, uint32_t& keyCodeValue)
{
    std::string standardKeyCode("");
    uint32_t standardKeyCodeValue = 0;
    switch (waylandKeyCode)
    {
    case WAYLAND_KEY_ENTER:
        standardKeyCode = NATIVEJS_KEY_ENTER;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ENTER;
        break;
    case WAYLAND_KEY_BACKSPACE:
        standardKeyCode = NATIVEJS_KEY_BACKSPACE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_BACKSPACE;
        break;
    case WAYLAND_KEY_TAB:
        standardKeyCode = NATIVEJS_KEY_TAB;
        standardKeyCodeValue = NATIVEJS_KEYCODE_TAB;
        break;
    case WAYLAND_KEY_RIGHTSHIFT:
        standardKeyCode = NATIVEJS_KEY_SHIFT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SHIFT;
        break;
    case WAYLAND_KEY_LEFTSHIFT:
        standardKeyCode = NATIVEJS_KEY_SHIFT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SHIFT;
        break;
    case WAYLAND_KEY_RIGHTCTRL:
        standardKeyCode = NATIVEJS_KEY_CTRL;
        standardKeyCodeValue = NATIVEJS_KEYCODE_CTRL;
        break;
    case WAYLAND_KEY_LEFTCTRL:
        standardKeyCode = NATIVEJS_KEY_CTRL;
        standardKeyCodeValue = NATIVEJS_KEYCODE_CTRL;
        break;
    case WAYLAND_KEY_RIGHTALT:
        standardKeyCode = NATIVEJS_KEY_ALT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ALT;
        break;
    case WAYLAND_KEY_LEFTALT:
        standardKeyCode = NATIVEJS_KEY_ALT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ALT;
        break;
    case WAYLAND_KEY_PAUSE:
        standardKeyCode = NATIVEJS_KEY_PAUSE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PAUSE;
        break;
    case WAYLAND_KEY_CAPSLOCK:
        standardKeyCode = NATIVEJS_KEY_CAPSLOCK;
        standardKeyCodeValue = NATIVEJS_KEYCODE_CAPSLOCK;
        break;
    case WAYLAND_KEY_ESC:
        standardKeyCode = NATIVEJS_KEY_ESCAPE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ESCAPE;
        break;
    case WAYLAND_KEY_SPACE:
        standardKeyCode = NATIVEJS_KEY_SPACE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SPACE;
        break;
    case WAYLAND_KEY_PAGEUP:
        standardKeyCode = NATIVEJS_KEY_PAGEUP;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PAGEUP;
        break;
    case WAYLAND_KEY_PAGEDOWN:
        standardKeyCode = NATIVEJS_KEY_PAGEDOWN;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PAGEDOWN;
        break;
    case WAYLAND_KEY_END:
        standardKeyCode = NATIVEJS_KEY_END;
        standardKeyCodeValue = NATIVEJS_KEYCODE_END;
        break;
    case WAYLAND_KEY_HOME:
        standardKeyCode = NATIVEJS_KEY_HOME;
        standardKeyCodeValue = NATIVEJS_KEYCODE_HOME;
        break;
    case WAYLAND_KEY_LEFT:
        standardKeyCode = NATIVEJS_KEY_LEFT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_LEFT;
        break;
    case WAYLAND_KEY_UP:
        standardKeyCode = NATIVEJS_KEY_UP;
        standardKeyCodeValue = NATIVEJS_KEYCODE_UP;
        break;
    case WAYLAND_KEY_RIGHT:
        standardKeyCode = NATIVEJS_KEY_RIGHT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_RIGHT;
        break;
    case WAYLAND_KEY_DOWN:
        standardKeyCode = NATIVEJS_KEY_DOWN;
        standardKeyCodeValue = NATIVEJS_KEYCODE_DOWN;
        break;
    case WAYLAND_KEY_COMMA:
        standardKeyCode = NATIVEJS_KEY_COMMA;
        standardKeyCodeValue = NATIVEJS_KEYCODE_COMMA;
        break;
    case WAYLAND_KEY_DOT:
        standardKeyCode = NATIVEJS_KEY_PERIOD;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PERIOD;
        break;
    case WAYLAND_KEY_SLASH:
        standardKeyCode = NATIVEJS_KEY_FORWARDSLASH;
        standardKeyCodeValue = NATIVEJS_KEYCODE_FORWARDSLASH;
        break;
    case WAYLAND_KEY_0:
        standardKeyCode = NATIVEJS_KEY_ZERO;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ZERO;
        break;
    case WAYLAND_KEY_1:
        standardKeyCode = NATIVEJS_KEY_ONE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ONE;
        break;
    case WAYLAND_KEY_2:
        standardKeyCode = NATIVEJS_KEY_TWO;
        standardKeyCodeValue = NATIVEJS_KEYCODE_TWO;
        break;
    case WAYLAND_KEY_3:
        standardKeyCode = NATIVEJS_KEY_THREE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_THREE;
        break;
    case WAYLAND_KEY_4:
        standardKeyCode = NATIVEJS_KEY_FOUR;
        standardKeyCodeValue = NATIVEJS_KEYCODE_FOUR;
        break;
    case WAYLAND_KEY_5:
        standardKeyCode = NATIVEJS_KEY_FIVE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_FIVE;
        break;
    case WAYLAND_KEY_6:
        standardKeyCode = NATIVEJS_KEY_SIX;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SIX;
        break;
    case WAYLAND_KEY_7:
        standardKeyCode = NATIVEJS_KEY_SEVEN;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SEVEN;
        break;
    case WAYLAND_KEY_8:
        standardKeyCode = NATIVEJS_KEY_EIGHT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_EIGHT;
        break;
    case WAYLAND_KEY_9:
        standardKeyCode = NATIVEJS_KEY_NINE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NINE;
        break;
    case WAYLAND_KEY_SEMICOLON:
        standardKeyCode = NATIVEJS_KEY_SEMICOLON;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SEMICOLON;
        break;
    case WAYLAND_KEY_EQUAL:
        standardKeyCode = NATIVEJS_KEY_EQUALS;
        standardKeyCodeValue = NATIVEJS_KEYCODE_EQUALS;
        break;
    case WAYLAND_KEY_A:
        standardKeyCode = NATIVEJS_KEY_A;
        standardKeyCodeValue = NATIVEJS_KEYCODE_A;
        break;
    case WAYLAND_KEY_B:
        standardKeyCode = NATIVEJS_KEY_B;
        standardKeyCodeValue = NATIVEJS_KEYCODE_B;
        break;
    case WAYLAND_KEY_C:
        standardKeyCode = NATIVEJS_KEY_C;
        standardKeyCodeValue = NATIVEJS_KEYCODE_C;
        break;
    case WAYLAND_KEY_D:
        standardKeyCode = NATIVEJS_KEY_D;
        standardKeyCodeValue = NATIVEJS_KEYCODE_D;
        break;
    case WAYLAND_KEY_E:
        standardKeyCode = NATIVEJS_KEY_E;
        standardKeyCodeValue = NATIVEJS_KEYCODE_E;
        break;
    case WAYLAND_KEY_F:
        standardKeyCode = NATIVEJS_KEY_F;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F;
        break;
    case WAYLAND_KEY_G:
        standardKeyCode = NATIVEJS_KEY_G;
        standardKeyCodeValue = NATIVEJS_KEYCODE_G;
        break;
    case WAYLAND_KEY_H:
        standardKeyCode = NATIVEJS_KEY_H;
        standardKeyCodeValue = NATIVEJS_KEYCODE_H;
        break;
    case WAYLAND_KEY_I:
        standardKeyCode = NATIVEJS_KEY_I;
        standardKeyCodeValue = NATIVEJS_KEYCODE_I;
        break;
    case WAYLAND_KEY_J:
        standardKeyCode = NATIVEJS_KEY_J;
        standardKeyCodeValue = NATIVEJS_KEYCODE_J;
        break;
    case WAYLAND_KEY_K:
        standardKeyCode = NATIVEJS_KEY_K;
        standardKeyCodeValue = NATIVEJS_KEYCODE_K;
        break;
    case WAYLAND_KEY_L:
        standardKeyCode = NATIVEJS_KEY_L;
        standardKeyCodeValue = NATIVEJS_KEYCODE_L;
        break;
    case WAYLAND_KEY_M:
        standardKeyCode = NATIVEJS_KEY_M;
        standardKeyCodeValue = NATIVEJS_KEYCODE_M;
        break;
    case WAYLAND_KEY_N:
        standardKeyCode = NATIVEJS_KEY_N;
        standardKeyCodeValue = NATIVEJS_KEYCODE_N;
        break;
    case WAYLAND_KEY_O:
        standardKeyCode = NATIVEJS_KEY_O;
        standardKeyCodeValue = NATIVEJS_KEYCODE_O;
        break;
    case WAYLAND_KEY_P:
        standardKeyCode = NATIVEJS_KEY_P;
        standardKeyCodeValue = NATIVEJS_KEYCODE_P;
        break;
    case WAYLAND_KEY_Q:
        standardKeyCode = NATIVEJS_KEY_Q;
        standardKeyCodeValue = NATIVEJS_KEYCODE_Q;
        break;
    case WAYLAND_KEY_R:
        standardKeyCode = NATIVEJS_KEY_R;
        standardKeyCodeValue = NATIVEJS_KEYCODE_R;
        break;
    case WAYLAND_KEY_S:
        standardKeyCode = NATIVEJS_KEY_S;
        standardKeyCodeValue = NATIVEJS_KEYCODE_S;
        break;
    case WAYLAND_KEY_T:
        standardKeyCode = NATIVEJS_KEY_T;
        standardKeyCodeValue = NATIVEJS_KEYCODE_T;
        break;
    case WAYLAND_KEY_U:
        standardKeyCode = NATIVEJS_KEY_U;
        standardKeyCodeValue = NATIVEJS_KEYCODE_U;
        break;
    case WAYLAND_KEY_V:
        standardKeyCode = NATIVEJS_KEY_V;
        standardKeyCodeValue = NATIVEJS_KEYCODE_V;
        break;
    case WAYLAND_KEY_W:
        standardKeyCode = NATIVEJS_KEY_W;
        standardKeyCodeValue = NATIVEJS_KEYCODE_W;
        break;
    case WAYLAND_KEY_X:
        standardKeyCode = NATIVEJS_KEY_X;
        standardKeyCodeValue = NATIVEJS_KEYCODE_X;
        break;
    case WAYLAND_KEY_Y:
        standardKeyCode = NATIVEJS_KEY_Y;
        standardKeyCodeValue = NATIVEJS_KEYCODE_Y;
        break;
    case WAYLAND_KEY_Z:
        standardKeyCode = NATIVEJS_KEY_Z;
        standardKeyCodeValue = NATIVEJS_KEYCODE_Z;
        break;
    case WAYLAND_KEY_LEFTBRACE:
        standardKeyCode = NATIVEJS_KEY_OPENBRACKET;
        standardKeyCodeValue = NATIVEJS_KEYCODE_OPENBRACKET;
        break;
    case WAYLAND_KEY_BACKSLASH:
        standardKeyCode = NATIVEJS_KEY_BACKSLASH;
        standardKeyCodeValue = NATIVEJS_KEYCODE_BACKSLASH;
        break;
    case WAYLAND_KEY_RIGHTBRACE:
        standardKeyCode = NATIVEJS_KEY_CLOSEBRACKET;
        standardKeyCodeValue = NATIVEJS_KEYCODE_CLOSEBRACKET;
        break;
    case WAYLAND_KEY_KP0:
        standardKeyCode = NATIVEJS_KEY_NUMPAD0;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD0;
        break;
    case WAYLAND_KEY_KP1:
        standardKeyCode = NATIVEJS_KEY_NUMPAD1;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD1;
        break;
    case WAYLAND_KEY_KP2:
        standardKeyCode = NATIVEJS_KEY_NUMPAD2;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD2;
        break;
    case WAYLAND_KEY_KP3:
        standardKeyCode = NATIVEJS_KEY_NUMPAD3;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD3;
        break;
    case WAYLAND_KEY_KP4:
        standardKeyCode = NATIVEJS_KEY_NUMPAD4;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD4;
        break;
    case WAYLAND_KEY_KP5:
        standardKeyCode = NATIVEJS_KEY_NUMPAD5;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD5;
        break;
    case WAYLAND_KEY_KP6:
        standardKeyCode = NATIVEJS_KEY_NUMPAD6;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD6;
        break;
    case WAYLAND_KEY_KP7:
        standardKeyCode = NATIVEJS_KEY_NUMPAD7;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD7;
        break;
    case WAYLAND_KEY_KP8:
        standardKeyCode = NATIVEJS_KEY_NUMPAD8;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD8;
        break;
    case WAYLAND_KEY_KP9:
        standardKeyCode = NATIVEJS_KEY_NUMPAD9;
        standardKeyCodeValue = NATIVEJS_KEYCODE_NUMPAD9;
        break;
    case WAYLAND_KEY_KPASTERISK:
        standardKeyCode = NATIVEJS_KEY_MULTIPLY;
        standardKeyCodeValue = NATIVEJS_KEYCODE_MULTIPLY;
        break;
    case WAYLAND_KEY_KPPLUS:
        standardKeyCode = NATIVEJS_KEY_ADD;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ADD;
        break;
    case WAYLAND_KEY_KPMINUS:
        standardKeyCode = NATIVEJS_KEY_SUBTRACT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SUBTRACT;
        break;
    case WAYLAND_KEY_KPDOT:
        standardKeyCode = NATIVEJS_KEY_DECIMAL;
        standardKeyCodeValue = NATIVEJS_KEYCODE_DECIMAL;
        break;
    case WAYLAND_KEY_KPSLASH:
        standardKeyCode = NATIVEJS_KEY_DIVIDE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_DIVIDE;
        break;
    case WAYLAND_KEY_F1:
        standardKeyCode = NATIVEJS_KEY_F1;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F1;
        break;
    case WAYLAND_KEY_F2:
        standardKeyCode = NATIVEJS_KEY_F2;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F2;
        break;
    case WAYLAND_KEY_F3:
        standardKeyCode = NATIVEJS_KEY_F3;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F3;
        break;
    case WAYLAND_KEY_F4:
        standardKeyCode = NATIVEJS_KEY_F4;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F4;
        break;
    case WAYLAND_KEY_F5:
        standardKeyCode = NATIVEJS_KEY_F5;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F5;
        break;
    case WAYLAND_KEY_F6:
        standardKeyCode = NATIVEJS_KEY_F6;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F6;
        break;
    case WAYLAND_KEY_F7:
        standardKeyCode = NATIVEJS_KEY_F7;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F7;
        break;
    case WAYLAND_KEY_F8:
        standardKeyCode = NATIVEJS_KEY_F8;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F8;
        break;
    case WAYLAND_KEY_F9:
        standardKeyCode = NATIVEJS_KEY_F9;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F9;
        break;
    case WAYLAND_KEY_F10:
        standardKeyCode = NATIVEJS_KEY_F10;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F10;
        break;
    case WAYLAND_KEY_F11:
        standardKeyCode = NATIVEJS_KEY_F11;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F11;
        break;
    case WAYLAND_KEY_F12:
        standardKeyCode = NATIVEJS_KEY_F12;
        standardKeyCodeValue = NATIVEJS_KEYCODE_F12;
        break;
    case WAYLAND_KEY_DELETE:
        standardKeyCode = NATIVEJS_KEY_DELETE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_DELETE;
        break;
    case WAYLAND_KEY_SCROLLLOCK:
        standardKeyCode = NATIVEJS_KEY_SCROLLLOCK;
        standardKeyCodeValue = NATIVEJS_KEYCODE_SCROLLLOCK;
        break;
    case WAYLAND_KEY_PRINT:
        standardKeyCode = NATIVEJS_KEY_PRINTSCREEN;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PRINTSCREEN;
        break;
    case WAYLAND_KEY_INSERT:
        standardKeyCode = NATIVEJS_KEY_INSERT;
        standardKeyCodeValue = NATIVEJS_KEYCODE_INSERT;
        break;
    case WAYLAND_KEY_MUTE:
        standardKeyCode = NATIVEJS_KEY_MUTE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_MUTE;
        break;
    case WAYLAND_KEY_VOLUME_DOWN:
        standardKeyCode = NATIVEJS_KEY_VOLUME_DOWN;
        standardKeyCodeValue = NATIVEJS_KEYCODE_VOLUME_DOWN;
        break;
    case WAYLAND_KEY_VOLUME_UP:
        standardKeyCode = NATIVEJS_KEY_VOLUME_UP;
        standardKeyCodeValue = NATIVEJS_KEYCODE_VOLUME_UP;
        break;
    
    #ifdef WAYLAND_KEY_PLAYPAUSE
    case WAYLAND_KEY_PLAYPAUSE:
        standardKeyCode = NATIVEJS_KEY_PLAYPAUSE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PLAYPAUSE;
        break;
    #endif /* WAYLAND_KEY_PLAYPAUSE */

    #ifdef WAYLAND_KEY_PLAY
    case WAYLAND_KEY_PLAY:
        standardKeyCode = NATIVEJS_KEY_PLAY;
        standardKeyCodeValue = NATIVEJS_KEYCODE_PLAY;
        break;
    #endif /* WAYLAND_KEY_PLAY */

    #ifdef WAYLAND_KEY_FASTFORWARD
    case WAYLAND_KEY_FASTFORWARD:
        standardKeyCode = NATIVEJS_KEY_FASTFORWARD;
        standardKeyCodeValue = NATIVEJS_KEYCODE_FASTFORWARD;
        break;
    #endif /* NATIVEJS_KEY_FASTFORWARD  */

    #ifdef WAYLAND_KEY_REWIND
    case WAYLAND_KEY_REWIND:
        standardKeyCode = NATIVEJS_KEY_REWIND;
        standardKeyCodeValue = NATIVEJS_KEYCODE_REWIND;
        break;
    #endif /* WAYLAND_KEY_REWIND */

    #ifdef WAYLAND_KEY_KPENTER
    case WAYLAND_KEY_KPENTER:
        standardKeyCode = NATIVEJS_KEY_ENTER;
        standardKeyCodeValue = NATIVEJS_KEYCODE_ENTER;
        break;
    #endif /* WAYLAND_KEY_KPENTER */

    #ifdef WAYLAND_KEY_BACK
    case WAYLAND_KEY_BACK:
        standardKeyCode = NATIVEJS_KEY_BACK;
        standardKeyCodeValue = NATIVEJS_KEYCODE_BACK;
        break;
    #endif /* WAYLAND_KEY_BACK */

    #ifdef WAYLAND_KEY_MENU
    case WAYLAND_KEY_MENU:
        standardKeyCode = NATIVEJS_KEY_MENU;
        standardKeyCodeValue = NATIVEJS_KEYCODE_MENU;
        break;
    #endif /* WAYLAND_KEY_MENU */

    #ifdef WAYLAND_KEY_HOMEPAGE
    case WAYLAND_KEY_HOMEPAGE:
        standardKeyCode = NATIVEJS_KEY_HOMEPAGE;
        standardKeyCodeValue = NATIVEJS_KEYCODE_HOMEPAGE;
        break;
    #endif /* WAYLAND_KEY_HOMEPAGE */

    default:
        break;
    }
    keyCode = standardKeyCode;
    keyCodeValue = standardKeyCodeValue;
}

static std::string getJavaScriptKeyValue(uint32_t waylandKeyCode, uint32_t waylandFlags)
{
    std::string standardKeyValue("");
    if (keyMappings.find(waylandKeyCode) != keyMappings.end())
    {
        if (waylandFlags == 0)
        {
            standardKeyValue = keyMappings[waylandKeyCode][0];
        }	    
	else if (waylandFlags & NATIVEJS_FLAGS_SHIFT)
        {
            standardKeyValue = keyMappings[waylandKeyCode][1];
        }
	else if (waylandFlags & NATIVEJS_FLAGS_CONTROL)
        {
            standardKeyValue = keyMappings[waylandKeyCode][2];
        }
	else if (waylandFlags & NATIVEJS_FLAGS_ALT)
        {
            standardKeyValue = keyMappings[waylandKeyCode][3];
        }
    }    
    return standardKeyValue;
}

bool keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, struct JavaScriptKeyDetails& details)
{
    std::cout << "key event - keyCode:" << waylandKeyCode <<  " flags: " << waylandFlags;
/*
    if (keyMappings.find(waylandKeyCode) == keyMappings.end() || keyMappings.find(waylandKeyCode) == keyMappings.end())
    {
        return false;
    }	    
    details.key = keyMappings[waylandKeyCode];
    details.code = keyMappings[waylandKeyCode];
*/
    std::string keyCode;
    uint32_t keyCodeValue = 0;
    getJavaScriptKeyCode(waylandKeyCode, keyCode, keyCodeValue);

    if (keyCode.size() == 0)
    {
        std::cout << "error in key mapping " << std::endl; 
        return false;
    } 
    details.code = keyCode;
    details.keyCode = keyCodeValue;

    std::string keyValue = getJavaScriptKeyValue(waylandKeyCode, waylandFlags);

    if (keyValue.size() == 0)
    {
        std::cout << "error in key value mapping " << std::endl;
        return false;
    }
    details.key = keyValue;
    
    return true;
}
