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
	
	for( var i = 0; i < this.content.length; ++i )
	{
		var c = this.content[i];
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
				doc.format.splice( contentIndex, 0, null );
				c = doc.content[++contentIndex];
			}
			else
			{
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementStart( op.elementStart.type, op.elementStart.attributes ), c.substring(inContentIndex, c.length) );
				doc.format.splice( contentIndex + 1, 0, null, null );
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
				doc.format.splice( contentIndex, 0, null );
				c = doc.content[++contentIndex];
			}
			else
			{
				doc.content.splice( contentIndex + 1, 0, new JSOT.Doc.ElementEnd(), c.substring(inContentIndex, c.length) );
				doc.format.splice( contentIndex + 1, 0, null, null );
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

			if ( inContentIndex == 0 && contentIndex > 0 && typeof(doc.content[contentIndex-1]) == "string" )
				doc.content[contentIndex-1] = doc.content[contentIndex-1] + op.characters;
			else if ( typeof(c) == "string" )
			{
				c = c.substring(0,inContentIndex) + op.characters + c.substring(inContentIndex,c.length);
				doc.content[contentIndex] = c;
				inContentIndex += op.characters.length;
			}
			else
			{
				doc.content.splice( contentIndex + 1, 0, op.characters );
				doc.format.splice( contentIndex + 1, 0, null );
				c = doc.content[++contentIndex];
				inContentIndex = 0;
			}			
		}
		else if ( op.deleteCharacters )
		{
			if ( !c )
				break;		
			var count = op.deleteCharacters.length;
			if ( typeof(c) != "string" )
				throw "Cannot delete characters here, because at this position there are no characters";
			if ( count > c.length - inContentIndex )
				throw "Cannot delete characters, because at this position there are too few characters.";
			if ( c.substr( inContentIndex, count ) != op.deleteCharacters )
				throw "Cannot delete characters, because the characters in the document and operation differ.";
			c = c.substring(0, inContentIndex).concat( c.substring(inContentIndex + count, c.length ) );
			if ( c.length == 0 )
			{
				doc.content.splice( contentIndex, 1 );
				c = doc.content[contentIndex];
				inContentIndex = 0;
			}
			else
				doc.content[contentIndex] = c;
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
				if ( c.elementStart || c.elementEnd )
				{
					count--;
					c = doc.content[++contentIndex];
					inContentIndex = 0;
				}
				else
				{
					var m = Math.min( count, c.length - inContentIndex );
					count -= m;
					inContentIndex += m;
					if ( inContentIndex == c.length )
					{
						c = doc.content[++contentIndex];
						inContentIndex = 0;
						if ( !c )
							break;
					}
				}
			}
		}
		else if ( op.deleteElementStart )
		{
			if ( !c )
				break;
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
			c = doc.content[contentIndex];
			inContentIndex = 0;			
		}
		else if ( op.deleteElementEnd )
		{
			if ( !c )
				break;
			if ( deleteDepth == 0 )
				throw "Cannot delete element end, because matching delete element start is missing";
			if ( !c.elementEnd )
				throw "Cannot delete element end at this position, because in the document there is none";
			doc.content.splice( contentIndex, 1 );
			c = doc.content[contentIndex];
			inContentIndex = 0;
			deleteDepth--;		

			if ( typeof(c) == "string" && contentIndex > 0 && typeof(doc.content[contentIndex-1]) == "string" )
			{
				contentIndex--;				
				inContentIndex = doc.content[contentIndex].length;
				c = doc.content[contentIndex] + c;
				doc.content[contentIndex] = c;		
				doc.content.splice( contentIndex + 1, 1 );
			}
		}
		else if ( op.updateAttributes )
		{		
			if ( !c )
				break;
			if ( !c.elementStart )
				throw "Cannot update attributes at this position, because in the document there is no start element";
			for( var a in op.updateAttributes )
			{
				var update = op.updateAttributes[a];
				// Add a new attribute?
				if ( !update.oldValue )
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
			c.attributes = { }
			for( var a in op.replaceAttributes.newAttributes )
				c.attributes[a] = op.replaceAttributes.newAttributes[a];
			c = doc.content[++contentIndex];
			inContentIndex = 0;							
		}
		else if ( op.annotationBoundary )
		{
			if ( !c )
				break;
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

JSOT.DocOp.AnnotationBoundary = function(ends, updates)
{
	this.annotationBoundary = { ends : ends, updates : updates };
};

JSOT.WaveletOp = function()
{
};

JSOT.WaveletOp.MutateDocument = function(documentId, documentOperation)
{
	this.documentId = documentId;
	this.documentOperation = documentOperation;
};
