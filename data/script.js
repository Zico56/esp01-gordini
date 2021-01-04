var ajaxRequest = null;

if (window.XMLHttpRequest) { 
	ajaxRequest = new XMLHttpRequest(); 
}
else { 
	ajaxRequest = new ActiveXObject("Microsoft.XMLHTTP"); 
}

function post(e) { 
	if(e.target.checked) {
		ajaxLoad('on', e.target.id);
	}
	else {
		ajaxLoad('off', e.target.id);
	}
}

function ajaxLoad(ajaxURL, id) {
	if(!ajaxRequest){ 
		alert("AJAX is not supported."); 
		return;
	}
	ajaxRequest.open("GET",ajaxURL+"?id="+id,true);
	ajaxRequest.onreadystatechange = function() {
		if (ajaxRequest.readyState == 4 && ajaxRequest.status==200) {
			var ajaxResult = ajaxRequest.responseText;
		}
	}
	ajaxRequest.send();
}	