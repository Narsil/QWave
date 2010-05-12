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
 * @param {Object} owner is the owner of this event. When an event is emitted, the listener gets a reference
 *                       to this event. Using the owner property, it is possible to find the object to which
 *                       this event belongs.
 * @constructor
 */
JSOT.Event = function(owner)
{
  /**
   * The object to which this event belongs.
   */
  this.owner = owner;
  this.counter = 0;
};

/**
 * Registers an event listener.
 *
 * @param {Function} func is a function which takes two arguments. The first argument is an instance of JSOT.EventArgs
 *                        or a compatible object. The second argument is a reference to the JSOT.Event that called the function.
 * @param {Object} obj is the object on which the function func will be invoked.
 * @return {String} a key that is required for unregistering the listener.
 */
JSOT.Event.prototype.addListener = function( func, obj )
{
  if ( !func )
  {
	var x = 0;
	var y = 0;
	window.console.log( "No function specified" );
  }
  var key = "+" + (this.counter++).toString();
  this[ key ] = [ func, obj ];
  return key;
};

JSOT.Event.prototype.removeListener = function( id )
{
  delete this[id];
};

/**
 * Sends the event with the specified event arguments to all registered listeners.
 */
JSOT.Event.prototype.emit = function(args)
{
  for( var key in this )
  {
    if ( key[0] == "+" )
    {
      var val = this[key];
      val[0].call( val[1] ? val[1] : window, args, this );
    }
  }
};

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
 *       Passing a null value results in a null url (see isNull)
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
  this.submitMutations( [mutation], add_participant, open_wavelet );
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
  /**
   * The first child node (ElementStart or TextNode) or null.
   */
  this.firstChild = null;
  /**
   * The first child node (ElementStart or TextNode) or null.
   */
  this.lastChild = null;  
};

/**
 * Adds an object that listens to all OT changes applied to the document.
 */
JSOT.Doc.prototype.addOTListener = function( listener )
{
  if ( !this.listeners )
    this.listeners = [];
  this.listeners.push( listener );
};

/**
 * Removes a lister.
 */
JSOT.Doc.prototype.removeOTListener = function( listener )
{
  if ( !this.listeners )
    return;
  for( var i = 0; i < this.listener.length; ++i )
  {
	if ( this.listeners[i] == listener )
	{
	  this.listeners.splice( i, 1 );
	  return;
	}
  }
};

/**
 * @param {string} event_type is either "newChild", "removeChild", "remove", or "textChange".
 * @return {JSOT.Event} the event object which can be used to register callbacks.
 */
JSOT.Doc.prototype.getEvent = function( event_type )
{
  if ( event_type == "newChild" )
  {
	if ( !this.new_child_event )
	  this.new_child_event = new JSOT.Event(this);
	return this.new_child_event;
  }
  if ( event_type == "removeChild" )
  {
	if ( !this.remove_child_event )
	  this.remove_child_event = new JSOT.Event(this);
	return this.remove_child_event;
  }
  if ( event_type == "remove" )
  {
	if ( !this.remove_event )
	  this.remove_event = new JSOT.Event(this);
	return this.remove_event;
  }
  if ( event_type == "textChange" )
  {
	if ( !this.text_change_event )
	  this.text_change_event = new JSOT.Event(this);
	return this.text_change_event;
  }
  if ( event_type == "attributeChange" )
  {
	if ( !this.attribute_change_event )
	  this.attribute_change_event = new JSOT.Event(this);
	return this.attribute_change_event;
  }
  throw "Unknown event " + event_type;
};

JSOT.Doc.prototype.insertBefore = function( newNode, beforeNode )
{  
  var m = new protocol.ProtocolWaveletOperation_MutateDocument();
  m.document_id = this.docId;
  m.document_operation = new protocol.ProtocolDocumentOperation();
  var docop = m.document_operation;
  docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( beforeNode ? beforeNode.start_index : this.content.length ) );
  newNode.createDocOps_( docop );
  docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.content.length - ( beforeNode ? beforeNode.start_index : 0 )  ) );

  // Apply locally and send to the server
  this.wavelet.submitMutation( m );
};

JSOT.Doc.prototype.appendChild = JSOT.Doc.prototype.insertBefore;

JSOT.Doc.prototype.removeChild = function( node )
{  
  assert( !node.parentNode && node.doc == this, "The node must be a root child of the document.");
  
  var m = new protocol.ProtocolWaveletOperation_MutateDocument();
  m.document_id = this.docId;
  m.document_operation = new protocol.ProtocolDocumentOperation();
  var docop = m.document_operation;
  docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( node.start_index ) );
  node.createRemoveDocOps_( docop );
  docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.content.length - node.end_index - 1  ) );

  // Apply locally and send to the server
  this.wavelet.submitMutation( m );
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

JSOT.Doc.TextNode = function(text, doc)
{
  /**
   * The document to which the text node belongs.
   */
  this.doc = doc;
  /**
   * The content of the text node.
   */
  this.text = text;
  /**
   * The next sibling node (ElementStart or TextNode) or null.
   */
  this.nextSibling = null;  
  /**
   * The next sibling node (ElementStart or TextNode) or null.
   */
  this.previousSibling = null;  
  /**
   * The parent node (ElementStart) or null if this is a root text node.
   * Unlike XML, text nodes can exist at the root level.
   */
  this.parentNode = null;  
};

/**
 * Internal helper function.
 * @private
 */
JSOT.Doc.TextNode.prototype.insertText_ = function ( pos, str )
{
  this.text = this.text.substr( 0, pos ) + str + this.text.substring( pos, this.text.length );
  this.has_text_change = true;
};

/**
 * Internal helper function.
 * @private 
 */
JSOT.Doc.TextNode.prototype.appendText_ = function ( str )
{
  this.text += str;
  this.has_text_change = true;
};

/**
 * Internal helper function.
 * @private 
 */
JSOT.Doc.TextNode.prototype.removeText_ = function ( pos, len )
{
  this.text = this.text.substr(0, pos ) + this.text.substring(pos + len, this.text.length);
  if ( this.text.length == 0 && this.parentNode )
	this.parentNode.has_text_change = true;
  this.has_text_change = true;
};

/**
 * Internal helper function.
 * @private
 */
JSOT.Doc.TextNode.prototype.appendTextNode_ = function ( textNode )
{
  this.text += textNode.text;
};

JSOT.Doc.TextNode.prototype.createDocOps_ = function( docop )
{
  docop.component.push( protocol.ProtocolDocumentOperation.newCharacters( this.text ) );
};

JSOT.Doc.TextNode.prototype.createRemoveDocOps_ = function( docop )
{
  docop.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( this.text ) );
};

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
   * The first child node (ElementStart or TextNode) or null.
   */
  this.firstChild = null;
  /**
   * The first child node (ElementStart or TextNode) or null.
   */
  this.lastChild = null;
  /**
   * The next sibling node (ElementStart or TextNode) or null.
   */
  this.nextSibling = null;  
  /**
   * The next sibling node (ElementStart or TextNode) or null.
   */
  this.previousSibling = null;  
  /**
   * The parent node (ElementStart) or null if this is a root element.
   */
  this.parentNode = null;
};

/**
 * @param {string} type is either "newChild", "removeChild", "remove", or "textChange".
 * @return {JSOT.Event} the event object which can be used to register callbacks.
 */
JSOT.Doc.ElementStart.prototype.getEvent = function( event_type )
{
  if ( event_type == "newChild" )
  {
	if ( !this.new_child_event )
	  this.new_child_event = new JSOT.Event(this);
	return this.new_child_event;
  }
  if ( event_type == "removeChild" )
  {
	if ( !this.remove_child_event )
	  this.remove_child_event = new JSOT.Event(this);
	return this.remove_child_event;
  }
  if ( event_type == "remove" )
  {
	if ( !this.remove_event )
	  this.remove_event = new JSOT.Event(this);
	return this.remove_event;
  }  
  if ( event_type == "textChange" )
  {
	if ( !this.text_change_event )
	  this.text_change_event = new JSOT.Event(this);
	return this.text_change_event;
  }
  if ( event_type == "attributeChange" )
  {
	if ( !this.attribute_change_event )
	  this.attribute_change_event = new JSOT.Event(this);
	return this.attribute_change_event;
  }  
  throw "Unknown event " + event_type;
};

JSOT.Doc.ElementStart.prototype.insertBefore = function( newNode, beforeNode )
{
  assert( !beforeNode || (beforeNode.element_start && beforeNode.parentNode == this ), "beforeNode must be null or a child element." );
  
  if ( this.doc )
  {
	var m = new protocol.ProtocolWaveletOperation_MutateDocument();
	m.document_id = this.doc.docId;
	m.document_operation = new protocol.ProtocolDocumentOperation();
	var docop = m.document_operation;
	docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( beforeNode ? beforeNode.start_index : this.end_index ) );
	newNode.createDocOps_( docop );
	docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.content.length - ( beforeNode ? beforeNode.start_index : this.end_index )  ) );

	// Apply locally and send to the server
	this.doc.wavelet.submitMutation( m );
  }
  else
  {
	newNode.parentNode = this;
	if ( !this.firstChild )
	{
	  this.firstChild = newNode;
	  this.lastChild = newNode;
	  newNode.previousSibling = null;
	  newNode.nextSibling = null;
	}
	else
	{
	  if ( this.firstChild == beforeNode )
		this.firstChild = newNode;
	  if ( beforeNode )
	  {
		if ( beforeNode.previousSibling )
		  beforeNode.previousSibling.nextSibling = newNode;
		newNode.previousSibling = beforeNode.previousSibling;
		newNode.nextSibling = beforeNode;
		beforeNode.previousSibling = newNode;
	  }
	  else
	  {
		this.lastChild.nextSibling = newNode;
		newNode.previousSibling = this.lastChild;
		newNode.nextSibling = null;
		this.lastChild = newNode;
	  }
	}
  }
};

JSOT.Doc.ElementStart.prototype.removeChild = function( node )
{
  assert( node.parentNode == this, "The node to be removed must be a direct child node");
  
  if ( this.doc )
  {
	var m = new protocol.ProtocolWaveletOperation_MutateDocument();
	m.document_id = this.doc.docId;
	m.document_operation = new protocol.ProtocolDocumentOperation();
	var docop = m.document_operation;
	docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( node.start_index ) );
	node.createRemoveDocOps_( docop );
	docop.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.content.length - node.end_index - 1 ) );

	// Apply locally and send to the server
	this.doc.wavelet.submitMutation( m );
  }
  else
  {
	delete node.parentNode;
	if ( this.firstChild == node )
	  this.firstChild = node.nextSibling;
	if ( this.lastChild == node )
	  this.lastChild = node.previousSibling;
	if ( node.nextSibling )
	  node.nextSibling.previousSibling = node.previousSibling;
	if ( node.previousSibling )
	  node.previousSibling.nextSibling = node.nextSibling;
  }
};

/**
 * Internal helper function.
 * @private
 */
JSOT.Doc.ElementStart.prototype.createDocOps_ = function( docop )
{
  var attribs = [];
  for( var key in this.attributes )  
	attribs.push( protocol.ProtocolDocumentOperation.newKeyValuePair( key, this.attributes[key] ) );
  
  docop.component.push( protocol.ProtocolDocumentOperation.newElementStart( this.type, attribs ) );
  var child = this.firstChild;
  while( child )
  {
	child.createDocOps_( docop );
	child = child.nextSibling;
  }
  docop.component.push( protocol.ProtocolDocumentOperation.newElementEnd() );
};

/**
 * Internal helper function.
 * @private
 */
JSOT.Doc.ElementStart.prototype.createRemoveDocOps_ = function( docop )
{
  var attribs = [];
  for( var key in this.attributes )  
	attribs.push( protocol.ProtocolDocumentOperation.newKeyValuePair( key, this.attributes[key] ) );
  
  docop.component.push( protocol.ProtocolDocumentOperation.newDeleteElementStart( this.type, attribs ) );
  var child = this.firstChild;
  while( child )
  {
	child.createRemoveDocOps_( docop );
	child = child.nextSibling;
  }
  docop.component.push( protocol.ProtocolDocumentOperation.newDeleteElementEnd() );
};

JSOT.Doc.ElementStart.prototype.setLocalAttribute = function( key, value )
{
  if ( !this.localAttributes )
	this.localAttributes = { };
  this.localAttributes[ key ] = value;
  
  // Emit a signal
  var p = this;
  while( p )
  {
	if ( p.attribute_change_event )
	  p.attribute_change_event.emit( new JSOT.Doc.EventArgs( this.doc, "attributeChange", this ) );
	p = parentNode;
  }
};

JSOT.Doc.ElementStart.prototype.getLocalAttribute = function( key )
{
  if ( !this.localAttributes )
	return null;
  return this.localAttributes[ key ];
};

/**
 * @return {int} the number of items in front of this element. Each element_start, element_end
 * and single characters count as one.
 */
JSOT.Doc.ElementStart.prototype.itemCountBefore = function()
{
  return this.start_index;
};

/**
 * @return {string} the concatenation of all text inside this element. This includes text of nested elements as well.
 */
JSOT.Doc.ElementStart.prototype.getText = function()
{  
  var t = "";

  var node = this.firstChild;
  while( node )
  {
	if ( node.text )
	  t += node.text;
	else
	  t += node.getText();
	node = node.nextSibling;
  }
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
	if ( item.element_start && item.type == type )
	  return item;    
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
	if ( item.element_start && item.type == type )
	  result.push( item );    
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
	if ( item.element_start && item.attributes["id"] == id )
	  return item;    
  }
  return null;
};

/**
 * This event is called if a new child element has been inserted as a (direct or indirect) child of this element.
 *
 * @name JSOT.Doc.ElementStart#newChild
 * @event
 * @param {JSOT,Doc,ElementStart} child is the created child element.
 * @see JSOT.Doc#event:newChild
 */

/**
 * This event is called if a (direct or indirect) child element is to be removed, i.e. 
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
  return this.content.length;
};

/**
 * @param pos is an item index. Each character, element_start and element_end
 *            counts as one in this index.
 * @return {string} The character at this position or null if there is no character.
 */
JSOT.Doc.prototype.getCharAt = function(pos)
{
  var c = this.content[pos];
  return c.text[pos - c.start_index];
};

/**
 * @param pos is an item index. Each character, element_start and element_end
 *            counts as one in this index.
 * @return {string|JSOT.Doc.ElementStart|JSOT.Doc.ElementEnd} The character or item at this position.
 *         If pos is outside of the allowed range, the function returns null.
 */
JSOT.Doc.prototype.getItemAt = function(pos)
{
  var c = this.content[pos];
  if ( c.text )
	return c.text[pos - c.start_index];
  return c;
};

JSOT.Doc.prototype.getFormatAt = function(pos)
{
  return this.format[pos];
};

JSOT.Doc.prototype.getElementByType = function(type)
{
  for( var i = 0; i < this.content.length; ++i )
  {
    var item = this.content[i];
	if ( item.element_start && item.type == type )
	  return item;    
  }
  return null;
};

JSOT.Doc.prototype.getElementsByType = function(type)
{
  var result = [];
  for( var i = 0; i < this.content.length; ++i )
  {
    var item = this.content[i];
	if ( item.element_start && item.type == type )
	  result.push( item );    
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
	if ( item.element_start && item.attributes["id"] == id )
	  return item;    
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
	str = "";
    var c = this.content[i];
    if( this.format[i] != anno )
    {
      var a = this.format[i];
      if ( a != anno )
      {
		str = "[";
        for( var key in a )
        {
          str += (key + "=\"" + a[key] + "\" ");
        }
        str += "] ";
        anno = a;
      }
    }
    if ( c.element_start )
    {
      stack.push( c.type );
      str += ("<" + c.type);
      if ( c.attributes )
      {
        for( var key in c.attributes )
        {
          str += (" " + key + "=\"");
          str += (c.attributes[key]);
          str += ("\"");
        }
      }
      str += (">");
    }
    else if ( c.element_end )
    {
      str += ("<" + stack.pop() + "/>");
    }
    else
    {
      str += ( c.text[ i - c.start_index ] );
    }
	result.push(str);
  }
  
  return result.join("\n");
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
  //window.console.log("PROCESSING " + this.docId);
  
  var current = this;
  var currentIndex = -1;
  var stack = [];
  
  // Iterate over the document
  for( var i = 0; i < this.content.length; ++i )
  {
    var item = this.content[i];
    if ( item.text )
    {
    }
    else if ( item.element_start )
    {
      // The item is a new element and its parent has a newChild event handler?
      if ( item.is_new )
      {
        // New root element?
        if ( currentIndex == -1 )
		{
		  if ( current.new_child_event )
			current.new_child_event.emit( new JSOT.Doc.EventArgs( this, "newChild", item ) ); 
		}
		// New sub-element
        else
		{
		  var e = new JSOT.Doc.EventArgs( this, "newChild", item );
		  // Let the event bubble upwards in the tree
		  var x = current;
		  while( x )
		  {
			if ( x.new_child_event )
			{
			  x.new_child_event.emit( e );
			  if ( e.cancel )
				breakl
			}
			if ( x == this )
			  break;
			x = x.parentNode;
			if ( !x )
			  x = this;
		  }
		}
        delete item.has_new_text;
      }
      else if ( item.has_new_text )
      {
        var e = item;
        while( e )
        {
          if ( e.blockTextChange || e.is_new )
            break;
          if ( e.text_change_event )
          {
            e.blockTextChange = true;
            e.text_change_event.emit( new JSOT.Doc.EventArgs( this, "textChange", item ) );
            break;
          }
          e = e.parentNode;
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
};

JSOT.Doc.EventArgs = function( doc, event_type, element )
{
  this.doc = doc;
  this.event_type = event_type;
  this.element = element;
  this.cancel = false;
};

JSOT.Doc.EventArgs.prototype.stopPropagation = function()
{
  this.cancel = true;
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
  // Transform the document
  //
  
  // Position in this.components
  var opIndex = 0;
  // Position in doc.content
  var contentIndex = 0;
  // Increased by one whenever a delete_element_start is encountered and decreased by 1 upon delete_element_end
  var deleteDepth = 0;
  var insertDepth = 0;
  
  // The annotation that applies to the left of the current document position, type is a dict
  var docAnnotation = null;
  var updatedAnnotation = null;
  // The annotation update that applies to the right of the current document position  
  var annotationUpdate = { };
  // The number if keys in annotationUpdate
  var annotationUpdateCount = 0;
    
  // Tell all OT-listeners that OT starts now
  if ( doc.listeners )
    for( var i = 0; i < doc.listeners.length; ++i )
      doc.listeners[i].begin( doc );
    
  var c = doc.content[contentIndex];
  // Loop until all ops are processed
  while( opIndex < this.component.length )
  {
    var op = this.component[opIndex++];
	//
	// Insert element start
	//
    if ( op.element_start )
    {
      if ( deleteDepth > 0 )
        throw "Cannot insert inside a delete sequence";

	  // Inserting in the middle of a text node?
	  if ( c && c.text && contentIndex > 0 && doc.content[contentIndex - 1] == c )
	  {
		// Create a new text node
		var n2 = new JSOT.Doc.TextNode(null, doc);
		n2.start_index = contentIndex;
		n2.text = c.text.substring( n2.start_index - c.start_index, c.text.length );
		c.text = c.text.substr(0, n2.start_index - c.start_index );
		// Replace the old with the new text node in the following characters
		var i = contentIndex;
		while( i < doc.content.length && doc.content[i] == c )
		  doc.content[i++] = n2;
	  }
	  
	  // Create a map of attributes
      var attribs = { };
      for( var a in op.element_start.attribute )
      {
        var v = op.element_start.attribute[a];
        attribs[v.key] = v.value;
      }

      var element = new JSOT.Doc.ElementStart( op.element_start.type, attribs, doc );
	  element.is_new = true;
	  
	  // Compute annotation
      updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);

	  // Insert the element & format in the content & format arrays
	  doc.content.splice( contentIndex, 0, element );
	  doc.format.splice( contentIndex, 0, updatedAnnotation );
	  c = doc.content[++contentIndex];
      
      insertDepth++;
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].insertElementStart( element, updatedAnnotation );
    }
	//
	// Insert element end
	//
    else if ( op.element_end )
    {
      if ( deleteDepth > 0 )
        throw "Cannot insert inside a delete sequence";
      if ( insertDepth == 0 )
        throw "Cannot delete element end without deleting element start";
      insertDepth--;
      
	  // Compute annotation
      updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);

	  // Insert the element & format in the content & format arrays
      doc.content.splice( contentIndex, 0, new JSOT.Doc.ElementEnd() );
      doc.format.splice( contentIndex, 0, updatedAnnotation );
      c = doc.content[++contentIndex];

      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].insertElementEnd( currentElement, updatedAnnotation );
    }
	//
	// Insert characters
	//
    else if ( op.characters )
    {
      if ( deleteDepth > 0 )
        throw "Cannot insert inside a delete sequence";
      if ( op.characters.length == 0 )
        continue;
      
	  // Compute annotation
      updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);

	  var textNode;
	  // Insert to the left of a text node?
	  if ( c && c.text && insertDepth == 0 )
	  {
		if ( contentIndex == 0 || doc.content[contentIndex-1] != c )
		  c.start_index = contentIndex;
		textNode = c;
		textNode.insertText_( contentIndex - textNode.start_index, op.characters );		
	  }
	  // Insert to the right of a text node?
	  else if ( contentIndex > 0 && doc.content[contentIndex-1].text )
	  {
		textNode = doc.content[contentIndex-1];
		textNode.appendText_( op.characters );
	  }
	  // No text node to the left and no text node to the right
      else
      {
		// Create a new text node
		textNode = new JSOT.Doc.TextNode(op.characters, doc);
		textNode.start_index = contentIndex;
		textNode.is_new = true;
      }
	  
	  // Insert the characters in content & format array
	  for( var i = 0; i < op.characters.length; ++i )
	  {
		doc.content.splice( contentIndex + i, 0, textNode );
		doc.format.splice( contentIndex + i, 0, updatedAnnotation );
	  }
	  // Skip the inserted characters
	  contentIndex += op.characters.length;
	  c = doc.content[contentIndex];
      
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].insertCharacters( op.characters, updatedAnnotation );
    }
    else if ( op.delete_characters )
    {      
      if ( insertDepth > 0 )
        throw "Cannot delete inside an insertion sequence";
      if ( !c || !c.text )
        break; // Results in an exception outside the loop
  	  
	  if ( contentIndex == 0 || doc.content[contentIndex-1] != c )
		c.start_index = contentIndex;

	  if ( c.text.substr( contentIndex - c.start_index, op.delete_characters.length ) != op.delete_characters )
          throw "Cannot delete characters here, because at this position there are no characters";

	  c.removeText_( contentIndex - c.start_index, op.delete_characters.length );
	  
	  doc.content.splice( contentIndex, op.delete_characters.length );
	  doc.format.splice( contentIndex, op.delete_characters.length );
	  c = doc.content[contentIndex];
	  
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].deleteCharacters( op.delete_characters, updatedAnnotation );
    }
    else if ( op.retain_item_count )
    {
      if ( insertDepth > 0 )
        throw "Cannot retain inside an insertion sequence";
      if ( !c )
        throw "document op is larger than doc";
      if ( deleteDepth > 0 )
        throw "Cannot retain inside a delete sequence";
            
      for( var count = 0; count < op.retain_item_count; ++count )
      {
        if ( !c )
          throw "document op is larger than doc";

		docAnnotation = doc.format[contentIndex];
		updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
		// Update the annotation
		doc.format[contentIndex] = updatedAnnotation;

		// Retaining an element start?
        if ( c.element_start )
        {
          if ( doc.listeners )
            for( var i = 0; i < doc.listeners.length; ++i )
              doc.listeners[i].retainElementStart( c, updatedAnnotation );         
        }
        // Retaining an element end?
        else if ( c.element_end )
        {
          if ( doc.listeners )
            for( var i = 0; i < doc.listeners.length; ++i )
              doc.listeners[i].retainElementEnd( currentElement, updatedAnnotation );
        }
        // Retaining characters
        else  
        {
		  // A text node starts here?
		  if ( contentIndex > 0 && doc.content[contentIndex - 1] != c )
			c.start_index = contentIndex;
          
          if ( doc.listeners )
            for( var i = 0; i < doc.listeners.length; ++i )
              doc.listeners[i].retainCharacters( 1, updatedAnnotation );
        }
		          
		// Next item
		c = doc.content[++contentIndex];
      }
    }
    else if ( op.delete_element_start )
    {
      if ( insertDepth > 0 )
        throw "Cannot delete inside an insertion sequence";
      if ( !c || !c.element_start )
        throw "Cannot delete element start at this position, because in the document there is none";
      if ( c.type != op.delete_element_start.type )
	  {
		window.console.log(c.type);
		window.console.log(op.delete_element_start.type);
        throw "Cannot delete element start because Op and Document have different element type";
	  }
      // Count how many opening elements have been deleted. The corresponding closing elements must be deleted, too.
      deleteDepth++;

      // How many attributes does the element in the doc have?
      var acount = 0;
      for( var a in c.attributes )
        ++acount;      
      if ( acount != op.delete_element_start.attribute.length )
        throw "Cannot delete element start because the attributes of Op and Doc differ";
      // Create a dictionary of attributes
      var attribs = { };
      for( var a in op.delete_element_start.attribute )
      {
        var v = op.delete_element_start.attribute[a];
        attribs[v.key] = v.value;
      }
      // Compare attributes from the doc and docop. They should be equal
      for( var a in c.attributes )
      {
        if ( c.attributes[a] != attribs[a] )
          throw "Cannot delete element start because attribute values differ";
      }
      
	  // Need to send events to potential listeners?
	  if ( deleteDepth == 1 )
	  {
		// Fix the tree
		this.buildTree_( doc, contentIndex );
		// Send an event to listeners of all sub-elements because these sub-elements are about to become deleted.
		var child = c.firstChild;
		while( child )
		{
		  this.emitRemoveEvent_( child );
		  child = child.nextSibling;
		}
		var e;
		// Send an event to listeners on the about-to-be-deleted element.
		if ( c.remove_event )
		{
		  e = new JSOT.Doc.EventArgs( this, "remove", c );
		  c.remove_event.emit( e );
		}		
		// Propagate the message upward in the tree?
		if ( !e || !e.cancel )
		{
		  var x = c.parentNode;
		  e = null;
		  while( x )
		  {
			if ( x.remove_child_event )
			{
			  if ( !e )
				e = new JSOT.Doc.EventArgs( this, "removeChild", c );
			  x.remove_child_event.emit( e );
			  if ( e.cancel )
				break;
			}
			if ( x.element_start )
			  x = x.parentNode ? x.parentNode : doc;
			else
			  break;
		  }
		}
	  }
	  
      var oldc = c;      
      doc.content.splice( contentIndex, 1 );
      doc.format.splice( contentIndex, 1 );
      c = doc.content[contentIndex];
      
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].deleteElementStart( oldc, updatedAnnotation );
    }
    else if ( op.delete_element_end )
    {
      if ( insertDepth > 0 )
        throw "Cannot delete inside an insertion sequence";
      if ( !c || !c.element_end )
        throw "Cannot delete element end at this position, because in the document there is none";
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

      doc.content.splice( contentIndex, 1 );
      doc.format.splice( contentIndex, 1 );
      c = doc.content[contentIndex];
      deleteDepth--;
      
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].deleteElementEnd(updatedAnnotation);
    }
    else if ( op.update_attributes )
    {
      if ( insertDepth > 0 || deleteDepth > 0 )
        throw "Cannot update attributes inside an insertion sequence";
      if ( !c || !c.element_start )
        throw "Cannot update attributes at this position, because in the document there is no start element";
      
      // Compute the annotation for this element start
      var anno = doc.format[contentIndex];
      if ( anno != docAnnotation )
      {
        docAnnotation = anno;
        updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
      }
      doc.format[contentIndex] = updatedAnnotation;
      
      // Update the attributes
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
      // Go to the next item
      c = doc.content[++contentIndex];
	  
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].updateAttributes( currentElement, updatedAnnotation );
    }
    else if ( op.replace_attributes )
    {
      if ( insertDepth > 0 || deleteDepth > 0 )
        throw "Cannot replace attributes inside an insertion sequence";
      if ( !c || !c.element_start )
        throw "Cannot replace attributes at this position, because in the document there is no start element";

      // Compare the attributes from the docop and the doc. They must be equal
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
      
      // Compute the annotation for this element start
      var anno = doc.format[contentIndex];
      if ( anno != docAnnotation )
      {
        docAnnotation = anno;
        updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
      }
      doc.format[contentIndex] = updatedAnnotation;
      
      // Change the attributes of the element
      c.attributes = { }
      for( var a in op.replace_attributes.new_attribute )
      {
		var keyvalue = op.replace_attributes.new_attribute[a];
        c.attributes[keyvalue.key] = keyvalue.value;
      }
      // Go to the next item
      c = doc.content[++contentIndex];
	  
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].replaceAttributes( currentElement, updatedAnnotation );
    }
    else if ( op.annotation_boundary )
    {
      // Change the 'annotationUpdate' and find out when the annotation update becomes empty.
      for( var a in op.annotation_boundary.end )
      {        
        var key = op.annotation_boundary.end[a];
        if ( !annotationUpdate[key] )
          throw "Cannot end annotation because the doc and op annotation do not match.";
        delete annotationUpdate[key];
        annotationUpdateCount--;
      }
      // Change the 'annotationUpdate' and find out when the annotation update becomes empty.
      for( var a in op.annotation_boundary.change )
      {
        var change = op.annotation_boundary.change[a];
        if ( !annotationUpdate[change.key] )
          annotationUpdateCount++;
        annotationUpdate[change.key] = change;
      }
      // The commented line below is WRONG because the update cannot be applied to the format on the left side of the cursor.
      // updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
      
      if ( doc.listeners )
        for( var i = 0; i < doc.listeners.length; ++i )
          doc.listeners[i].annotationBoundary( annotationUpdateCount == 0 ? null : annotationUpdate, updatedAnnotation );
    }
  }
    
  if ( opIndex < this.component.length )
    throw "Document is too small for op"
  // Should be impossible if the document is well formed ... Paranoia
  if ( deleteDepth != 0 )
    throw "Not all delete element starts have been matched with a delete element end";
  if ( insertDepth != 0 )
    throw "Not all opened elements have been closed";
  if ( contentIndex < doc.content.length )
    throw "op is too small for document";
    
  this.buildTree_( doc, doc.content.length );

  if ( doc.listeners )
    for( var i = 0; i < doc.listeners.length; ++i )  
      doc.listeners[i].end( doc );	
};

/**
 * Repairs the tree structure of the entire document or parts of it.
 * If invoked several times in sequence, it always starts where it left off and executes until 'toContentIndex'.
 *
 * @param {int} toContentIndex is an index in doc.content. It is either the index of an ElementStart or the
 *              length of doc.content.
 */
protocol.ProtocolDocumentOperation.prototype.buildTree_ = function( doc, toContentIndex )
{
  // First run?
  if ( !this.build_tree )
  {
	this.build_tree = { };
	this.build_tree.stack = [ ];
	if ( doc.content.length > 0 )
	  doc.firstChild = doc.content[0];
	else
	  doc.firstChild = null;
	this.build_tree.i = 0;
	this.build_tree.currentElement = null;
	this.build_tree.currentSibling = null;
	this.build_tree.currentTextNode = null;
  }
  
  while ( this.build_tree.i < toContentIndex )
  {
	var item = doc.content[this.build_tree.i];
	
	if ( item.element_start || item.text )
	{
	  item.parentNode = this.build_tree.currentElement;
	  item.previousSibling = this.build_tree.currentSibling;
	  if ( this.build_tree.currentSibling )
		this.build_tree.currentSibling.nextSibling = item;
	}
	
	if ( item.element_start )
	{
	  this.build_tree.stack.push( this.build_tree.currentElement );
	  this.build_tree.currentElement = item;
	  this.build_tree.currentElement.start_index = this.build_tree.i;
	  this.build_tree.currentSibling = null;	  
	  this.build_tree.i++;
	}
	else if ( item.element_end )
	{
	  if ( this.build_tree.currentSibling )
		this.build_tree.currentSibling.nextSibling = null;
	  
	  this.build_tree.currentElement.end_index = this.build_tree.i;
	  this.build_tree.currentElement.lastChild = this.build_tree.currentSibling;
	  if ( this.build_tree.currentElement.end_index - this.build_tree.currentElement.start_index == 1 )
		this.build_tree.currentElement.firstChild = null;
	  else
		this.build_tree.currentElement.firstChild = doc.content[this.build_tree.currentElement.start_index + 1];
	  this.build_tree.currentSibling = this.build_tree.currentElement;
	  this.build_tree.currentElement = this.build_tree.stack.pop();
	  
	  this.build_tree.i++;
	}
	else
	{
	  // Need to merge this text node with the previous one?
	  if ( this.build_tree.i > 0 && doc.content[this.build_tree.i-1] != item && doc.content[this.build_tree.i-1].text )
	  {
		var t = doc.content[this.build_tree.i-1];
		t.appendTextNode_( item );
		while( this.build_tree.i < doc.content.length && doc.content[this.build_tree.i] == item )
		  doc.content[this.build_tree.i++] = t;
		item = t;
	  }
	  else
	  {
		item.start_index = this.build_tree.i;
		while( this.build_tree.i < doc.content.length && doc.content[this.build_tree.i] == item )
		{
		  if ( this.build_tree.i >= toContentIndex ) throw "Index in buildTree_ is in the middle of text. Must be on ElementStart instead.";
		  this.build_tree.i++;
		}
	  }
	  item.end_index = this.build_tree.i;
	  this.build_tree.currentSibling = item;
	}
  }
  
  if ( doc.content.length != toContentIndex )
  {
	// Get the first element that has not been treated by the loop above and fix its
	// previous sibling property.
	var item = doc.content[toContentIndex];
	if ( !item.element_start ) throw "Wrong index in buildTree_";
	item.previousSibling = this.build_tree.currentSibling;
	if ( this.build_tree.currentSibling )
	  this.build_tree.currentSibling.nextSibling = item;
  }
  else  
  {
	doc.lastChild = this.build_tree.currentSibling;
	if ( doc.lastChild )
	  doc.lastChild.nextSibling = null;
	delete this.build_tree;
  }
}

protocol.ProtocolDocumentOperation.prototype.emitRemoveEvent_ = function( child )
{
  // Recursion over all children
  var c = child.firstChild;
  while( c )
  {
	this.emitRemoveEvent_( c );
	c = c.nextSibling;
  }
  
  // Emit the event for this child (if required )
  if ( child.remove_event )
	child.remove_event.emit( new JSOT.Doc.EventArgs( child.doc, "remove", child ) );
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
    
    // Tests
    if ( !change.old_value )
    {
      if ( docAnnotation && docAnnotation[a] )
        throw "Annotation update " + change.old_value + "and current annotation do not match";
    }
    else
    {
      if ( !docAnnotation || docAnnotation[a] != change.old_value )
        throw "Annotation update " + change.old_value + "and current annotation do not match";
    }
    
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


