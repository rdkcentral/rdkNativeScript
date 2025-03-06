var EventLib;
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ 629:
/***/ ((module, __unused_webpack_exports, __webpack_require__) => {



const idlUtils = __webpack_require__(288);
const EventInit = __webpack_require__(164);

class EventImpl {
  constructor(globalObject, args, privateData) {
    const [type, eventInitDict = this.constructor.defaultInit] = args;

    this.type = type;

    this.bubbles = false;
    this.cancelable = false;
    for (const key in eventInitDict) {
      if (key in this.constructor.defaultInit) {
        this[key] = eventInitDict[key];
      }
    }
    for (const key in this.constructor.defaultInit) {
      if (!(key in this)) {
        this[key] = this.constructor.defaultInit[key];
      }
    }

    this.target = null;
    this.currentTarget = null;
    this.eventPhase = 0;

    this._globalObject = globalObject;
    this._initializedFlag = true;
    this._stopPropagationFlag = false;
    this._stopImmediatePropagationFlag = false;
    this._canceledFlag = false;
    this._inPassiveListenerFlag = false;
    this._dispatchFlag = false;
    this._path = [];

    this.isTrusted = privateData.isTrusted || false;
    this.timeStamp = Date.now();
  }

  // https://dom.spec.whatwg.org/#set-the-canceled-flag
  _setTheCanceledFlag() {
    if (this.cancelable && !this._inPassiveListenerFlag) {
      this._canceledFlag = true;
    }
  }

  get srcElement() {
    return this.target;
  }

  get returnValue() {
    return !this._canceledFlag;
  }

  set returnValue(v) {
    if (v === false) {
      this._setTheCanceledFlag();
    }
  }

  get defaultPrevented() {
    return this._canceledFlag;
  }

  stopPropagation() {
    this._stopPropagationFlag = true;
  }

  get cancelBubble() {
    return this._stopPropagationFlag;
  }

  set cancelBubble(v) {
    if (v) {
      this._stopPropagationFlag = true;
    }
  }

  stopImmediatePropagation() {
    this._stopPropagationFlag = true;
    this._stopImmediatePropagationFlag = true;
  }

  preventDefault() {
    this._setTheCanceledFlag();
  }

  // https://dom.spec.whatwg.org/#dom-event-composedpath
  // Current implementation is based of https://whatpr.org/dom/699.html#dom-event-composedpath
  // due to a bug in composed path implementation https://github.com/whatwg/dom/issues/684
  composedPath() {
    const composedPath = [];

    const { currentTarget, _path: path } = this;

    if (path.length === 0) {
      return composedPath;
    }

    composedPath.push(currentTarget);

    let currentTargetIndex = 0;
    let currentTargetHiddenSubtreeLevel = 0;

    for (let index = path.length - 1; index >= 0; index--) {
      const { item, rootOfClosedTree, slotInClosedTree } = path[index];

      if (rootOfClosedTree) {
        currentTargetHiddenSubtreeLevel++;
      }

      if (item === idlUtils.implForWrapper(currentTarget)) {
        currentTargetIndex = index;
        break;
      }

      if (slotInClosedTree) {
        currentTargetHiddenSubtreeLevel--;
      }
    }

    let currentHiddenLevel = currentTargetHiddenSubtreeLevel;
    let maxHiddenLevel = currentTargetHiddenSubtreeLevel;

    for (let i = currentTargetIndex - 1; i >= 0; i--) {
      const { item, rootOfClosedTree, slotInClosedTree } = path[i];

      if (rootOfClosedTree) {
        currentHiddenLevel++;
      }

      if (currentHiddenLevel <= maxHiddenLevel) {
        composedPath.unshift(idlUtils.wrapperForImpl(item));
      }

      if (slotInClosedTree) {
        currentHiddenLevel--;
        if (currentHiddenLevel < maxHiddenLevel) {
          maxHiddenLevel = currentHiddenLevel;
        }
      }
    }

    currentHiddenLevel = currentTargetHiddenSubtreeLevel;
    maxHiddenLevel = currentTargetHiddenSubtreeLevel;

    for (let index = currentTargetIndex + 1; index < path.length; index++) {
      const { item, rootOfClosedTree, slotInClosedTree } = path[index];

      if (slotInClosedTree) {
        currentHiddenLevel++;
      }

      if (currentHiddenLevel <= maxHiddenLevel) {
        composedPath.push(idlUtils.wrapperForImpl(item));
      }

      if (rootOfClosedTree) {
        currentHiddenLevel--;
        if (currentHiddenLevel < maxHiddenLevel) {
          maxHiddenLevel = currentHiddenLevel;
        }
      }
    }

    return composedPath;
  }

  _initialize(type, bubbles, cancelable) {
    this.type = type;
    this._initializedFlag = true;

    this._stopPropagationFlag = false;
    this._stopImmediatePropagationFlag = false;
    this._canceledFlag = false;

    this.isTrusted = false;
    this.target = null;
    this.bubbles = bubbles;
    this.cancelable = cancelable;
  }

  initEvent(type, bubbles, cancelable) {
    if (this._dispatchFlag) {
      return;
    }

    this._initialize(type, bubbles, cancelable);
  }
}
EventImpl.defaultInit = EventInit.convert(undefined);

module.exports = {
  implementation: EventImpl
};


/***/ }),

/***/ 674:
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {



const conversions = __webpack_require__(341);
const utils = __webpack_require__(288);

const convertEventInit = (__webpack_require__(164).convert);
const impl = utils.implSymbol;
const ctorRegistry = utils.ctorRegistrySymbol;

/**
 * When an interface-module that implements this interface as a mixin is loaded, it will append its own `.is()`
 * method into this array. It allows objects that directly implements *those* interfaces to be recognized as
 * implementing this mixin interface.
 */
exports._mixedIntoPredicates = [];
exports.is = function is(obj) {
  if (obj) {
    if (utils.hasOwn(obj, impl) && obj[impl] instanceof Impl.implementation) {
      return true;
    }
    for (const isMixedInto of exports._mixedIntoPredicates) {
      if (isMixedInto(obj)) {
        return true;
      }
    }
  }
  return false;
};
exports.isImpl = function isImpl(obj) {
  if (obj) {
    if (obj instanceof Impl.implementation) {
      return true;
    }

    const wrapper = utils.wrapperForImpl(obj);
    for (const isMixedInto of exports._mixedIntoPredicates) {
      if (isMixedInto(wrapper)) {
        return true;
      }
    }
  }
  return false;
};
exports.convert = function convert(obj, { context = "The provided value" } = {}) {
  if (exports.is(obj)) {
    return utils.implForWrapper(obj);
  }
  throw new TypeError(`${context} is not of type 'Event'.`);
};

exports.create = function create(globalObject, constructorArgs, privateData) {
  if (globalObject[ctorRegistry] === undefined) {
    throw new Error("Internal error: invalid global object");
  }

  const ctor = globalObject[ctorRegistry]["Event"];
  if (ctor === undefined) {
    throw new Error("Internal error: constructor Event is not installed on the passed global object");
  }

  let obj = Object.create(ctor.prototype);
  obj = exports.setup(obj, globalObject, constructorArgs, privateData);
  return obj;
};
exports.createImpl = function createImpl(globalObject, constructorArgs, privateData) {
  const obj = exports.create(globalObject, constructorArgs, privateData);
  return utils.implForWrapper(obj);
};
exports._internalSetup = function _internalSetup(obj) {
  Object.defineProperties(
    obj,
    Object.getOwnPropertyDescriptors({
      get isTrusted() {
        if (!this || !exports.is(this)) {
          throw new TypeError("Illegal invocation");
        }

        return obj[impl]["isTrusted"];
      }
    })
  );

  Object.defineProperties(obj, { isTrusted: { configurable: false } });
};
exports.setup = function setup(obj, globalObject, constructorArgs = [], privateData = {}) {
  privateData.wrapper = obj;

  exports._internalSetup(obj);
  Object.defineProperty(obj, impl, {
    value: new Impl.implementation(globalObject, constructorArgs, privateData),
    configurable: true
  });

  obj[impl][utils.wrapperSymbol] = obj;
  if (Impl.init) {
    Impl.init(obj[impl], privateData);
  }
  return obj;
};

exports.install = function install(globalObject) {
  class Event {
    constructor(type) {
      if (arguments.length < 1) {
        throw new TypeError(
          "Failed to construct 'Event': 1 argument required, but only " + arguments.length + " present."
        );
      }
      const args = [];
      {
        let curArg = arguments[0];
        curArg = conversions["DOMString"](curArg, { context: "Failed to construct 'Event': parameter 1" });
        args.push(curArg);
      }
      {
        let curArg = arguments[1];
        curArg = convertEventInit(curArg, { context: "Failed to construct 'Event': parameter 2" });
        args.push(curArg);
      }
      return exports.setup(Object.create(new.target.prototype), globalObject, args);
    }

    composedPath() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return utils.tryWrapperForImpl(this[impl].composedPath());
    }

    stopPropagation() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl].stopPropagation();
    }

    stopImmediatePropagation() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl].stopImmediatePropagation();
    }

    preventDefault() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl].preventDefault();
    }

    initEvent(type) {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      if (arguments.length < 1) {
        throw new TypeError(
          "Failed to execute 'initEvent' on 'Event': 1 argument required, but only " + arguments.length + " present."
        );
      }
      const args = [];
      {
        let curArg = arguments[0];
        curArg = conversions["DOMString"](curArg, { context: "Failed to execute 'initEvent' on 'Event': parameter 1" });
        args.push(curArg);
      }
      {
        let curArg = arguments[1];
        if (curArg !== undefined) {
          curArg = conversions["boolean"](curArg, { context: "Failed to execute 'initEvent' on 'Event': parameter 2" });
        } else {
          curArg = false;
        }
        args.push(curArg);
      }
      {
        let curArg = arguments[2];
        if (curArg !== undefined) {
          curArg = conversions["boolean"](curArg, { context: "Failed to execute 'initEvent' on 'Event': parameter 3" });
        } else {
          curArg = false;
        }
        args.push(curArg);
      }
      return this[impl].initEvent(...args);
    }

    get type() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["type"];
    }

    get target() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return utils.tryWrapperForImpl(this[impl]["target"]);
    }

    get srcElement() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return utils.tryWrapperForImpl(this[impl]["srcElement"]);
    }

    get currentTarget() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return utils.tryWrapperForImpl(this[impl]["currentTarget"]);
    }

    get eventPhase() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["eventPhase"];
    }

    get cancelBubble() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["cancelBubble"];
    }

    set cancelBubble(V) {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      V = conversions["boolean"](V, {
        context: "Failed to set the 'cancelBubble' property on 'Event': The provided value"
      });

      this[impl]["cancelBubble"] = V;
    }

    get bubbles() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["bubbles"];
    }

    get cancelable() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["cancelable"];
    }

    get returnValue() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["returnValue"];
    }

    set returnValue(V) {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      V = conversions["boolean"](V, {
        context: "Failed to set the 'returnValue' property on 'Event': The provided value"
      });

      this[impl]["returnValue"] = V;
    }

    get defaultPrevented() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["defaultPrevented"];
    }

    get composed() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["composed"];
    }

    get timeStamp() {
      if (!this || !exports.is(this)) {
        throw new TypeError("Illegal invocation");
      }

      return this[impl]["timeStamp"];
    }
  }
  Object.defineProperties(Event.prototype, {
    composedPath: { enumerable: true },
    stopPropagation: { enumerable: true },
    stopImmediatePropagation: { enumerable: true },
    preventDefault: { enumerable: true },
    initEvent: { enumerable: true },
    type: { enumerable: true },
    target: { enumerable: true },
    srcElement: { enumerable: true },
    currentTarget: { enumerable: true },
    eventPhase: { enumerable: true },
    cancelBubble: { enumerable: true },
    bubbles: { enumerable: true },
    cancelable: { enumerable: true },
    returnValue: { enumerable: true },
    defaultPrevented: { enumerable: true },
    composed: { enumerable: true },
    timeStamp: { enumerable: true },
    [Symbol.toStringTag]: { value: "Event", configurable: true },
    NONE: { value: 0, enumerable: true },
    CAPTURING_PHASE: { value: 1, enumerable: true },
    AT_TARGET: { value: 2, enumerable: true },
    BUBBLING_PHASE: { value: 3, enumerable: true }
  });
  Object.defineProperties(Event, {
    NONE: { value: 0, enumerable: true },
    CAPTURING_PHASE: { value: 1, enumerable: true },
    AT_TARGET: { value: 2, enumerable: true },
    BUBBLING_PHASE: { value: 3, enumerable: true }
  });
  if (globalObject[ctorRegistry] === undefined) {
    globalObject[ctorRegistry] = Object.create(null);
  }
  globalObject[ctorRegistry]["Event"] = Event;

  Object.defineProperty(globalObject, "Event", {
    configurable: true,
    writable: true,
    value: Event
  });
};

const Impl = __webpack_require__(629);


/***/ }),

/***/ 164:
/***/ ((module, exports, __webpack_require__) => {



const conversions = __webpack_require__(341);
const utils = __webpack_require__(288);

exports.convertInherit = function convertInherit(obj, ret, { context = "The provided value" } = {}) {
  {
    const key = "bubbles";
    let value = obj === undefined || obj === null ? undefined : obj[key];
    if (value !== undefined) {
      value = conversions["boolean"](value, { context: context + " has member bubbles that" });

      ret[key] = value;
    } else {
      ret[key] = false;
    }
  }

  {
    const key = "cancelable";
    let value = obj === undefined || obj === null ? undefined : obj[key];
    if (value !== undefined) {
      value = conversions["boolean"](value, { context: context + " has member cancelable that" });

      ret[key] = value;
    } else {
      ret[key] = false;
    }
  }

  {
    const key = "composed";
    let value = obj === undefined || obj === null ? undefined : obj[key];
    if (value !== undefined) {
      value = conversions["boolean"](value, { context: context + " has member composed that" });

      ret[key] = value;
    } else {
      ret[key] = false;
    }
  }
};

exports.convert = function convert(obj, { context = "The provided value" } = {}) {
  if (obj !== undefined && typeof obj !== "object" && typeof obj !== "function") {
    throw new TypeError(`${context} is not an object.`);
  }

  const ret = Object.create(null);
  module.exports.convertInherit(obj, ret, { context });
  return ret;
};


/***/ }),

/***/ 288:
/***/ ((module, exports) => {



// Returns "Type(value) is Object" in ES terminology.
function isObject(value) {
  return typeof value === "object" && value !== null || typeof value === "function";
}

function hasOwn(obj, prop) {
  return Object.prototype.hasOwnProperty.call(obj, prop);
}

const wrapperSymbol = Symbol("wrapper");
const implSymbol = Symbol("impl");
const sameObjectCaches = Symbol("SameObject caches");
const ctorRegistrySymbol = Symbol.for("[webidl2js]  constructor registry");

function getSameObject(wrapper, prop, creator) {
  if (!wrapper[sameObjectCaches]) {
    wrapper[sameObjectCaches] = Object.create(null);
  }

  if (prop in wrapper[sameObjectCaches]) {
    return wrapper[sameObjectCaches][prop];
  }

  wrapper[sameObjectCaches][prop] = creator();
  return wrapper[sameObjectCaches][prop];
}

function wrapperForImpl(impl) {
  return impl ? impl[wrapperSymbol] : null;
}

function implForWrapper(wrapper) {
  return wrapper ? wrapper[implSymbol] : null;
}

function tryWrapperForImpl(impl) {
  const wrapper = wrapperForImpl(impl);
  return wrapper ? wrapper : impl;
}

function tryImplForWrapper(wrapper) {
  const impl = implForWrapper(wrapper);
  return impl ? impl : wrapper;
}

const iterInternalSymbol = Symbol("internal");
const IteratorPrototype = Object.getPrototypeOf(Object.getPrototypeOf([][Symbol.iterator]()));

function isArrayIndexPropName(P) {
  if (typeof P !== "string") {
    return false;
  }
  const i = P >>> 0;
  if (i === Math.pow(2, 32) - 1) {
    return false;
  }
  const s = `${i}`;
  if (P !== s) {
    return false;
  }
  return true;
}

const byteLengthGetter =
    Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, "byteLength").get;
function isArrayBuffer(value) {
  try {
    byteLengthGetter.call(value);
    return true;
  } catch (e) {
    return false;
  }
}

const supportsPropertyIndex = Symbol("supports property index");
const supportedPropertyIndices = Symbol("supported property indices");
const supportsPropertyName = Symbol("supports property name");
const supportedPropertyNames = Symbol("supported property names");
const indexedGet = Symbol("indexed property get");
const indexedSetNew = Symbol("indexed property set new");
const indexedSetExisting = Symbol("indexed property set existing");
const namedGet = Symbol("named property get");
const namedSetNew = Symbol("named property set new");
const namedSetExisting = Symbol("named property set existing");
const namedDelete = Symbol("named property delete");

module.exports = exports = {
  isObject,
  hasOwn,
  wrapperSymbol,
  implSymbol,
  getSameObject,
  ctorRegistrySymbol,
  wrapperForImpl,
  implForWrapper,
  tryWrapperForImpl,
  tryImplForWrapper,
  iterInternalSymbol,
  IteratorPrototype,
  isArrayBuffer,
  isArrayIndexPropName,
  supportsPropertyIndex,
  supportedPropertyIndices,
  supportsPropertyName,
  supportedPropertyNames,
  indexedGet,
  indexedSetNew,
  indexedSetExisting,
  namedGet,
  namedSetNew,
  namedSetExisting,
  namedDelete
};


/***/ }),

/***/ 341:
/***/ ((__unused_webpack_module, exports) => {



function _(message, opts) {
    return `${opts && opts.context ? opts.context : "Value"} ${message}.`;
}

function type(V) {
    if (V === null) {
        return "Null";
    }
    switch (typeof V) {
        case "undefined":
            return "Undefined";
        case "boolean":
            return "Boolean";
        case "number":
            return "Number";
        case "string":
            return "String";
        case "symbol":
            return "Symbol";
        case "object":
            // Falls through
        case "function":
            // Falls through
        default:
            // Per ES spec, typeof returns an implemention-defined value that is not any of the existing ones for
            // uncallable non-standard exotic objects. Yet Type() which the Web IDL spec depends on returns Object for
            // such cases. So treat the default case as an object.
            return "Object";
    }
}

// Round x to the nearest integer, choosing the even integer if it lies halfway between two.
function evenRound(x) {
    // There are four cases for numbers with fractional part being .5:
    //
    // case |     x     | floor(x) | round(x) | expected | x <> 0 | x % 1 | x & 1 |   example
    //   1  |  2n + 0.5 |  2n      |  2n + 1  |  2n      |   >    |  0.5  |   0   |  0.5 ->  0
    //   2  |  2n + 1.5 |  2n + 1  |  2n + 2  |  2n + 2  |   >    |  0.5  |   1   |  1.5 ->  2
    //   3  | -2n - 0.5 | -2n - 1  | -2n      | -2n      |   <    | -0.5  |   0   | -0.5 ->  0
    //   4  | -2n - 1.5 | -2n - 2  | -2n - 1  | -2n - 2  |   <    | -0.5  |   1   | -1.5 -> -2
    // (where n is a non-negative integer)
    //
    // Branch here for cases 1 and 4
    if ((x > 0 && (x % 1) === +0.5 && (x & 1) === 0) ||
        (x < 0 && (x % 1) === -0.5 && (x & 1) === 1)) {
        return censorNegativeZero(Math.floor(x));
    }

    return censorNegativeZero(Math.round(x));
}

function integerPart(n) {
    return censorNegativeZero(Math.trunc(n));
}

function sign(x) {
    return x < 0 ? -1 : 1;
}

function modulo(x, y) {
    // https://tc39.github.io/ecma262/#eqn-modulo
    // Note that http://stackoverflow.com/a/4467559/3191 does NOT work for large modulos
    const signMightNotMatch = x % y;
    if (sign(y) !== sign(signMightNotMatch)) {
        return signMightNotMatch + y;
    }
    return signMightNotMatch;
}

function censorNegativeZero(x) {
    return x === 0 ? 0 : x;
}

function createIntegerConversion(bitLength, typeOpts) {
    const isSigned = !typeOpts.unsigned;

    let lowerBound;
    let upperBound;
    if (bitLength === 64) {
        upperBound = Math.pow(2, 53) - 1;
        lowerBound = !isSigned ? 0 : -Math.pow(2, 53) + 1;
    } else if (!isSigned) {
        lowerBound = 0;
        upperBound = Math.pow(2, bitLength) - 1;
    } else {
        lowerBound = -Math.pow(2, bitLength - 1);
        upperBound = Math.pow(2, bitLength - 1) - 1;
    }

    const twoToTheBitLength = Math.pow(2, bitLength);
    const twoToOneLessThanTheBitLength = Math.pow(2, bitLength - 1);

    return (V, opts) => {
        if (opts === undefined) {
            opts = {};
        }

        let x = +V;
        x = censorNegativeZero(x); // Spec discussion ongoing: https://github.com/heycam/webidl/issues/306

        if (opts.enforceRange) {
            if (!Number.isFinite(x)) {
                throw new TypeError(_("is not a finite number", opts));
            }

            x = integerPart(x);

            if (x < lowerBound || x > upperBound) {
                throw new TypeError(_(
                    `is outside the accepted range of ${lowerBound} to ${upperBound}, inclusive`, opts));
            }

            return x;
        }

        if (!Number.isNaN(x) && opts.clamp) {
            x = Math.min(Math.max(x, lowerBound), upperBound);
            x = evenRound(x);
            return x;
        }

        if (!Number.isFinite(x) || x === 0) {
            return 0;
        }
        x = integerPart(x);

        // Math.pow(2, 64) is not accurately representable in JavaScript, so try to avoid these per-spec operations if
        // possible. Hopefully it's an optimization for the non-64-bitLength cases too.
        if (x >= lowerBound && x <= upperBound) {
            return x;
        }

        // These will not work great for bitLength of 64, but oh well. See the README for more details.
        x = modulo(x, twoToTheBitLength);
        if (isSigned && x >= twoToOneLessThanTheBitLength) {
            return x - twoToTheBitLength;
        }
        return x;
    };
}

exports.any = V => {
    return V;
};

exports["void"] = function () {
    return undefined;
};

exports.boolean = function (val) {
    return !!val;
};

exports.byte = createIntegerConversion(8, { unsigned: false });
exports.octet = createIntegerConversion(8, { unsigned: true });

exports.short = createIntegerConversion(16, { unsigned: false });
exports["unsigned short"] = createIntegerConversion(16, { unsigned: true });

exports.long = createIntegerConversion(32, { unsigned: false });
exports["unsigned long"] = createIntegerConversion(32, { unsigned: true });

exports["long long"] = createIntegerConversion(64, { unsigned: false });
exports["unsigned long long"] = createIntegerConversion(64, { unsigned: true });

exports.double = (V, opts) => {
    const x = +V;

    if (!Number.isFinite(x)) {
        throw new TypeError(_("is not a finite floating-point value", opts));
    }

    return x;
};

exports["unrestricted double"] = V => {
    const x = +V;

    return x;
};

exports.float = (V, opts) => {
    const x = +V;

    if (!Number.isFinite(x)) {
        throw new TypeError(_("is not a finite floating-point value", opts));
    }

    if (Object.is(x, -0)) {
        return x;
    }

    const y = Math.fround(x);

    if (!Number.isFinite(y)) {
        throw new TypeError(_("is outside the range of a single-precision floating-point value", opts));
    }

    return y;
};

exports["unrestricted float"] = V => {
    const x = +V;

    if (isNaN(x)) {
        return x;
    }

    if (Object.is(x, -0)) {
        return x;
    }

    return Math.fround(x);
};

exports.DOMString = function (V, opts) {
    if (opts === undefined) {
        opts = {};
    }

    if (opts.treatNullAsEmptyString && V === null) {
        return "";
    }

    if (typeof V === "symbol") {
        throw new TypeError(_("is a symbol, which cannot be converted to a string", opts));
    }

    return String(V);
};

exports.ByteString = (V, opts) => {
    const x = exports.DOMString(V, opts);
    let c;
    for (let i = 0; (c = x.codePointAt(i)) !== undefined; ++i) {
        if (c > 255) {
            throw new TypeError(_("is not a valid ByteString", opts));
        }
    }

    return x;
};

exports.USVString = (V, opts) => {
    const S = exports.DOMString(V, opts);
    const n = S.length;
    const U = [];
    for (let i = 0; i < n; ++i) {
        const c = S.charCodeAt(i);
        if (c < 0xD800 || c > 0xDFFF) {
            U.push(String.fromCodePoint(c));
        } else if (0xDC00 <= c && c <= 0xDFFF) {
            U.push(String.fromCodePoint(0xFFFD));
        } else if (i === n - 1) {
            U.push(String.fromCodePoint(0xFFFD));
        } else {
            const d = S.charCodeAt(i + 1);
            if (0xDC00 <= d && d <= 0xDFFF) {
                const a = c & 0x3FF;
                const b = d & 0x3FF;
                U.push(String.fromCodePoint((2 << 15) + ((2 << 9) * a) + b));
                ++i;
            } else {
                U.push(String.fromCodePoint(0xFFFD));
            }
        }
    }

    return U.join("");
};

exports.object = (V, opts) => {
    if (type(V) !== "Object") {
        throw new TypeError(_("is not an object", opts));
    }

    return V;
};

// Not exported, but used in Function and VoidFunction.

// Neither Function nor VoidFunction is defined with [TreatNonObjectAsNull], so
// handling for that is omitted.
function convertCallbackFunction(V, opts) {
    if (typeof V !== "function") {
        throw new TypeError(_("is not a function", opts));
    }
    return V;
}

const abByteLengthGetter =
    Object.getOwnPropertyDescriptor(ArrayBuffer.prototype, "byteLength").get;

function isArrayBuffer(V) {
    try {
        abByteLengthGetter.call(V);
        return true;
    } catch (e) {
        return false;
    }
}

// I don't think we can reliably detect detached ArrayBuffers.
exports.ArrayBuffer = (V, opts) => {
    if (!isArrayBuffer(V)) {
        throw new TypeError(_("is not a view on an ArrayBuffer object", opts));
    }
    return V;
};

const dvByteLengthGetter =
    Object.getOwnPropertyDescriptor(DataView.prototype, "byteLength").get;
exports.DataView = (V, opts) => {
    try {
        dvByteLengthGetter.call(V);
        return V;
    } catch (e) {
        throw new TypeError(_("is not a view on an DataView object", opts));
    }
};

[
    Int8Array, Int16Array, Int32Array, Uint8Array,
    Uint16Array, Uint32Array, Uint8ClampedArray, Float32Array, Float64Array
].forEach(func => {
    const name = func.name;
    const article = /^[AEIOU]/.test(name) ? "an" : "a";
    exports[name] = (V, opts) => {
        if (!ArrayBuffer.isView(V) || V.constructor.name !== name) {
            throw new TypeError(_(`is not ${article} ${name} object`, opts));
        }

        return V;
    };
});

// Common definitions

exports.ArrayBufferView = (V, opts) => {
    if (!ArrayBuffer.isView(V)) {
        throw new TypeError(_("is not a view on an ArrayBuffer object", opts));
    }

    return V;
};

exports.BufferSource = (V, opts) => {
    if (!ArrayBuffer.isView(V) && !isArrayBuffer(V)) {
        throw new TypeError(_("is not an ArrayBuffer object or a view on one", opts));
    }

    return V;
};

exports.DOMTimeStamp = exports["unsigned long long"];

exports.Function = convertCallbackFunction;

exports.VoidFunction = convertCallbackFunction;


/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module is referenced by other modules so it can't be inlined
/******/ 	var __webpack_exports__ = __webpack_require__(674);
/******/ 	EventLib = __webpack_exports__;
/******/ 	
/******/ })()
;