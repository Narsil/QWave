var JSOT = { };

/////////////////////////////////////////////////
//
// WaveURL
//
/////////////////////////////////////////////////

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

JSOT.WaveUrl.prototype.isNull = function()
{
	return this.waveletId == null;
};

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

JSOT.Wave = function(id, domain)
{
	JSOT.Wave.waves[ id + "$" + domain] = this;
	this.id = id;
	this.domain = domain;
	this.wavelets = { };
};

JSOT.Wave.waves = { };

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

JSOT.Wave.process = function( update )
{
	var url = new JSOT.WaveUrl( update.wavelet_name );
	if ( url.isNull() ) throw "Malformed wave URL";
	var wave = JSOT.Wave.getWave( url.waveId, url.waveDomain );
	var wavelet = wave.getWavelet( url.waveletId, url.waveletDomain );
	for( var i = 0; i < update.applied_delta.length; ++i )
		wavelet.applyDelta( update.applied_delta[i] );
	wavelet.hashed_version = update.resulting_version;
};

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

JSOT.Wavelet = function(wave, id, domain)
{
	this.id = id;
	this.domain = domain;
	this.wave = wave;
	this.documents = { };
	this.participants = [ ];
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
};

JSOT.Wavelet.prototype.getDoc = function(docname)
{
	var doc = this.documents[docname];
	if ( doc )
		return doc;
	doc = new JSOT.Doc(docname);
	this.documents[docname] = doc;
	return doc;
};

JSOT.Wavelet.prototype.addParticipant = function( jid )
{
	var i = this.participants.indexOf(jid);
	if ( i != -1 )
		return;
	this.participants.push( jid );
};

JSOT.Wavelet.prototype.removeParticipant = function( jid )
{
	var i = this.participants.indexOf(jid);
	if ( i == -1 )
		return;
	delete this.participants[i];
};

/**
 * @param delta is of type protocol.ProtocolWaveletDelta.
 */
JSOT.Wavelet.prototype.applyDelta = function( delta )
{
	for( var i = 0; i < delta.operation.length; ++i )
	{
		var op = delta.operation[i];
		if ( op.has_add_participant() )
		{
			this.addParticipant( op.add_participant );
		}
		if ( op.has_remove_participant() )
		{
			this.removeParticipant( op.remove_participant );
		}
		if ( op.has_mutate_document() )
		{
			var doc = this.getDoc( op.mutate_document.document_id );
			var docop = op.mutate_document.document_operation;
			docop.applyTo( doc );
		}		
	}
};

JSOT.Wavelet.prototype.url = function()
{
	var url = new JSOT.WaveUrl();
	url.waveletDomain = this.domain;
	url.waveletId = this.id;
	url.waveDomain = this.wave.domain;
	url.waveId = this.wave.id;
	return url;
};

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

/////////////////////////////////////////////////
//
// WaveletDocument
//
/////////////////////////////////////////////////

JSOT.Doc = function(docId)
{
	this.docId = docId;
	this.content = [ ];
	this.format = [ ];
};

JSOT.Doc.ElementStart = function(type, attributes)
{
	this.element_start = true;
	this.type = type;
	if ( !attributes )
		this.attributes = { };
	else
		this.attributes = attributes;
};

JSOT.Doc.ElementEnd = function()
{
	this.element_end = true;
};

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

/////////////////////////////////////////////////
//
// DocumentOperation
//
/////////////////////////////////////////////////

protocol.ProtocolDocumentOperation.prototype.applyTo = function(doc)
{
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
			
			if ( inContentIndex == 0 )
			{
				doc.content.splice( contentIndex, 0, new JSOT.Doc.ElementStart( op.element_start.type, attribs ) );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
			}
			else
			{
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementStart( op.element_start.type, attribs ), c.substring(inContentIndex, c.length) );
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

			if ( inContentIndex == 0 )
			{
				doc.content.splice( contentIndex, 0, new JSOT.Doc.ElementEnd() );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
			}
			else
			{
				if ( updatedAnnotation )
					doc.format[contentIndex+1] = updatedAnnotation;
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementEnd(), c.substring(inContentIndex, c.length) );
				doc.format.splice( contentIndex + 1, 0, updatedAnnotation, docAnnotation );
				doc.content[contentIndex] = c.slice(0, inContentIndex);
				contentIndex += 2;
				c = doc.content[contentIndex];
				inContentIndex = 0;
			}
		}
		else if ( op.characters )
		{
			if ( deleteDepth > 0 )
				throw "Cannot insert inside a delete sequence";

			// if ( inContentIndex == 0 && contentIndex > 0 && typeof(doc.content[contentIndex-1]) == "string" )
			//	doc.content[contentIndex-1] = doc.content[contentIndex-1] + op.characters;
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
			var count = op.delete_characters.length;
			var done = 0;
			while( count > 0 )
			{
				if ( typeof(c) != "string" )
					throw "Cannot delete characters here, because at this position there are no characters";
				var i = Math.min( count, c.length - inContentIndex );
				if ( c.substr( inContentIndex, i ) != op.delete_characters.substr( done, i ) )
					throw "Cannot delete characters, because the characters in the document and operation differ.";
				c = c.substring(0, inContentIndex).concat( c.substring(inContentIndex + i, c.length ) );
				done += i;
				count -= i;
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
					doc.content[contentIndex] = c;
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
				if ( c.element_start || c.element_end )
				{
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
		}
		else if ( op.replace_attributes )
		{
			if ( !c )
				break;
			if ( !c.element_start )
				throw "Cannot replace attributes at this position, because in the document there is no start element";
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
		}
	}
	
	if ( opIndex < this.component.length )
		throw "Document is too small for op"
	// Should be impossible if the document is well formed ... Paranoia
	if ( deleteDepth != 0 )
		throw "Not all delete element starts have been matched with a delete element end";
	if ( contentIndex < doc.content.length )
		throw "op is too small for document";
};

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

protocol.ProtocolDocumentOperation.newElementEnd = function()
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.element_end = true;
	return c;
};

protocol.ProtocolDocumentOperation.newCharacters = function(characters)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.characters = characters;
	return c;
};

protocol.ProtocolDocumentOperation.newDeleteCharacters = function(delete_characters)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.delete_characters = delete_characters;
	return c;
};

protocol.ProtocolDocumentOperation.newRetainItemCount = function(retain_item_count)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.retain_item_count = retain_item_count;
	return c;
};
 
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

protocol.ProtocolDocumentOperation.newDeleteElementEnd = function()
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.delete_element_end = true;
	return c;
};

protocol.ProtocolDocumentOperation.newReplaceAttributes = function(old_attribute, new_attribute)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.replace_attributes = new protocol.ProtocolDocumentOperation_Component_ReplaceAttributes();
	c.replace_attributes.old_attribute = old_attribute;
	c.replace_attributes.new_attribute = new_attribute;
	return c;
};

protocol.ProtocolDocumentOperation.newUpdateAttributes = function(attributeUpdates)
{
	var c = new protocol.ProtocolDocumentOperation_Component();
	c.update_attributes = new protocol.ProtocolDocumentOperation_Component_ReplaceAttributes();
	c.update_attributes.attribute_update = attributeUpdates;
	return c;
};

protocol.ProtocolDocumentOperation.newKeyValueUpdate = function( key, old_value, new_value )
{
	var c = new protocol.ProtocolDocumentOperation_Component_KeyValueUpdate();
	c.key = key;
	c.old_value = old_value;
	c.new_value = new_value;
	return c;
};

protocol.ProtocolDocumentOperation.newKeyValuePair = function( key, value )
{
	var c = new protocol.ProtocolDocumentOperation_Component_KeyValuePair();
	c.key = key;
	c.value = value;
	return c;
};

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
			protocol.ProtocolDocumentOperations.xform( m1.mutate_document, m2.mutate_document );
	}
};

/////////////////////////////////////////////////
//
// OT
//
/////////////////////////////////////////////////

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
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
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
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
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
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
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
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
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
	}
	else if ( item2.has_element_end() )
	{
		protocol.ProtocolDocumentOperation.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		protocol.ProtocolDocumentOperation.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		protocol.ProtocolDocumentOperation.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
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


