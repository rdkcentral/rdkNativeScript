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

#include "jsc_lib.h"
#include "JavaScriptEngine.h"
#include "JavaScriptWrapper.h"
#include <cassert>
#include <memory>
#include <map>
#include <set>
#include <array>
#include <unordered_map>
#include <string>
#include <alloca.h>

struct rtObjectWrapperPrivate
{
    rtValue v;
};

static std::unordered_map<rtIObject*, rtJSCWeak> globalWrapperCache;
static bool gEnableWrapperCache = true;
static const char* kIsJSObjectWrapper = "833fba0e-31fd-11e9-b210-d663bd873d93";

static JSValueRef rtObjectWrapper_wrapArray(JSContextRef context, rtObjectRef obj);
static JSValueRef rtObjectWrapper_wrapMap(JSContextRef context, rtObjectRef obj);
static JSValueRef rtObjectWrapper_wrapObject(JSContextRef context, rtObjectRef obj);
static JSValueRef rtFunctionWrapper_wrapFunction(JSContextRef context, rtFunctionRef func);

std::hash<std::string> hashFn;

static bool isJSObjectWrapper(const rtObjectRef& obj)
{
  rtValue value;
  return obj && obj->Get(kIsJSObjectWrapper, &value) == RT_OK;
}

static bool isRtArray(const rtObjectRef& objRef)
{
  if (objRef)
  {
    rtMethodMap* methodMap = objRef.getPtr()->getMap();
    return methodMap && methodMap->className && ((strcmp(methodMap->className, "rtArrayObject") == 0));
  }
  return false;
}

static bool isRtMap(const rtObjectRef& objRef)
{
  if (objRef)
  {
    rtMethodMap* methodMap = objRef.getPtr()->getMap();
    return methodMap && methodMap->className && strcmp(methodMap->className, "rtMapObject") == 0;
  }
  return false;
}

class SymbolIteratorImpl: public rtObject
{
  rtObjectRef mObject;
  uint32_t mCurrent { 0 };
public:
  rtDeclareObject(SymbolIteratorImpl, rtObject);
  rtMethodNoArgAndReturn("next", next, rtValue);

  SymbolIteratorImpl(rtObjectRef o)
    : mObject(o)
  { }

  rtError next(rtValue& result)
  {
    if (!mObject)
      return RT_PROPERTY_NOT_FOUND;

    rtValue lenValue;
    rtError rc = mObject->Get("length", &lenValue);
    if (rc != RT_OK)
      return rc;

    rtObjectRef m = new rtMapObject;

    uint32_t length = lenValue.toUInt32();
    if (mCurrent >= length) {
      m.set("done", true);
      result.setObject(m);
      return RT_OK;
    }

    rtValue v;
    rc = mObject->Get(mCurrent, &v);
    if (rc != RT_OK) {
      m.set("done", true);
      result.setObject(m);
      return rc;
    }

    m.set("done", false);
    m.set("value", v);
    result.setObject(m);

    ++mCurrent;
    return RT_OK;
  }
};
rtDefineObject(SymbolIteratorImpl, rtObject);
rtDefineMethod(SymbolIteratorImpl, next);

class CreateSymbolIteratorCB : public jsruntime::RefCounted<rtIFunction>
{
  rtObjectRef mObject;
  size_t hash() final { return -1;  }
  void setHash(size_t hash) final {  UNUSED_PARAM(hash);  }
  rtError Send(int numArgs, const rtValue* args, rtValue* result) final
  {
    UNUSED_PARAM(numArgs);
    UNUSED_PARAM(args);
    if (result) {
      rtObjectRef o = new SymbolIteratorImpl(mObject);
      *result = rtValue(o);
    }
    return RT_OK;
  }
public:
  CreateSymbolIteratorCB(rtObjectRef ref)
    : mObject(ref)
  {
  }
};

static JSValueRef rtFunctionWrapper_callAsFunction(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  rtValue *v = (rtValue *)JSObjectGetPrivate(function);
  rtFunctionRef funcRef = v->toFunction();
  if (!funcRef) {
    rtLogError("No rt func object");
    return JSValueMakeUndefined(context);
  }

  std::vector<rtValue> args;
  if (argumentCount > 0) {
    args.reserve(argumentCount);
    for (size_t i = 0; i < argumentCount; ++i) {
      rtValue val;
      if (jsToRt(context, arguments[i], val, exception) == RT_OK) {
        args.push_back(val);
      } else {
        rtLogError("Cannot convert js to rt value");
        printException(context, *exception);
        return JSValueMakeUndefined(context);
      }
    }
  }

  rtValue result;
  rtError rc = funcRef.SendReturns(argumentCount, args.data(), result);
  if (rc != RT_OK) {
    rtLogError("SendReturns failed, rc = %d", rc);
    JSStringRef errStr = JSStringCreateWithUTF8CString("rt SendReturns failed");
    *exception = JSValueMakeString(context, errStr);
    JSStringRelease(errStr);
    return JSValueMakeUndefined(context);
  }
  return rtToJs(context, result);
}

static void rtFunctionWrapper_finalize(JSObjectRef thisObject)
{
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(thisObject);
  JSObjectSetPrivate(thisObject, nullptr);
  dispatchOnMainLoop([p=p] { delete p; });
}

static const JSClassDefinition rtFunctionWrapper_class_def =
{
  0,                                // version
  kJSClassAttributeNone,            // attributes
  "__rtFunction__class",            // className
  nullptr,                          // parentClass
  nullptr,                          // staticValues
  nullptr,                          // staticFunctions
  nullptr,                          // initialize
  rtFunctionWrapper_finalize,       // finalize
  nullptr,                          // hasProperty
  nullptr,                          // getProperty
  nullptr,                          // setProperty
  nullptr,                          // deleteProperty
  nullptr,                          // getPropertyNames
  rtFunctionWrapper_callAsFunction, // callAsFunction
  nullptr,                          // callAsConstructor
  nullptr,                          // hasInstance
  nullptr                           // convertToType
};

static JSValueRef rtFunctionWrapper_wrapFunction(JSContextRef context, rtFunctionRef func)
{
  if (!func)
    return JSValueMakeNull(context);
  static JSClassRef classRef = JSClassCreate(&rtFunctionWrapper_class_def);
  rtObjectWrapperPrivate *p = new rtObjectWrapperPrivate;
  p->v.setFunction(func);
  return JSObjectMake(context, classRef, p);
}

static bool rtObjectWrapper_setProperty(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef *exception)
{
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(thisObject);
  rtObjectRef objectRef = p->v.toObject();
  if (!objectRef) {
    JSStringRef errStr = JSStringCreateWithUTF8CString("Not an rt object");
    *exception = JSValueMakeString(context, errStr);
    JSStringRelease(errStr);
    return false;
  }

  rtValue val;
  if (jsToRt(context, value, val, exception) != RT_OK) {
    printException(context, *exception);
    return false;
  }

  rtString name = jsToRtString(propertyName);
  rtError e = objectRef.set(name, val);
  if (e != RT_OK) {
    rtLogDebug("Failed to set property: %s", name.cString());
    return false;
  }
  return true;
}

static JSValueRef rtObjectWrapper_convertToType(JSContextRef context, JSObjectRef object, JSType type, JSValueRef* exception)
{
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(object);
  rtObjectRef objectRef = p->v.toObject();
  if (!objectRef || type != kJSTypeString)
    return JSValueMakeUndefined(context);

  rtString desc;
  rtError e = objectRef.sendReturns<rtString>("description", desc);
  if (e != RT_OK) {
    rtMethodMap* objMap = objectRef->getMap();
    desc = objMap ? objMap->className : "rtIObject";
  }

  rtString resultStr = "[object ";
  resultStr += desc;
  resultStr += "]";

  JSStringRef jsStr = JSStringCreateWithUTF8CString(resultStr.cString());
  JSValueRef jsVal = JSValueMakeString(context, jsStr);
  JSStringRelease(jsStr);
  return jsVal;
}

static JSValueRef rtObjectWrapper_toStringCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
  return rtObjectWrapper_convertToType(context, thisObject, kJSTypeString, exception);
}

static bool shouldIgnoreProperty(JSStringRef propertyName)
{
  static std::array<JSStringRef, 3> ignoreArr = {
    JSStringCreateWithUTF8CString("Symbol.toPrimitive"),
    JSStringCreateWithUTF8CString("valueOf"),
    JSStringCreateWithUTF8CString("toJSON")
  };
  for (const auto& name : ignoreArr)
    if (JSStringIsEqual(name, propertyName))
      return true;
  return false;
}

static bool rtObjectWrapper_hasProperty(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
{
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(object);
  rtObjectRef objectRef = p->v.toObject();
  if (!objectRef)
    return false;

  if (shouldIgnoreProperty(propertyName))
  {
    // rtMethodMap* objMap = objectRef->getMap();
    // const char* className = objMap ? objMap->className : "<unknown>";
    // rtString propName = jsToRtString(propertyName);
    // printf("rtObjectWrapper_hasProperty shouldIgnoreProperty class=%s prop=%s not found\n", className, propName.cString());
    return false;
  }

  rtString propName = jsToRtString(propertyName);
  if (propName.isEmpty())
    return false;

  if (false)
  {
    rtMethodMap* objMap = objectRef->getMap();
    const char* className = objMap ? objMap->className : "<unknown>";
    printf("rtObjectWrapper_hasProperty class=%s prop=%s\n", className, propName.cString());
  }

  if ( !strcmp(propName.cString(), "Symbol.iterator") ||
       !strcmp(propName.cString(), "toString") )
    return true;

  bool isIndex = std::isdigit(*propName.cString());
  do
  {
    if ( isIndex )
      break;

    rtMethodMap* objMap = objectRef->getMap();
    if ( !objMap || !objMap->className )
      break;

    if ( !strcmp(objMap->className, "rtMapObject") ||
         !strcmp(objMap->className, "rtArrayObject"))
      break;

    bool found = false;
    for ( rtMethodMap* m = objMap; m && !found; m = m->parentsMap )
    {
      rtPropertyEntry* p = m->getFirstProperty();
      while(p && !found)
      {
        if (!strcmp(p->mPropertyName, propName.cString()))
          found = true;
        p = p->mNext;
      }
    }

    for ( rtMethodMap* m = objMap; m && !found; m = m->parentsMap )
    {
      rtMethodEntry* e = m->getFirstMethod();
      while(e && !found)
      {
        if (!strcmp(e->mMethodName, propName.cString()))
          found = true;
        e = e->mNext;
      }
    }
    return found;
  } while (0);

  // {
  //   rtMethodMap* objMap = objectRef->getMap();
  //   printf("%s hasProperty took slow path\n", objMap ? objMap->className : "rtIObject");
  // }

  rtValue ignore;
  rtError rc;
  if (isIndex) {
    uint32_t idx = std::stoul(propName.cString());
    rc = objectRef->Get(idx, &ignore);
  } else {
    rc = objectRef->Get(propName.cString(), &ignore);
  }
  return rc == RT_OK;
}

static JSValueRef rtObjectWrapper_getProperty(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef *exception)
{
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(thisObject);
  rtObjectRef objectRef = p->v.toObject();
  if (!objectRef)
    return nullptr;

  if (shouldIgnoreProperty(propertyName))
    return nullptr;

  rtString propName = jsToRtString(propertyName);
  if (propName.isEmpty())
    return nullptr;

  if (false)
  {
    rtMethodMap* objMap = objectRef->getMap();
    const char* className = objMap ? objMap->className : "<unknown>";
    printf("rtObjectWrapper_getProperty class=%s prop=%s\n", className, propName.cString());
  }

  if (!strcmp(propName.cString(), "Symbol.iterator")) {
    rtFunctionRef cb = new CreateSymbolIteratorCB(objectRef);
    return rtToJs(context, rtValue(cb));
  }

  if (!strcmp(propName.cString(), "toString"))
    return JSObjectMakeFunctionWithCallback(context, nullptr, rtObjectWrapper_toStringCallback);

  rtValue v;
  rtError e = RT_OK;
  if (std::isdigit(*propName.cString())) {
    uint32_t idx = std::stoul(propName.cString());
    e = objectRef->Get(idx, &v);
  } else {
    e = objectRef->Get(propName.cString(), &v);
  }

  if (e != RT_OK) {
    rtLogWarn("rtObjectWrapper_getProperty failed name=%s, err=%d", propName.cString(), e);
    return nullptr;
  }

  return rtToJs(context, v);
}

static void rtObjectWrapper_getPropertyNames(JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames)
{
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(object);
  rtObjectRef objectRef = p->v.toObject();
  if (!objectRef)
    return;

  rtValue allKeys;
  rtError rc = objectRef->Get("allKeys", &allKeys);
  if (rc != RT_OK)
  {
    rtValue length;
    rc = objectRef->Get("length", &length);
    if (rc != RT_OK)
      return;
    uint32_t l = length.toUInt32();
    for (uint32_t i = 0; i < l; ++i)
    {
      JSStringRef jsStr = JSStringCreateWithUTF8CString(std::to_string(i).c_str());
      JSPropertyNameAccumulatorAddName(propertyNames, jsStr);
      JSStringRelease(jsStr);
    }
    return;
  }

  rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();
  if (!arr)
    return;

  for (uint32_t i = 0, l = arr->length(); i < l; ++i)
  {
    rtValue key;
    if (arr->Get(i, &key) == RT_OK && !key.isEmpty())
    {
      JSStringRef jsStr = JSStringCreateWithUTF8CString(key.toString().cString());
      JSPropertyNameAccumulatorAddName(propertyNames, jsStr);
      JSStringRelease(jsStr);
    }
  }
}

static void rtObjectWrapper_finalize(JSObjectRef thisObject)
{
  assertIsMainThread();
  rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(thisObject);
  JSObjectSetPrivate(thisObject, nullptr);

  if (gEnableWrapperCache)
  {
    rtIObject* ptr = p->v.toObject().getPtr();
    auto it = globalWrapperCache.find(ptr);
    if (it != globalWrapperCache.end()) {
      globalWrapperCache.erase(it);
    }
  }

  dispatchOnMainLoop([p=p] {
      {
        rtObjectRef temp = p->v.toObject();
        rtObjectRef parentRef;
        rtError err = temp.get<rtObjectRef>("parent",parentRef);
        if (err == RT_OK)
        {
          if (!parentRef)
          {
            temp.send("dispose");
          }
        }
      }

      delete p;
    });
}

static const JSClassDefinition rtObjectWrapper_class_def =
{
  0,                                 // version
  kJSClassAttributeNone,             // attributes
  "__rtObject__class",               // className
  nullptr,                           // parentClass
  nullptr,                           // staticValues
  nullptr,                           // staticFunctions
  nullptr,                           // initialize
  rtObjectWrapper_finalize,          // finalize
  rtObjectWrapper_hasProperty,       // hasProperty
  rtObjectWrapper_getProperty,       // getProperty
  rtObjectWrapper_setProperty,       // setProperty
  nullptr,                           // deleteProperty
  rtObjectWrapper_getPropertyNames,  // getPropertyNames
  nullptr,                           // callAsFunction
  nullptr,                           // callAsConstructor
  nullptr,                           // hasInstance
  rtObjectWrapper_convertToType      // convertToType
};

static JSValueRef rtObjectWrapper_wrapArray(JSContextRef context, rtObjectRef obj)
{
  if (!obj)
    return JSValueMakeNull(context);

  assert(isRtArray(obj));

  rtValue length;
  obj->Get("length", &length);
  uint32_t n = length.toUInt32();

  std::vector<JSValueRef> args;
  for (uint32_t i = 0; i < n; ++i) {
    rtValue value;
    obj->Get(i, &value);
    args.push_back(rtToJs(context, value));
  }

  return JSObjectMakeArray(context, n, args.data(), nullptr);
}

static JSValueRef rtObjectWrapper_wrapMap(JSContextRef context, rtObjectRef obj)
{
  if (!obj)
    return JSValueMakeNull(context);

  assert(isRtMap(obj));

  rtValue allKeys;
  obj->Get("allKeys", &allKeys);
  rtObjectRef arr = allKeys.toObject();
  rtValue length;
  arr->Get("length", &length);
  uint32_t l = length.toUInt32();

  JSObjectRef object = JSObjectMake(context, nullptr, nullptr);
  for (uint32_t i = 0; i < l; ++i) {
    rtValue key;
    arr->Get(i, &key);
    rtString str = key.toString();
    rtValue value;
    obj->Get(str.cString(), &value);

    JSStringRef name = JSStringCreateWithUTF8CString(str.cString());
    JSObjectSetProperty(context, object, name, rtToJs(context, value), kJSPropertyAttributeNone, nullptr);
    JSStringRelease(name);
  }
  return object;
}

static JSValueRef rtObjectWrapper_wrapObject(JSContextRef context, rtObjectRef obj)
{
  if (!obj)
    return JSValueMakeNull(context);

  if (isRtArray(obj))
    return rtObjectWrapper_wrapArray(JSContextGetGlobalContext(context), obj);
  if (isRtMap(obj))
    return rtObjectWrapper_wrapMap(JSContextGetGlobalContext(context), obj);

  if (gEnableWrapperCache)
  {
    rtIObject* ptr = obj.getPtr();
    auto it = globalWrapperCache.find(ptr);
    if (it != globalWrapperCache.end()) {
      JSObjectRef res = it->second.wrapped();
      if (!res) {
        globalWrapperCache.erase(it);
      } else {
        return res;
      }
    }
  }

  static JSClassRef sClassRef = JSClassCreate(&rtObjectWrapper_class_def);

  rtObjectWrapperPrivate *p = new rtObjectWrapperPrivate;
  p->v.setObject(obj);
  JSObjectRef res = JSObjectMake(context, sClassRef, p);

  if (gEnableWrapperCache)
  {
    globalWrapperCache[obj.getPtr()] = rtJSCWeak(context, res);
  }

  return res;
}

rtError jsToRt(JSContextRef context, JSValueRef valueRef, rtValue &result, JSValueRef *exception)
{
  assertIsMainThread();
  static const auto convertObject =
    [](JSContextRef ctx, JSValueRef valueRef, rtValue &result, JSValueRef &exc) {
      if (JSValueIsDate(ctx, valueRef)) {
        JSStringRef str = JSValueToStringCopy(ctx, valueRef, &exc);
        result.setString(jsToRtString(str));
        JSStringRelease(str);
        return;
      }

      JSObjectRef objectRef = JSValueToObject(ctx, valueRef, &exc);
      if (exc)
        return;

      rtObjectWrapperPrivate *p = (rtObjectWrapperPrivate *)JSObjectGetPrivate(objectRef);
      if (p) {
        result = p->v;
        return;
      }

      if (JSObjectIsFunction(ctx, objectRef)) {
        rtFunctionRef callback = new JSFunctionWrapper(ctx, objectRef);
        result = rtValue(callback);
        return;
      }
      
      rtObjectRef obj = new JSObjectWrapper(ctx, objectRef, JSValueIsArray(ctx, valueRef));
      result.setObject(obj);
    };

  JSValueRef exc = nullptr;
  JSType type = JSValueGetType(context, valueRef);
  switch (type)
  {
    case kJSTypeUndefined:
    case kJSTypeNull:
    {
      result.setEmpty();
      break;
    }
    case kJSTypeBoolean:
    {
      result.setBool(JSValueToBoolean(context, valueRef));
      break;
    }
    case kJSTypeNumber:
    {
      result.setDouble(JSValueToNumber(context, valueRef, &exc));
      break;
    }
    case kJSTypeString:
    {
      JSStringRef str = JSValueToStringCopy(context, valueRef, &exc);
      result.setString(jsToRtString(str));
      JSStringRelease(str);
      break;
    }
    case kJSTypeObject:
    {
      convertObject(context, valueRef, result, exc);
      break;
    }
    default:
    {
      JSStringRef str = JSStringCreateWithUTF8CString("Unknown value type!");
      exc = JSValueMakeString(context, str);
      JSStringRelease(str);
      break;
    }
  }

  if (exception)
    *exception = exc;
  return exc ? RT_FAIL : RT_OK;
}

JSValueRef rtToJs(JSContextRef context, const rtValue &v)
{
  assertIsMainThread();

  if (!context) {
    rtLogWarn("Lost JS context!");
    return nullptr;
  }

  if (v.isEmpty())
  {
    return JSValueMakeUndefined(context); // return JSValueMakeNull(context);
  }

  JSValueRef jsVal = nullptr;

  switch(v.getType())
  {
    case RT_objectType:
    {
      rtObjectRef o = v.toObject();
      if (o && !o->getMap() && isJSObjectWrapper(o)) {
        JSObjectWrapper* wrapper = static_cast<JSObjectWrapper*>(o.getPtr());
        if (JSContextGetGroup(wrapper->context()) == JSContextGetGroup(JSContextGetGlobalContext(context))) {
          jsVal = wrapper->wrapped();
        }
      }
      if (!jsVal) {
        jsVal = rtObjectWrapper_wrapObject(context, o);
      }
      break;
    }
    case RT_functionType:
    {
      jsVal = rtFunctionWrapper_wrapFunction(context, v.toFunction());
      break;
    }
    case RT_voidPtrType:
    case RT_voidType:
    {
      jsVal = JSValueMakeUndefined(context);
      break;
    }
    case RT_int32_tType:
    case RT_uint32_tType:
    case RT_int64_tType:
    case RT_floatType:
    case RT_doubleType:
    case RT_uint64_tType:
    {
      jsVal = JSValueMakeNumber(context, v.toDouble());
      break;
    }
    case RT_boolType:
    {
      jsVal = JSValueMakeBoolean(context, v.toBool());
      break;
    }
    case RT_rtStringType:
    default:
    {
      JSStringRef jsStr = JSStringCreateWithUTF8CString(v.toString().cString());
      jsVal = JSValueMakeString(context, jsStr);
      JSStringRelease(jsStr);
      break;
    }
  }
  return jsVal;
}

JSObjectWrapper::JSObjectWrapper(JSContextRef context, JSObjectRef object, bool isArray)
  : rtJSCWrapperBase(context, object)
  , m_isArray(isArray)
{
  assertIsMainThread();
}

JSObjectWrapper::~JSObjectWrapper()
{
  assertIsMainThread();
}

rtError JSObjectWrapper::Get(const char* name, rtValue* value) const
{
  assertIsMainThread();
  if (!context() || !wrapped()) {
    rtLogWarn("Lost JS context! [%s] [%p] [%p]", name, context(), wrapped());
    return RT_FAIL;
  }

  if (!name || !value)
    return RT_ERROR_INVALID_ARG;

  if (strcmp(name, kIsJSObjectWrapper) == 0)
    return RT_OK;

  if (strcmp(name, "description") == 0)
    // FIXME: this should return a function
    return RT_PROP_NOT_FOUND;

  JSValueRef exc = nullptr;
  if (strcmp(name, "allKeys") == 0) {
    rtArrayObject* array = new rtArrayObject;
    JSPropertyNameArrayRef namesRef = JSObjectCopyPropertyNames(context(), wrapped());
    size_t size = JSPropertyNameArrayGetCount(namesRef);
    for (size_t i = 0; i < size; ++i) {
      JSStringRef namePtr = JSPropertyNameArrayGetNameAtIndex(namesRef, i);
      array->pushBack(jsToRtString(namePtr));
    }
    JSPropertyNameArrayRelease(namesRef);
    value->setObject(array);
    return RT_OK;
  }

  if (strcmp(name, "arrayData") == 0) {
    JSTypedArrayType arrType = JSValueGetTypedArrayType(context(), wrapped(), nullptr);
    if (arrType != kJSTypedArrayTypeNone) {
      value->setVoidPtr(JSObjectGetTypedArrayBytesPtr(context(), wrapped(), &exc));
      if (exc) {
        printException(context(), exc);
        return RT_FAIL;
      }
      return RT_OK;
    }
  }

  JSStringRef namePtr = JSStringCreateWithUTF8CString(name);
  JSValueRef valueRef = JSObjectGetProperty(context(), wrapped(), namePtr, &exc);
  JSStringRelease(namePtr);
  if (exc) {
    printException(context(), exc);
    return RT_FAIL;
  }

  // FIXME: a special case to capture 'this', figure out if it is really required
  if (!m_isArray && JSValueGetType(context(), valueRef) == kJSTypeObject) {
    JSObjectRef objectRef = JSValueToObject(context(), valueRef, &exc);
    if (exc) {
      printException(context(), exc);
      return RT_FAIL;
    }
    if (JSObjectIsFunction(context(), objectRef)) {
      value->setFunction(new JSFunctionWrapper(context(), wrapped(), objectRef));
      return RT_OK;
    }
  }

  rtError ret = jsToRt(context(), valueRef, *value, &exc);
  if (exc) {
    printException(context(), exc);
    return RT_FAIL;
  }
  return ret;
}

rtError JSObjectWrapper::Get(uint32_t i, rtValue* value) const
{
  assertIsMainThread();
  if (!value)
    return RT_ERROR_INVALID_ARG;
  JSValueRef exc = nullptr;
  JSValueRef valueRef = JSObjectGetPropertyAtIndex(context(), wrapped(), i, &exc);
  if (exc) {
    printException(context(), exc);
    return RT_FAIL;
  }
  rtError ret = jsToRt(context(), valueRef, *value, &exc);
  if (exc) {
    printException(context(), exc);
    return RT_FAIL;
  }
  return ret;
}

rtError JSObjectWrapper::Set(const char* name, const rtValue* value)
{
  assertIsMainThread();
  if (!context()) {
    rtLogWarn("Lost JS context!");
    return RT_FAIL;
  }
  if (!name || !value)
    return RT_FAIL;
  if (m_isArray)
    return RT_PROP_NOT_FOUND;
  JSValueRef valueRef = rtToJs(context(), *value);
  JSValueRef exc = nullptr;
  JSStringRef namePtr = JSStringCreateWithUTF8CString(name);
  JSObjectSetProperty(context(), wrapped(), namePtr, valueRef, kJSPropertyAttributeNone, &exc);
  JSStringRelease(namePtr);
  if (exc) {
    printException(context(), exc);
    return RT_FAIL;
  }
  return RT_OK;
}

rtError JSObjectWrapper::Set(uint32_t i, const rtValue* value)
{
  assertIsMainThread();
  if (!value)
    return RT_FAIL;
  JSValueRef valueRef = rtToJs(context(), *value);
  JSValueRef exc = nullptr;
  JSObjectSetPropertyAtIndex(context(), wrapped(), i, valueRef, &exc);
  if (exc) {
    printException(context(), exc);
    return RT_FAIL;
  }
  return RT_OK;
}

JSFunctionWrapper::JSFunctionWrapper(JSContextRef context, JSObjectRef thisObj, JSObjectRef funcObj)
  : rtJSCWrapperBase(context, funcObj)
  , m_thisObj(context, thisObj)
  , mHash(-1)
{
  assertIsMainThread();

  JSStringRef str = JSValueToStringCopy(context, funcObj, nullptr);
  mHash = hashFn(jsToRtString(str).cString());
  JSStringRelease(str);
}

JSFunctionWrapper::JSFunctionWrapper(JSContextRef context, JSObjectRef funcObj)
  : rtJSCWrapperBase(context, funcObj)
  , mHash(-1)
{
  assertIsMainThread();

  JSStringRef str = JSValueToStringCopy(context, funcObj, nullptr);
  mHash = hashFn(jsToRtString(str).cString());
  JSStringRelease(str);
}

JSFunctionWrapper::~JSFunctionWrapper()
{
  assertIsMainThread();
}

rtError JSFunctionWrapper::Send(int numArgs, const rtValue* args, rtValue* result)
{
  assertIsMainThread();
  if (!context() || !wrapped()) {
    rtLogWarn("Lost JS context!");
    return RT_FAIL;
  }
  if (numArgs > 10) {
    rtLogWarn("Too many arguments!");
    return RT_FAIL;
  }
  JSValueRef jsArgs[10] = { 0 };
  if (numArgs) {
    for (int i = 0; i < numArgs; ++i) {
      const rtValue &rtVal = args[i];
      jsArgs[i] = rtToJs(context(), rtVal);
    }
  }
  if (result) {
    JSValueRef exception = nullptr;
    JSValueRef jsResult = JSObjectCallAsFunction(context(), wrapped(), m_thisObj.wrapped(), numArgs, jsArgs,
                                                 &exception);
    if (exception) {
      printException(context(), exception);
      return RT_FAIL;
    }
    rtError ret = RT_OK;
    ret = jsToRt(context(), jsResult, *result, &exception);
    if (exception) {
      printException(context(), exception);
      return RT_FAIL;
    }
    return ret;
  } else {
    dispatchOnMainLoop(
        [context = context(), object = wrapped(), thisObject = m_thisObj.wrapped(), numArgs, jsArgs] () mutable {
          JSValueRef exception = nullptr;
          JSObjectCallAsFunction(context, object, thisObject, numArgs, jsArgs, &exception);
          if (exception) {
            printException(context, exception);
          }
        });
    return RT_OK;
  }
}
