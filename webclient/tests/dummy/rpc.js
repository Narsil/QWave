if ( !window.JSOT )
	JSOT = { };
  
JSOT.Rpc = {
	sessionId : "",
	clientSequenceNr : 0,
	serverSequenceNr : 0,
	serverAck : 0,
	jid : null,
	waveDomain : "wave1.vs.uni-due.de",
	queue : { },
	pendingSubmitUrl : null
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
	var submit = new waveserver.ProtocolSubmitRequest();
	submit.wavelet_name = wavelet.url().toString();
	submit.delta = new protocol.ProtocolWaveletDelta();
	submit.delta.author = JSOT.Rpc.jid;
	submit.delta.hashed_version = wavelet.hashed_version;
	submit.delta.operation = operations;

	JSOT.Rpc.processSubmitRequest( wavelet, submit, openWavelet );
/*

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
*/
};

JSOT.Rpc.openWavelet = function( wavelet_name )
{
	var url = new JSOT.WaveUrl( wavelet_name );
	
	// Make sure that the wave and wavelet objects exist locally
	var wave = JSOT.Wave.getWave( url.waveId, url.waveDomain );
	var wavelet = wave.getWavelet( url.waveletId, url.waveletDomain );
	
    var r = new webclient.Request();
    r.session_id = JSOT.Rpc.sessionId;
    r.client_ack = JSOT.Rpc.serverSequenceNr;
    r.client_sequence_number = ++(JSOT.Rpc.clientSequenceNr);
    r.open = new waveserver.ProtocolOpenRequest();
    r.open.participant_id = JSOT.Rpc.jid;
	r.open.wave_id = url.waveDomain + "!" + url.waveId;
	r.open.wavelet_id_prefix.push( url.waveletDomain + "!" + url.waveletId );
    var json = JSON.stringify(r.serialize());
    JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );
};

JSOT.Rpc.processSubmitRequest = function( wavelet, submitRequest, openWavelet )
{
	// Enqueue the delta. It might be needed for OT
	var url = wavelet.url().toString();
	var q = JSOT.Rpc.queue[ url ];
	if ( !q )
	{
		q = { outgoing : [ submitRequest ] };
		JSOT.Rpc.queue[ url ] = q;
	}
	else
		q.outgoing.push( submitRequest );
	if ( openWavelet )
		q.openWavelet = true;
		
	// Apply the delta locally
	JSOT.Wave.processSubmit( submitRequest );

	// Print the wave1
	var c = document.createElement("pre");
	c.appendChild( document.createTextNode( wavelet.toString() ) );
	document.getElementById("out").appendChild(c);	

	JSOT.Rpc.sendNextSubmitRequest();
};

window.qin = [];

JSOT.Rpc.processUpdate = function( update )
{
	window.qin.push( update );
	
	// Is this the echo to our submit? TODO: This sucks like hell.
	if ( JSOT.Rpc.pendingSubmitUrl == update.wavelet_name )
	{
		var q = JSOT.Rpc.queue[JSOT.Rpc.pendingSubmitUrl];
		if ( q.updateMissing )
		{
			for( var i = 0; i < update.applied_delta.length; ++i )
			{
				if ( update.applied_delta[i].author == JSOT.Rpc.jid )
				{
					q.updateMissing = false;
					if ( !q.submitResponseMissing )
					{
						q.outgoing.shift();
						q.sent = false;
						if ( q.outgoing.length == 0 )
							delete JSOT.Rpc.queue[JSOT.Rpc.pendingSubmitUrl];
						delete JSOT.Rpc.pendingSubmitUrl;  
					}
					var wavelet = JSOT.Wavelet.getWavelet( update.wavelet_name );
					wavelet.hashed_version = update.resulting_version;
					return;
				}
			}
		}
	}
	
	var q = JSOT.Rpc.queue[update.wavelet_name];
	// No pending submits? -> No OT
	if ( q )
	{
		// Transform the received delta and transform the operations which have not
		// yet been acknowledged by the server
		// Disabled because of a bug in the google code: Q_ASSERT( incoming.version().version == m_serverMsgCount );
		for( var i = 0; i < q.outgoing.length; ++i )
		{
			var outgoing_submit = q.outgoing[i];
			for( var c = 0; c < outgoing_submit.delta.operation.length; ++c )
			{
				for( var a = 0; a < update.applied_delta.length; ++a )
				{
					var incoming_delta = update.applied_delta[a];
					for( var s = 0; s < incoming_delta.operation.length; ++s )
					{
						protocol.ProtocolWaveletOperation.xform( incoming_delta.operation[s], outgoing_submit.delta.operation[c] )
					}
				}
            }
        }
	}
		
	// Apply the deltas locally
	JSOT.Rpc.applyUpdate( update );
};

JSOT.Rpc.processSubmitResponse = function( submitResponse )
{
  	var q = JSOT.Rpc.queue[JSOT.Rpc.pendingSubmitUrl];
	q.submitResponseMissing = false;
	
	if ( q.outgoing[0].delta.operation.length != submitResponse.operations_applied )
		throw "Submit failed";
	
	if ( !q.updateMissing )
	{
		var submitRequest = q.outgoing.shift();
		q.sent = false;
		if ( q.outgoing.length == 0 )
			delete JSOT.Rpc.queue[JSOT.Rpc.pendingSubmitUrl];
		delete JSOT.Rpc.pendingSubmitUrl;  
	}
};

JSOT.Rpc.sendNextSubmitRequest = function()
{
  	if ( JSOT.Rpc.pendingSubmitUrl )
		return false;
		
	for( var u in JSOT.Rpc.queue )
	{
		var q = JSOT.Rpc.queue[u];
		if ( !q.sent )
		{
			var submitRequest = q.outgoing[ 0 ];
			var wavelet = JSOT.Wavelet.getWavelet( submitRequest.wavelet_name );
			submitRequest.delta.hashed_version = wavelet.hashed_version;
			
			JSOT.Rpc.pendingSubmitUrl = submitRequest.wavelet_name;
			q.sent = true;
			q.updateMissing = true;
			q.submitResponseMissing = true;
			
			var r = new webclient.Request();
			r.session_id = JSOT.Rpc.sessionId;
			r.client_ack = JSOT.Rpc.serverSequenceNr;
			r.client_sequence_number = ++(JSOT.Rpc.clientSequenceNr);
			r.submit = submitRequest;

			if ( q.openWavelet )
			{
				var url = new JSOT.WaveUrl( submitRequest.wavelet_name );
				delete q.openWavelet;
				r.open = new waveserver.ProtocolOpenRequest();
				r.open.participant_id = JSOT.Rpc.jid;
				r.open.wave_id = url.waveDomain + "!" + url.waveId;
				r.open.wavelet_id_prefix.push( url.wavletDomain + "!" + url.waveletId );
			}
		
			var json = JSON.stringify(r.serialize());
			JSOT.Rpc.callServer( json, JSOT.Rpc.onMessage );  		
			return true;
		}
	}
};

JSOT.Rpc.applyUpdate = function( update )
{
	try
	{
		JSOT.Wave.processUpdate( update );
	
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
	}
	catch( e )
	{
		console.log(e);
	}
};

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
				{
					try
					{
						callback(xmlHttp.responseText);						
					}
					catch( e )
					{
						console.log(e);
					}
				}
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
		return;
	}
	else if (response.has_update())
	{
		var update = response.update;
		JSOT.Rpc.processUpdate( update );
	}
	else if ( response.has_submit() )
	{
		JSOT.Rpc.processSubmitResponse( response.submit );
	}
	
	// Is it possible to send more submits?
	if ( JSOT.Rpc.sendNextSubmitRequest() )
		return;
	
	if ( response.has_login() || response.has_update() || response.has_submit() )
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