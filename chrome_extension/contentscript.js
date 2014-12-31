// This content scripts will do the following for the extension
// 1. get the current page and do the regex search for pageaction.
// 2. get the current page and find out the link in it (key term search)
// 3. Send the page action and the links to background page.
// 4. Receive the checked link in the popup page and search the DOMs
// 5. Send back the searched results.
var regex = /download/;
var port = chrome.extension.connect({name: "conversations"});
	port.postMessage({sentence: "hello"});
	port.onMessage.addListener(function(msg){
		if(msg.sent == "pageAction is activeted")
			alert("communicated!!!")
		});

/*==========================original file===================================*/
//var regex = /download/;

// Test the text of the body element against our regular expression.
if (regex.test(document.body.innerText)) {
  // The regular expression produced a match, so notify the background page.
  chrome.extension.sendRequest({}, function(response) {});
} else {
  // No match was found.
  //alert("No Download Item found.");
}


