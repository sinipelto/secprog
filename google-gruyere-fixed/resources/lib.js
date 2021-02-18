/* Copyright 2017 Google Inc. */
/**
 * Swaps the display settings of two elements. Before calling, exactly one
 * of the two elements should have style="display:none" and it shows the one
 * that is hidden and hides the one that is shown.
 */
function _showHide(id1, id2) {
  var x1 = document.getElementById(id1);
  var x2 = document.getElementById(id2);
  var shown = x1.style.display != '';
  x1.style.display = shown ? '' : 'none';
  x2.style.display = shown ? 'none' : '';
};

/**
 * Refreshes the current page by loading the url and then passing the
 * json response object to the callback.
 */
function _refresh(url, callback) {
  var httpRequest = window.XMLHttpRequest ? new XMLHttpRequest()
      : new ActiveXObject("Microsoft.XMLHTTP");
  httpRequest.onreadystatechange = function() {
    if (httpRequest.readyState == 4) {
      _feed = callback;
      const resp = httpRequest.responseText
          .replaceAll("\r\n", "")
          .replaceAll("\r", "")
          .replaceAll("\n", "")
      const obj = safeJSONParse(resp);
      if (obj !== null)
        _feed(obj);
      httpRequest = null;
    }
  }
  httpRequest.open("GET", url, true);
  httpRequest.send(null);
};

/**
 * Processes refresh response {'private_snippet':snippet, user:snippet, ...}
 */
function _finishRefreshHome(response) {
  for (var uid in response) {
    var element = document.getElementById(uid);
    if (element) {
      element.innerHTML = response[uid];
    }
  }
};

function _refreshHome(uniqueId, actionToken) {
    console.log(actionToken);
  _refresh("/" + uniqueId + "/feed.gtl" + "?action_token=" + actionToken, _finishRefreshHome);
};

/**
 * Processes refresh response [name, snippet, ...]
 */
function _finishRefreshSnippets(response) {
  var n = response.length;
  var element = document.getElementById('user_name');
  if (n && element) {
    element.innerHTML =
        JSON.parse(response[0]
            .replaceAll("<", "&lt")
            .replaceAll(">", "\x3e"));
  }

  // The snippet divs are numbered 0, 1, ...
  for (var i = 1; i < n; ++i) {
    var element = document.getElementById(i - 1);
    if (element) {
      element.innerHTML = response[i];
    }
  }
};

function _refreshSnippets(uniqueId, uid) {
  var u = uid
      .replaceAll("<", "\x3c")
      .replaceAll(">", "\x3e");
  _refresh("/" + uniqueId + "/feed.gtl?uid=" + u, _finishRefreshSnippets);
};

// pass expected list of properties and optional maxLen
// returns obj or null
function safeJSONParse(str, propArray, maxLen) {
    var parsedObj, safeObj = {};
    try {
        if (maxLen && str.length > maxLen) {
            return null;
        } else {
            parsedObj = JSON.parse(str);
            if (typeof parsedObj !== "object" || Array.isArray(parsedObj)) {
                safeObj = parseObj;
            } else {
                // copy only expected properties to the safeObj
                propArray.forEach(function(prop) {
                    if (parsedObj.hasOwnProperty(prop)) {
                        safeObj[prop] = parseObj[prop];
                    }
                });
            }
            return safeObj;
        }
    } catch(e) {
        return null;
    }
}