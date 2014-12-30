// This content scripts will do the following for the extension
// 1. get the background page and find out the link in it (key term search)
// 2. 

var regex = /download/;

// Test the text of the body element against our regular expression.
if (regex.test(document.body.innerText)) {
  // The regular expression produced a match, so notify the background page.
  chrome.extension.sendRequest({}, function(response) {});
} else {
  // No match was found.
  //alert("No Download Item found.");
}


