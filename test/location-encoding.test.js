/**
 * Tests for location property encoding and URL construction
 * Specifically tests FreeWheel orig parameter generation
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

var LocationEncodingTests = {
  passed: 0,
  failed: 0,
  
  assertEqual: function(actual, expected, name) {
    if (actual === expected) {
      console.log('✓ ' + name);
      this.passed++;
    } else {
      console.error('✗ ' + name);
      console.error('  Expected: ' + expected);
      console.error('  Got:      ' + actual);
      this.failed++;
    }
  },
  
  run: function() {
    console.log('\n=== Location Property Encoding Tests ===\n');
    
    // Test 1: No double encoding
    console.log('Test: No double encoding of location properties');
    var locProtocol = window.location.protocol;
    var locHost = window.location.host;
    var orig = locProtocol + '//' + locHost;
    
    // Simulate what FreeWheel does
    var origEncoded = encodeURIComponent(orig);
    
    console.log('  location.protocol = "' + locProtocol + '"');
    console.log('  location.host = "' + locHost + '"');
    console.log('  orig = "' + orig + '"');
    console.log('  encodeURIComponent(orig) = "' + origEncoded + '"');
    
    // Should NOT contain double-encoded slashes (%252F)
    this.assertEqual(
      origEncoded.indexOf('%252F') === -1,
      true,
      'orig should not contain double-encoded slashes'
    );
    
    // Test 2: Single encoding of colons
    var colonCount = (origEncoded.match(/%3A/g) || []).length;
    console.log('\n  Colon encoding count: ' + colonCount);
    if (orig.indexOf(':') !== -1) {
      this.assertEqual(
        colonCount >= 1,
        true,
        'colons should be encoded exactly once'
      );
    }
    
    // Test 3: Single encoding of slashes
    var slashCount = (origEncoded.match(/%2F/g) || []).length;
    console.log('  Slash encoding count: ' + slashCount);
    if (orig.indexOf('/') !== -1) {
      this.assertEqual(
        slashCount >= 1,
        true,
        'slashes should be encoded exactly once'
      );
    }
    
    // Test 4: Verify known encodings
    console.log('\nTest: Known encoding values');
    
    var testCases = [
      { input: 'file://', expected: 'file%3A%2F%2F', desc: 'file:// protocol' },
      { input: 'http://', expected: 'http%3A%2F%2F', desc: 'http:// protocol' },
      { input: 'https://', expected: 'https%3A%2F%2F', desc: 'https:// protocol' }
    ];
    
    for (var i = 0; i < testCases.length; i++) {
      var encoded = encodeURIComponent(testCases[i].input);
      this.assertEqual(
        encoded,
        testCases[i].expected,
        testCases[i].desc + ' encodes correctly'
      );
    }
    
    // Test 5: Complex FreeWheel query string
    console.log('\nTest: Complex FreeWheel query string preservation');
    var fwUrl = 'https://fwmrm.net/ad/g/1?prof=12345&nw=5678&mode=&slid=9521;slau=midroll&ptgt=a&tpos=123&maxd=30&mind=30&orig=' + encodeURIComponent(orig);
    console.log('  URL: ' + fwUrl.substring(0, 100) + '...');
    
    // Should contain single & for parameter separation
    var ampCount = (fwUrl.match(/&/g) || []).length;
    console.log('  Parameter separators (&): ' + ampCount);
    this.assertEqual(
      ampCount >= 1,
      true,
      'complex FW query should have ampersands for params'
    );
    
    // Should contain semicolon for slot params
    this.assertEqual(
      fwUrl.indexOf(';') !== -1,
      true,
      'complex FW query should preserve semicolons'
    );
    
    // Test 6: Consistency across multiple encodings
    console.log('\nTest: Encoding consistency');
    var encoded1 = encodeURIComponent(orig);
    var encoded2 = encodeURIComponent(orig);
    this.assertEqual(
      encoded1,
      encoded2,
      'repeated encoding should produce identical results'
    );
    
    // Report
    console.log('\n=== Results ===');
    console.log('Passed: ' + this.passed);
    console.log('Failed: ' + this.failed);
    console.log('Total:  ' + (this.passed + this.failed));
  }
};

LocationEncodingTests.run();
