// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Called when a message is passed.  We assume that the content script
// wants to show the page action.
function onRequest(request, sender, sendResponse) {
  // Show the page action for the tab that the sender (content script)
  // was on.
  chrome.pageAction.show(sender.tab.id);
  
  //popup.html

  // Return nothing to let the connection be cleaned up.
  sendResponse({msg:"got your messages"});
};

// Listen for the content script to send a message to the background page.
chrome.extension.onRequest.addListener(onRequest);

/*
chrome.extension.onConnect.addListener(function(port) {
  console.assert(port.name == "recursiveSearch");
  port.onMessage.addListener(function(msg) {    
    // I got the checkedLink at here, what I need to do is use it to traverse DOM
    var totalLinksElement = document.getElementByTagName('a');
    //var totalLinksNums = totalLinks.length;
    for(var i = 0; i < totalLinksElement.length; ++i){
			if(msg.checkedHref.test(totalLinksElement[i])){
				var appDescription = totalLinksElelment[i].innerHTML;
				port.postMessage({Description: appDescription});
				}else 
				console.log('Did not find the app description for the ext-link');
			}
  });
});
*/
