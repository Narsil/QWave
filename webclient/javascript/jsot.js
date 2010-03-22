if ( !window.JSOT )
	/**
	  * @namespace the JavaScript Operation Transformation library
	  */
	JSOT = { };

/////////////////////////////////////////////////
//
// WaveURL
//
/////////////////////////////////////////////////

/**
 * Creates a new wave url by parsing a string. A parsing error results in a null url (see isNull).
 * @constructor
 *
 * @param {string} url a string of the form wave://waveletId/waveDomain$waveId/waveletId or the
 *           short format wave://waveletId/waveId/waveletId.
 *			 Passing a null value results in a null url (see isNull)
 */
JSOT.WaveUrl = function(url)
{
	if ( url )
	{
		var uriParts = new RegExp("^(?:([^:/?#.]+):)?(?://)?(([^:/?#]*)(?::(\\d*))?)?((/(?:[^?#](?![^?#/]*\\.[^?#/.]+(?:[\\?#]|$)))*/?)?([^?#/]*))?(?:\\?([^#]*))?(?:#(.*))?").exec(url);
		var decoded = {};
		var uriPartNames = ["source","protocol","authority","domain","port","path","directoryPath","fileName","query","anchor"];    
		for(var i = 0; i < 10; i++)
		{
			decoded[uriPartNames[i]] = (uriParts[i] ? uriParts[i] : "");
		}	
		if ( decoded.protocol != "wave" )
			return;
		this.waveletDomain = decoded.domain;
		var wave = decoded.path.substring(1,decoded.path.length);
		var i = wave.indexOf('/');
		if ( i == -1 )
			return;
		this.waveletId = wave.substring( i + 1, wave.length );
		wave = wave.substr(0,i);
		var i = wave.indexOf('$');
		if ( i == -1 )
		{
		  this.waveDomain = this.waveletDomain;
		  this.waveId = wave;
		}
		else
		{
		  this.waveDomain = wave.substr(0,i);
		  this.waveId = wave.substring(i+1,wave.length);
		}
	}	
};

/**
 * The id of the wave.
 * @type string
 */
JSOT.WaveUrl.prototype.waveId = null;
/**
 * The domain of the wave.
 * @type string
 */
JSOT.WaveUrl.prototype.waveDomain = null;
/**
 * The id of the wavelet.
 * @type string
 */
JSOT.WaveUrl.prototype.waveletId = null;
/**
 * The domain of the wavelet.
 * @type string
 */
JSOT.WaveUrl.prototype.waveletDomain = null;

/**
 * @return true if the url is null.
 */
JSOT.WaveUrl.prototype.isNull = function()
{
	return this.waveletId == null;
};

/**
 * @return a URL of the form wave://waveletDomain/waveDomain$waveId/waveletId or null.
 */
JSOT.WaveUrl.prototype.toString = function()
{
	if ( this.isNull() )
		return null;
	var str = "wave://" + this.waveletDomain + "/";
	if ( this.waveDomain == this.waveletDomain )
		str += this.waveId;
	else
		str += this.waveDomain + "$" + this.waveId;
	str += "/" + this.waveletId;
	return str;
};

/////////////////////////////////////////////////
//
// Wave
//
/////////////////////////////////////////////////

/**
 * Do not create waves directly. Use getWave instead.
 * @constructor
 * @see JSOT#getWave
 */
JSOT.Wave = function(id, domain)
{
	JSOT.Wave.waves[ id + "$" + domain] = this;
	this.id = id;
	this.domain = domain;
	this.wavelets = { };
};

/**
 * Dictionary of all wave objects. This is NOT equal to all waves a user might have access to.
 * @private
 */
JSOT.Wave.waves = { };

/**
 * Gets or creates a wave.
 *
 * @param id is the wave-id, such as "w+1234".
 * @param domain is the domain of the wave such as "foobar.com".
 */
JSOT.Wave.getWave = function(id, domain)
{
	var wname = id + "$" + domain;
	var w = JSOT.Wave.waves[wname];
	if ( w )
		return w;
	w = new JSOT.Wave( id, domain );
	JSOT.Wave.waves[wname] = w;
	return w;
};

/**
 * Called when a new wavelet update is received from the server.
 *
 * @param update is an instance of waverserver.ProtocolWaveletUpdate.
 */
JSOT.Wave.processUpdate = function( update )
{
	var url = new JSOT.WaveUrl( update.wavelet_name );
	if ( url.isNull() ) throw "Malformed wave URL";
	var wave = JSOT.Wave.getWave( url.waveId, url.waveDomain );
	var wavelet = wave.getWavelet( url.waveletId, url.waveletDomain );
	for( var i = 0; i < update.applied_delta.length; ++i )
		wavelet.applyDelta( update.applied_delta[i] );
	if ( update.has_resulting_version() )
		wavelet.hashed_version = update.resulting_version;
};

/**
 * Called when a submit has been generated. The submit ist first applied locally
 * using this function. This function is called BEFORE the submit is sent to the server.
 *
 * @param submitRequest is an instance of waveserver.ProtocolSubmitRequest.
 */
JSOT.Wave.processSubmit = function( submitRequest )
{
	var wavelet = JSOT.Wavelet.getWavelet( submitRequest.wavelet_name );
	wavelet.applyDelta( submitRequest.delta );
	// wavelet.hashed_version = update.resulting_version;
};

/**
 * Gets or creates a wavelet of this wave.
 *
 * @param id is the wavelet-id, such as "conv+root".
 * @param domain is the domain of the wavelet such as "foobar.com". 
 */
JSOT.Wave.prototype.getWavelet = function(id, domain)
{
	var wname = id + "$" + domain;
	var w = this.wavelets[wname];
	if ( w )
		return w;
	w = new JSOT.Wavelet(this, id, domain);
	this.wavelets[wname] = w;
	return w;
};

/////////////////////////////////////////////////
//
// Wavelet
//
/////////////////////////////////////////////////

/**
 * Do not use this function directly. Use Wave.getWavelet instead.
 * @constructor
 *
 * @param wave is the wave containing the wavelet.
 * @param id is the wavelet-id, such as "conv+root".
 * @param domain is the domain of the wavelet such as "foobar.com".   
 */
JSOT.Wavelet = function(wave, id, domain)
{
	/**
	 * The id of the wave, for example "conv+root"
	 * @type string
	 */
	this.id = id;
	/**
	 * The domain of the wave, for example "example.com"
	 * @type string
	 */
	this.domain = domain;
	/**
	 * The wave to which this wavelet belongs
	 * @type JSOT.Wave
	 */	
	this.wave = wave;
	this.documents = { };
	/**
	 * The participants of the wave.
	 * @type string[]
	 */
	this.participants = [ ];
	/**
	 * The current version of the wavelet. The wavelet may have additional applied deltas
	 * which have been applied locally, sent to the server, but not yet acknowledged
	 * by the server.
	 *
	 * @type protocol.ProtocolHashedVersion
	 */
	this.hashed_version = new protocol.ProtocolHashedVersion();
	this.hashed_version.version = 0;
	
	function toArray(str)
	{
		result = [];
		for( var i = 0; i < str.length; ++i )
		{
			result.push( str.charCodeAt(i) );
		}
		return result;
	}
	this.hashed_version.history_hash = toArray( this.url().toString() );
	JSOT.Wavelet.wavelets[ this.url().toString() ] = this;
};

/**
 * A dictionary of all wavelet objects indexed by their URL.
 */
JSOT.Wavelet.wavelets = { };

/**
 * Gets a wavelet or returns false if it does not exist.
 *
 * @param wavelet_name is a URL of the form wave://waveletDomain/waveDomain$waveId/waveletId.
 */
JSOT.Wavelet.getWavelet = function( wavelet_name )
{
	return JSOT.Wavelet.wavelets[ wavelet_name ];
};

/**
 * Gets or creates a wavelet document.
 *
 * @param docname is a document name such as "b+124".
 * @param return an instance of JSOT.Doc.
 */
JSOT.Wavelet.prototype.getDoc = function(docname)
{
	var doc = this.documents[docname];
	if ( doc )
		return doc;
	doc = new JSOT.Doc(docname, this);
	this.documents[docname] = doc;
	return doc;
};

/**
 * @internal
 * Updates the data structure.
 */
JSOT.Wavelet.prototype.addParticipantIntern = function( jid )
{
	var i = this.participants.indexOf(jid);
	if ( i != -1 )
		return;
	this.participants.push( jid );
};

/**
 * @internal
 * Updates the data structure.
 */
JSOT.Wavelet.prototype.removeParticipantIntern = function( jid )
{
	var i = this.participants.indexOf(jid);
	if ( i == -1 )
		return;
	delete this.participants[i];
};

/**
 * Applies a delta to this wavelet.
 *
 * @param delta is of type protocol.ProtocolWaveletDelta.
 */
JSOT.Wavelet.prototype.applyDelta = function( delta )
{
	for( var i = 0; i < delta.operation.length; ++i )
	{
		var op = delta.operation[i];
		if ( op.has_add_participant() )
		{
			this.addParticipantIntern( op.add_participant );
		}
		if ( op.has_remove_participant() )
		{
			this.removeParticipantIntern( op.remove_participant );
		}
		if ( op.has_mutate_document() )
		{
			var doc = this.getDoc( op.mutate_document.document_id );
			var docop = op.mutate_document.document_operation;
			docop.applyTo( doc );
		}		
	}
};

/**
 * @return the URL of this wavelet.
 */
JSOT.Wavelet.prototype.url = function()
{
	var url = new JSOT.WaveUrl();
	url.waveletDomain = this.domain;
	url.waveletId = this.id;
	url.waveDomain = this.wave.domain;
	url.waveId = this.wave.id;
	return url;
};

/**
 * @return the content and participant of a wavelet as a multi-line string.
 */
JSOT.Wavelet.prototype.toString = function()
{
	var str = this.url().toString() + "\r\n\tParticipants:\r\n";	
	
	for( var a in this.participants )
	{
		str += "\t\t" + this.participants[a] + "\r\n";
	}
	
	str += "\tDocuments:\r\n";
	for( var a in this.documents )
	{
		str += "\t\t" + a + ":" + this.documents[a].toString() + "\r\n";
	}
	
	return str;
};

/**
 * Submits a request for adding a participant. The local wavelet is changed immediately
 * and the request is being sent to the server.
 */
JSOT.Wavelet.prototype.submitAddParticipant = function(jid)
{
	var op1 = new protocol.ProtocolWaveletOperation();
	op1.add_participant = jid;
	JSOT.Rpc.submitOperation( this, op1 );
};

/**
 * Submits a request for removing a participant. The local wavelet is changed immediately
 * and the request is being sent to the server.
 */
JSOT.Wavelet.prototype.submitRemoveParticipant = function(jid)
{
	var op1 = new protocol.ProtocolWaveletOperation();
	op1.remove_participant = jid;
	JSOT.Rpc.submitOperation( this, op1 );
};

/**
 * Submits a request for mutating a wavelet document. The local wavelet is changed immediately
 * and the request is being sent to the server.
 *
 * @param mutation is an instance of protocol.ProtocolWaveletOperation_MutateDocument.
 * @param add_participant is either null or a JID. This parameter is useful when sending the initial
 *                        mutation for a document together with adding its initial participant.
 * @param open_wavelet can be used to open the wavelet right after submitting its initial mutation.
 */
JSOT.Wavelet.prototype.submitMutation = function( mutation, add_participant, open_wavelet )
{
	this.submitMutations( this, [mutation], add_participant, open_wavelet );
};

/**
 * Submits a request for mutating a set of wavelet documents. The local wavelet is changed immediately
 * and the request is being sent to the server.
 *
 * @param mutations is an array of instances of protocol.ProtocolWaveletOperation_MutateDocument.
 * @param add_participant is either null of a JID. This parameter is useful when sending the initial
 *                        mutation for a document together with adding its initial participant.
 * @param open_wavelet can be used to open the wavelet right after submitting its initial mutation. 
 */
JSOT.Wavelet.prototype.submitMutations = function( mutations, add_participant, open_wavelet )
{
	var operations = [];
	for( var i = 0; i < mutations.length; ++i )
	{
	  var op = new protocol.ProtocolWaveletOperation();
	  op.mutate_document = mutations[i];
	  operations.push( op );
	}
	
	if ( add_participant )
	{
		var op = new protocol.ProtocolWaveletOperation();
		op.add_participant = add_participant;
		operations.push( op );
	}
	
	JSOT.Rpc.submitOperations( this, operations, open_wavelet );
};

/////////////////////////////////////////////////
//
// WaveletDocument
//
/////////////////////////////////////////////////

/**
 * Do not instantiate directly. Use Wavelet.getDoc instead.
 * @constructor
 * @see JSOT.Wavelet#getDoc
 *
 * @param docId is the name of a wavelet document such as "b+124".
 * @param wavelet is of type Wavelet.
 */
JSOT.Doc = function(docId, wavelet)
{
    /**
	  * The Wavelet to which this document belongs.
	  */
	this.wavelet = wavelet;
	/**
	 * The name of a wavelet document such as "b+124".
	 */
	this.docId = docId;
	/**
	 * An array of objects which are either strings, JSOT.Doc.ElementStart or JSOT.Doc.ElementEnd.
	 */
	this.content = [ ];
	/**
	 * For each item in content this array can optionally contain a key-value dictionary with
	 * the annotation of an element or string.
	 */
	this.format = [ ];
	this.listeners = null;
};

JSOT.Doc.prototype.addListener = function( listener )
{
	if ( !this.listeners )
		this.listeners = [];
	this.listeners.push( listener );
};

/**
 * This event is called if some text inside any element has changed and if none
 * of the elements on the path have implemented the textChange event handler.
 *
 * @name JSOT.Doc#textChange
 * @see JSOT.Doc.ElementStart#event:textChange
 * @event
 */

/**
 * This event is called if a new root element has been inserted in the document.
 *
 * @param {JSOT,Doc,ElementStart} child is the created child element.
 * @see JSOT.Doc.ElementStart#event:newChild
 * @name JSOT.Doc#newChild
 * @event
 */

/**
 * This event is called if a root element has will be removed from the document.
 * The handler is called before it is actually removed.
 *
 * @param {JSOT,Doc,ElementStart} child is the child element that is to be removed.
 * @see JSOT.Doc.ElementStart#event:removeChild
 * @name JSOT.Doc#removeChild
 * @event
 */

/**
 * The beginning of an element is marked with an instance of this class.
 * @constructor
 *
 * @param type is a string.
 * @param attributes is null or a key/value dictionary.
 * @param doc is an instance of JSOT.Doc. It is the document containing this element.
 */
JSOT.Doc.ElementStart = function(type, attributes, doc)
{
	/**
	 * The document to which this element belongs. Type is JSOT.Doc.
	 */
	this.doc = doc;
	/**
	 * This flag can be used to tell what kind of object it is,
	 * i.e. element_start or element_end.
	 */
	this.element_start = true;
	this.type = type;
	if ( !attributes )
		/**
		 * A dictionary of all attributes belonging to the element.
		 */
		this.attributes = { };
	else
		this.attributes = attributes;
	/**
	 * The index in doc.content where this element can be found.
	 */
	this.start_index = -1;
	/**
	 * The index in doc.content where the corresponding end element can be found.
	 */
	this.end_index = -1;
	/**
	 * The index in doc.content where the parent element can be found or -1.
	 */
	this.parent_index = -1;
};

/**
 * @return the parent element or null. The return type is JSOT.Doc.ElementStart.
 */
JSOT.Doc.ElementStart.prototype.parent = function()
{
	if ( this.parent_index == -1 )
		return null;
	return this.doc.content[this.parent_index];
};

/**
 * @param type is either null or a string denoting an element type, In this case their
 *             previous sibling of the requested element type is returned.
 * @return the previous sibling element or null. The return type is JSOT.Doc.ElementStart.
 *
 * Please note that text strings are not treated as siblings.
 */
JSOT.Doc.ElementStart.prototype.previousSibling = function(type)
{
	var depth = 0;
	for( var i = this.start_index - 1; i >= 0; --i )
	{
		var item = this.doc.content[i];
		if ( typeof(item) == "string" )
			continue;
		if ( item.element_start )
		{
			depth--;
			if ( depth == 0 && (!type || item.type == type ) )
				return item;
		}
		else if ( item.element_end )
			depth++;
	}

	return null;
};

/**
 * @param type is either null or a string denoting an element type, In this case their
 *             next sibling of the requested element type is returned.
 * @return the next sibling element or null. The return type is JSOT.Doc.ElementStart.
 *
 * Please note that text strings are not treated as siblings.
 */
JSOT.Doc.ElementStart.prototype.nextSibling = function(type)
{
	var depth = 0;
	for( var i = this.end_index + 1; i < this.doc.content.length; i++ )
	{
		var item = this.doc.content[i];
		if ( typeof(item) == "string" )
			continue;
		if ( item.element_start && (!type || item.type == type ) )
		{
			if ( depth == 0 )
				return item;
			depth++;
		}
		else if ( item.element_end )
			depth--;
	}

	return null;
};

/**
 * @return {int} the number of items in front of this element. Each element_start, element_end
 * and single characters count as one.
 */
JSOT.Doc.ElementStart.prototype.itemCountBefore = function()
{
	var count = 0;
	for( var i = 0; i < this.start_index; ++i )
	{
		if ( typeof(this.doc.content[i]) == "string" )
			count += this.doc.content[i].length;
		else
			count++;
	}
	return count;
};

/**
 * @return {string} the concatenation of all text inside this element. This includes text of nested elements as well.
 */
JSOT.Doc.ElementStart.prototype.getText = function()
{
	var t = ""
	for( var i = this.start_index + 1; i < this.end_index; ++i )
		if ( typeof(this.doc.content[i]) == "string" )
			t += this.doc.content[i];
	return t;
};

/**
 * Searches for a direct or indirect child element. The document tree is travered in infix order
 * and the first match is returned.
 *
 * @param {string} type the requested element type.
 * @return {JSOT.Doc.ElementStart} a nested element of the requested element type or null.
 */
JSOT.Doc.ElementStart.prototype.getElementByType = function(type)
{
	for( var i = this.start_index + 1; i < this.end_index; ++i )
	{
		var item = this.doc.content[i];
		if ( typeof(item) != "string" )
		{
			if ( item.element_start && item.type == type )
				return item;
		}
	}
	return null;
};

/**
 * Searches for a all direct or indirect child elements.
 *
 * @param {string} type the requested element type.
 * @return {JSOT.Doc.ElementStart[]} am array of all nested elements of the requested element type.
 */
JSOT.Doc.ElementStart.prototype.getElementsByType = function(type)
{
	var result = [];
	for( var i = this.start_index; i < this.end_index; ++i )
	{
		var item = this.doc.content[i];
		if ( typeof(item) != "string" )
		{
			if ( item.element_start && item.type == type )
				result.push( item );
		}
	}
	return result;
};

/**
 * Searches for a direct or indirect child element. The document tree is travered in infix order
 * and the first match is returned. The function inspects the {@link JSOT.Doc.ElementStart#attributes} field
 * and looks up the id value there.
 *
 * @param {string} id the requested id.
 * @return {JSOT.Doc.ElementStart} a nested element of the requested id or null.
 */
JSOT.Doc.ElementStart.prototype.getElementById = function(id)
{
	for( var i = this.start_index; i < this.end_index; ++i )
	{
		var item = this.doc.content[i];
		if ( typeof(item) != "string" )
		{
			if ( item.element_start && item.attributes["id"] == id )
				return item;
		}
	}
	return null;
};

/**
 * This event is called if a new child element has been inserted in the document.
 *
 * @name JSOT.Doc.ElementStart#newChild
 * @event
 * @param {JSOT,Doc,ElementStart} child is the created child element.
 * @see JSOT.Doc#event:newChild
 */

/**
 * This event is called if a child element is to be removed, i.e. 
 * before it is really removed from the document.
 *
 * @name JSOT.Doc.ElementStart#removeChild
 * @event
 * @param {JSOT,Doc,ElementStart} child is the child element that is to be removed.
 * @see JSOT.Doc#event:removeChild
 */

/**
 * This event is called if some text inside this element has changed. The text change
 * may have occured in a nested element as well. The event bubbles up the document tree
 * until some element has implemented the the textChange event handler.
 *
 * The event bubbling ends latest by calling the textChange handler of the document.
 * @see JSOT.Doc#event:textChange
 *
 * @name JSOT.Doc.ElementStart#textChange
 * @event
 */

/**
 * The end of an element is marked with an instance of this class.
 * @constructor
 */
JSOT.Doc.ElementEnd = function()
{
	/**
	 * This flag can be used to tell what kind of object it is,
	 * i.e. element_start or element_end.
	 */  
	this.element_end = true;
};

/**
 * @return the number of items. Each element counts as one and each single character counts as one.
 */
JSOT.Doc.prototype.itemCount = function()
{
	var count = 0;
	for( var i = 0; i < this.content.length; ++i )
	{
		if ( typeof(this.content[i]) == "string" )
			count += this.content[i].length;
		else
			count++;
	}
	return count;
};

/**
 * @param pos is an item index. Each character, element_start and element_end
 *            counts as one in this index.
 * @return {string} The character at this position or null if there is no character.
 */
JSOT.Doc.prototype.getCharAt = function(pos)
{
	var count = 0;
	var i = 0;
	while( i < this.content.length )
	{
		var c = this.content[i++];
		if ( typeof(c) == "string" )
		{
			if ( count + c.length > pos )
				return c[pos - count];
			count += c.length;
		}
		else
		{
			if ( pos == count )
				return null;
			count++;
		}
	}
	return null;
};

/**
 * @param pos is an item index. Each character, element_start and element_end
 *            counts as one in this index.
 * @return {string|JSOT.Doc.ElementStart|JSOT.Doc.ElementEnd} The character or item at this position.
 *         If pos is outside of the allowed range, the function returns null.
 */
JSOT.Doc.prototype.getItemAt = function(pos)
{
	var count = 0;
	var i = 0;
	while( i < this.content.length )
	{
		var c = this.content[i++];
		if ( typeof(c) == "string" )
		{
			if ( count + c.length > pos )
				return c[pos - count];
			count += c.length;
		}
		else
		{
			if ( pos == count )
				return c;
			count++;
		}
	}
	return null;
};

JSOT.Doc.prototype.getFormatAt = function(pos)
{
	var count = 0;
	var i = 0;
	while( i < this.content.length )
	{
		var c = this.content[i++];
		if ( typeof(c) == "string" )
		{
			if ( count + c.length > pos )
				return this.format[i];
			count += c.length;
		}
		else
		{
			if ( pos == count )
				return this.format[i];
			count++;
		}
	}
	return null;
};

JSOT.Doc.prototype.getElementByType = function(type)
{
	for( var i = 0; i < this.content.length; ++i )
	{
		var item = this.content[i];
		if ( typeof(item) != "string" )
		{
			if ( item.element_start && item.type == type )
				return item;
		}
	}
	return null;
};

JSOT.Doc.prototype.getElementsByType = function(type)
{
	var result = [];
	for( var i = 0; i < this.content.length; ++i )
	{
		var item = this.content[i];
		if ( typeof(item) != "string" )
		{
			if ( item.element_start && item.type == type )
				result.push( item );
		}
	}
	return result;
};

/**
 * Searches for an element. The document tree is traversed in infix order
 * and the first match is returned. The function inspects the {@link JSOT.Doc.ElementStart#attributes} field
 * and looks up the id value there.
 *
 * @param {string} id the requested id.
 * @return {JSOT.Doc.ElementStart} a nested element of the requested id or null.
 */
JSOT.Doc.prototype.getElementById = function(id)
{
	for( var i = 0; i < this.content.length; ++i )
	{
		var item = this.content[i];
		if ( typeof(item) != "string" )
		{
			if ( item.element_start && item.attributes["id"] == id )
				return item;
		}
	}
	return null;
};

JSOT.Doc.prototype.isEmpty = function()
{
	return this.content.length == 0;
}

/**
 * @return a simple string representation of the document.
 */
JSOT.Doc.prototype.toString = function()
{
	var result = [];
	var stack = [];
	var anno = null;
	
	for( var i = 0; i < this.content.length; ++i )
	{
		var c = this.content[i];
		if( this.format[i] != anno )
		{
			var a = this.format[i];
			if ( a != anno )
			{
				result.push("[");
				for( var key in a )
				{
					result.push(key + "=\"" + a[key] + "\" ");
				}
				result.push("]");
				anno = a;
			}
		}
		if ( c.element_start )
		{
			stack.push( c.type );
			result.push("<" + c.type);
			if ( c.attributes )
			{
				for( var key in c.attributes )
				{
					result.push(" " + key + "=\"");
					result.push(c.attributes[key]);
					result.push("\"");
				}
			}
			result.push(">");
		}
		else if ( c.element_end )
		{
			result.push("<" + stack.pop() + "/>");
		}
		else
		{
			result.push( c );
		}
	}
	
	return result.join("");
};

/**
 * Iterates over the document to find new or changed elements.
 * If these elements have callback functions, they are invoked to update the GUI.
 *
 * This function is automatically invoked when an update has been processed or a
 * submit has beeen sent.
 *
 * @return a list of values, one for each top-level element. The type of the values
 *         equals the type of the newChild or updateChild callback functions.
 */
JSOT.Doc.prototype.createGUI = function()
{
	this.has_gui = true;
	window.console.log("PROCESSING " + this.docId);
	
	var result = []
	var current = this;
	var currentIndex = -1;
	var stack = [];
	
	for( var i = 0; i < this.content.length; ++i )
	{
		var item = this.content[i];
		if ( typeof(item) == "string" )
		{
		}
		else if ( item.element_start )
		{
			if ( item.is_new && current.newChild )
			{
				if ( currentIndex == -1 )
					result.push( current.newChild( item ) ); 
				else				
					current.newChild(item);
				delete item.has_new_text;
			}
			else if ( item.has_new_text )
			{
				var e = item;
				while( e )
				{
					if ( e.blockTextChange || e.is_new )
						break;
					if ( e.textChange )
					{
						e.blockTextChange = true;
						e.textChange();
						break;
					}
					e = e.parent();
				}
				delete item.has_new_text;
			}
			stack.push( currentIndex );
			current = item;
			currentIndex = i;
		}
		else if ( item.element_end )
		{
			delete current.blockTextChange;
			delete current.is_new;
			currentIndex = stack.pop();
			if ( currentIndex == -1 )
				current = this;
			else
				current = this.content[ currentIndex ];
		}
	}
		
	return result;
};

/////////////////////////////////////////////////
//
// DocumentOperation
//
/////////////////////////////////////////////////

/**
 * Applies the document operation to a wavelet document.
 *
 * @param doc is of type JSOT.Doc.
 */
protocol.ProtocolDocumentOperation.prototype.applyTo = function(doc)
{
	//
	// Find out which elements are to be deleted and notify the application logic
	// before the document is changed.
	//
	
	// Position in this.components
	var opIndex = 0;
	// Position in doc.content
	var contentIndex = 0;
	// Position in the text of variable 'c'
	var inContentIndex = 0;
	var c = doc.content[contentIndex];
	// Loop until all ops are processed
	while( opIndex < this.component.length )
	{
		var op = this.component[opIndex++];
		if ( op.element_start || op.element_end || op.characters )
		{
		}
		else if ( op.retain_item_count || op.delete_characters )
		{
			if ( !c )
				break;
			if ( op.delete_characters )
				var count = op.delete_characters.length;
			else
				var count = op.retain_item_count;
			while( count > 0 )
			{
				if ( !c )
					throw "document op is larger than doc";	
				if ( c.element_start || c.element_end )
				{
					count--;
					c = doc.content[++contentIndex];
					inContentIndex = 0;
				}
				else  // Characters
				{
					// How many characters can be retained?
					var m = Math.min( count, c.length - inContentIndex );
					// Skip characters
					count -= m;
					inContentIndex += m;
					// Retained/Deleted the entire string? -> Move to the next element
					if ( c.length == inContentIndex )
					{
						// Go to the next content entry
						c = doc.content[++contentIndex];
						inContentIndex = 0;
					}
				}
			}
		}
		else if ( op.delete_element_start )
		{
			if ( !c )
				break;
			var p = c.parent();
			if ( p )
			{
				if ( p.removeChild )
					p.removeChild( c );
			}
			else if ( doc.removeChild )
				doc.removeChild( c );
			c = doc.content[++contentIndex];
		}
		else if ( op.delete_element_end )
		{
			if ( !c )
				break;
			c = doc.content[++contentIndex];
		}
		else if ( op.update_attributes || op.replace_attributes )
		{
			if ( !c )
				break;
			c = doc.content[++contentIndex];
		}
	}

	//
	// Transform the document
	//
	
	// Position in this.components
	var opIndex = 0;
	// Position in doc.content
	var contentIndex = 0;
	// Position in the text of variable 'c'
	var inContentIndex = 0;
	// Increased by one whenever a delete_element_start is encountered and decreased by 1 upon delete_element_end
	var deleteDepth = 0;
	
	// The annotation that applies to the left of the current document position
	var docAnnotation = null;
	var updatedAnnotation = null;
	// The annotation update that applies to the right of the current document position	
	var annotationUpdate = { };
	var annotationUpdateCount = 0;
	
	var currentElement = doc;
	var currentElementIndex = -1;
	var stack = [];
	
	if ( doc.listeners )
		for( var i = 0; i < doc.listeners.length; ++i )
			doc.listeners[i].begin( doc );
		
	var c = doc.content[contentIndex];
	// Loop until all ops are processed
	while( opIndex < this.component.length )
	{
		var op = this.component[opIndex++];
		if ( op.element_start )
		{
			if ( deleteDepth > 0 )
				throw "Cannot insert inside a delete sequence";

			var attribs = { };
			for( var a in op.element_start.attribute )
			{
				var v = op.element_start.attribute[a];
				attribs[v.key] = v.value;
			}

			stack.push( currentElementIndex );
			currentElement = new JSOT.Doc.ElementStart( op.element_start.type, attribs, doc );
			currentElement.start_index = contentIndex;
			currentElement.parent_index = currentElementIndex;
			currentElementIndex = contentIndex;
			currentElement.is_new = true;
			
			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].insertElementStart( currentElement, updatedAnnotation );
				
			if ( inContentIndex == 0 )
			{
				doc.content.splice( contentIndex, 0, currentElement );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
			}
			else
			{
				currentElement.start_index++;
				doc.content.splice( contentIndex + 1, 0, currentElement, c.substring(inContentIndex, c.length) );
				doc.format.splice( contentIndex + 1, 0, updatedAnnotation, docAnnotation );
				doc.content[contentIndex] = c.slice(0, inContentIndex);
				contentIndex += 2;
				c = doc.content[contentIndex];
				inContentIndex = 0;
			}
		}
		else if ( op.element_end )
		{
			if ( deleteDepth > 0 )
				throw "Cannot insert inside a delete sequence";

			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].insertElementEnd( currentElement, updatedAnnotation );

			currentElement.end_index = contentIndex;

			if ( inContentIndex == 0 )
			{
				doc.content.splice( contentIndex, 0, new JSOT.Doc.ElementEnd() );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
			}
			else
			{
				currentElement.end_index++
				if ( updatedAnnotation )
					doc.format[contentIndex+1] = updatedAnnotation;
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementEnd(), c.substring(inContentIndex, c.length) );
				doc.format.splice( contentIndex + 1, 0, updatedAnnotation, docAnnotation );
				doc.content[contentIndex] = c.slice(0, inContentIndex);
				contentIndex += 2;
				c = doc.content[contentIndex];
				inContentIndex = 0;
			}
			
			currentElementIndex = stack.pop();
			if ( currentElementIndex == -1 )
				currentElement = doc;
			else
				currentElement = doc.content[ currentElementIndex ];
		}
		else if ( op.characters )
		{
			if ( deleteDepth > 0 )
				throw "Cannot insert inside a delete sequence";

			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].insertCharacters( op.characters, updatedAnnotation );

			currentElement.has_new_text = true;
			
			if ( typeof(c) == "string" )
			{
				// If the annotation does not change here, simply insert some text
				if ( docAnnotation == updatedAnnotation )
				{
					c = c.substring(0,inContentIndex) + op.characters + c.substring(inContentIndex,c.length);
					doc.content[contentIndex] = c;
					inContentIndex += op.characters.length;
				}
				else if ( inContentIndex == 0 )
				{
					doc.content.splice( contentIndex, 0, op.characters );
					doc.format.splice( contentIndex, 0, updatedAnnotation );
					contentIndex += 1;
					c = doc.content[contentIndex];
					inContentIndex = 0;
				}
				else
				{
					doc.content.splice( contentIndex, 1, c.substr(0, inContentIndex ), op.characters, c.substring( inContentIndex, c.length ) );
					doc.format.splice( contentIndex, 1, docAnnotation, updatedAnnotation, docAnnotation );
					contentIndex += 2;
					c = doc.content[contentIndex];
					inContentIndex = 0;				
				}
			}
			else
			{
				doc.content.splice( contentIndex, 0, op.characters );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
				inContentIndex = 0;
			}			
		}
		else if ( op.delete_characters )
		{
			if ( !c )
				break;
			
			currentElement.has_new_text = true;
			
			var count = op.delete_characters.length;
			var done = 0;
			while( count > 0 )
			{
				if ( typeof(c) != "string" )
					throw "Cannot delete characters here, because at this position there are no characters";

				if ( doc.listeners )
					for( var i = 0; i < doc.listeners.length; ++i )
						doc.listeners[i].deleteCharacters( op.delete_characters, updatedAnnotation );

				var i = Math.min( count, c.length - inContentIndex );
				if ( c.substr( inContentIndex, i ) != op.delete_characters.substr( done, i ) )
					throw "Cannot delete characters, because the characters in the document and operation differ.";
				c = c.substring(0, inContentIndex).concat( c.substring(inContentIndex + i, c.length ) );
				done += i;
				count -= i;
				// The entire string has been deleted?
				if ( c.length == 0 )
				{
					// If there is an annotation boundary change in the deleted characters,, this change must be applied
					var anno = doc.format[contentIndex];
					if ( anno != docAnnotation )
					{
						docAnnotation = anno;
						updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
					}					
					doc.content.splice( contentIndex, 1 );
					doc.format.splice( contentIndex, 1 );
					c = doc.content[contentIndex];
					inContentIndex = 0;
				}
				else
				{
					doc.content[contentIndex] = c;
					if ( c.length == inContentIndex )
					{
						c = doc.content[++contentIndex];
						inContentIndex = 0;
					}
				}
			}
		}
		else if ( op.retain_item_count )
		{
			if ( !c )
				break;
			if ( deleteDepth > 0 )
				throw "Cannot retain inside a delete sequence";
			var count = op.retain_item_count;
			while( count > 0 )
			{
				if ( !c )
					throw "document op is larger than doc";
				// Check for annotation changes in the document
				if ( inContentIndex == 0 )
				{
					var anno = doc.format[contentIndex];
					// Something changed?
					if ( anno != docAnnotation )
					{
						// Get the document annotation and update it
						docAnnotation = anno;
						updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
					}
					// Update the annotation
					doc.format[contentIndex] = updatedAnnotation;
				}
				if ( c.element_start )
				{
					stack.push( currentElementIndex );
					currentElement = c;
					currentElement.start_index = contentIndex;
					currentElement.parent_index = currentElementIndex;
					currentElementIndex = contentIndex;
				 
					if ( doc.listeners )
						for( var i = 0; i < doc.listeners.length; ++i )
							doc.listeners[i].retainElementStart( c, updatedAnnotation );
				 
					// Skip the element
					count--;
					c = doc.content[++contentIndex];
					inContentIndex = 0;
				}
				else if ( c.element_end )
				{
					currentElement.end_index = contentIndex;
					currentElementIndex = stack.pop();
					if ( currentElementIndex == -1 )
						currentElement = doc;
					else
						currentElement = doc.content[ currentElementIndex ];

					if ( doc.listeners )
						for( var i = 0; i < doc.listeners.length; ++i )
							doc.listeners[i].retainElementEnd( currentElement, updatedAnnotation );

					// Skip the element
					count--;
					c = doc.content[++contentIndex];
					inContentIndex = 0;
				}
				else  // Characters
				{
					// How many characters can be retained?
					var m = Math.min( count, c.length - inContentIndex );
					// Skip characters
					count -= m;
					inContentIndex += m;
					
					if ( doc.listeners )
						for( var i = 0; i < doc.listeners.length; ++i )
							doc.listeners[i].retainCharacters( m, updatedAnnotation );

					// Retained the entire string? -> Move to the next element
					if ( inContentIndex == c.length )
					{						
						// Go to the next content entry
						c = doc.content[++contentIndex];
						inContentIndex = 0;
						if ( !c )
							break;
					}
					// There is an annotation update and we just re-annotated some characters in 'c' but not all -> split c in the treated part and the to-be-treated part
					else if ( updatedAnnotation != docAnnotation )
					{
						doc.content.splice( contentIndex, 1, c.substr( 0, inContentIndex ), c.substring(inContentIndex, c.length) );
						doc.format.splice( contentIndex + 1, 0, docAnnotation );
						c = doc.content[++contentIndex];
						inContentIndex = 0;
					}
				}
			}
		}
		else if ( op.delete_element_start )
		{
			if ( !c )
				break;
			// If there is an annotation boundary change in the deleted characters,, this change must be applied
			var anno = doc.format[contentIndex];
			if ( anno != docAnnotation )
			{
				docAnnotation = anno;
				updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			}
			
			// Count how many opening elements have been deleted. The corresponding closing elements must be deleted, too.
			deleteDepth++;
			if ( !c.element_start )
				throw "Cannot delete element start at this position, because in the document there is none";
			if ( c.type != op.delete_element_start.type )
				throw "Cannot delete element start because Op and Document have different element type";
			var acount = 0;
			for( var a in c.attributes )
				++acount;			
			if ( acount != op.delete_element_start.attribute.length )
				throw "Cannot delete element start because the attributes of Op and Doc differ";
		
			var attribs = { };
			for( var a in op.delete_element_start.attribute )
			{
				var v = op.delete_element_start.attribute[a];
				attribs[v.key] = v.value;
			}
			for( var a in c.attributes )
			{
				if ( c.attributes[a] != attribs[a] )
					throw "Cannot delete element start because attribute values differ";
			}
			
			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].deleteElementStart( c, updatedAnnotation );
	
			doc.content.splice( contentIndex, 1 );
			doc.format.splice( contentIndex, 1 );
			c = doc.content[contentIndex];
			inContentIndex = 0;			
		}
		else if ( op.delete_element_end )
		{
			if ( !c )
				break;
			// If there is an annotation boundary change in the deleted characters, this change must be applied
			var anno = doc.format[contentIndex];
			if ( anno != docAnnotation )
			{
				docAnnotation = anno;
				updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			}				
			// Is there a matching openeing element?
			if ( deleteDepth == 0 )
				throw "Cannot delete element end, because matching delete element start is missing";
			if ( !c.element_end )
				throw "Cannot delete element end at this position, because in the document there is none";
				
			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].deleteElementEnd(updatedAnnotation);

			doc.content.splice( contentIndex, 1 );
			doc.format.splice( contentIndex, 1 );
			c = doc.content[contentIndex];
			inContentIndex = 0;
			deleteDepth--;		
		}
		else if ( op.update_attributes )
		{		
			if ( !c )
				break;
			if ( !c.element_start )
				throw "Cannot update attributes at this position, because in the document there is no start element";
			
			stack.push( currentElementIndex );
			currentElement = c;
			currentElement.start_index = contentIndex;
			currentElement.parent_index = currentElementIndex;
			currentElementIndex = contentIndex;

			// If there is an annotation boundary change in the deleted characters, this change must be applied
			var anno = doc.format[contentIndex];
			if ( anno != docAnnotation )
			{
				docAnnotation = anno;
				updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			}								
			doc.format[contentIndex] = updatedAnnotation;
			for( var a in op.update_attributes.attribute_update )
			{
				var update = op.update_attributes.attribute_update[a];
				// Add a new attribute?
				if ( update.old_value == null )
				{
					if ( c.attributes[update.key] )
						throw "Cannot update attributes because old attribute value is not mentioned in Op";
					if ( !update.new_value )
						throw "Cannot update attributes because new value is missing";
					c.attributes[update.key] = update.new_value;
				}
				// Delete or change an attribute
				else
				{
					if ( c.attributes[update.key] != update.old_value )
						throw "Cannot update attributes because old attribute value does not match with Op";
					if ( !update.new_value )
						delete c.attributes[update.key];
					else
						c.attributes[update.key] = update.new_value;
				}
			}
			c = doc.content[++contentIndex];
			inContentIndex = 0;

			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].updateAttributes( currentElement, updatedAnnotation );
		}
		else if ( op.replace_attributes )
		{
			if ( !c )
				break;
			if ( !c.element_start )
				throw "Cannot replace attributes at this position, because in the document there is no start element";
			
			stack.push( currentElementIndex );
			currentElement = c;
			currentElement.start_index = contentIndex;
			currentElement.parent_index = currentElementIndex;
			currentElementIndex = contentIndex;

			var acount = 0;
			for( var a in c.attributes )
				++acount;
			for( var a in op.replace_attributes.old_attribute )
			{
			   var keyvalue = op.replace_attributes.old_attribute[a];
				if ( c.attributes[keyvalue.key] != keyvalue.value )
					throw "Cannot replace attributes because the value of the old attributes do not match";
			}
			if ( acount != op.replace_attributes.old_attribute.length )
				throw "Cannot replace attributes because the old attributes do not match";
			// If there is an annotation boundary change in the deleted characters, this change must be applied
			var anno = doc.format[contentIndex];
			if ( anno != docAnnotation )
			{
				docAnnotation = anno;
				updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			}
			doc.format[contentIndex] = updatedAnnotation;
			c.attributes = { }
			for( var a in op.replace_attributes.new_attribute )
			{
			   var keyvalue = op.replace_attributes.new_attribute[a];
				c.attributes[keyvalue.key] = keyvalue.value;
			}
			c = doc.content[++contentIndex];
			inContentIndex = 0;

			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].replaceAttributes( currentElement, updatedAnnotation );
		}
		else if ( op.annotation_boundary )
		{
			for( var a in op.annotation_boundary.end )
			{				
				var key = op.annotation_boundary.end[a];
				if ( !annotationUpdate[key] )
					throw "Cannot end annotation because the doc and op annotation do not match.";
				delete annotationUpdate[key];
				annotationUpdateCount--;
			}
			for( var a in op.annotation_boundary.change )
			{
				var change = op.annotation_boundary.change[a];
				if ( !annotationUpdate[change.key] )
					annotationUpdateCount++;
				annotationUpdate[change.key] = change;
			}
			updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			
			// If in the middle of a string -> break it because the annotation of the following characters is different
			if ( inContentIndex > 0 )
			{
				doc.content.splice( contentIndex, 1, c.substr(0, inContentIndex ), c.substring( inContentIndex, c.length ) );
				doc.format.splice( contentIndex, 1, docAnnotation, docAnnotation );
				contentIndex += 1;
				c = doc.content[contentIndex];
				inContentIndex = 0;
			}
			
			if ( doc.listeners )
				for( var i = 0; i < doc.listeners.length; ++i )
					doc.listeners[i].annotationBoundary( annotationUpdateCount == 0 ? null : annotationUpdate, updatedAnnotation );
					// doc.listeners[i].annotationBoundary( annotationUpdateCount == 0 ? null : updatedAnnotation );
		}
	}

	if ( doc.listeners )
		for( var i = 0; i < doc.listeners.length; ++i )	
			doc.listeners[i].end( doc );
	
	if ( opIndex < this.component.length )
		throw "Document is too small for op"
	// Should be impossible if the document is well formed ... Paranoia
	if ( deleteDepth != 0 )
		throw "Not all delete element starts have been matched with a delete element end";
	if ( contentIndex < doc.content.length )
		throw "op is too small for document";
};

/**
 * @internal
 */
protocol.ProtocolDocumentOperation.prototype.computeAnnotation = function(docAnnotation, annotationUpdate, annotationUpdateCount)
{
	if ( annotationUpdateCount == 0 )
		return docAnnotation;
		
	var count = 0;
	anno = { };
	if ( docAnnotation )
	{
		// Copy the current annotation
		for( var a in docAnnotation )
		{
			count++;
			anno[a] = docAnnotation[a];
		}
	}
	// Update
	for( var a in annotationUpdate )
	{
		var change = annotationUpdate[a];		
		if ( change.new_value == null )
		{
			count--;
			delete anno[a];
		}
		else if ( change.old_value == null )
		{
			count++;
			anno[a] = change.new_value;
		}
		else
		{
			if ( !docAnnotation || docAnnotation[a] != change.old_value )
				throw "Annotation update and current annotation do not match";
			anno[a] = change.new_value;
		}
	}
	if ( count == 0 )
		return null;
	return anno;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newElementStart = function(type, attributes)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	if ( !attributes )
		attributes = [ ];
	c.element_start = new protocol.ProtocolDocumentOperation_Component_ElementStart();
	c.element_start.type = type;
	c.element_start.attribute = attributes;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newElementEnd = function()
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.element_end = true;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newCharacters = function(characters)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.characters = characters;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newDeleteCharacters = function(delete_characters)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.delete_characters = delete_characters;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newRetainItemCount = function(retain_item_count)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.retain_item_count = retain_item_count;
	return c;
};
 
/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newDeleteElementStart = function(type, attributes)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	if ( !attributes )
		attributes = [ ];
	c.delete_element_start = new protocol.ProtocolDocumentOperation_Component_ElementStart();
	c.delete_element_start.type = type;
	c.delete_element_start.attribute = attributes;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @param {JSOT.Doc.ElementStart} element is the element that is to be deleted.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newDeleteElementStartFromElement = function(element)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	if ( !attributes )
		attributes = [ ];
	c.delete_element_start = new protocol.ProtocolDocumentOperation_Component_ElementStart();
	c.delete_element_start.type = element.type;
	var attributes = [ ];
	for( var a in element.attributes )	
		attributes.push( protocol.ProtocolDocumentOperation.newKeyValuePair( a, element.attributes[a] ) );
	c.delete_element_start.attribute = attributes;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newDeleteElementEnd = function()
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.delete_element_end = true;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newReplaceAttributes = function(old_attribute, new_attribute)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.replace_attributes = new protocol.ProtocolDocumentOperation_Component_ReplaceAttributes();
	c.replace_attributes.old_attribute = old_attribute;
	c.replace_attributes.new_attribute = new_attribute;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newUpdateAttributes = function(attributeUpdates)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.update_attributes = new protocol.ProtocolDocumentOperation_Component_ReplaceAttributes();
	c.update_attributes.attribute_update = attributeUpdates;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component_KeyValueUpdate.
 */
protocol.ProtocolDocumentOperation.newKeyValueUpdate = function( key, old_value, new_value )
{
	var c = new protocol.ProtocolDocumentOperation_Component_KeyValueUpdate();
	c.key = key;
	c.old_value = old_value;
	c.new_value = new_value;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component_KeyValuePair.
 */
protocol.ProtocolDocumentOperation.newKeyValuePair = function( key, value )
{
	var c = new protocol.ProtocolDocumentOperation_Component_KeyValuePair();
	c.key = key;
	c.value = value;
	return c;
};

/**
 * Helper function for manually constructing a document operation.
 *
 * @return an instance of protocol.ProtocolDocumentOperation_Component.
 */
protocol.ProtocolDocumentOperation.newAnnotationBoundary = function(end, change)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.annotation_boundary = new protocol.ProtocolDocumentOperation_Component_AnnotationBoundary();
	c.annotation_boundary.end = end;
	c.annotation_boundary.change = change;
	return c;
};

/////////////////////////////////////////////////
//
// WaveletOperation
//
/////////////////////////////////////////////////

/**
 * Performs operation transformation on the two wavelet operations.
 * The following constraint holds:
 * m1_orig = copy(m1);
 * m2_orig = copy(m2);
 * xform( m1, m2 );
 * wavelet1 = copy(wavelet);
 * wavelet2 = copy(wavelet);
 * m1_orig.applyTo(wavelet1);
 * m2.applyTo(wavelet1);
 * m2_orig.applyTo(wavelet2);
 * m1.applyTo(wavelet2);
 * Now it is true that wavelet1 == wavelet2
 *
 * @param m1 is an instance of protocol.ProtocolWaveletOperation. The function modifies m1.
 * @param m2 is an instance of protocol.ProtocolWaveletOperation. The function modifies m2.
 */
protocol.ProtocolWaveletOperation.xform = function( m1, m2 )
{
	if ( m1.has_add_participant() )
	{
		if ( m2.has_add_participant() && m2.add_participant == m1.add_participant )
			m2.clear_add_participant();
		else if ( m2.has_remove_participant() && m2.remove_participant == m1.add_participant )
			m1.clear_add_participant();
	}
	if ( m1.has_remove_participant() )
	{
		if ( m2.has_remove_participant() && m2.remove_participant == m1.remove_participant )
			m2.clear_remove_participant();
		else if ( m2.has_add_participant() && m2.add_participant == m1.remove_participant )
			m1.clear_remove_participant();
	}
	if ( m1.has_mutate_document() && m2.has_mutate_document() )
	{
		if ( m1.mutate_document.document_id == m2.mutate_document.document_id )
			protocol.ProtocolDocumentOperation.xform( m1.mutate_document.document_operation, m2.mutate_document.document_operation );
	}
};

/////////////////////////////////////////////////
//
// OT
//
/////////////////////////////////////////////////

/**
 * Performs operation transformation on the two wavelet operations.
 * The following constraint holds:
 * m1_orig = copy(m1);
 * m2_orig = copy(m2);
 * xform( m1, m2 );
 * doc1 = copy(doc);
 * doc2 = copy(doc);
 * m1_orig.applyTo(doc1);
 * m2.applyTo(doc1);
 * m2_orig.applyTo(doc2);
 * m1.applyTo(doc2);
 * Now it is true that doc1 == doc2
 *
 * @param m1 is an instance of protocol.ProtocolWaveletOperation_MutateDocument. The function modifies m1.
 * @param m2 is an instance of protocol.ProtocolWaveletOperation_MutateDocument. The function modifies m2.
 */
protocol.ProtocolDocumentOperation.xform = function( m1, m2 )
{
    if ( m1.component.length == 0 || m2.component.length == 0 )
        return;

	var r1 = new protocol.ProtocolDocumentOperation();
	var r2 = new protocol.ProtocolDocumentOperation();
	var anno1, anno2;
	var c1 = 0;
	var c2 = 0;
	var item1 = m1.component[c1];
	var item2 = m2.component[c2];
	var next = { next1 : false, next2 : false };
    while( c1 < m1.component.length && c2 < m2.component.length )
    {
        if ( item1.has_element_start() )
        {           
			protocol.ProtocolDocumentOperation.xformInsertElementStart( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_element_end() )
		{
			protocol.ProtocolDocumentOperation.xformInsertElementEnd( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_characters() )
		{
			protocol.ProtocolDocumentOperation.xformInsertChars( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_retain_item_count() )
		{
			protocol.ProtocolDocumentOperation.xformRetain( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_delete_element_start() )
		{
			protocol.ProtocolDocumentOperation.xformDeleteElementStart( r1, r2, item1, item2, next, anno1, anno2);
		}
		else if ( item1.has_delete_element_end() )
		{
			protocol.ProtocolDocumentOperation.xformDeleteElementEnd( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_delete_characters() )
		{
			protocol.ProtocolDocumentOperation.xformDeleteChars( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_update_attributes() )
		{
			protocol.ProtocolDocumentOperation.xformUpdateAttributes( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_replace_attributes() )
		{
			protocol.ProtocolDocumentOperation.xformReplaceAttributes( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_annotation_boundary() )
		{
			protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r1, r2, item1, item2, nextanno1, anno2, false );
        }

        if ( next.next1 )
        {
            c1++;
            if ( c1 < m1.component.length )
                item1 = m1.component[c1];
        }
        if ( next.next2 )
        {
            c2++;
            if ( c2 < m2.component.length )
                item2 = m2.component[c2];
        }
		next.next1 = false;
		next.next2 = false;
    }

    if ( c1 != m1.component.length || c2 != m2.component.length )    
        throw "Length mismatch when transforming deltas.";

	m1.component = r1.component;
	m2.component = r2.component;
};

protocol.ProtocolDocumentOperation.xformInsertElementStart = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	r1.component.push( item1 );
	r2.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( 1 ) );
    next.next1 = true;
    next.next2 = false;
};

protocol.ProtocolDocumentOperation.xformInsertElementEnd = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	r1.component.push( item1 );
	r2.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( 1 ) );
    next.next1 = true;
    next.next2 = false;
};

protocol.ProtocolDocumentOperation.xformInsertChars = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	r1.component.push( item1 );
	r2.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( item1.characters.length ) );
    next.next1 = true;
    next.next2 = false;
};

protocol.ProtocolDocumentOperation.xformRetain = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_retain_item_count() )
	{
		var len = Math.min(item1.retain_item_count, item2.retain_item_count);
		r1.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( len ) );
		r2.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( len ) );
		if ( len < item1.retain_item_count )
			item1.retain_item_count -= len;
		else
			next.next1 = true;
		if ( len < item2.retain_item_count )
			item2.retain_item_count -= len;
		else
			next.next2 = true;
	}
	else if ( item2.has_delete_element_start() || item2.has_delete_element_end() )
	{
		if ( item1.retain_item_count > 1 )
			item1.retain_item_count -= 1;
		else
			next.next1 = true;
		r2.component.push( item2 );
		next.next2 = true;
	}
    else if ( item2.has_delete_characters() )
	{
		var len = Math.min(item1.retain_item_count, item2.delete_characters.length);
		if ( len < item1.retain_item_count )
			item1.retain_item_count -= len;
		else
			next.next1 = true;
		r2.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( item2.delete_characters.substr(0, len ) ) );
		if ( len < item2.delete_characters.length )
			item2.delete_characters = item2.delete_characters.substring( len, item2.delete_characters.length );
		else
			next.next2 = true;
	}
	else if ( item2.has_replace_attributes() || item2.has_update_attribute() )
	{
		r1.components.push( protocol.ProtocolDocumentOperation.newRetainItemCount(1) );
		if ( item1.retain_item_count > 1 )
			item1.retain_item_count -= 1;
		else
			next.next1 = true;
		r2.component.push( item2 );
		next.next2 = true;
	}
	else
		throw "Unexpected case";
};

protocol.ProtocolDocumentOperation.xformDeleteElementStart = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_retain_item_count() )
	{
		r1.component.push( item1 );
		next.next1 = true;
		if ( item2.retain_item_count > 1 )
			item2.retain_item_count -= 1;
		else
			next.next2 = true;		
	}
	else if ( item2.has_delete_element_start() )
	{
		next.next1 = true;
		next.next2 = true;
	}
	else if ( item2.has_replace_attributes() || item2.has_update_attributes() )
	{
		r1.component.push( item1 );
		next.next1 = true;
		next.next2 = true;
	}
	else
		throw "Unexpected case";
};

protocol.ProtocolDocumentOperation.xformDeleteElementEnd = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_retain_item_count() )
	{
		r1.component.push( item1 );
        next.next1 = true;
		if ( item2.retain_item_count > 1 )
			item2.retain_item_count -= 1;
		else
			next.next2 = true;					
	}
	else if ( item2.has_delete_element_end() )
	{
        next.next1 = true;
        next.next2 = true;
	}
	else
        throw "The two mutations are not compatible";
};

protocol.ProtocolDocumentOperation.xformDeleteChars = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_retain_item_count() )
	{
		var len = Math.min(item2.retain_item_count, item1.delete_characters.length);
		r1.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( item1.delete_characters.substr(0, len ) ) );
		if ( len < item1.delete_characters.length )
			item1.delete_characters = item1.delete_characters.substring( len, item1.delete_characters.length );
		else
			next.next1 = true;
		if ( len < item2.retain_item_count )
			item2.retain_item_count -= len;
		else
			next.next2 = true;
	}
    else if ( item2.has_delete_characters() )
	{
		var len = Math.min(item1.delete_characters.length, item2.delete_characters.length);
		if ( len < item1.delete_characters.length )
			item1.delete_characters = item1.delete_characters.substring( len, item1.delete_characters.length );
		else
			next.next1 = true;
		if ( len < item2.delete_characters.length )
			item2.delete_characters = item2.delete_characters.substring( len, item2.delete_characters.length );
		else
			next.next2 = true;			
	}
	else
		throw "The two mutations are not compatible";
};

protocol.ProtocolDocumentOperation.xformUpdateAttributes = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
		var tmp = next.next1;
		next.next1 = next.next2;
		next.next2 = tmp;
	}
	else if ( item2.has_retain_item_count() )
	{
		r1.component.push( item1 );
		next.next1 = true;
		if ( len < item2.retain_item_count )
			item2.retain_item_count -= 1;
		else
		{
			r2.component.push( item2 );
			next.next2 = true;
		}
	}
	else if ( item2.has_delete_element_start() )
	{
		next.next1 = true;
		r2.component.push( item2 );
		next.next2 = true;
	}
	else if ( item2.has_update_attributes() )
	{
		var attribs1 = { }
		for( var i = 0; i < item1.update_attributes.attribute_update.length; ++i )
		{
			var update = item1.update_attributes.attribute_update[i];
			attribs1[update.key] = update;
		}
		var attribs2 = { }
		for( var i = 0; i < item2.update_attributes.attribute_update.length; ++i )
		{
			var update = item2.update_attributes.attribute_update[i];
			attribs2[update.key] = update;
		}
		
		for( var i = 0; i < item2.update_attributes.attribute_update.length; ++i )
		{
			var update2 = item2.update_attributes.attribute_update[i];
			var update1 = attribs1[update2.key];
			if ( update1 )
				update2.old_value = update1.new_value;
		}
		var updates1 = [];
		for( var i = 0; i < item1.update_attributes.attribute_update.length; ++i )
		{
			var update1 = item1.update_attributes.attribute_update[i];
			var update2 = attribs2[update1.key];
			if ( update2 == null )
				updates1.push( update1 );
		}
		item1.update_attributes.attribute_update = updates1;
		r1.component.push(item1);
		r2.component.push(item2);
		next.next1 = true;
		next.next2 = true;
	}
	else if ( item2.has_replace_attributes() )
	{
		r1.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount(1) );
		next.next1 = true;
		
		var attribs1 = { }
		for( var i = 0; i < item1.update_attributes.attribute_update.length; ++i )
		{
			var update = item1.update_attributes.attribute_update[i];
			attribs1[update.key] = update;
		}
		
		var old_attribute = [];
		for( var i = 0; i < item2.replace_attributes.old_attribute.length; ++i )
		{
			var update2 = item2.replace_attributes.old_attribute[i];
			var update1 = attribs1[update2.key];
			if ( update1 )				
			{
				if ( !update1.has_new_value() )
					continue;
				else
					update2.value = update1.new_value;
			}
        }
		item2.replace_attributes.old_attribute = old_attribute;
		r2.component.push( item2 );
		next.next2 = true;
    }
	else
        throw "The two mutations are not compatible";
};


