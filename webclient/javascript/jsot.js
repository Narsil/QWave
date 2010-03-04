var JSOT = { };

JSOT.Doc = function()
{
	this.content = [ ];
	this.format = [ ];
};

JSOT.Doc.ElementStart = function(type, attributes)
{
	this.elementStart = true;
	this.type = type;
	if ( !attributes )
		this.attributes = { };
	else
		this.attributes = attributes;
};

JSOT.Doc.ElementEnd = function()
{
	this.elementEnd = true;
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
		if ( c.elementStart )
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
		else if ( c.elementEnd )
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
	this.components = [ ];
};

JSOT.DocOp.prototype.applyTo = function(doc)
{
	// Position in this.components
	var opIndex = 0;
	// Position in doc.content
	var contentIndex = 0;
	// Position in the text of variable 'c'
	var inContentIndex = 0;
	// Increased by one whenever a deleteElementStart is encountered and decreased by 1 upon deleteElementEnd
	var deleteDepth = 0;
	
	// The annotation that applies to the left of the current document position
	var docAnnotation = null;
	var updatedAnnotation = null;
	// The annotation update that applies to the right of the current document position	
	var annotationUpdate = { };
	var annotationUpdateCount = 0;
	
	var c = doc.content[contentIndex];
	// Loop until all ops are processed
	while( opIndex < this.components.length )
	{
		var op = this.components[opIndex++];
		if ( op.elementStart )
		{
			if ( deleteDepth > 0 )
				throw "Cannot insert inside a delete sequence";

			if ( inContentIndex == 0 )
			{
				doc.content.splice( contentIndex, 0, new JSOT.Doc.ElementStart( op.elementStart.type, op.elementStart.attributes ) );
				doc.format.splice( contentIndex, 0, updatedAnnotation );
				c = doc.content[++contentIndex];
			}
			else
			{
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementStart( op.elementStart.type, op.elementStart.attributes ), c.substring(inContentIndex, c.length) );
				doc.format.splice( contentIndex + 1, 0, updatedAnnotation, docAnnotation );
				doc.content[contentIndex] = c.slice(0, inContentIndex);
				contentIndex += 2;
				c = doc.content[contentIndex];
				inContentIndex = 0;
			}			
		}
		else if ( op.elementEnd )
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
		else if ( op.deleteCharacters )
		{
			if ( !c )
				break;		
			var count = op.deleteCharacters.length;
			var done = 0;
			while( count > 0 )
			{
				if ( typeof(c) != "string" )
					throw "Cannot delete characters here, because at this position there are no characters";
				var i = Math.min( count, c.length - inContentIndex );
				if ( c.substr( inContentIndex, i ) != op.deleteCharacters.substr( done, i ) )
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
		else if ( op.retainItemCount )
		{
			if ( !c )
				break;
			if ( deleteDepth > 0 )
				throw "Cannot retain inside a delete sequence";
			var count = op.retainItemCount;
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
				if ( c.elementStart || c.elementEnd )
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
		else if ( op.deleteElementStart )
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
			if ( !c.elementStart )
				throw "Cannot delete element start at this position, because in the document there is none";
			if ( c.type != op.deleteElementStart.type )
				throw "Cannot delete element start because Op and Document have different element type";
			if ( c.attributes.length != op.deleteElementStart.attributes.length )
				throw "Cannot delete element start because the attributes of Op and Doc differ";
			for( var a in c.attributes )
			{
				if ( c.attributes[a] != op.deleteElementStart.attributes[a] )
					throw "Cannot delete element start because attribute values differ";
			}
			doc.content.splice( contentIndex, 1 );
			doc.format.splice( contentIndex, 1 );
			c = doc.content[contentIndex];
			inContentIndex = 0;			
		}
		else if ( op.deleteElementEnd )
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
			if ( !c.elementEnd )
				throw "Cannot delete element end at this position, because in the document there is none";
			doc.content.splice( contentIndex, 1 );
			doc.format.splice( contentIndex, 1 );
			c = doc.content[contentIndex];
			inContentIndex = 0;
			deleteDepth--;		
		}
		else if ( op.updateAttributes )
		{		
			if ( !c )
				break;
			if ( !c.elementStart )
				throw "Cannot update attributes at this position, because in the document there is no start element";
			// If there is an annotation boundary change in the deleted characters, this change must be applied
			var anno = doc.format[contentIndex];
			if ( anno != docAnnotation )
			{
				docAnnotation = anno;
				updatedAnnotation = this.computeAnnotation(docAnnotation, annotationUpdate, annotationUpdateCount);
			}								
			doc.format[contentIndex] = updatedAnnotation;
			for( var a in op.updateAttributes )
			{
				var update = op.updateAttributes[a];
				// Add a new attribute?
				if ( update.oldValue == null )
				{
					if ( c.attributes[update.key] )
						throw "Cannot update attributes because old attribute value is not mentioned in Op";
					if ( !update.newValue )
						throw "Cannot update attributes because new value is missing";
					c.attributes[update.key] = update.newValue;
				}
				// Delete or change an attribute
				else
				{
					if ( c.attributes[update.key] != update.oldValue )
						throw "Cannot update attributes because old attribute value does not match with Op";
					if ( !update.newValue )
						delete c.attributes[update.key];
					else
						c.attributes[update.key] = update.newValue;
				}
			}
			c = doc.content[++contentIndex];
			inContentIndex = 0;							
		}
		else if ( op.replaceAttributes )
		{
			if ( !c )
				break;
			if ( !c.elementStart )
				throw "Cannot replace attributes at this position, because in the document there is no start element";
			if ( c.attributes.length != op.replaceAttributes.oldAttributes.length )
				throw "Cannot replace attributes because the old attributes do not match";
			for( var a in c.attributes )
			{
				if ( c.attributes[a] != op.replaceAttributes.oldAttributes[a] )
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
			for( var a in op.replaceAttributes.newAttributes )
				c.attributes[a] = op.replaceAttributes.newAttributes[a];
			c = doc.content[++contentIndex];
			inContentIndex = 0;							
		}
		else if ( op.annotationBoundary )
		{
			for( var a in op.annotationBoundary.ends )
			{				
				var key = op.annotationBoundary.ends[a];
				if ( !annotationUpdate[key] )
					throw "Cannot end annotation because the doc and op annotation do not match.";
				delete annotationUpdate[key];
				annotationUpdateCount--;
			}
			for( var a in op.annotationBoundary.changes )
			{
				var change = op.annotationBoundary.changes[a];
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
	
	if ( opIndex < this.components.length )
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
		if ( change.newValue == null )
		{
			count--;
			delete anno[a];
		}
		else if ( change.oldValue == null )
		{
			count++;
			anno[a] = change.newValue;
		}
		else
		{
			if ( !docAnnotation || docAnnotation[a] != change.oldValue )
				throw "Annotation update and current annotation do not match";
			anno[a] = change.newValue;
		}
	}
	if ( count == 0 )
		return null;
	return anno;
};

JSOT.DocOp.ElementStart = function(type, attributes)
{
	if ( !attributes )
		attributes = { };
	this.elementStart = { type : type, attributes : attributes };
};

JSOT.DocOp.ElementEnd = function()
{
	this.elementEnd = true;
};

JSOT.DocOp.Characters = function(characters)
{
	this.characters = characters;
};

JSOT.DocOp.DeleteCharacters = function(deleteCharacters)
{
	this.deleteCharacters = deleteCharacters;
};

JSOT.DocOp.RetainItemCount = function(retainItemCount)
{
	this.retainItemCount = retainItemCount;
};
 
JSOT.DocOp.DeleteElementStart = function(type, attributes)
{
	if ( !attributes )
		attributes = { };
	this.deleteElementStart = { type : type, attributes : attributes };
};

JSOT.DocOp.DeleteElementEnd = function()
{
	this.deleteElementEnd = true;
};

JSOT.DocOp.ReplaceAttributes = function(oldAttributes, newAttributes)
{
	this.replaceAttributes = { oldAttributes : oldAttributes, newAttributes : newAttributes };
};

JSOT.DocOp.UpdateAttributes = function(attributeUpdates)
{
	this.updateAttributes = attributeUpdates;
};

JSOT.DocOp.KeyValueUpdate = function( key, oldValue, newValue )
{
	this.key = key;
	this.oldValue = oldValue;
	this.newValue = newValue;
};

JSOT.DocOp.AnnotationBoundary = function(ends, changes)
{
	this.annotationBoundary = { ends : ends, changes : changes };
};

JSOT.WaveletOp = function()
{
};

JSOT.WaveletOp.MutateDocument = function(documentId, documentOperation)
{
	this.documentId = documentId;
	this.documentOperation = documentOperation;
};
