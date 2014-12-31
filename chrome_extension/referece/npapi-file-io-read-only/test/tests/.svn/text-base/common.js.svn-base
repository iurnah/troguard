var plugin = document.createElement("embed");
plugin.setAttribute("type", "application/x-npapi-file-io");
document.documentElement.appendChild(plugin);

function getPlatform() {
	return plugin.getPlatform();
}

function getPath(filename) {
	if (getPlatform() === "windows") {
		return "C:\\svn\\npapi-file-io\\test\\files\\" + filename;
	} else if (getPlatform() === "linux") {
		return "/home/daw63/wcs/npapi-file-io/test/files/" + filename;
	}
	throw "Unsupported platform";
}

function getPlatformSlash() {
  if (getPlatform() === "windows") {
    return "\\";
  } else {
    return "/";
  }
  throw "Unsupported platform";
}

function getRandom() {
  return Math.floor(Math.random() * 100000000000000);
}

function assertThrows(fun) {
  try {
    fun();
    fail("Expected exception");
  } catch (e) {
    if (e.isJsUnitFailure !== undefined) {
      throw e;
    }
  }
}

function callWithNonStringThrows(fun) {
  assertThrows(function () { fun(1); });
  assertThrows(function () { fun(true); });
  assertThrows(function () { fun({}); });
  assertThrows(function () { fun([]); });
}

function callWithTwoArgumentsThrows(fun) {
  assertThrows(function() { fun("foo", "bar"); });
}
