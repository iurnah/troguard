/*var hrefNodes = document.getElementsByAttribute('href',"http://www.w3schools.com");
for (var i = 0; i<hrefNodes.length; ++i){
	console.log(document.getElementsByAttribute('href', "http://www.w3schools.com")[0].innerText);
}


for(var x =0; x < hrefNodes.length; ++x){
	hrefNodes[x].onclick = downloadCheckedLinks;
	alert('get the attribute!!');
}

//alert("this is worked");
*/
window.onload = function(){
alert(document.getElementsByTagName('a')[0].innerText);
document.getElementsByTagName('a')[0].onclick = function(){
	alert('add onclick successful.');
	}
}

console.log(document.getElementsByTagName('a')[0].innerText);

