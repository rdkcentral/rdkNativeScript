# Build Diagnostics & Deployment Verification

## Phase 1: Prove New Binary is Deployed

### 1.1 Build Identity Generation

Create a unique build identifier that includes:

- **NativeScript component version** - semantic version tag
- **Git commit SHA** - 40-character commit hash  
- **Build timestamp** - ISO 8601 format
- **Build type** - debug/release
- **Location compatibility revision** - semantic version
- **XMLHttpRequest implementation revision** - semantic version

### 1.2 Startup Log Output

Every NativeScript startup should produce exactly this log:

```
[NS_BUILD_ID] version=1.0.0 commit=a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6 build=2026-08-21T15:31:00Z buildType=release locationRev=2.0 xhrRev=3.1
```

### 1.3 Boot Sequence Logging

Immediately after build ID, log the initialization order:

```
[NS_BOOT_ORDER] 01 context-created contextId=ctx-main
[NS_BOOT_ORDER] 02 globals-installed
[NS_BOOT_ORDER] 03 location-installed protocol=file: host= origin=null
[NS_BOOT_ORDER] 04 xhr-installed version=3.1
[NS_BOOT_ORDER] 05 bundle-loaded bundle.js
[NS_BOOT_ORDER] 06 playerplatform-preload
[NS_BOOT_ORDER] 07 freewheel-ready
```

### 1.4 Device Verification

To verify correct binary is deployed:

1. **Via Console:**
   ```javascript
   // Open RDK Inspector console and run:
   window.NS_BUILD_INFO  // Should return object with all build metadata
   ```

2. **Via Logs:**
   ```bash
   # Device SSH
   ssh root@<device-ip>
   
   # Check deployment location
   ls -lh /opt/apps/*/librdknativescript.so  # Or equivalent binary path
   
   # Check version metadata
   strings /opt/apps/*/librdknativescript.so | grep "NS_BUILD_ID"
   
   # Watch live logs
   journalctl -fu sky-vipa  # Or equivalent service name
   ```

3. **Via Package Manager:**
   ```bash
   # Check version
   dpkg -l | grep rdknativescript
   # OR
   rpm -q rdknativescript
   # OR
   opkg info rdknativescript
   ```

## Phase 2: Verify Location Installation

Before any third-party code (PlayerPlatform, FreeWheel) loads:

```
[NS_LOCATION_INSTALL] contextId=ctx-main globalId=window
  protocol=file:
  host=
  hostname=
  port=
  origin=null
  href=file:///index.html
  pathname=/index.html
  search=
  hash=
  computed=file://
  status=success
```

### Validation Checklist:

```javascript
// Should all be true
window.location.protocol === "file:"
window.location.host === ""
window.location.hostname === ""
window.location.port === ""
window.location.origin === "null"
window.location.pathname === "/index.html"
window.location.protocol + "//" + window.location.host === "file://"
globalThis.location === window.location
typeof window.location.toString === "function"

// URL encoding validation
encodeURIComponent(window.location.protocol + "//" + window.location.host) === "file%3A%2F%2F"
```

## Phase 3: Verify XHR Installation

```
[NS_XHR_INSTALL] contextId=ctx-main version=3.1
  properties: response, responseURL, timeout, statusText
  methods: open, send, abort, addEventListener, setTimeout
  events: readystatechange, load, error, timeout, abort, loadend
  status=success
```

### Validation Checklist:

```javascript
// Should all exist
typeof window.XMLHttpRequest === "function"
typeof XMLHttpRequest.prototype.response !== "undefined"
typeof XMLHttpRequest.prototype.responseURL !== "undefined"
typeof XMLHttpRequest.prototype.timeout !== "undefined"
typeof XMLHttpRequest.prototype.statusText !== "undefined"
typeof XMLHttpRequest.prototype.setTimeout === "function"
typeof XMLHttpRequest.prototype.addEventListener === "function"

// Test event dispatch
var xhr = new XMLHttpRequest();
var events = [];
xhr.addEventListener("readystatechange", function() { events.push("readystatechange"); });
xhr.addEventListener("load", function() { events.push("load"); });
xhr.addEventListener("loadend", function() { events.push("loadend"); });
// Events should dispatch in correct order after send()
```

## Phase 4: Monitor FreeWheel Requests

### 4.1 First Request Validation

When a FreeWheel request initiates, check:

```
[NS_FW_ENV] contextId=ctx-main
  protocol=file:
  host=
  computed=file://
  xhr=3.1
  playerPlatform=5.140.0
  freewheel=6.55.0
```

### 4.2 Request Execution Timeline

For each FreeWheel ad request:

```
[NS_FW_REQUEST] requestId=abc123 contextId=ctx-main
  start=15:31.127
  url=https://604fc.v.fwmrm.net/ad/g/1?...&orig=file%3A%2F%2F...
  timeout=5000ms
  expectedCompletion=<start + 5000ms>

[NS_XHR_CONSTRUCT] requestId=abc123 timestamp=15:31.127
[NS_XHR_OPEN] requestId=abc123 method=GET host=604fc.v.fwmrm.net timestamp=15:31.128
[NS_XHR_SEND] requestId=abc123 timestamp=15:31.130

[NS_HTTP_CONNECT] requestId=abc123 elapsed=45ms
[NS_HTTP_TLS] requestId=abc123 elapsed=120ms
[NS_HTTP_REQUEST] requestId=abc123 elapsed=165ms

[NS_HTTP_HEADERS] requestId=abc123 
  status=200 
  contentType=application/json
  contentLength=12345
  elapsed=245ms

[NS_HTTP_BODY] requestId=abc123 
  chunkCount=3
  totalBytes=12345
  elapsed=310ms

[NS_XHR_STATE] requestId=abc123 readyState=2 status=200 elapsed=245ms
[NS_XHR_STATE] requestId=abc123 readyState=3 status=200 elapsed=250ms
[NS_XHR_STATE] requestId=abc123 readyState=4 status=200 elapsed=310ms

[NS_XHR_EVENT] requestId=abc123 event=readystatechange timestamp=15:31.437
[NS_XHR_EVENT] requestId=abc123 event=load timestamp=15:31.437
[NS_XHR_EVENT] requestId=abc123 event=loadend timestamp=15:31.437

[NS_HTTP_COMPLETE] requestId=abc123 
  result=success
  category=SUCCESS
  status=200
  elapsed=310ms
  timestamp=15:31.437

[NS_FW_RESPONSE] requestId=abc123 
  status=success
  adsCount=1
  duration=310ms
  completed=15:31.437
```

### 4.3 Failure Timeline

If request fails:

```
[NS_XHR_CONSTRUCT] requestId=xyz789
[NS_XHR_OPEN] requestId=xyz789
[NS_XHR_SEND] requestId=xyz789

[NS_HTTP_CONNECT] requestId=xyz789 elapsed=5012ms
[NS_HTTP_FAILURE] requestId=xyz789
  category=NATIVE_TIMEOUT  (or DNS, TLS, CALLBACK_DELAY, SDK_TIMEOUT)
  status=0
  error=Connection timeout
  elapsed=5012ms

[NS_XHR_EVENT] requestId=xyz789 event=timeout timestamp=15:36.142
[NS_XHR_EVENT] requestId=xyz789 event=loadend timestamp=15:36.142

[NS_HTTP_COMPLETE] requestId=xyz789
  result=failure
  category=NATIVE_TIMEOUT
  elapsed=5012ms
```

## Phase 5: Check Multiple Contexts

If device creates multiple JavaScript contexts:

```
[NS_CONTEXT_CREATED] contextId=ctx-main
[NS_CONTEXT_CREATED] contextId=ctx-fw-1
[NS_CONTEXT_CREATED] contextId=ctx-fw-2
[NS_CONTEXT_CREATED] contextId=ctx-fw-3
...
[NS_CONTEXT_CREATED] contextId=ctx-fw-29

// Each context should get independent initialization:
[NS_LOCATION_INSTALL] contextId=ctx-fw-1 protocol=file: host= computed=file://
[NS_XHR_INSTALL] contextId=ctx-fw-1 version=3.1

[NS_LOCATION_INSTALL] contextId=ctx-fw-2 protocol=file: host= computed=file://
[NS_XHR_INSTALL] contextId=ctx-fw-2 version=3.1
```

## Phase 6: Test Cases

### 6.1 Unit Tests (Automated)

```bash
# Run tests
npm test

# Expected output:
# ✓ 37/37 XHR tests passed
# ✓ 11/11 Location tests passed  
# ✓ 5/5 Encoding tests passed
# Total: 53 tests, 0 failures
```

### 6.2 Integration Test: Initial Tune

**Setup:** Device running with new binary
**Action:** Tune to channel with CDAI (e.g., Sky UK Channel 107)
**Expected Results:**

```
[NS_BUILD_ID] version=... commit=... # Correct new version
[NS_BOOT_ORDER] 01 context-created
[NS_BOOT_ORDER] 02 globals-installed
[NS_BOOT_ORDER] 03 location-installed
[NS_BOOT_ORDER] 04 xhr-installed
[NS_BOOT_ORDER] 05 bundle-loaded
[NS_BOOT_ORDER] 06 playerplatform-preload
[NS_BOOT_ORDER] 07 freewheel-ready

[NS_LOCATION_INSTALL] contextId=ctx-main protocol=file: host= computed=file://

[NS_FW_REQUEST] requestId=req-1
  url=https://604fc.v.fwmrm.net/ad/g/1?...&orig=file%3A%2F%2F...

[NS_FW_RESPONSE] requestId=req-1
  status=success
  adsCount=1  # CRITICAL: Was "undefined" before fix
  duration=245ms
  completed=<timestamp>

[Bundle Log] Ad request succeeded, Received 1 ads in the slot
```

**Failure Indicators:**
- `orig=undefined%2F%2Fundefined` in request URL
- `status=timeout` in FreeWheel response
- Log contains "Ad request failed, Returned slot object is undefined"
- readyState never reaches 4 (DONE)
- No load event dispatched

### 6.3 Integration Test: SCTE-35 Midroll

**Setup:** Live channel playing, waiting for SCTE-35 cue
**Action:** Trigger SCTE-35 cue manually
**Expected Results:**

```
[NS_FW_REQUEST] requestId=req-2 type=midroll duration=30000ms
  url=https://...&orig=file%3A%2F%2F&slid=9605&slau=midroll...

[NS_XHR_HEADERS] requestId=req-2 status=200 elapsed=156ms
[NS_FW_RESPONSE] requestId=req-2 status=success adsCount=1 duration=156ms

# CRITICAL: Ads should play, not timeout after 5 seconds
[Bundle Log] Playing ad break (duration: 30s)
[Bundle Log] Ad 1/1 playing...
```

### 6.4 Integration Test: Concurrent Requests

**Setup:** Multiple FreeWheel contexts active
**Action:** Trigger 3+ simultaneous ad requests
**Expected Results:**

```
[NS_FW_REQUEST] requestId=req-3a
[NS_FW_REQUEST] requestId=req-3b
[NS_FW_REQUEST] requestId=req-3c

[NS_XHR_HEADERS] requestId=req-3a elapsed=200ms
[NS_XHR_HEADERS] requestId=req-3b elapsed=210ms
[NS_XHR_HEADERS] requestId=req-3c elapsed=205ms

[NS_FW_RESPONSE] requestId=req-3a status=success
[NS_FW_RESPONSE] requestId=req-3b status=success
[NS_FW_RESPONSE] requestId=req-3c status=success

# All should complete successfully without cross-context interference
```

## Phase 7: Performance Validation

### 7.1 Request Timing

**Expected FreeWheel request timeline:**

| Phase | Min | Typical | Max |
|-------|-----|---------|-----|
| DNS lookup | 10ms | 30ms | 100ms |
| TLS handshake | 50ms | 120ms | 300ms |
| Request send | 5ms | 10ms | 50ms |
| Response headers | 20ms | 50ms | 200ms |
| Response body | 10ms | 100ms | 500ms |
| **Total** | **95ms** | **310ms** | **1150ms** |

**Timeout setting:** 5000ms (5 seconds)
**Buffer:** 3.85 seconds before timeout

### 7.2 Memory Impact

- No memory leaks on repeated requests (check after 100 requests)
- Timeout handlers properly cleaned up
- Event listeners properly removed

### 7.3 CPU Impact

- No busy-waiting (timeout uses native setTimeout, not polling)
- No blocking synchronous operations
- Non-blocking logging when enabled

## Phase 8: Rollback Verification

If new build causes regressions:

1. **Verify fallback binary is correct:**
   ```
   [NS_BUILD_ID] version=<old-version> commit=<old-commit>
   ```

2. **Re-test VOD ads:**
   - VOD content should still play ads
   - VMAP requests should work
   - Existing ad functionality should not regress

3. **Identify what failed:**
   - Was it location fix?
   - Was it XHR changes?
   - Was it timeout implementation?

## Troubleshooting Guide

### Problem: orig still shows undefined//undefined

**Diagnostics:**
```javascript
console.log(window.location);
console.log(window.location.protocol + "//" + window.location.host);
console.log(JSON.stringify(globalThis.location));
```

**Root Cause Check:**
- Build ID shows old version? → Old binary still deployed
- Location object missing properties? → Initialization skipped
- protocol not ending with colon? → Malformed in initialization
- host is object instead of string? → Type error

### Problem: FreeWheel request timeout after 5s

**Diagnostics:**
```
// Check request timeline
grep "NS_FW_REQUEST\|NS_FW_RESPONSE\|NS_XHR" device-logs.txt
grep "elapsed" device-logs.txt | tail -20
```

**Root Cause Check:**
- No [NS_HTTP_HEADERS] log? → Native response not arriving
- [NS_HTTP_HEADERS] logged but no [NS_FW_RESPONSE]? → Callback dispatch issue
- [NS_FW_RESPONSE] after timeout? → Race condition

### Problem: No diagnostic logs appearing

**Check:**
```javascript
window.RDK_XHR_DIAGNOSTICS  // Should be true
typeof XHRDiagnostics !== 'undefined'  // Should be true
```

**Enable from console:**
```javascript
window.RDK_XHR_DIAGNOSTICS = true;
// Then trigger new FreeWheel request
```

## Success Criteria

All of the following must be true:

- [ ] NS_BUILD_ID log shows new commit SHA at startup
- [ ] NS_BOOT_ORDER shows correct 7-step sequence
- [ ] NS_LOCATION_INSTALL shows protocol=file: with computed=file://
- [ ] No "orig=undefined" in FreeWheel request URL
- [ ] FreeWheel request completes before 5s timeout (typical 200-400ms)
- [ ] FreeWheel response includes adsCount >= 1
- [ ] Bundle logs show "Ad request succeeded"
- [ ] Multiple concurrent requests all succeed without interference
- [ ] All 53 automated tests pass
- [ ] VOD ads still play (backward compatibility)

## Post-Deployment Monitoring

After successful deployment:

1. **Week 1:** Monitor error rates
   - Should drop from ~50% timeout to <5%
   - Look for patterns in remaining failures

2. **Week 2:** Verify all scenarios
   - Linear CDAI
   - SCTE-35 midroll
   - CDVR time-shift
   - VOD ads

3. **Ongoing:** Check performance
   - Average FreeWheel response time
   - Max request duration
   - Error categories

## References

- FreeWheel SDK: https://sdk.freewheel.com/
- W3C XMLHttpRequest: https://xhr.spec.whatwg.org/
- W3C URL Standard: https://url.spec.whatwg.org/
- RDK Documentation: https://github.com/rdkcentral/rdkcentral.github.io
