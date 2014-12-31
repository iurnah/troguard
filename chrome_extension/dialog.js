// mainly to get the sorted keywords list from background page,
// use those keywords to update the option for the selection box.

var no1 = chrome.extension.getBackgroundPage().keyword1;
var no2 = chrome.extension.getBackgroundPage().keyword2;
var no3 = chrome.extension.getBackgroundPage().keyword3;
var no4 = chrome.extension.getBackgroundPage().keyword4;
var no5 = chrome.extension.getBackgroundPage().keyword5;
var no6 = chrome.extension.getBackgroundPage().keyword6;
var no7 = chrome.extension.getBackgroundPage().keyword7;
var no8 = chrome.extension.getBackgroundPage().keyword8;
var no9 = chrome.extension.getBackgroundPage().keyword9;
var no10 = chrome.extension.getBackgroundPage().keyword10;

var selectbox = document.getElementById("dropdown");

selectbox.options[1].text = no1 + " (Recommendation level:  very likely)";
selectbox.options[2].text = no2 + " (Recommendation level:  likely)";
selectbox.options[3].text = no3 + " (Recommendation level:  probably)";
selectbox.options[4].text = no4 + " (Recommendation level:  probably)";
selectbox.options[5].text = no5 + " (Recommendation level:  possibly)";
selectbox.options[6].text = no6 + " (Recommendation level:  possibly)";
selectbox.options[7].text = no7 + " (Recommendation level:  very unlikely)";
selectbox.options[8].text = no8 + " (Recommendation level:  very unlikely)";
selectbox.options[9].text = no9 + " (Recommendation level:  very unlikely)";
selectbox.options[10].text = no10 + " (Recommendation level:  very unlikely)";
