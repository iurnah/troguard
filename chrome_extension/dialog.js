// mainly to get the sorted keywords list from background page,
// use those keywords to update the option for the selection box.

var no1 = chrome.extension.getBackgroundPage().keyword1;
var no2 = chrome.extension.getBackgroundPage().keyword2;
var no3 = chrome.extension.getBackgroundPage().keyword3;
var no4 = chrome.extension.getBackgroundPage().keyword4;
var no5 = chrome.extension.getBackgroundPage().keyword5;
var selectbox = document.getElementById("dropdown");

selectbox.options[1].text = no1 + " (Recommendation Level: very likely)";
selectbox.options[2].text = no2 + " (Recommendation Level: likely)";
selectbox.options[3].text = no3 + " (Recommendation Level: probably)";
selectbox.options[4].text = no4 + " (Recommendation Level: possibly)";
selectbox.options[5].text = no5 + " (Recommendation Level: very rare)";
