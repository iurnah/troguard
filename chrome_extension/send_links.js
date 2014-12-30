// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Send back to the popup a sorted deduped list of valid link URLs on this page.
// The popup injects this script into all frames in the active tab.

//convert the "document.getElementsByTagName('a')" into array. 
var links = [].slice.apply(document.getElementsByTagName('a'));
links = links.map(function(element) {//here element is the links value
  
  // Return an anchor's href attribute, stripping any URL fragment (hash '#').
  // If the html specifies a relative path, chrome converts it to an absolute
  // URL.
  var href = element.href;
  var hashIndex = href.indexOf('#');
  if (hashIndex >= 0) {
    href = href.substr(0, hashIndex);
  }
  return href;
});

links.sort();

// Remove duplicates and invalid URLs.
var kBadPrefix = 'javascript';
for (var i = 0; i < links.length;) {
  if (((i > 0) && (links[i] == links[i - 1])) ||
      (links[i] == '') ||
      (kBadPrefix == links[i].toLowerCase().substr(0, kBadPrefix.length))) {
    links.splice(i, 1);
  } else {
    ++i;
  }
}

//long lived connection:
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


chrome.extension.sendRequest(links);


