// This content scripts will do the following for the extension
// 1. get the current page and do the regex search for the keywords pre-defined (var appTypes)
// 2. search keywords using regex and send back to the background page.

//This is to modified to remove the popup page. just inject onclick to 
//href element.

//Global virable for the application keywords, the list can be expanded based on our exmperiment.
var appTypes = new Array("game","antivirus","email","reader","browser") ;

//sort function for the keywords frequency
function sortObject(obj) {
    var arr = [];
    var prop;
    for (prop in obj) {
        if (obj.hasOwnProperty(prop)) {
            arr.push({
                'key': prop,
                'value': obj[prop]
            });
        }
    }
    
    arr.sort(function(a, b) {
        return b.value - a.value;
    });
    return arr; // returns array
}

//extract and sort the keywords
function keywordsFreqSorting(){
	var keywordsObj={};
	for(var i=0; i< appTypes.length; i++){
		var regx = new RegExp(appTypes[i],'gi'); // /appTypes[i]/gi;
		matches = document.body.innerText.match(regx);
		if(matches){
			keywordsObj[appTypes[i]] = matches.length;	
		}else{
			keywordsObj[appTypes[i]] = 0; //those keywords not appeared 
		}
	}

	var sortedKeywords = sortObject(keywordsObj);
	return sortedKeywords;
}

//send the keywords object to the background page.
var results = keywordsFreqSorting();
chrome.extension.sendMessage({results: results}, function(response) {});

//send the createpopup command
function downloadCheckedLinks() {
	if (confirm('Download this file?'))
		chrome.extension.sendMessage({type:'createpopup'});	
}

var hrefNodes = document.getElementsByTagName('a');
for(var x =0; x < hrefNodes.length; ++x){
	if(hrefNodes[x].hasAttribute("href")){
		hrefNodes[x].onclick = downloadCheckedLinks;
	}
}
