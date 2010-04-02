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
	 * The HTML div element that hosts the editor.
	 * @type {HTMLElementDiv}
	 */
	this.dom = dom;
	/**
	 * Number of the current line.
	 */
	this.lineno = -1;
	/**
	 * The HTMLElementDiv of the current line or null at the beginning.
	 * @type {HTMLElementDiv}
	 */
	this.line = null;
	/**
	 * Number of characters inside the line
	 */
	this.charCount;
	/**
	 * A span or text node or null. A value of null means the position at the end of the current line
	 * @type {HTMLElementSpan}
	 */
	this.current = null;
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
	 * @type {bool}
	 */
	this.styleChanged = false;
	/**
	  * Map from annotation keys (especially user/e/xxxx} to a struct {dom, user} where dom is their
	  * HTML represenation of the caret and user is the jid of the corresponding user.
	  */
	this.carets = { };
	/**
	 * Map from user JID to a string of the form user/e/xxxx. Carets for this user&session will be shown. Others not.
	 * Overwrite this property after the constructor. Changing it later yields unspecified results.
	 */
	this.showCarets = { };
}

JSOT.DomIterator.prototype.dispose = function()
{
	delete this.current;
	delete this.formatUpdate;
	delete this.format;
	delete this.line;
	delete this.carets;
};

/**
 * Move the iterator over a linebreak.
 */
JSOT.DomIterator.prototype.skipLineBreak = function()
{
	if ( this.lineno == -1 )
	{
		this.line = this.dom.firstChild;
	}
	else
	{
		this.finalizeLine();
		this.line = this.line.nextSibling;
	}
	if ( !this.line )
		throw "There is no more line to go to";
	this.current = this.line.firstChild;
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
	if ( !this.current )
		throw "Cannot skip characters at the end of a line";

	this.checkForCaret( format );
	
	// If inside a SPAN go down to the text node
	if ( this.current.nodeType == 1 )
	{
		this.current = this.current.firstChild;
		// Detected an empty span?
		if ( !this.current )
			throw "Detected an empty span"
	}
	  
	if ( this.index == 0 && (this.formatUpdate || this.styleChanged ) )
	{
		JSOT.DomIterator.setSpanStyle( this.current.parentNode, format );
		this.styleChanged = false;
	}
		
	var min = Math.min( this.current.data.length - this.index, count );
	this.index += min;
	this.charCount += min;
	count -= min;

	// Need to skip more characters?
	if ( count > 0 )
	{
		this.current = this.current.parentNode.nextSibling;
		this.index = 0;
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
  
	// Beginning of line?
	if ( this.charCount == 0 )
		this.removeBr();
	
	// The document is empty? -> Insert a first line
	if ( !this.line )
	{
		this.line = document.createElement("div");
		this.line.className = "jsot_line";
		this.dom.appendChild(this.line);
		this.current = null;
		this.index = 0;
		this.lineno = 0;
		this.line.lineno = this.lineno;
		this.charCount = 0;
		return;
	}

	var before = null;
	// The cursor is not at the end of the line
	if ( this.current )
	{
		// Inside a text node? Split it
		if ( this.current.nodeType == 3 )
		{
			before = this.splitTextNode( this.current, this.index );
			this.current = this.current.parentNode;
		}
		else
			before = this.current.firstChild;

		// Split the SPAN. If the cursor is on the right end of the span or
		// if the span is empty, then there is no reason to split it
		if ( before )
		{
			before = this.splitNodeBefore( this.current, before );
			this.copySpanStyle( before, this.current );
		}
		else
			before = this.current.nextSibling;
	}
	// Split the line DIV
	before = this.splitNodeBefore( this.line, before );
	this.finalizeLine();
	
	// Move to the new line
	this.line = before;
	this.current = this.line.firstChild;
	this.index = 0;
	this.lineno++;
	this.line.lineno = this.lineno;
	this.charCount = 0;
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

	// Beginning of line?
	if ( this.charCount == 0 )
		this.removeBr();

	this.checkForCaret( format );	

	this.charCount += str.length;

	// Empty line or at the beginning of a SPAN? -> Insert a new span with the proper formatting
	if ( !this.current || this.current.nodeType == 1 )
	{
		if ( this.current && this.current.nodeName != "SPAN" )
			throw "Expected a span";
		// Create a new span
		var span = document.createElement("span");
		JSOT.DomIterator.setSpanStyle( span, format );
		var t = document.createTextNode( str );
		span.appendChild(t);
		this.line.insertBefore( span, this.current );
		this.current = t;
		this.index = str.length;
		return;
	}
			
	// Need to set a format for the new text? This can only happen on the first characters.
	// If inserting in the middle, the new characters gets the format of its left neightbour character
	// which is already correct.
	if ( this.index == 0 )
	{
		this.current = this.current.parentNode;
		this.insertChars( str, format )
		return;
	}
	
	this.current.insertData( this.index, str );
	this.index += str.length;
};

/**
 * Ensures that the cursor is positioned either in an empty line or on a span.
 *
 * @param {object} format is a dictionary mapping style keys to their value.
 * @param {object} update is a dictionary mapping style keys to a KeyValueUpdate object.
 */
JSOT.DomIterator.prototype.setStyle = function( format, update )
{
	this.formatUpdate = update;
	this.format = format;
	this.styleChanged = true;

	
	// Cursor is on a span or empty line -> ok
	if ( !this.current || this.current.nodeType == 1 || !this.line )
		return;

	// Cursor is at the end of a text node. Go to the next span
	if ( this.index == this.current.data.length )
	{
		this.current = this.current.parentNode.nextSibling;
		this.index = 0;
		return;
	}
	
	if ( this.index == 0 )
	{
		this.current = this.current.parentNode;
		this.index = 0;
		return;
	}
	
	// Somewhere inside a text node
	var node = this.current;
	this.current = this.splitTextNode( this.current, this.index );

	// The text node must be inside a span
	if ( this.current.parentNode.nodeName != "SPAN" )
		throw "Expected a SPAN"
	// Split the span
	this.current = this.splitNodeBefore( this.current.parentNode, this.current );
	this.index = 0;
};

JSOT.DomIterator.prototype.deleteLineBreak = function()
{
	if ( this.lineno == -1 )
		throw "Deleting the first line tag is not allowed";
	// Beginning of line?
	if ( this.charCount == 0 )
		this.removeBr();

	var l = this.line.nextSibling;
	if ( !l )
		throw "There is no more line to go to";
	this.removeBr( l );
	// End of current line?
	if ( !this.current )
		this.current = l.firstChild;
	// Copy all children from the deleted line to the current line
	while( l.firstChild )
	{
		var f = l.firstChild;
		l.removeChild(f);
		this.line.appendChild(f);
	};
	// Remove the deleted line
	l.parentNode.removeChild(l);
};

JSOT.DomIterator.prototype.deleteChars = function( count )
{
	if ( this.lineno == -1 )
		throw "Deleting chars before the first line tag is not allowed";
	if ( count == 0 )
		return;
	if ( !this.current )
		throw "Cannot delete characters at the end of a line";

	// Inside a text node
	if ( this.current.nodeType == 3 )
	{
		var data = this.current.data;
		var min = Math.min( data.length - this.index, count );
		count -= min;
		// Delete the entire text node (including its parent span)?
		if ( this.index == 0 && min == data.length )
		{
			var t = this.current;
			// Go to the next span
			this.current = t.parentNode.nextSibling;
			this.index = 0;
			// Remove the span
			t.parentNode.parentNode.removeChild( t.parentNode );
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
				// Go to the next span
				this.current = this.current.parentNode.nextSibling;
				this.deleteChars( count );
			}
		}
	}
	else if ( this.current.nodeType == 1 )
	{
		if ( !this.current.firstChild )
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
	el.className = node.className;
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
		this.line.appendChild( document.createElement("span") );
		this.line.lastChild.appendChild( document.createElement("br") );
	}
};

/**
  * @param l is optional. It is the <div> tag of a line. If not specified, the current line is used.
  */
JSOT.DomIterator.prototype.removeBr = function(l)
{
	if ( !l ) l = this.line;
	if ( l && l.firstChild && l.firstChild.firstChild && l.firstChild.firstChild.nodeName == "BR" )	  
	{
		if ( this.current == l.firstChild )
			this.current = l.firstChild.nextSibling;
		l.removeChild( l.firstChild );
	}
};

/*
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
*/

JSOT.DomIterator.setSpanStyle = function( span, format )
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

/*
JSOT.DomIterator.prototype.compareStyles = function( span1, span2 )
{
	return span1.style.fontWeight == span2.style.fontWeight && span1.style.fontStyle == span2.style.fontStyle && span1.style.fontSize == span2.style.fontSize;
};
*/

/*
 * Positions the cursor behin the last character in the line.
 */
/*
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
		this.current = null;
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
	// Go behind the last characters
	this.index = this.current.data.length;
};
*/

JSOT.DomIterator.prototype.isEndOfLine = function()
{
	// Empty document?
	if ( !this.line )
		return true;
	// End of line? (The trivial case)
	if ( !this.current )
	  return true;
	var node = this.current;

	// Inside text?
	if ( node.nodeType == 3 )
	{	
		// In the middle of a text node?
		if ( this.index < node.data.length )
			return false;
		// Go to the next span
		node = node.parentNode.nextSibling;
	}

	while( node )
	{
		if ( node.className != "jsot_caret" )
		{
			// Does the span have any child text nodes?
			if ( node.firstChild && node.firstChild.nodeType == 3 )
				return false;
			node = node.nextSibling;
		}
	}
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

JSOT.DomIterator.prototype.checkForCaret = function( format )
{
	if ( !format )
		return;
	
	// Display a new cursor?
	for( var key in format )
	{
		if ( this.carets[key] )
			return;
		if ( key.substr(0, 7) == "user/e/" )
		{
			var v = format[key];
			// Caret of the local user?
			if ( v == JSOT.Rpc.jid )
				continue;
			// Show  this caret?
			if ( this.showCarets[v] == key )
			{
				var dom = this.insertCaret( v );
				this.carets[key] = { dom : dom, user : v };
			}
		}
	}
};

/**
 * @param {HTMLElementDiv} the caret to insert or null. In the latter case a HTML caret will be constructed.
 */
JSOT.DomIterator.prototype.insertCaret = function(user, div)
{
	if ( !div )
	{
		var name = user;
		var i = name.indexOf( '@' );
		if ( i != -1 ) name = name.substr(0,i);
	
		div = document.createElement("div");
		div.id = Math.random().toString();
		window.console.log("Added CARET " + div.id);
		div.className = "jsot_caret";
		div.appendChild( document.createTextNode( name ) );	
	}
	
	// End of line?
	if ( !this.current )
	{
		this.removeBr();
		this.line.appendChild( div );
		return div;
	}
	else if ( this.current.nodeType == 1 )
	{
		this.current.parentNode.insertBefore( div, this.current );
		return div;
	}
	
	// Beginning of a text node?
	if ( this.index == 0 )
	{
		this.current = this.current.parentNode;
		return this.insertCaret(user, div);
	}
	// End of a text node?
	if ( this.index == this.current.data.length )
	{
		this.current = this.current.parentNode.nextSibling;
		this.index = 0;
		return this.insertCaret(user, div);
	}
	
	// Somewhere inside a text node
	this.current = this.splitTextNode( this.current, this.index );

	// The text node is inside a span -> Split the span as well
	if ( this.current.parentNode.nodeName != "SPAN" )
		throw "Expected a SPAN";
	this.current = this.splitNodeBefore( this.current.parentNode, this.current );
	this.index = 0;
	return this.insertCaret(user, div);
};

JSOT.DomIterator.prototype.end = function()
{
	// Some carets at the end of the document?
	for( var user in this.showCarets )
	{
		var key = this.showCarets[user];
		if ( !this.carets[key] )
		{
			var dom = this.insertCaret( user );
			this.carets[key] = { dom : dom, user : user };
		}
	}
};