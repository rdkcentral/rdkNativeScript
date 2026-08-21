/**
 * Browser-compatible location object factory for NativeScript
 * 
 * Ensures window.location and globalThis.location work consistently
 * and produce valid values for third-party libraries like FreeWheel.
 *
 * This addresses:
 * - FreeWheel URL construction: window.location.protocol + "//" + window.location.host
 * - Standards compliance for URL encoding
 * - Consistency across execution contexts
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

/**
 * Create a standards-compliant location object
 * 
 * @param {string} baseProtocol - Protocol (e.g., 'file:', 'http:', 'https:')
 * @param {string} baseHost - Host with optional port (e.g., '', '127.0.0.1', '127.0.0.1:8080')
 * @param {string} basePathname - Path component (e.g., '/index.html', '/')
 * @returns {object} Location object with all W3C properties
 */
function createLocationObject(baseProtocol, baseHost, basePathname) {
  baseProtocol = baseProtocol || 'file:';
  baseHost = baseHost || '';
  basePathname = basePathname || '/index.html';

  // Validate and normalize protocol - must end with colon
  if (baseProtocol && !baseProtocol.endsWith(':')) {
    baseProtocol = baseProtocol + ':';
  }

  // Parse hostname and port from host string
  var hostname = baseHost;
  var port = '';
  var colonIndex = baseHost.indexOf(':');
  if (colonIndex !== -1) {
    hostname = baseHost.substring(0, colonIndex);
    port = baseHost.substring(colonIndex + 1);
  }

  // Construct origin per W3C spec
  // file:// URLs have origin "null" per the spec
  // http/https use protocol://host:port format
  var origin;
  if (baseProtocol === 'file:') {
    origin = 'null';
  } else if (baseHost) {
    origin = baseProtocol + '//' + baseHost;
  } else {
    origin = baseProtocol + '//';
  }

  // Construct href (full URL)
  var href;
  if (baseProtocol === 'file:') {
    href = baseProtocol + '//' + basePathname;
  } else {
    href = origin + basePathname;
  }

  // Return location object with all W3C standard properties
  var location = {
    // W3C Standard Properties
    href: href,
    protocol: baseProtocol,
    host: baseHost,
    hostname: hostname,
    port: port,
    origin: origin,
    pathname: basePathname,
    search: '',
    hash: '',

    // Utility methods
    toString: function() {
      return this.href;
    },

    // Reload (no-op in NativeScript)
    reload: function() {
      console.warn('location.reload() is not implemented in NativeScript');
    },

    // Replace (no-op in NativeScript)
    replace: function(url) {
      console.warn('location.replace() is not implemented in NativeScript');
    }
  };

  return location;
}

/**
 * Get the default location object for NativeScript
 * Uses file:// protocol for local widget execution
 * 
 * @returns {object} Default location object
 */
function getDefaultLocation() {
  return createLocationObject('file:', '', '/index.html');
}

/**
 * Get a loopback location object for NativeScript with HTTP
 * Used in environments requiring HTTP loopback transport
 * 
 * @returns {object} Loopback location object
 */
function getLoopbackLocation() {
  return createLocationObject('http:', '127.0.0.1', '/index.html');
}

// Export for use in Node/CommonJS environments
if (typeof module !== 'undefined' && module.exports) {
  module.exports = {
    createLocationObject: createLocationObject,
    getDefaultLocation: getDefaultLocation,
    getLoopbackLocation: getLoopbackLocation
  };
}
