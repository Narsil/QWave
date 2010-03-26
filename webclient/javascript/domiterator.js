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
	/**
	 * @type bool
	 */
	this.styleChanged = false;
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
		this.line = this.current.firstChild;
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
	this.styleChanged = false;
};

/**
 * Moves the iterator over a number of characters inside a line.
 */
JSOT.DomIterator.prototype.skipChars = function( count, format )
{
	if ( count == 0 )
		return;
	if ( this.lineno == -1 )
		throw "Must skip line break first";
		
	// Go down in the tree without skipping a character.
	// This will either lead to a text node or an empty span
	while( this.current.firstChild )
		this.current = this.current.firstChild;
	
	// Detected an empty span? Remove it and put the cursor in front of the next item (text node or span)
	if ( this.current.nodeType == 1 )
	{
		if ( this.current.nodeName == "DIV" )
			throw "Skipping characters in an empty line is not possible";
		var n = this.current;
		// Skip the empty span
		var ok = this.goRightUp();
		if ( !ok )
			throw "Skipping characters in an empty line is not possible";
		n.parentNode.removeChild(n);
		// Try again
		this.skipChars(count, format);
	}

	if ( this.current.nodeType != 3 )
		throw "Expeccted a text node"
	  
	if ( this.index == 0 && (this.formatUpdate || this.styleChanged ) )
	{
		this.setTextNodeStyle( this.current, format );
		this.styleChanged = false;
	}
		
	var min = Math.min( this.current.data.length - this.index, count );
	this.index += min;
	this.charCount += min;
	count -= min;

	// Need to skip more characters?
	if ( count > 0 )
	{
		var ok = this.goRightUp();
		if ( !ok )
			throw "Skipping characters in an empty line is not possible";
		this.skipChars( count, format );
	}
};

/**
 * @param {HTMLElementDiv} element is the line to split, i.e. a DIV element.
 *
 * Inserts a linebreak and puts the cursor in front of the first character in the line.
 * If the line did not contain empty spans before, the two resulting lines will not contain empty spans either.
 */
JSOT.DomIterator.prototype.insertLineBreak = function(element)
{
	this.styleChanged = false;
  
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
		// Split the SPAN. If the cursor is on the right end of the span or
		// if the span is empty, then there is no reason to split it
		if ( before )
		{
			before = this.splitNodeBefore( node, before );
			this.copySpanStyle( before, node );
		}
		else
			before = node.nextSibling;
		node = node.parentNode;
	}
	if ( node.nodeName != "DIV" )
		throw "Expected DIV";
	// Split the DIV
	before = this.splitNodeBefore( node, before );
	this.finalizeLine();
	
	// Move to the new line
	this.line = before;
	this.current = this.line;
	this.index = 0;
	this.lineno++;
	this.line.lineno = this.lineno;
	this.charCount = 0;
	
	// Go down in the tree without skipping a character.
	// This is required to set the cursor of the browser correctly.
	// For the following DomIterator operations this is not required.
	while( this.current.firstChild )
		this.current = this.current.firstChild;
};

/**
 * @internal
 */
JSOT.DomIterator.prototype.setTextNodeStyle = function( node, format )
{
	// Wrap the text node (and all text node siblings) in a span
	if ( node.parentNode.nodeName == "DIV" )
	{
		var span = document.createElement("span");
		node.parentNode.insertBefore( span, node );
		node.parentNode.removeChild(node);
		span.appendChild(node);
	}
	this.setSpanStyle( node.parentNode, format );
};

/**
 * Inserts characters at the current position. Uses the current format for these characters.
 */
JSOT.DomIterator.prototype.insertChars = function( str, format )
{
	this.styleChanged = false;

	if ( str.length == 0 )
		return;
	if ( this.lineno == -1 )
		throw "Must skip line break first";

	this.charCount += str.length;
	
	while ( this.current.nodeType == 1 && this.current.firstChild )
		this.current = this.current.firstChild;
		
	// Empty div?
	if ( this.current.nodeType == 1 && this.current.nodeName == "DIV" )
	{
		var span = document.createElement("span");
		this.current.appendChild( span );
		this.current = span;
	}
	
	// Empty span?
	if ( this.current.nodeType == 1 && this.current.nodeName == "SPAN" )
	{
		var t = document.createTextNode( str );
		this.current.appendChild(t);
		this.setSpanStyle( this.current, format );
		this.current = t;
		this.index = str.length;
		return;
	}

	// Insert inside a text node?
	if ( this.current.nodeType != 3 )
		throw "Expected a text node";
		
	// Need to set a format for the new text? This can only happen on the first characters.
	// If inserting in the middle, the new characters gets the format of its left neightbour character
	// which is already correct.
	if ( this.index == 0 )
	{
		var t = document.createTextNode( str );
		this.current.parentNode.insertBefore( t, this.current.nextSibling );
		this.setTextNodeStyle( t, format )
		this.current = t;
		this.index = str.length;
		return;
	}
	
	this.current.insertData( this.index, str );
	this.index += str.length;
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
	this.styleChanged = true;

	// Cursor is at the end of a text node. Try to go right up to the next text node or the next span
	if ( this.current.nodeType == 3 && this.index == this.current.data.length )
	{
		var ok = this.goRightUp();
	
		// At end of the line? Could not go right up.
		if ( !ok )
		{
			var span = document.createElement("span");
			this.line.appendChild( span );
			this.current = span;
			this.index = 0;
			return;
		}
	}
	
	// Cursor is on a span or at the beginning of a top-level text node -> ok
	if ( this.current.nodeType == 1 || ( this.index == 0 && this.current.parentNode.nodeName == "DIV" ) )
		return;
	
	// Somewhere inside a text node
	var node = this.current;
	this.current = this.splitTextNode( this.current, this.index );

	// The text node is inside a span? -> Split the span as well
	if ( this.current.parentNode.nodeName == "SPAN" )
		this.current = this.splitNodeBefore( this.current.parentNode, this.current );
	
	this.index = 0;
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
};

JSOT.DomIterator.prototype.deleteChars = function( count )
{
	if ( this.lineno == -1 )
		throw "Deleting chars before the first line tag is not allowed";
	if ( count == 0 )
		return;
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
 * @param {HTMLElement} node the node to split.
 * @param {HTMLElement} before is a child of node and tells where to split. A value of null
 *                      means to split after the last child, i.e. split before nothing.
 *
 * @return a new HTML element of the same tagName as node. All children of node starting with
 *         'before' is moved to the new HTML element. Finally, the new HTML element
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

JSOT.DomIterator.prototype.goRightUp = function()
{
	var node = this.current;
	while ( node.nodeType != 1 || node.nodeName != "DIV" )
	{
		if ( node.nextSibling )
		{
			// Go right
			this.current = node.nextSibling;
			this.index = 0;
			return true;
		}
		if ( node.parentNode.nodeName == "DIV" )
			return false;
		// Go up
		node = node.parentNode;
	}
	return false;
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
		// Inside text?
		if ( node.nodeType == 3 )
		{
			if ( this.index < node.data.length )
				return false;
		}
		// The element has a child (which is NOT <br>)
		else if ( node.firstChild && !(node.firstChild.nodeType == 1 && node.firstChild.nodeName == "BR" ) )
			return false;
		// Empty line?
		if ( node.nodeName == "DIV" )
			return true;
		// Sibling -> not end of line
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
