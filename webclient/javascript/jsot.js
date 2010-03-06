var JSOT = { };

JSOT.Doc = function()
{
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

JSOT.DocOp = function()
{
	this.component = [ ];
};

JSOT.DocOp.prototype.applyTo = function(doc)
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

			if ( inContentIndex == 0 )
			{
				doc.content.splice( contentIndex, 0, new JSOT.Doc.ElementStart( op.element_start.type, op.element_start.attribute ) );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
			}
			else
			{
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementStart( op.element_start.type, op.element_start.attribute ), c.substring(inContentIndex, c.length) );
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
			if ( c.attributes.length != op.delete_element_start.attribute.length )
				throw "Cannot delete element start because the attributes of Op and Doc differ";
			for( var a in c.attributes )
			{
				if ( c.attributes[a] != op.delete_element_start.attribute[a] )
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
			for( var a in op.update_attributes )
			{
				var update = op.update_attributes[a];
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
			if ( c.attributes.length != op.replace_attributes.oldAttributes.length )
				throw "Cannot replace attributes because the old attributes do not match";
			for( var a in c.attributes )
			{
				if ( c.attributes[a] != op.replace_attributes.oldAttributes[a] )
					throw "Cannot replace attributes because the value of the old attributes do not match";
			}
			// If there is an annotation boundary change in the deleted characters, this change must be applied
			var anno = doc.format[contentIndex];
			if ( anno != docAnnotation )
			{
				docAnnotation = anno;
				updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			}											
			doc.format[contentIndex] = updatedAnnotation;
			c.attributes = { }
			for( var a in op.replace_attributes.newAttributes )
				c.attributes[a] = op.replace_attributes.newAttributes[a];
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

JSOT.DocOp.prototype.computeAnnotation = function(docAnnotation, annotationUpdate, annotationUpdateCount)
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

JSOT.DocOp.prototype.has_element_start = function()
{
	return this.element_start != null;
};

JSOT.DocOp.prototype.has_element_end = function()
{
	return this.element_end == true;
};

JSOT.DocOp.prototype.has_delete_element_start = function()
{
	return this.delete_element_start != null;
};

JSOT.DocOp.prototype.has_delete_element_end = function()
{
	return this.delete_element_end == true;
};

JSOT.DocOp.prototype.has_characters = function()
{
	return this.characters != null;
};

JSOT.DocOp.prototype.has_delete_characters = function()
{
	return this.delete_characters != null;
};

JSOT.DocOp.prototype.has_retain_item_count = function()
{
	return this.retain_item_count != null;
};

JSOT.DocOp.prototype.has_replace_attributes = function()
{
	return this.replace_attributes != null;
};

JSOT.DocOp.prototype.has_update_attributes = function()
{
	return this.update_attributes != null;
};

JSOT.DocOp.prototype.has_annotation_boundary = function()
{
	return this.annotation_boundary != null;
};

JSOT.DocOp.newElementStart = function(type, attributes)
{
	if ( !attributes )
		attributes = { };
	this.element_start = { type : type, attribute : attributes };
};

JSOT.DocOp.newElementEnd = function()
{
	this.element_end = true;
};

JSOT.DocOp.newCharacters = function(characters)
{
	this.characters = characters;
};

JSOT.DocOp.newDeleteCharacters = function(delete_characters)
{
	this.delete_characters = delete_characters;
};

JSOT.DocOp.newRetainItemCount = function(retain_item_count)
{
	this.retain_item_count = retain_item_count;
};
 
JSOT.DocOp.newDeleteElementStart = function(type, attributes)
{
	if ( !attributes )
		attributes = { };
	this.delete_element_start = { type : type, attribute : attributes };
};

JSOT.DocOp.newDeleteElementEnd = function()
{
	this.delete_element_end = true;
};

JSOT.DocOp.newReplaceAttributes = function(oldAttributes, newAttributes)
{
	this.replace_attributes = { oldAttributes : oldAttributes, newAttributes : newAttributes };
};

JSOT.DocOp.newUpdateAttributes = function(attributeUpdates)
{
	this.update_attributes = attributeUpdates;
};

JSOT.DocOp.newKeyValueUpdate = function( key, old_value, new_value )
{
	this.key = key;
	this.old_value = old_value;
	this.new_value = new_value;
};

JSOT.DocOp.newAnnotationBoundary = function(end, change)
{
	this.annotation_boundary = { end : end, change : change };
};

JSOT.ProtocolWaveletOperation = function()
{
};

JSOT.ProtocolWaveletOperation.MutateDocument = function(documentId, documentOperation)
{
	this.document_id = documentId;
	this.document_operation = documentOperation;
};

/////////////////////////////////////////////////
//
// OT
//
/////////////////////////////////////////////////

JSOT.DocOp.xform = function( m1, m2 )
{
    if ( m1.component.length == 0 || m2.component.length == 0 )
        return;

	var r1, r2, anno1, anno2;
	var c1 = 0;
	var c2 = 0;
	var item1 = m1.component[c1];
	var item2 = m2.component[c2];
	var next = { next1 : false, next2 : false };
    while( c1 < m1.component.length && c2 < m2.component.length )
    {
        if ( item1.has_element_start() )
        {           
			JSOT.DocOp.xformInsertElementStart( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_element_end() )
		{
			JSOT.DocOp.xformInsertElementEnd( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_characters() )
		{
			JSOT.DocOp.xformInsertChars( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_retain_item_count() )
		{
			JSOT.DocOp.xformRetain( r1, r2, item1, item2, next, anno1, anno2 );
		}
		else if ( item1.has_delete_element_start() )
		{
			JSOT.DocOp.xformDeleteElementStart( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
		}
		else if ( item1.has_delete_element_end() )
		{
			JSOT.DocOp.xformDeleteElementEnd( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
		}
		else if ( item1.has_delete_characters() )
		{
			JSOT.DocOp.xformDeleteChars( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
		}
		else if ( item1.has_update_attributes() )
		{
			JSOT.DocOp.xformUpdateAttributes( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
		}
		else if ( item1.has_replace_attributes() )
		{
			JSOT.DocOp.xformReplaceAttributes( r1, r2, item1, item2, next1, next2, anno1, anno2, ok );
		}
		else if ( item1.has_annotation_boundary() )
		{
			JSOT.DocOp.xformAnnotationBoundary( r1, r2, item1, item2, next1, next2, anno1, anno2, false, ok );
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

JSOT.DocOp.xformInsertElementStart = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	r1.component.push( item1 );
	r2.component.push( JSOT.DocOp.newRetainItemCount( 1 ) );
    next.next1 = true;
    next.next2 = false;
};

JSOT.DocOp.xformInsertElementEnd = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	r1.component.push( item1 );
	r2.component.push( JSOT.DocOp.newRetainItemCount( 1 ) );
    next.next1 = true;
    next.next2 = false;
};

JSOT.DocOp.xformInsertChars = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	r1.component.push( item1 );
	r2.component.push( JSOT.DocOp.newRetainItemCount( item1.characters.length ) );
    next.next1 = true;
    next.next2 = false;
};

JSOT.DocOp.xformRetain = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		JSOT.DocOp.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_element_end() )
	{
		JSOT.DocOp.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		JSOT.DocOp.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		JSOT.DocOp.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
	}
	else if ( item2.has_retain_item_count() )
	{
		var len = Math.min(item1.retain_item_count, item2.retain_item_count);
		r1.component.push( JSOT.DocOp.newRetainItemCount( len ) );
		r2.component.push( JSOT.DocOp.newRetainItemCount( len ) );
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
		r2.component.push( JSOT.DocOp.newDeleteCharacters( item2.delete_characters.substr(0, len ) ) );
		if ( len < item2.delete_characters.length )
			item2.delete_characters = item2.delete_characters.substring( len, item2.delete_characters.length );
		else
			next.next2 = true;
	}
	else if ( item2.has_replace_attributes() || item2.has_update_attribute() )
	{
		r1.components.push( JSOT.DocOp.newRetainItemCount(1) );
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

JSOT.DocOp.xformDeleteElementStart = function( r1, r2, item1, item2, next, anno1, anno2 )
{
	if ( item2.has_element_start() )
	{
		JSOT.DocOp.xformInsertElementStart( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_element_end() )
	{
		JSOT.DocOp.xformInsertElementEnd( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_characters() )
	{
		JSOT.DocOp.xformInsertChars( r2, r1, item2, item1, next, anno2, anno1);
	}
	else if ( item2.has_annotation_boundary() )
	{
		JSOT.DocOp.xformAnnotationBoundary( r2, r1, item2, item1, next, anno2, anno1, true );
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
	else if ( item2.has_delete_start() )
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
