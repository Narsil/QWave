if ( !window.JSOT )
	JSOT = { };
  
JSOT.Rpc = {
	sessionId : "",
	clientSequenceNr : 0,
	serverSequenceNr : 0,
	serverAck : 0,
	jid : null,
	waveDomain : "wave1.vs.uni-due.de"
};

JSOT.Rpc.login = function( jid )
{
	JSOT.Rpc.jid = jid;
	
    var r = new webclient.Request();
	r.session_id = JSOT.Rpc.sessionId;
	r.client_ack = JSOT.Rpc.serverSequenceNr;
	r.client_sequence_number = ++(JSOT.Rpc.clientSequenceNr);
	r.login = new webclient.LoginRequest();
	r.login.jid = jid;
	var json = JSON.stringify(r.serialize());
	JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );
};

JSOT.Rpc.openWavelets = { };

JSOT.Rpc.submitOperation = function( wavelet, op, openWavelet )
{
	JSOT.Rpc.submitOperations( wavelet, [op], openWavelet );
};

JSOT.Rpc.submitOperations = function( wavelet, operations, openWavelet )
{
	var r = new webclient.Request();
	r.session_id = JSOT.Rpc.sessionId;
	r.client_ack = JSOT.Rpc.serverSequenceNr;
	r.client_sequence_number = ++(JSOT.Rpc.clientSequenceNr);
	r.submit = new waveserver.ProtocolSubmitRequest();
	r.submit.wavelet_name = wavelet.url().toString();
	r.submit.delta = new protocol.ProtocolWaveletDelta();
	r.submit.delta.author = JSOT.Rpc.jid;
	r.submit.delta.hashed_version = wavelet.hashed_version;
	r.submit.delta.operation = operations;

	if ( openWavelet )
	{
		r.open = new waveserver.ProtocolOpenRequest();
		r.open.participant_id = JSOT.Rpc.jid;
		r.open.wave_id = wavelet.wave.id;
		r.open.wavelet_id_prefix.push( wavelet.id );
	}
	  
	var json = JSON.stringify(r.serialize());
	JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );  
};

JSOT.Rpc.openWavelet = function( urlString )
{
	var url = new JSOT.WaveUrl( urlString );
	
    var r = new webclient.Request();
    r.session_id = JSOT.Rpc.sessionId;
    r.client_ack = JSOT.Rpc.serverSequenceNr;
    r.client_sequence_number = ++(JSOT.Rpc.clientSequenceNr);
    r.open = new waveserver.ProtocolOpenRequest();
    r.open.participant_id = JSOT.Rpc.jid;
	if ( url.waveDomain != JSOT.Rpc.waveDomain )
		r.open.wave_id = url.waveDomain + "!" + url.waveId;
	else
		r.open.wave_id = url.waveId;
	r.open.wavelet_id_prefix.push( url.waveletId );
    var json = JSON.stringify(r.serialize());
    JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );
  }

JSOT.Rpc.callServer = function(jsonData, callback)
{
	var xmlHttp = null;
	try
	{
		// Mozilla, Opera, Safari sowie Internet Explorer (ab v7)
		xmlHttp = new XMLHttpRequest();
	} 
	catch(e)
	{
		try
		{
			// MS Internet Explorer (ab v6)
			xmlHttp  = new ActiveXObject("Microsoft.XMLHTTP");
		}
		catch(e)
		{
			try {
				// MS Internet Explorer (ab v5)
				xmlHttp  = new ActiveXObject("Msxml2.XMLHTTP");
			}
			catch(e)
			{
				xmlHttp  = null;
			}
		}
	}
	if (xmlHttp)
	{
		var c = document.createElement("pre");
		c.appendChild( document.createTextNode("OUT:") );
		c.appendChild( document.createTextNode(jsonData) );
		document.getElementById("out").appendChild(c);
    
		xmlHttp.open('POST', 'wave.fcgi', true);
		xmlHttp.onreadystatechange = function ()
		{
			if (xmlHttp.readyState == 4)
			{
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
};

JSOT.Rpc.onMessage = function(jsonData)
{
  var response = webclient.Response.parse( JSON.parse( jsonData ) );
  // Get the ack of the server
  JSOT.Rpc.serverAck = Math.max(response.server_ack, JSOT.Rpc.serverAck);
  // Get the latest server sequence number  
  if ( response.server_sequence_number > 0 )
    JSOT.Rpc.serverSequenceNr = response.server_sequence_number;
	
	// Login?
  if ( response.has_login() )
  {
    var login = response.login;    
    JSOT.Rpc.sessionId = login.session_id;
    document.getElementById("sessionId").innerHTML = JSOT.Rpc.sessionId;

	// After login open the index wave
    var r = new webclient.Request();
    r.session_id = JSOT.Rpc.sessionId;
    r.client_ack = JSOT.Rpc.serverSequenceNr;
    r.client_sequence_number = ++(JSOT.Rpc.clientSequenceNr);
    r.open = new waveserver.ProtocolOpenRequest();
    r.open.participant_id = JSOT.Rpc.jid;
    r.open.wave_id = "!indexwave";
    var json = JSON.stringify(r.serialize());
    JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );
  }
  else if (response.has_update())
  {
    var update = response.update;
	
	try
	{
	JSOT.Wave.process( update );
	
	// Print the wave1
	var url = new JSOT.WaveUrl( update.wavelet_name );
	var wave = JSOT.Wave.getWave( url.waveId, url.waveDomain );
	for( var a in wave.wavelets )
	{
		var wavelet = wave.wavelets[a];
		var c = document.createElement("pre");
		c.appendChild( document.createTextNode( wavelet.toString() ) );
		document.getElementById("out").appendChild(c);
	}
	} catch( e )
	{
	  console.log(e);
	}
	
	// Ask for more
    var r = new webclient.Request();
    r.session_id = JSOT.Rpc.sessionId;
    r.client_ack = JSOT.Rpc.serverSequenceNr;
    r.client_sequence_number = 0;
    var json = JSON.stringify(r.serialize());
    JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );
  }
  else if ( response.has_submit() )
  {
	// Ask for more
    var r = new webclient.Request();
    r.session_id = JSOT.Rpc.sessionId;
    r.client_ack = JSOT.Rpc.serverSequenceNr;
    r.client_sequence_number = 0;
    var json = JSON.stringify(r.serialize());
	JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );
  }
};
