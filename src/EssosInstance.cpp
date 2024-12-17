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

#include "EssosInstance.h"
#include "KeyInput.h"

#include <iostream>

EssInputDeviceMetadata gInputDeviceMetadata = {};

EssosInstance* EssosInstance::mInstance = NULL;

static void essosTerminated( void * )
{
}

static EssTerminateListener essosTerminateListener=
{
    essosTerminated
};

static bool rightShiftPressed = false;
static bool leftShiftPressed = false;
static bool rightAltPressed = false;
static bool leftAltPressed = false;
static bool rightCtrlPressed = false;
static bool leftCtrlPressed = false;

static void processKeyEvent(bool pressEvent, unsigned int key, void *metadata)
{
    struct JavaScriptKeyDetails details;
    bool isSpecialKeyPressed = false;
    switch( key )
    {
        case WAYLAND_KEY_RIGHTSHIFT:
            isSpecialKeyPressed = true;
            rightShiftPressed = pressEvent ? true : false;
        break;
        case WAYLAND_KEY_LEFTSHIFT:
            isSpecialKeyPressed = true;
            leftShiftPressed = pressEvent ? true : false;
        break;
        case WAYLAND_KEY_RIGHTCTRL:
            isSpecialKeyPressed = true;
            rightCtrlPressed = pressEvent ? true : false;
        break;
        case WAYLAND_KEY_LEFTCTRL:
            isSpecialKeyPressed = true;
            leftCtrlPressed = pressEvent ? true : false;
        break;
        case WAYLAND_KEY_RIGHTALT:
            rightAltPressed = pressEvent ? true : false;
        break;
        case WAYLAND_KEY_LEFTALT:
            isSpecialKeyPressed = true;
            leftAltPressed = pressEvent ? true : false;
        break;
        default:
            break;
    }
    unsigned long flags = 0;
    if (rightShiftPressed || leftShiftPressed)
    {
        details.shiftKey = true;
        flags |= NATIVEJS_FLAGS_SHIFT;
    }
    if (rightCtrlPressed || leftCtrlPressed)
    {
        details.ctrlKey = true;
        flags |= NATIVEJS_FLAGS_CONTROL;
    }
    if (rightAltPressed || leftAltPressed)
    {
        details.altKey = true;
        flags |= NATIVEJS_FLAGS_ALT;
    }	    

    bool ret = keyCodeFromWayland(key, flags, details);

    if (!isSpecialKeyPressed)
    {	    
        if (pressEvent)
        {
            details.type = "keydown";
            EssosInstance::instance()->onKeyPress(details);
        }
        else
        {
            details.type = "keyup";
            EssosInstance::instance()->onKeyRelease(details);
        }
    }
}

static void essosKeyAndMetadataPressed( void *userData, unsigned int key, EssInputDeviceMetadata *metadata )
{
    processKeyEvent(true, key, metadata);
}

static void essosKeyAndMetadataReleased( void *userData, unsigned int key, EssInputDeviceMetadata *metadata )
{
    processKeyEvent(false, key, metadata);
}

static EssKeyAndMetadataListener essosKeyAndMetadataListener=
{
    essosKeyAndMetadataPressed,
    essosKeyAndMetadataReleased
};

static void essosKeyPressed( void *userData, unsigned int key )
{
    processKeyEvent(true, key, nullptr);
}

static void essosKeyReleased( void *userData, unsigned int key )
{
    processKeyEvent(false, key, nullptr);
}

static EssKeyListener essosKeyListener=
{
    essosKeyPressed,
    essosKeyReleased
};

EssosInstance::EssosInstance() : mEssosContext(NULL), mUseWayland(false), mJavaScriptKeyListener(nullptr)
{
}

EssosInstance::~EssosInstance()
{
    if (mEssosContext)
    {
        EssContextDestroy(mEssosContext);
    }
    mEssosContext = NULL;
}

EssosInstance* EssosInstance::instance()
{
    if (mInstance == NULL)
    {
        mInstance = new EssosInstance();
    }
    return mInstance;
}

void EssosInstance::registerKeyListener(JavaScriptKeyListener* listener)
{
    mJavaScriptKeyListener = listener;
}

bool EssosInstance::initialize(bool useWayland)
{
    mUseWayland = useWayland;
    mEssosContext = EssContextCreate();
    bool essosError = false;
    if (mEssosContext)
    {
        if ( !EssContextSetUseWayland( mEssosContext, mUseWayland ) )
        {
            essosError = true;
        }
        if ( !EssContextSetTerminateListener( mEssosContext, 0, &essosTerminateListener ) )
        {
            essosError = true;
        }
        if ( !EssContextSetKeyListener( mEssosContext, 0, &essosKeyListener ) )
        {
            essosError = true;
        }
        if ( !essosError )
        {
            if ( !EssContextStart( mEssosContext ) )
            {
                essosError = true;
            }
        }
        if ( essosError )
        {
            const char *errorDetail = EssContextGetLastErrorDetail(mEssosContext);
	    std::cout << "Essos error during initialization: " <<  errorDetail;
        }
    }
    return !essosError;
}

void EssosInstance::onKeyPress(struct JavaScriptKeyDetails& details)
{
    if (mJavaScriptKeyListener)
    {
        mJavaScriptKeyListener->onKeyPress(details);	    
    }
}

void EssosInstance::onKeyRelease(struct JavaScriptKeyDetails& details)
{
    if (mJavaScriptKeyListener)
    {
        mJavaScriptKeyListener->onKeyRelease(details);	    
    }
}

void EssosInstance::update()
{
    if (mEssosContext)
    {
        EssContextRunEventLoopOnce(mEssosContext);
    }
}
