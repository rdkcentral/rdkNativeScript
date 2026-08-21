/**
 * XHR Diagnostic logging utility for FreeWheel compatibility debugging
 * 
 * Provides structured logging around XMLHttpRequest lifecycle events
 * to help diagnose FreeWheel ad request timeouts and failures.
 *
 * Usage:
 *   window.RDK_XHR_DIAGNOSTICS = true;  // Enable in console or startup
 *   XHRDiagnostics.log(requestId, 'message');
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

var XHRDiagnostics = {
  // Enable/disable via window.RDK_XHR_DIAGNOSTICS
  _enabled: false,

  get enabled() {
    return typeof window !== 'undefined' && window.RDK_XHR_DIAGNOSTICS === true;
  },

  /**
   * Log a message for a specific XHR request
   * @param {string} requestId - Unique request identifier
   * @param {string} message - Log message
   */
  log: function(requestId, message) {
    if (!this.enabled) return;
    var timestamp = new Date().toISOString();
    console.log('[XHR][' + requestId + '] ' + timestamp + ' ' + message);
  },

  /**
   * Log request opening
   * @param {string} requestId - Unique request identifier
   * @param {string} method - HTTP method (GET, POST, etc)
   * @param {string} url - Request URL
   * @param {object} headers - Request headers object
   */
  logRequest: function(requestId, method, url, headers) {
    this.log(requestId, 'open method=' + method);
    
    // Parse and log URL components
    try {
      var urlObj = new URL(url);
      this.log(requestId, 'host=' + urlObj.hostname + ' path=' + urlObj.pathname);
      this.log(requestId, 'protocol=' + urlObj.protocol);
      
      // Log sanitized query parameters (don't log auth tokens, SAT values, device IDs)
      if (urlObj.search) {
        var params = urlObj.search.substring(1).split('&');
        for (var i = 0; i < params.length; i++) {
          var param = params[i].split('=')[0];
          // Only log parameter names, not values (for security)
          if (param !== 'Authorization' && param !== 'auth' && param !== 'token') {
            this.log(requestId, 'param=' + param);
          }
        }
      }
    } catch (e) {
      this.log(requestId, 'url=' + url);
    }

    // Log headers (redacting sensitive ones)
    if (headers) {
      for (var h in headers) {
        if (h.toLowerCase() !== 'authorization' && 
            h.toLowerCase() !== 'cookie' &&
            h.toLowerCase() !== 'x-auth-token') {
          this.log(requestId, 'header ' + h + '=' + headers[h]);
        }
      }
    }
  },

  /**
   * Log request send
   * @param {string} requestId - Unique request identifier
   */
  logSend: function(requestId) {
    this.log(requestId, 'send started');
  },

  /**
   * Log readyState change
   * @param {string} requestId - Unique request identifier
   * @param {number} state - readyState value (0-4)
   * @param {number} status - HTTP status code (if available)
   */
  logStateChange: function(requestId, state, status) {
    var stateNames = ['UNSENT', 'OPENED', 'HEADERS_RECEIVED', 'LOADING', 'DONE'];
    var stateName = stateNames[state] || 'UNKNOWN';
    var statusStr = status ? ' status=' + status : '';
    this.log(requestId, 'readyState=' + state + ' (' + stateName + ')' + statusStr);
  },

  /**
   * Log response headers received
   * @param {string} requestId - Unique request identifier
   * @param {number} status - HTTP status code
   * @param {object} headers - Response headers
   * @param {number} elapsedMs - Elapsed time in milliseconds
   */
  logResponseHeaders: function(requestId, status, headers, elapsedMs) {
    this.log(requestId, 'response headers status=' + status + ' elapsedMs=' + elapsedMs);
    if (headers && headers['content-type']) {
      this.log(requestId, 'content-type=' + headers['content-type']);
    }
  },

  /**
   * Log response body received
   * @param {string} requestId - Unique request identifier
   * @param {number} bodyLength - Response body length in bytes
   * @param {number} elapsedMs - Elapsed time in milliseconds
   */
  logResponseBody: function(requestId, bodyLength, elapsedMs) {
    this.log(requestId, 'response body bytes=' + bodyLength + ' elapsedMs=' + elapsedMs);
  },

  /**
   * Log dispatch event
   * @param {string} requestId - Unique request identifier
   * @param {string} event - Event name (load, error, timeout, etc)
   */
  logEvent: function(requestId, event) {
    this.log(requestId, 'dispatch ' + event);
  },

  /**
   * Log request completion
   * @param {string} requestId - Unique request identifier
   * @param {string} result - Result (success, error, timeout, abort)
   * @param {number} elapsedMs - Total elapsed time
   */
  logCompletion: function(requestId, result, elapsedMs) {
    this.log(requestId, 'completed ' + result + ' elapsedMs=' + elapsedMs);
  },

  /**
   * Log error condition
   * @param {string} requestId - Unique request identifier
   * @param {string} category - Error category (DNS, TLS, CONNECT, HTTP, TIMEOUT, CALLBACK, etc)
   * @param {string} errorMsg - Error message (sanitized)
   */
  logError: function(requestId, category, errorMsg) {
    // Sanitize error message - remove potential tokens/IDs
    var sanitized = errorMsg
      .replace(/token[=:][^\s&;]*/gi, 'token=[REDACTED]')
      .replace(/auth[=:][^\s&;]*/gi, 'auth=[REDACTED]')
      .replace(/([0-9]{1,3}\.){3}[0-9]{1,3}/g, '[IP]');
    
    this.log(requestId, 'error category=' + category + ' msg=' + sanitized);
  }
};

// Export for use
if (typeof module !== 'undefined' && module.exports) {
  module.exports = XHRDiagnostics;
}
