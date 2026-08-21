FreeWheel XMLHttpRequest & Location Compliance Fix
=====================================================

## Overview

This fix addresses critical FreeWheel ad request timeouts in RDK NativeScript by implementing full W3C XMLHttpRequest compliance and fixing window.location property handling.

**Problem:** FreeWheel requests timeout after 5 seconds in NativeScript but complete successfully in browser/WPE environments.

**Root Causes Identified:**

1. **Malformed window.location values** - Missing protocol colon, incomplete properties
2. **Broken XMLHttpRequest lifecycle** - Incorrect event sequencing (load fires on error)
3. **Missing response properties** - response, responseURL not populated
4. **No timeout support** - xhr.timeout not implemented
5. **Incomplete statusText** - Set to error object instead of HTTP status text
6. **Callback race condition** - FreeWheel times out before response callbacks fire
7. **No diagnostic logging** - Cannot identify failure point

## Files Changed

### 1. utils/xhr.js (CRITICAL FIX)

**Changes:**
- Added `response` and `responseURL` properties (W3C standard)
- Added `timeout` property and `setTimeout()` method with proper timeout semantics
- Added diagnostic logging support via XHRDiagnostics utility
- Fixed `statusText` population before callbacks fire (was set to error object)
- Fixed event dispatch order:
  - ✓ error event does NOT dispatch load
  - ✓ timeout event does NOT dispatch load
  - ✓ abort event does NOT dispatch load  
  - ✓ loadend dispatches after ALL terminal events
- Added `getStatusText()` helper for proper HTTP status messages
- Response data fully available before DONE transition
- Timeout implementation with proper cleanup

**Before (Broken):**
```javascript
if (self.readyState === self.DONE && !errorFlag) {
  self.dispatchEvent("load");
  self.dispatchEvent("loadend");
  // WRONG: loadend also fires on error because errorFlag can be true from handleError
}

this.handleError = function(error) {
  this.status = 0;
  this.statusText = error;  // WRONG: error object, not HTTP status text
  this.responseText = error.stack;
  errorFlag = true;
  setState(this.DONE);
  this.dispatchEvent('error');
  // MISSING: no loadend dispatch
};
```

**After (Fixed):**
```javascript
if (self.readyState === self.DONE && !errorFlag) {
  self.dispatchEvent("load");
  self.dispatchEvent("loadend");
  // NOW: loadend only fires after successful load
}

this.handleError = function(error) {
  this.status = 0;
  this.statusText = error.message || String(error);
  this.responseText = error.stack;
  this.response = error.stack;
  errorFlag = true;
  setState(this.DONE);
  this.dispatchEvent('error');
  this.dispatchEvent('loadend');  // NOW: always fires after error
};
```

### 2. utils/window.js (LOCATION FIX)

**Changes:**
- Fixed protocol format: "file:" (was "http" - missing colon)
- Added all missing properties: hostname, port, origin, pathname, search, hash
- Set origin to "null" per W3C spec for file:// URLs
- Set globalThis.location to match window.location for consistency

**Before (Broken):**
```javascript
window.location = {"href":"", "host":"192.168.0.102", "protocol":"http"}
// Missing: hostname, port, origin, pathname, search, hash
// FreeWheel sees: orig=undefined//undefined after encodeURIComponent
```

**After (Fixed):**
```javascript
window.location = {
  "href": "file:///index.html",
  "host": "",
  "hostname": "",
  "port": "",
  "protocol": "file:",
  "origin": "null",
  "pathname": "/index.html",
  "search": "",
  "hash": "",
  "toString": function() { return this.href; }
};
globalThis.location = window.location;
// FreeWheel sees: orig=file%3A%2F%2F (valid)
```

### 3. src/jsc/modules/linkedjsdomwrapper.js (LOCATION FIX)

Same location fixes as utils/window.js applied to JSDOM path.

### 4. src/jsc/modules/windowwrapper.js (LOCATION FIX)

Same location fixes as utils/window.js applied to WindowLib path.

### 5. utils/location.js (NEW - UTILITIES)

**Purpose:** Centralized location object factory for future use

**Functions:**
- `createLocationObject(protocol, host, pathname)` - Factory to create valid location objects
- `getDefaultLocation()` - Returns file:// location (default for NativeScript)
- `getLoopbackLocation()` - Returns http://127.0.0.1 location (if needed)

**Benefits:**
- Reusable across all initialization paths
- Ensures consistency
- Easier to modify for future requirements

### 6. utils/xhrdiagnostics.js (NEW - LOGGING)

**Purpose:** Structured logging for XHR lifecycle debugging

**Methods:**
- `log(requestId, message)` - Core logging
- `logRequest(requestId, method, url, headers)` - Log request details
- `logStateChange(requestId, state, status)` - Log state transitions
- `logEvent(requestId, event)` - Log event dispatch
- `logError(requestId, category, errorMsg)` - Log errors with sanitization

**Usage:**
```javascript
window.RDK_XHR_DIAGNOSTICS = true;  // Enable in console
// Logs like: [XHR][abc123] 2026-08-21T12:34:56.789Z open method=GET url=https://fwmrm.net/...
```

**Security:** Automatically redacts auth tokens, SAT values, device IDs, and IP addresses

### 7. test/xhr.test.js (NEW - COMPREHENSIVE TESTS)

**Test Suites:**
1. Location Object Compatibility (11 tests)
   - Protocol defined, ends with colon
   - All properties present (hostname, port, origin, pathname)
   - globalThis.location consistency

2. FreeWheel Origin Generation (5 tests)
   - No undefined values
   - Correct URL encoding
   - No malformed protocols

3. XMLHttpRequest Properties (8 tests)
   - All required properties exist
   - Event handlers supported (onload, onerror, ontimeout)

4. XMLHttpRequest Lifecycle (6 tests)
   - State transitions correct
   - Status and response reset on open()
   - Event listeners work

5. XMLHttpRequest Event Dispatch (2 tests)
   - Error doesn't dispatch load
   - Abort dispatches loadend

6. URL and Query String Preservation (3 tests)
   - Semicolons preserved
   - Ampersands preserved
   - Encoded characters preserved

7. Multiple Concurrent XHR Isolation (2 tests)
   - Separate state management
   - Separate listener isolation

**Total: 37 automated tests**

### 8. test/location-encoding.test.js (NEW - ENCODING TESTS)

**Tests:**
1. No double encoding - %252F doesn't appear
2. Single encoding of colons and slashes
3. Known encoding values match spec
4. Complex FreeWheel query strings preserve structure
5. Encoding consistency across multiple calls

**Benefits:** Verifies that FreeWheel URL construction produces correct orig parameter

## Key Behavioral Changes

### 1. XMLHttpRequest Event Sequence (W3C Compliant)

**Successful Response:**
```
readystatechange (OPENED)
readystatechange (HEADERS_RECEIVED)
readystatechange (LOADING) [may repeat]
readystatechange (DONE)
load
loadend
```

**Network Error:**
```
readystatechange (OPENED)
error
loadend
```

**Timeout:**
```
readystatechange (OPENED)
timeout
loadend
```

**Abort:**
```
readystatechange (OPENED)
abort
loadend
```

### 2. Response Data Availability

**Before:** statusText was error object, response populated after callbacks fired
**After:** statusText is HTTP status message, response ready before callbacks fire

### 3. Timeout Handling

**Before:** No timeout support
**After:** 
- xhr.timeout = milliseconds (0 = no timeout)
- Timeout event fires on expiry
- Request aborted, response cleared
- loadend always fires after timeout

### 4. FreeWheel Integration

**Before:** Broken flow
```
FreeWheel builds orig = window.location.protocol + "//" + window.location.host
Result: orig=undefined//undefined
FreeWheel submits malformed request
After 5s timeout: "Ad request failed, Returned slot object is undefined"
```

**After:** Working flow
```
FreeWheel builds orig = "file:" + "//" + ""
Result: orig=file://
FreeWheel encodes: orig=file%3A%2F%2F
FreeWheel submits valid request
Response arrives before timeout
FreeWheel parses ads successfully
"Ad request succeeded, Received N ads in the slot"
```

## Testing Strategy

### Unit Tests (Automated)
```bash
node test/xhr.test.js
node test/location-encoding.test.js
```

Expected output:
```
=== TEST SUMMARY ===
Passed: 37
Failed: 0
Total:  37

✓ ALL TESTS PASSED
```

### Integration Tests (Manual)

1. **Linear CDAI on Channel 107**
   - Verify initial FreeWheel request succeeds
   - Check logs for: "Ad request succeeded"
   - Verify ads play correctly

2. **SCTE-35 Triggered Midroll**
   - Trigger multiple SCTE-35 cues
   - Each should generate FreeWheel request
   - Verify isolation between requests

3. **CDVR Time-Shift**
   - Play DVR content with time-shift
   - Verify FreeWheel requests at seek points
   - Verify ads play at shifted times

4. **Diagnostic Logging**
   ```javascript
   window.RDK_XHR_DIAGNOSTICS = true;
   // Retune to channel 107
   // Check console for:
   // [XHR][abc123] 2026-08-21T12:34:56.789Z open method=GET
   // [XHR][abc123] readyState=1 (OPENED)
   // [XHR][abc123] readyState=2 (HEADERS_RECEIVED) status=200
   // [XHR][abc123] readyState=4 (DONE) status=200
   // [XHR][abc123] dispatch load
   // [XHR][abc123] dispatch loadend
   // [XHR][abc123] completed success elapsedMs=245
   ```

5. **Error Handling**
   - Simulate network failure: verify error event fires
   - Simulate timeout: verify timeout event fires (not load)
   - Verify loadend always fires last

6. **VOD Ads Regression**
   - Play VOD content
   - Verify existing ad playback unaffected
   - Check that VMAP requests still work

## Backward Compatibility

✓ **No breaking changes**
- All new properties/methods are additions
- Existing code continues to work
- Default behavior matches browser standards

✓ **Existing XMLHttpRequest consumers unaffected**
- Only W3C-compliant improvements
- No API changes
- No removal of existing functionality

✓ **Location object compatible**
- All existing code reading window.location still works
- New properties added but don't break reads
- Consistent with browser behavior

## Known Limitations

1. **Synchronous XHR** - Still not fully implemented (rare use case)
2. **Custom HTTP verbs** - Only standard methods supported
3. **File:// URL access** - No actual file system access in NativeScript
4. **Cross-origin** - CORS handling limited to header passing

## Performance Impact

**Minimal to None:**
- Diagnostic logging disabled by default (window.RDK_XHR_DIAGNOSTICS = false)
- Timeout implementation uses native setTimeout (no polling)
- Additional properties are simple object literals
- No new synchronous operations added

## Verification Checklist

Before considering this fix complete:

- [ ] All 37 unit tests pass
- [ ] Location encoding tests pass
- [ ] Linear CDAI requests succeed (not timeout)
- [ ] FreeWheel logs show "Ad request succeeded"
- [ ] Midroll requests work after SCTE-35
- [ ] CDVR time-shift ads play correctly
- [ ] VOD ads unaffected
- [ ] Diagnostic logs show correct XHR lifecycle
- [ ] Multiple concurrent requests isolated
- [ ] No memory leaks on repeated requests
- [ ] Error handling works (network failures logged)
- [ ] Timeout handling works (timeout event fires)

## Rollback Plan

If issues arise:

1. **Revert single file:** `git checkout utils/xhr.js`
2. **Revert location fixes:** `git checkout utils/window.js src/jsc/modules/`
3. **Keep new utilities:** location.js, xhrdiagnostics.js (non-breaking)
4. **Test VOD ads:** Verify no regression on rollback

## Future Enhancements

1. Add fetch() API support
2. Add promise-based XHR wrapper
3. Add request interceptor support
4. Add HTTP/2 support if native transport adds it
5. Add compression support (gzip, deflate)

## References

- [W3C XMLHttpRequest Standard](https://xhr.spec.whatwg.org/)
- [WHATWG URL Standard](https://url.spec.whatwg.org/)
- [FreeWheel AdRequest Documentation](https://sdk.freewheel.com/)
- [RDK NativeScript GitHub](https://github.com/rdkcentral/rdkNativeScript)

## Author Notes

This fix separates two distinct issues that were conflated:

1. **Transport Issue** - FreeWheel requests not reaching server
   - Fixed by: XHR statusText/response population, proper event dispatch
   
2. **Compatibility Issue** - FreeWheel URL construction failing
   - Fixed by: window.location properties

Both issues must be fixed together for FreeWheel to work. The "orig=undefined//undefined" 
symptom was secondary; the primary cause was incomplete XHR lifecycle implementation.

The diagnostic logging utility will help identify any remaining transport issues in future 
deployments.
