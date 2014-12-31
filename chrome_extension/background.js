// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Called when a message is passed.  We assume that the content script
// wants to show the page action.
/*
chrome.extension.onConnect.addListener(function(port) {
  console.assert(port.name == "conversations");
	port.onMessage.addListener(function(msg){
		if(msg.sentence == "hello"){
			//chrome.pageAction.shown(msg.sentence);
			port.postMessage({sent: "pageAction is activeted"});
			alert("message transfered!!!");
		}else{
			//
			}
		});
});
*/
/*===========================original file================================*/
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

