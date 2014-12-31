This directory contain the working files about create a extension for 
The DriveBy Download project. the following directory is extensions that
I modified or wrote:

./reference
	All the other directory are reference extensions, from which I draw some 
	insight.

./DrivebyDownload_v0.0
	/*Include the plugin creation*/
	This is just trigger the notification and start firefox by checking the 
	popup page, which is involked from page action. the contentscript.js 
	detect the download key word.

./DrivebyDownload_v0.1
	This version is the one that used for testing and produce the 
	./DrivebyDownload_v0.0 as shown bellow.

./DrivebyDownload_v0.2
	this version removed popup page. The effect of it is to display two alert
	window which indicate the longlived communication is successful. what's 
	more, it also detect click link (onclick = function(){alert();}) action in a page, 

./DrivebyDownload_v0.3
	This version is start with the v0.2 to complete the frame work of both print and
	detect the download click. may open another configuration page when the user want
	download one file from the



