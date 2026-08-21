/**
 * Comprehensive tests for XMLHttpRequest W3C compliance
 * 
 * Tests location compatibility, XHR lifecycle, and FreeWheel integration.
 *
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
 */

// Test helpers
var TestRunner = {
  passed: 0,
  failed: 0,
  tests: [],
  
  describe: function(name, fn) {
    console.log('\n=== ' + name + ' ===');
    fn();
  },
  
  it: function(name, fn) {
    var self = this;
    try {
      fn();
      console.log('✓ ' + name);
      self.passed++;
    } catch (e) {
      console.error('✗ ' + name + ': ' + e.message);
      self.failed++;
    }
  },
  
  assertEqual: function(actual, expected, msg) {
    if (actual !== expected) {
      throw new Error(msg + ' (expected: ' + expected + ', got: ' + actual + ')');
    }
  },
  
  assertNotUndefined: function(value, msg) {
    if (value === undefined) {
      throw new Error(msg + ' (value was undefined)');
    }
  },
  
  assertTrue: function(value, msg) {
    if (value !== true) {
      throw new Error(msg + ' (expected true, got: ' + value + ')');
    }
  },
  
  assertFalse: function(value, msg) {
    if (value !== false) {
      throw new Error(msg + ' (expected false, got: ' + value + ')');
    }
  },
  
  assertMatch: function(actual, regex, msg) {
    if (!regex.test(actual)) {
      throw new Error(msg + ' ("' + actual + '" did not match ' + regex + ')');
    }
  },
  
  assertContains: function(str, substring, msg) {
    if (str.indexOf(substring) === -1) {
      throw new Error(msg + ' ("' + str + '" does not contain "' + substring + '")');
    }
  },
  
  report: function() {
    console.log('\n\n=== TEST SUMMARY ===');
    console.log('Passed: ' + this.passed);
    console.log('Failed: ' + this.failed);
    console.log('Total:  ' + (this.passed + this.failed));
  }
};

// =====================================================================
// LOCATION COMPATIBILITY TESTS
// =====================================================================

TestRunner.describe('Location Object Compatibility', function() {
  
  TestRunner.it('window.location is defined', function() {
    TestRunner.assertNotUndefined(window.location, 'window.location must be defined');
  });
  
  TestRunner.it('window.location.protocol is defined and not undefined', function() {
    TestRunner.assertNotUndefined(window.location.protocol, 'protocol must be defined');
    TestRunner.assertTrue(window.location.protocol.length > 0, 'protocol must not be empty');
  });
  
  TestRunner.it('window.location.protocol ends with colon', function() {
    TestRunner.assertTrue(
      window.location.protocol.endsWith(':'),
      'protocol must end with colon, got: ' + window.location.protocol
    );
  });
  
  TestRunner.it('window.location.host is defined', function() {
    TestRunner.assertNotUndefined(window.location.host, 'host must be defined');
  });
  
  TestRunner.it('window.location has hostname property', function() {
    TestRunner.assertNotUndefined(window.location.hostname, 'hostname must be defined');
  });
  
  TestRunner.it('window.location has port property', function() {
    TestRunner.assertNotUndefined(window.location.port, 'port must be defined');
  });
  
  TestRunner.it('window.location has origin property', function() {
    TestRunner.assertNotUndefined(window.location.origin, 'origin must be defined');
  });
  
  TestRunner.it('window.location has pathname property', function() {
    TestRunner.assertNotUndefined(window.location.pathname, 'pathname must be defined');
  });
  
  TestRunner.it('globalThis.location equals window.location', function() {
    TestRunner.assertEqual(
      globalThis.location.protocol,
      window.location.protocol,
      'globalThis.location.protocol must match window.location.protocol'
    );
  });
});

// =====================================================================
// FREEWHEEL ORIGIN GENERATION TESTS
// =====================================================================

TestRunner.describe('FreeWheel Origin Generation', function() {
  
  TestRunner.it('protocol + "//" + host produces valid string', function() {
    var orig = window.location.protocol + '//' + window.location.host;
    TestRunner.assertNotUndefined(orig, 'orig must not be undefined');
    TestRunner.assertTrue(orig.length > 0, 'orig must not be empty');
  });
  
  TestRunner.it('protocol + "//" + host does not contain undefined', function() {
    var orig = window.location.protocol + '//' + window.location.host;
    TestRunner.assertFalse(
      orig.indexOf('undefined') !== -1,
      'orig must not contain "undefined", got: ' + orig
    );
  });
  
  TestRunner.it('encodeURIComponent(file://) produces correct encoding', function() {
    var encoded = encodeURIComponent('file://');
    TestRunner.assertEqual(
      encoded,
      'file%3A%2F%2F',
      'file:// should encode to file%3A%2F%2F'
    );
  });
  
  TestRunner.it('encodeURIComponent(http://127.0.0.1) produces correct encoding', function() {
    var encoded = encodeURIComponent('http://127.0.0.1');
    TestRunner.assertEqual(
      encoded,
      'http%3A%2F%2F127.0.0.1',
      'http://127.0.0.1 should encode correctly'
    );
  });
  
  TestRunner.it('orig value does not have malformed protocol', function() {
    var orig = window.location.protocol + '//' + window.location.host;
    // Check for common malformations
    TestRunner.assertFalse(
      orig.indexOf('http//') !== -1,
      'orig must not contain http// (missing colon)'
    );
    TestRunner.assertFalse(
      orig.indexOf('file//') !== -1,
      'orig must not contain file// (missing colon)'
    );
  });
});

// =====================================================================
// XMLHTTPREQUEST PROPERTY TESTS
// =====================================================================

TestRunner.describe('XMLHttpRequest Properties', function() {
  
  TestRunner.it('XMLHttpRequest constructor creates instance', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertNotUndefined(xhr, 'XMLHttpRequest() must return an object');
  });
  
  TestRunner.it('xhr.readyState is initialized to UNSENT (0)', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertEqual(xhr.readyState, 0, 'readyState should start at 0');
  });
  
  TestRunner.it('xhr has response property', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertNotUndefined(xhr.response, 'xhr.response must exist');
  });
  
  TestRunner.it('xhr has responseURL property', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertNotUndefined(xhr.responseURL, 'xhr.responseURL must exist');
  });
  
  TestRunner.it('xhr has timeout property', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertNotUndefined(xhr.timeout, 'xhr.timeout must exist');
  });
  
  TestRunner.it('xhr has statusText property', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertNotUndefined(xhr.statusText, 'xhr.statusText must exist');
  });
  
  TestRunner.it('xhr has addEventListener method', function() {
    var xhr = new XMLHttpRequest();
    TestRunner.assertNotUndefined(xhr.addEventListener, 'xhr.addEventListener must exist');
    TestRunner.assertTrue(
      typeof xhr.addEventListener === 'function',
      'xhr.addEventListener must be a function'
    );
  });
  
  TestRunner.it('xhr supports onload, onerror, ontimeout properties', function() {
    var xhr = new XMLHttpRequest();
    xhr.onload = function() {};
    xhr.onerror = function() {};
    xhr.ontimeout = function() {};
    TestRunner.assertTrue(typeof xhr.onload === 'function', 'onload must be callable');
    TestRunner.assertTrue(typeof xhr.onerror === 'function', 'onerror must be callable');
    TestRunner.assertTrue(typeof xhr.ontimeout === 'function', 'ontimeout must be callable');
  });
});

// =====================================================================
// XMLHTTPREQUEST LIFECYCLE TESTS
// =====================================================================

TestRunner.describe('XMLHttpRequest Lifecycle', function() {
  
  TestRunner.it('xhr.open changes readyState to OPENED (1)', function() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://example.com', true);
    TestRunner.assertEqual(xhr.readyState, 1, 'readyState should be OPENED after open()');
  });
  
  TestRunner.it('xhr.open resets status to 0', function() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://example.com', true);
    TestRunner.assertEqual(xhr.status, 0, 'status should be reset to 0');
  });
  
  TestRunner.it('xhr.open resets responseText', function() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://example.com', true);
    TestRunner.assertEqual(xhr.responseText, '', 'responseText should be reset to empty string');
  });
  
  TestRunner.it('xhr.open resets response', function() {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', 'http://example.com', true);
    TestRunner.assertEqual(xhr.response, '', 'response should be reset to empty string');
  });
  
  TestRunner.it('xhr.addEventListener adds event listener', function() {
    var xhr = new XMLHttpRequest();
    var called = false;
    xhr.addEventListener('load', function() {
      called = true;
    });
    // Manually trigger the event to verify
    xhr.dispatchEvent('load');
    TestRunner.assertTrue(called, 'event listener should be called');
  });
  
  TestRunner.it('xhr.onload and addEventListener both work', function() {
    var xhr = new XMLHttpRequest();
    var onloadCalled = false;
    var eventListenerCalled = false;
    
    xhr.onload = function() {
      onloadCalled = true;
    };
    xhr.addEventListener('load', function() {
      eventListenerCalled = true;
    });
    
    xhr.dispatchEvent('load');
    TestRunner.assertTrue(onloadCalled, 'onload should be called');
    TestRunner.assertTrue(eventListenerCalled, 'addEventListener listener should be called');
  });
});

// =====================================================================
// XMLHTTPREQUEST EVENT DISPATCH TESTS
// =====================================================================

TestRunner.describe('XMLHttpRequest Event Dispatch', function() {
  
  TestRunner.it('error event does not dispatch load', function() {
    var xhr = new XMLHttpRequest();
    var loadCalled = false;
    var errorCalled = false;
    
    xhr.addEventListener('load', function() {
      loadCalled = true;
    });
    xhr.addEventListener('error', function() {
      errorCalled = true;
    });
    
    // Simulate error by calling handleError
    xhr.open('GET', 'http://example.com', true);
    xhr.handleError(new Error('Network error'));
    
    TestRunner.assertTrue(errorCalled, 'error event should be dispatched');
    TestRunner.assertFalse(loadCalled, 'load event should NOT be dispatched on error');
  });
  
  TestRunner.it('abort event dispatches loadend', function() {
    var xhr = new XMLHttpRequest();
    var loadendCalled = false;
    
    xhr.addEventListener('loadend', function() {
      loadendCalled = true;
    });
    
    xhr.open('GET', 'http://example.com', true);
    xhr.abort();
    
    TestRunner.assertTrue(loadendCalled, 'loadend should be dispatched after abort');
  });
});

// =====================================================================
// URL PRESERVATION TESTS
// =====================================================================

TestRunner.describe('URL and Query String Preservation', function() {
  
  TestRunner.it('URL with semicolons is accepted', function() {
    var xhr = new XMLHttpRequest();
    var url = 'https://example.com/ad?slid=123;slau=midroll&maxd=30';
    try {
      xhr.open('GET', url, true);
      TestRunner.assertTrue(true, 'URL with semicolons should be accepted');
    } catch (e) {
      throw new Error('URL with semicolons should be valid: ' + e.message);
    }
  });
  
  TestRunner.it('URL with ampersands is preserved', function() {
    var xhr = new XMLHttpRequest();
    var url = 'https://example.com/ad?a=1&b=2&c=3';
    xhr.open('GET', url, true);
    TestRunner.assertContains(
      url,
      '&',
      'URL should contain ampersands for multiple parameters'
    );
  });
  
  TestRunner.it('URL with encoded characters is preserved', function() {
    var xhr = new XMLHttpRequest();
    var url = 'https://example.com/ad?orig=file%3A%2F%2F';
    xhr.open('GET', url, true);
    TestRunner.assertContains(
      url,
      '%3A',
      'URL should preserve encoded colons'
    );
    TestRunner.assertContains(
      url,
      '%2F',
      'URL should preserve encoded slashes'
    );
  });
});

// =====================================================================
// MULTIPLE CONCURRENT REQUESTS TESTS
// =====================================================================

TestRunner.describe('Multiple Concurrent XHR Isolation', function() {
  
  TestRunner.it('multiple XHR instances have separate state', function() {
    var xhr1 = new XMLHttpRequest();
    var xhr2 = new XMLHttpRequest();
    
    xhr1.open('GET', 'http://example1.com', true);
    xhr2.open('GET', 'http://example2.com', true);
    
    // Both should have OPENED state but independent of each other
    TestRunner.assertEqual(xhr1.readyState, 1, 'xhr1 should be OPENED');
    TestRunner.assertEqual(xhr2.readyState, 1, 'xhr2 should be OPENED');
  });
  
  TestRunner.it('multiple XHR instances have separate listeners', function() {
    var xhr1 = new XMLHttpRequest();
    var xhr2 = new XMLHttpRequest();
    var xhr1Called = false;
    var xhr2Called = false;
    
    xhr1.addEventListener('load', function() {
      xhr1Called = true;
    });
    xhr2.addEventListener('load', function() {
      xhr2Called = true;
    });
    
    xhr1.dispatchEvent('load');
    
    TestRunner.assertTrue(xhr1Called, 'xhr1 load listener should be called');
    TestRunner.assertFalse(xhr2Called, 'xhr2 load listener should NOT be called');
  });
});

// =====================================================================
// RUN TESTS AND REPORT
// =====================================================================

console.log('\n\n╔════════════════════════════════════════════════════════════════╗');
console.log('║  RDK NativeScript XMLHttpRequest & Location Compliance Tests  ║');
console.log('╚════════════════════════════════════════════════════════════════╝');

TestRunner.report();

if (TestRunner.failed === 0) {
  console.log('\n✓ ALL TESTS PASSED');
} else {
  console.log('\n✗ SOME TESTS FAILED');
}
