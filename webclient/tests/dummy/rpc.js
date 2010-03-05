function callServer(jsonData, callback)
{
  var xmlHttp = null;
  try {
      // Mozilla, Opera, Safari sowie Internet Explorer (ab v7)
      xmlHttp = new XMLHttpRequest();
  } catch(e) {
    try {
        // MS Internet Explorer (ab v6)
        xmlHttp  = new ActiveXObject("Microsoft.XMLHTTP");
    } catch(e) {
        try {
            // MS Internet Explorer (ab v5)
            xmlHttp  = new ActiveXObject("Msxml2.XMLHTTP");
        } catch(e) {
            xmlHttp  = null;
        }
    }
  }
  if (xmlHttp) {
    var c = document.createElement("pre");
    c.appendChild( document.createTextNode("OUT:") );
    c.appendChild( document.createTextNode(jsonData) );
    document.getElementById("out").appendChild(c);
    
    xmlHttp.open('POST', 'wave.fcgi', true);
    xmlHttp.onreadystatechange = function () {
        if (xmlHttp.readyState == 4) {

	  var c = document.createElement("pre");
	  c.appendChild( document.createTextNode("IN:") );
	  c.appendChild( document.createTextNode(xmlHttp.responseText) );
	  document.getElementById("out").appendChild(c);
	  
	  if( callback )
	    callback(xmlHttp.responseText);
        }
    };
    xmlHttp.send(jsonData);
    return true;
  }
  return false;
}