if ( !window.JSOT )
	JSOT = { };

/**
 * This class is used by the editor to iterate over and modify the HTML content in response
 * to user actions or applied deltas.
 *
 * @param {HTMLElement} dom is the HTML element that is contentEditable=true.
 *
 * @constructor
 */
JSOT.DomIterator = function( dom )
{
	/**
	 * Number of the current line.
	 */
	this.lineno = -1;
	/**
	 * A HTMLElementDiv.
	 */
	this.line = null;
	/**
	 * Number of characters inside the line
	 */
	this.charCount;
	/**
	 * A div, span or text node.
	 */
	this.current = dom;
	/**
	 * Position inside a text node. Only relevant when this,current is a text node.
	 */
	this.index = 0;
	/**
	 * List of KeyValueUpdate objects.
	 */
	this.formatUpdate = null;
	/**
	 * A dictionary mapping style keys to their value.
	 */
	this.format = null;
}

JSOT.DomIterator.prototype.dispose = function()
{
	delete this.current;
	delete this.formatUpdate;
	delete this.format;
	delete this.line;
};

/**
 * Move the iterator over a linebreak.
 */
JSOT.DomIterator.prototype.skipLineBreak = function()
{
	if ( this.lineno == -1 )
	{
		this.line = this.current.childNodes[0];
	}
	else
	{
		this.finalizeLine();
		this.line = this.line.nextSibling;
	}
	if ( !this.line )
		throw "There is no more line to go to";
	this.current = this.line; //.firstChild;
	this.index = 0;
	this.charCount = 0;
	this.lineno++;
	this.line.lineno = this.lineno;
	if ( this.formatUpdate )
		this.setStyle( this.format, this.formatUpdate );
};

/**
 * Moves the iterator over a number of characters inside a line.
 */
JSOT.DomIterator.prototype.skipChars = function( count )
{
	if ( this.lineno == -1 )
		throw "Must skip line break first";
	
	// Inside a text node?
	if ( this.current.nodeType == 3 )
	{
		var min = Math.min( this.current.data.length - this.index, count );
		this.index += min;
		this.charCount += min;
		count -= min;
		if ( count > 0 )
		{
			this.index = 0;
			var x = this.current.nextSibling;
			if ( x )
				this.current = x;
			else
			{
				if ( this.current.parentNode.nodeName != "SPAN" )
					throw "Expected a span in the DOM";
				this.current = this.current.parentNode.nextSibling;
			}
			this.skipChars( count );
		}
	}
	// Inside a HTML element?
	else if ( this.current.nodeType == 1 )
	{
		if ( this.current.childNodes.length == 0 )
			throw "Did not expect an empty span";
		// Reformat the span if we are supposed to perform a format update.
		if ( this.formatUpdate && this.current.nodeName == "SPAN" )
			this.updateSpanStyle(this.current, this.formatUpdate);
		this.current = this.current.firstChild;
		this.skipChars( count );
	}
	else
		throw "There are no more characters in the line to skip";
};

/**
 * Inserts characters at the current position. Uses the current format for these characters.
 */
JSOT.DomIterator.prototype.insertChars = function( str, format )
{
	if ( this.lineno == -1 )
		throw "Must skip line break first";

	// The formatting has changed?
	if ( format != this.format )
		this.setStyle( format, this.formatUpdate );
	
	// Insert inside a text node?
	if ( this.current.nodeType == 3 )
	{
		this.current.insertData( this.index, str );
		this.index += str.length;
		this.charCount += str.length;
		return;
	}
	// At the beginning of a line?
	else if ( this.current.nodeName == "DIV" )
	{
		if ( this.current == this.line )
			this.removeBr();
		// Insert a new span at the beginning of the document
		if ( !this.current.firstChild || this.formatUpdate || this.current.firstChild.nodeType != 3 )
		{
			var span = document.createElement("span");
			this.setSpanStyle( span, this.format );
			var t = document.createTextNode( str );
			span.appendChild(t);
			this.current.insertBefore( span, this.current.firstChild );
			this.current = t;
			this.index += str.length;
			this.charCount += str.length;
			return;
		}
		// Prepend to the existing text node
		var t = this.current.firstChild;
		t.insertData( 0, str );
		this.current = t;
		this.index = str.length;
		this.charCount += str.length;
		return;
	}
	// At the beginning of a span
	else
	{
		// The span has a text node? -> Use it
		if ( this.current.firstChild )
		{
			this.current = this.current.firstChild;
			this.index = 0;
			this.insertChars( str );
			return;
		}
		// Insert a new text node in the span
		var t = document.createTextNode( str );
		this.current.appendChild( t );
		this.current = t;
		this.index = str.length;
		this.charCount += str.length;
	}
};

/**
 * Sets the style for all characters which are inserted from here on. The update
 * will be applied to all skipped items.
 *
 * @param {object} format is a dictionary mapping style keys to their value.
 * @param {KeyValueUpdate[]} update is a list of style updates or null if nothing needs updating.
 */
JSOT.DomIterator.prototype.setStyle = function( format, update )
{
	if ( this.lineno == -1 )
		throw "Must skip line break first";
	if ( this.current == this.line )
		this.removeBr();

	this.formatUpdate = update;
	this.format = format;
	
	// Changing the formatting inside a text node?
	if ( this.current.nodeType == 3 )
	{
		// The text node itself is inside a SPAN?
		if ( this.current.parentNode.nodeName == "SPAN" )
		{
			// Defensive programming. Should not happen
			if ( this.index == 0 )
			{
				this.current = this.current.parentNode;
				this.setStyle( format, update );
				return;
			}
			// At the end of this span? Go to its next sibling (if there is one)
			if ( this.index == this.current.data.length && !this.current.nextSibling && this.current.parentNode.nextSibling )
			{
				var next = this.current.parentNode.nextSibling;
				
				if ( format )
				{
					this.current = next;
					this.index = 0;
					this.setStyle( format, update );
				}
				else
				{
					// The next span has the same style? -> Join them
					if ( next.nodeType == 1 && this.compareStyles( this.current.parentNode, next ) )
					{
						while( next.firstChild )
						{
							var f = next.firstChild;
							next.removeChild( f );
							this.current.parentNode.appendChild( f );
						}
						next.parentNode.removeChild(next);
					}
					else
					{
						// This is the end of an annotation update. There is no need to do anything about the next node
						this.current = next;
						this.index = 0;
					}
				}
				return;
			}
			// Split the text node (if required)
			var t = this.splitTextNode( this.current, this.index );
			// Split the span
			var span = this.splitNodeBefore( this.current.parentNode, t );
			this.setSpanStyle( span, format );
			this.current = t || span;
			this.index = 0;
			return;
		}	
		else // Inside the DIV
		{
			// Wrap the text inside a span and repeat
			var span = document.createElement( "span" );
			this.current.parentNode.insertBefore( span, this.current );
			while( span.nextSibling && span.nextSibling.nodeType == 3 )
			{
				var t = span.nextSibling;
				t.parentNode.removeChild( t );
				span.appendChild(t);
			}
			this.current = span.firstChild;
			this.setStyle( format, update );
			return;
		}
	}
	else if ( this.current.nodeType == 1 )
	{
		if ( this.current.nodeName == "SPAN" )
		{
			if ( format )
				this.setSpanStyle( this.current, format );
		}
		else if ( this.current.nodeName == "DIV" )
		{
			if ( format )
			{
				if ( this.current.firstChild )
				{
					this.current = this.current.firstChild;
					this.setStyle( format, update );
				}
				else
				{
					var span = document.createElement( "span" );
					this.setSpanStyle( span, format );
					this.current.appendChild(span);
					this.current = span;
				}
			}
		}
		else
			throw "Busted";
	}
	else
		throw "Busted";
};

JSOT.DomIterator.prototype.deleteLineBreak = function()
{
	if ( this.lineno == -1 )
		throw "Deleting the first line tag is not allowed";
	if ( this.current == this.line )
		this.removeBr();

	var l = this.line.nextSibling;
	if ( !l )
		throw "There is no more line to go to";
	this.removeBr( l );
	while( l.firstChild )
	{
		var f = l.firstChild;
		l.removeChild(f);
		this.line.appendChild(f);
	};
	l.parentNode.removeChild(l);
	if ( this.formatUpdate )
		this.setStyle( this.format, this.formatUpdate );
};

JSOT.DomIterator.prototype.deleteChars = function( count )
{
	if ( this.lineno == -1 )
		throw "Deleting chars before the first line tag is not allowed";
	if ( this.current == this.line )
		this.removeBr();
  
	// Inside a text node
	if ( this.current.nodeType == 3 )
	{
		var data = this.current.data;
		var min = Math.min( data.length - this.index, count );
		count -= min;
		// Delete the entire text node?
		if ( this.index == 0 && min == data.length )
		{
			var t = this.current;
			var p = t.parentNode;
			this.index = 0;
			this.current = t.previousSibling;
			// Delete the text node
			p.removeChild(t);
			if ( !this.current && p.tagName == "SPAN" )
			{
				// There is no text node on the left. Go to the left sibling of the parent span
				this.current = p.previousSibling;
				// Is the SPAN itself empty
				if ( !p.firstChild )
					p.parentNode.removeChild(p);
			}
			if ( !this.current )
				// There is nothing on the left. Go to the beginning of the line
				this.current = this.line;
			// On a SPAN? -> Go to the right end of the span
			if ( this.current.nodeType == 1 && this.current.nodeName == "SPAN" )
				this.current = this.current.lastChild;			
			if ( this.current.nodeType == 3 )
				// Go to the right end of the text node
				this.index = this.current.data.length;
			// Need to delete more?
			if ( count > 0 )
				this.deleteChars( count );
		}
		// Delete only some part of the text node?
		else
		{
			this.current.deleteData( this.index, min );
			if ( count > 0 )
			{
				this.index = 0;
				var t = this.current.nextSibling;
				if ( t )
					this.current = t;
				else
					this.current = this.current.parentNode.nextSibling;
				this.deleteChars( count );
			}
		}
	}
	else if ( this.current.nodeType == 1 )
	{
		if ( this.current.childNodes.length == 0 )
			throw "Did not expect an empty span";
		this.current = this.current.firstChild;
		this.deleteChars( count );
	}
	else
		throw "There are no more characters in the line to delete";
};

/**
 * Splits a text node and inserts a new text node right of it. The split is at character
 * number pos. The new text node is retuned.
 * If pos is zero, the function does nothing and returns node.
 * If pos is at then end of the text node, the function does nothing and returns the nextSibling
 * of node, which can be null.
 *
 * @return a text node or null.
 */
JSOT.DomIterator.prototype.splitTextNode = function(node, pos)
{
	if ( pos == 0 )
		return node;
	var data = node.data;
	if ( data.length == pos )
		return node.nextSibling;
	var t = document.createTextNode( data.substring( pos, data.length ) );
	node.data = data.substr( 0, pos );
	node.parentNode.insertBefore( t, node.nextSibling );
	return t;
};

/**
 * @return a new HTML element of the same tagName as node. All children of node starting with
 *         child number pos is moved to the new HTML element. Finally, the new HTML element
 *         is inserted as the next sibling of node.
 */
JSOT.DomIterator.prototype.splitNodeBefore = function(node, before)
{
	var el = document.createElement( node.nodeName );
	var b = before;
	while( b )
	{
		var n = b.nextSibling;
		node.removeChild( b );
		el.appendChild( b );
		b = n;
	}
	node.parentNode.insertBefore( el, node.nextSibling );
	return el;
};

JSOT.DomIterator.prototype.insertLineBreak = function(element)
{
	// The document is empty?
	if ( !this.line )
	{
		this.line = document.createElement("div");
		this.current.appendChild(this.line);
		this.current = this.line;
		this.index = 0;
		this.lineno = 0;
		this.line.lineno = this.lineno;
		this.charCount = 0;
		return;
	}
	
	var node = this.current;
	var before;
	if ( this.current.nodeType == 3 )
	{
		before = this.splitTextNode( node, this.index );
		node = node.parentNode;
	}
	else
		before = this.current.firstChild;
	if ( node.nodeName == "SPAN" )
	{
		if ( !before )
			before = node.nextSibling;
		else
		{
			before = this.splitNodeBefore( node, before );
			before.class = "split";
			this.copySpanStyle( before, node );
		}
		node = node.parentNode;
	}
	if ( node.nodeName != "DIV" )
		throw "Expected DIV";
	before = this.splitNodeBefore( node, before );
	this.finalizeLine();
	this.line = before;
	this.current = this.line;
	this.index = 0;
	this.lineno++;
	this.line.lineno = this.lineno;
	this.charCount = 0;
	if ( this.line.firstChild && this.line.firstChild.nodeType == 1 && this.line.firstChild.class == "split" )
	{
		this.current = this.line.firstChild;
		this.current.class = null;
	}
	else if ( this.formatUpdate )
		this.setStyle( this.format, this.formatUpdate );
};

JSOT.DomIterator.prototype.finalizeLine = function()
{
	if ( !this.line )
		return;
		
	// Last span is empty?
	if ( this.line.lastChild && this.line.lastChild.nodeType == 1 && this.line.lastChild.childNodes.length == 0 )	
		this.line.removeChild( this.line.lastChild );
	
	// Empty line?
	if ( !this.line.firstChild )
	{
		this.line.appendChild( document.createElement("br") );
	}
};

/**
  * @param l is optional. It is the <div> tag of a line. If not specified, the current line is used.
  */
JSOT.DomIterator.prototype.removeBr = function(l)
{
	if ( !l ) l = this.line;
	if ( l && l.firstChild && l.firstChild.nodeType == 1 && l.firstChild.nodeName == "BR" )
		l.removeChild( l.firstChild );
};

JSOT.DomIterator.prototype.updateSpanStyle = function( span, update )
{
	if ( !update )
		return;
	for( var key in update )
	{
		if ( key == "style/backgroundColor" )
			span.backgroundColor = update[key].new_value;
		if ( key == "style/color" )
			span.color = update[key].new_value;
		if ( key == "style/fontFamily" )
			span.fontFamily = update[key].new_value;
		if ( key == "style/fontSize" )
			span.fontSize = update[key].new_value;
		if ( key == "style/fontStyle" )
			span.fontStyle = update[key].new_value;
		if ( key == "style/fontWeight" )
			span.fontWeight = update[key].new_value;
		if ( key == "style/textDecoration" )
			span.textDecoration = update[key].new_value;
		if ( key == "style/verticalAlign" )
			span.verticalAlign = update[key].new_value;
	}
};

JSOT.DomIterator.prototype.setSpanStyle = function( span, format )
{
	if ( format && format["style/backgroundColor"] )
		span.style.backgroundColor = format["style/backgroundColor"];
	else
		span.style.backgroundColor = null;
	if ( format && format["style/color"] )
		span.style.color = format["style/color"];
	else
		span.style.color = null;
	if ( format && format["style/fontFamily"] )
		span.style.fontFamily = format["style/fontFamily"];
	else
		span.style.fontFamily = null;	
	if ( format && format["style/fontWeight"] )
		span.style.fontWeight = format["style/fontWeight"];
	else
		span.style.fontWeight = null;
	if ( format && format["style/fontStyle"] )
		span.style.fontStyle = format["style/fontStyle"];
	else
		span.style.fontStyle = null;
	if ( format && format["style/fontSize"] )
		span.style.fontSize = format["style/fontSize"];
	else
		span.style.fontSize = null;
	if ( format && format["style/textDecoration"] )
		span.style.textDecoration = format["style/textDecoration"];
	else
		span.style.textDecoration = null;	
	if ( format && format["style/verticalAlign"] )
		span.style.verticalAlign = format["style/verticalAlign"];
	else
		span.style.verticalAlign = null;
};

JSOT.DomIterator.prototype.copySpanStyle = function( dest, source )
{
	dest.style.fontWeight = source.style.fontWeight;
	dest.style.fontSize = source.style.fontSize;
	dest.style.fontStyle = source.style.fontStyle;
};

JSOT.DomIterator.prototype.compareStyles = function( span1, span2 )
{
	return span1.style.fontWeight == span2.style.fontWeight && span1.style.fontStyle == span2.style.fontStyle && span1.style.fontSize == span2.style.fontSize;
};

JSOT.DomIterator.prototype.gotoEndOfLine = function()
{
	// Empty document?
	if ( !this.line )
		return;
	this.index = 0;
	this.current = this.line.lastChild;
	// The line is empty?
	if ( !this.current || (this.current.nodeType == 1 && this.current.nodeName == "BR" ) )
	{
		this.current = this.line;
		return;
	}
	// Currently on a span?
	if( this.current.nodeType == 1 )
	{
		// Span is empty?
		if ( !this.current.lastChild )
			return;
		this.current = this.current.lastChild;
	}
	this.index = this.current.data.length;
};

JSOT.DomIterator.prototype.isEndOfLine = function()
{
	// Empty document?
	if ( !this.line )
		return true;
	var node = this.current;
	do
	{
		if ( node.nodeType == 3 )
		{
			if ( this.index < node.data.length )
				return false;
		}
		else if ( node.firstChild && !(node.firstChild.nodeType == 1 && node.firstChild.nodeName == "BR" ) )
			return false;
		if ( node.nodeName == "DIV" )
			return true;
		if ( node.nextSibling )
			return false;
		node = node.parentNode.nextSibling;
	} while( node );
	
	return true;
};

JSOT.DomIterator.prototype.isEndOfDocument = function()
{
	// Empty document?
	if ( !this.line )
		return true;
	if ( line.nextSibling )
		return false;
	return this.isEndOfLine();
};
