// two functions: 1. lister the message shared from contentscripts.
//				  2. Create the popup when user authorize this.	 	
//
// Reference: http://stackoverflow.com/questions/10340481/popup-window-in-chrome-extension
//

var rankedKeywords;
var keyword1;
var keyword2;
var keyword3;
var keyword4;
var keyword5;
var keyword6;
var keyword7;
var keyword8;
var keyword9;
var keyword10;

var start = new Date().getMilliseconds();
chrome.extension.onMessage.addListener(function(request, sender, sendResponse){

	rankedKeywords = request.results;
	keyword1 = rankedKeywords[0].key;
	keyword2 = rankedKeywords[1].key;
	keyword3 = rankedKeywords[2].key;
	keyword4 = rankedKeywords[3].key;
	keyword5 = rankedKeywords[4].key;
	keyword6 = rankedKeywords[5].key;
	keyword7 = rankedKeywords[6].key;
	keyword8 = rankedKeywords[7].key;
	keyword9 = rankedKeywords[8].key;
	keyword10 = rankedKeywords[9].key;
	
	sendResponse({});
});

//create the popup window when receive request from
chrome.extension.onMessage.addListener(function(request) {
    if (request.type === 'createpopup') {
        chrome.tabs.create({
            url: chrome.extension.getURL('dialog.html'),
            active: false
        }, function(tab) {
            // After the tab has been created, open a window to inject the tab
            chrome.windows.create({
                tabId: tab.id,
                type: 'popup',
                //type: 'normal',
                focused: true,
                top: 350,	
                left: 700,
                width: 500,
                height: 500
                
            });
			
        });
    }
var stop = new Date().getMilliseconds();
var executionTime = stop - start;
//alert("execution time:" + executionTime + " milliseconds");
});





