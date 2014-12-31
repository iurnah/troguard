// This content scripts will do the following for the extension
// 1. get the current page and do the regex search for pageaction.
// 2. get the current page and find out the link in it (key term search)
// 3. Send the page action and the links to background page.
// 4. Receive the checked link in the popup page and search the DOMs
// 5. Send back the searched results.
//This is to modified to remove the popup page. just inject onclick to 
//href element.

// Download all visible checked links.
/*
function downloadCheckedLinks() {
	for (var i = 0; i < visibleLinks.length; ++i) {
		if (document.getElementById('check' + i).checked) {			
				chrome.downloads.download({url: visibleLinks[i]},function(id){
				var notification = window.webkitNotifications.createNotification('', 'OMG!', 'Hello within for, succeed!');
				notification.show();
				// this is to get the background page and run the startFirefox() 
				//function that is defined as a plugin API called by the browser.
				var bgPage = chrome.extension.getBackgroundPage();
				bgPage.plugin.startFirefox();
				
				var bgPage = chrome.extension.getBackgroundPage();
				bgPage.searchDOM();
			});
		
		alert("function execurted!");
		}
	}
	window.close();
}
*/

function downloadCheckedLinks(hrefNodes) {
	alert('click link detected!!!');
	//alert(hrefNodes[0]);
}

var hrefNodes = document.getElementsByTagName('a');
for(var x =0; x < hrefNodes.length; ++x){
	if(hrefNodes[x].hasAttribute("href")){
	hrefNodes[x].onclick = downloadCheckedLinks(hrefNodes);
	
	//alert('has the href attribute!!');
	}
}

// This is the long-lived communication part.
var port = chrome.extension.connect({name: "conversations"});
	port.postMessage({sentence: "hello"});
	port.onMessage.addListener(function(msg){
		if(msg.sent == "pageAction is activeted")
			alert("communicated!!!")
		});

/*==========================original file===================================
var regex = /download/;

// Test the text of the body element against our regular expression.
if (regex.test(document.body.innerText)) {
  // The regular expression produced a match, so notify the background page.
  chrome.extension.sendRequest({}, function(response) {});
} else {
  // No match was found.
  //alert("No Download Item found.");
}
*/

