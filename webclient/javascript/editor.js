if ( !window.JSOT )
	JSOT = { };

/**
  * @constructor
  */
JSOT.OTListener = function(editor)
{
	this.editor = editor;
	this.dom = editor.dom;
	this.suspend = false;
	this.cursorAnnoKey = "user/e/" + editor.session;
	this.cursorAnnoValue = editor.jid;
	this.hasSelection = false;
	this.format = null;
};

JSOT.OTListener.prototype.begin = function( doc )
{
	if ( this.suspend )
		return;
	delete this.cursor;
	this.doc = doc;
	this.it = new JSOT.DomIterator( this.dom );
}

JSOT.OTListener.prototype.retainElementStart = function( element, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.format = format;
	if ( element.type == "line" )
		this.it.skipLineBreak();
};

JSOT.OTListener.prototype.retainElementEnd = function( element, format )
{
	if ( this.suspend )
		return;
	this.format = format;
};

JSOT.OTListener.prototype.insertElementStart = function( element, format )
{
	if ( this.suspend )
		return;
	// this.it.setStyle( format, this.it.formatUpdate );
	this.checkForCursor( format );
	this.format = format;
	if ( element.type == "line" )
		this.it.insertLineBreak( element );
};

JSOT.OTListener.prototype.insertElementEnd = function( element, format )
{
	if ( this.suspend )
		return;
	// this.it.setStyle( format, this.it.formatUpdate );
	this.checkForCursor( format );
	this.format = format;
};

JSOT.OTListener.prototype.deleteElementStart = function( element, format )
{
	if ( this.suspend )
		return;
	// this.it.setStyle( format, this.it.formatUpdate );
	this.format = format;
	// this.checkForCursor( format );
	if ( element.type == "line" )
		this.it.deleteLineBreak();
};

JSOT.OTListener.prototype.deleteElementEnd = function(format)
{
	// if ( this.suspend )
 		// return;
	// this.it.format = format;
};

JSOT.OTListener.prototype.insertCharacters = function( chars, format )
{
	if ( this.suspend )
		return;
	// this.it.setStyle( format, this.it.formatUpdate );
	this.checkForCursor( format );
	this.format = format;
	this.it.insertChars( chars, format );
};

JSOT.OTListener.prototype.deleteCharacters = function( chars, format )
{
	if ( this.suspend )
		return;
	this.format = format;
	// this.checkForCursor( format );
	this.it.deleteChars( chars.length );
};

JSOT.OTListener.prototype.retainCharacters = function( count, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.it.skipChars( count, format );
	this.format = format;
};

JSOT.OTListener.prototype.updateAttributes = function( element, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.format = format;
};

JSOT.OTListener.prototype.replaceAttributes = function( element, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.format = format;
};

/**
 * ATTENTION: There is little one can really do in this function. The annotation update
 * applies to the items on the right side of the cursor which are not yet run through.
 * DO NOT attempt to combine the update with the format passed as parametere here, because
 * the format is the one left to the cursor.
 *
 * Typical implementations store the update for later reference and the format parameter can be ignored.
 *
 * @param {object} update is the new annotation update.
 * @param {object} format is the current annotation to the left of the cursor.
 *                 This does NOT include the changed update. It is the format of the
 *                 last item to the left of the boundary.
 */
JSOT.OTListener.prototype.annotationBoundary = function( update, format )
{
	if ( this.suspend )
		return;
	// this.checkForCursor( format );
	// this.format = format;
	this.it.setStyle( this.format, update );
};

JSOT.OTListener.prototype.end = function()
{
	if ( this.suspend )
		return;	
	this.it.finalizeLine();
	
	if ( !this.cursor )
		this.cursor = { line : this.it.line, lineno : this.it.lineno, charCount : this.it.charCount };

	if ( this.hasSelection )
		this.editor.showSelection();
	
	this.it.dispose();
	delete this.it;
};

JSOT.OTListener.prototype.setSuspend = function( suspend )
{
	this.suspend = suspend;
};

JSOT.OTListener.prototype.checkForCursor = function(format)
{
	if ( !this.cursor && format && format[ this.cursorAnnoKey ] == this.cursorAnnoValue )
		this.cursor = { line : this.it.line, lineno : this.it.lineno, charCount : this.it.charCount, format : this.format };
}


/**
  * @constructor
  */
JSOT.Editor = function(doc, dom)
{
	this.dom = dom;
	this.doc = doc;
	this.session = JSOT.Rpc.sessionId;
	this.jid = JSOT.Rpc.jid;
	var self = this;
	dom.onkeypress = function(e) { self.keypress(e); }
	dom.onkeydown = function(e) { self.keydown(e); }
	
	this.listener = new JSOT.OTListener( this );
	this.doc.addListener( this.listener );
}

JSOT.Editor.prototype.keydown = function(e)
{
	window.console.log("KeyDown = " + e.keyIdentifier.toString() + " code=" + e.keyCode.toString());

	if ( e.keyCode != 8 && e.keyCode != 46 )
		return;
	
	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	
	var charCount = selDom.nodeType == 3 ? selOffset : 0;
	var lineno = 0;
	// Find the line, i.e. <div> element
	var line = selDom;
	// Document is empty?
	if ( !line )
		return;
	while( line.nodeType != 1 || line.nodeName != "DIV" )
	{
		var p = line.previousSibling;
		while( p )
		{
			charCount += this.charCount(p);
			p = p.previousSibling;
		}
		line = line.parentNode;
	}
	// Find the line number
	var l = line.previousSibling;
	while( l )
	{
		lineno++;
		l = l.previousSibling;
	}
	
	// window.console.log("Line No " + line.lineno.toString() + " and pos " + charCount.toString() );
	
	// Delete selection?
	if ( !sel.isCollapsed )
	{
		window.console.log("DeleteSelection");
		e.stopPropagation();
		e.preventDefault();

		this.deleteSelection();
		return;
	}
		
	// Backspace?
	if ( e.keyCode == 8 )
	{
		var element = this.getElementByLineNo( lineno );

		// Delete a linebreak?
		if ( charCount == 0 )
		{
			e.stopPropagation();
			e.preventDefault();

			// At the beginning of the document? -> Do nothing
			if ( !line.previousSibling )
				return;

			var iter = new JSOT.DomIterator( this.dom );
			iter.line = line.previousSibling;
			iter.lineno = lineno - 1;
			iter.current = iter.line;
			iter.index = 0;
			iter.gotoEndOfLine();
			iter.deleteLineBreak();
			iter.finalizeLine();
			
			// Position the cursor
			if ( iter.current.nodeType == 1 )
				sel.collapse( iter.current, 0 );
			else
				sel.collapse( iter.current, iter.index );
			
			var pos = element.itemCountBefore();
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( pos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementStart("line") );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementEnd() );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - pos - 2 ) );
			this.submit( ops );
			// ops.applyTo( this.doc );
			this.listener.setSuspend(false);
			return;
		}
		else
		{
			var pos = element.itemCountBefore() + 2 + charCount - 1;
			var ch = this.doc.getCharAt(pos);
			window.console.log("Backspace " + ch);
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( pos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( ch ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - pos - 1 ) );
			this.submit( ops );
			// ops.applyTo( this.doc );
			this.listener.setSuspend(false);
			
			// Chrome inserts some tags when a line becomes empty. Don't want this.
			// Thus, I do the deletion myself and end up with a nice <div><br><div>
			if ( this.charCount( line ) == 1 )
			{
				e.stopPropagation();
				e.preventDefault();
				
				line.innerHTML = "<br>";
				sel.collapse( line, 0 );
			}			
			return;
		}
	}
	// Delete?
	else if ( e.keyCode == 46 )
	{
		var element = this.getElementByLineNo( lineno );
		var pos = element.itemCountBefore() + 2 + charCount;

		var iter = new JSOT.DomIterator( this.dom );
		iter.line = line;
		iter.lineno = lineno;
		iter.current = selDom;
		iter.index = selOffset;
		
		if ( iter.isEndOfLine() )
		{		  
			e.stopPropagation();
			e.preventDefault();

			// End of document? -> Do nothing
			if ( !iter.line.nextSibling )
				return;
			
			iter.deleteLineBreak();
			iter.finalizeLine();
			
			// Position the cursor
			if ( iter.current.nodeType == 1 )
				sel.collapse( iter.current, 0 );
			else
				sel.collapse( iter.current, iter.index );

			window.console.log("Delete line break" + ch);
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( pos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementStart("line") );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementEnd() );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - pos - 2 ) );
			this.submit( ops );
			// ops.applyTo( this.doc );
			this.listener.setSuspend(false);
			return;
		}
		else
		{
			var ch = this.doc.getCharAt(pos);
			window.console.log("Delete " + ch);
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( pos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( ch ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - pos - 1 ) );
			this.submit( ops );
			// ops.applyTo( this.doc );
			this.listener.setSuspend(false);

			// Chrome inserts some tags when a line becomes empty. Don't want this.
			// Thus, I do the deletion myself and end up with a nice <div><br><div>
			if ( this.charCount( line ) == 1 )
			{
				e.stopPropagation();
				e.preventDefault();
				
				line.innerHTML = "<br>";
				sel.collapse( line, 0 );
			}
			return;
		}
	}
	else
		throw "Unsupported keycode";
};

JSOT.Editor.prototype.keypress = function(e)
{
	window.console.log("Key Press = " + e.keyIdentifier);

	// The document is currently empty?
	if ( this.doc.isEmpty() )
	{
		this.dom.innerHTML = "<div><br></div>";
		var sel = window.getSelection();
		sel.collapse( this.dom.firstChild, 0 );
		
		this.listener.setSuspend(true);
		var ops = new protocol.ProtocolDocumentOperation();
		ops.component.push( protocol.ProtocolDocumentOperation.newElementStart("line") );
		ops.component.push( protocol.ProtocolDocumentOperation.newElementEnd() );
		// ops.applyTo( this.doc );
		this.submit( ops );
		this.listener.setSuspend(false);
	}

	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	
	var charCount = selDom.nodeType == 3 ? selOffset : 0;
	var lineno = 0;
	// Find the line, i.e. <div> element
	var line = selDom;
	while( line.nodeType != 1 || line.nodeName != "DIV" )
	{
		var p = line.previousSibling;
		while( p )
		{
			charCount += this.charCount(p);
			p = p.previousSibling;
		}
		line = line.parentNode;
	}
	// Find the line number
	var l = line.previousSibling;
	while( l )
	{
		lineno++;
		l = l.previousSibling;
	}
	
	// window.console.log("Line No " + line.lineno.toString() + " and pos " + charCount.toString() );
	
	if ( e.keyIdentifier == "Enter" || e.keyCode == 13)
	{
		window.console.log("Return");
		e.stopPropagation();
		e.preventDefault();

		var iter = new JSOT.DomIterator( this.dom );
		iter.line = line;
		iter.lineno = lineno;
		iter.current = selDom;
		iter.index = selOffset;
		
		// Insert a line break
		iter.insertLineBreak();
		iter.finalizeLine();
		
		// Position the cursor
		if ( iter.current.nodeType == 1 )
			sel.collapse( iter.current, 0 );
		else
			sel.collapse( iter.current, iter.index );
			
		var element = this.getElementByLineNo( lineno );
		var pos = element.itemCountBefore() + 2 + charCount;
		var count = this.doc.itemCount();
		
		this.listener.setSuspend(true);
		var ops = new protocol.ProtocolDocumentOperation();
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( pos ) );
		ops.component.push( protocol.ProtocolDocumentOperation.newElementStart("line") );
		ops.component.push( protocol.ProtocolDocumentOperation.newElementEnd() );
		if ( count - pos > 0 )
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( count - pos ) );
		this.submit( ops );
		// ops.applyTo( this.doc );
		this.listener.setSuspend(false);
		return;
	}
	
	this.listener.setSuspend(true);
	var element = this.getElementByLineNo( lineno );
	var pos = element.itemCountBefore() + 2 + charCount;
	var count = this.doc.itemCount();
	window.console.log("Before = " + element.itemCountBefore().toString() + " charCount=" + charCount.toString() );
	var ops = new protocol.ProtocolDocumentOperation();
	ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( pos ) );
	ops.component.push( protocol.ProtocolDocumentOperation.newCharacters( String.fromCharCode(e.keyCode) ) );
	if ( count - pos > 0 )
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( count - pos ) );
	this.submit( ops );
	// ops.applyTo( this.doc );
	this.listener.setSuspend(false);

};

JSOT.Editor.prototype.getElementByLineNo = function( lineno )
{
	var l = 0;
	for( var i = 0; i < this.doc.content.length; ++i )
	{
		var c = this.doc.content[i];
		if ( typeof(c) == "string" )
			continue;
		if ( c.element_start && c.type == "line" )
		{
			if ( l == lineno ) return c;
			l++;
		}
	}
	return null;
};

/**
 * @return the number of characters in a HTML node.
 */
JSOT.Editor.prototype.charCount = function( node )
{
	if ( node.nodeType == 3 )
		return node.data.length;
	var result = 0;
	var c = node.firstChild;
	while( c )
	{
		result += this.charCount(c);
		c = c.nextSibling;
	}
	return result;
};

JSOT.Editor.prototype.isEmptyElement = function(node)
{
	if ( node.nodeType == 3 )
		return node.data.length == 0;
	for( var i = 0; i < node.childNodes.length; ++i )
		if ( !this.isEmptyElement( node.childNodes[i] ) )
			return false;
	return true;
};

/**
 * Changes the style of the current selection.
 *
 * @param {string} styleKey defines the style, e.g. "style/fontWeight"
 * @param {string} styleValue defines the value for the style, e.g. "bold".
 */
JSOT.Editor.prototype.setStyle = function(styleKey, styleValue)
{
	this.markSelection();
	
	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	
	if ( sel.isCollapsed )
		return;

	var pos1 = this.getLinePosition( sel.anchorNode, sel.anchorOffset );
	var pos2 = this.getLinePosition( sel.focusNode, sel.focusOffset );
	
	if ( pos2.lineno < pos1.lineno || ( pos2.lineno == pos1.lineno && pos2.charCount < pos1.charCount ) )
	{
		var tmp = pos2;
		pos2 = pos1;
		pos1 = tmp;
	}
	
	var docpos1 = this.getDocPosition( pos1.lineno, pos1.charCount );
	var docpos2 = this.getDocPosition( pos2.lineno, pos2.charCount );

	var style = false;

	var ops = new protocol.ProtocolDocumentOperation();
	ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos1 ) );
	for( var i = docpos1; i < docpos2; ++i )
	{
		var format = this.doc.getFormatAt( i );
		var s = ( format && format[ styleKey ] == styleValue );
		if ( (s && style) || (!s && !style) )
		{
			var ends = [];
			var begins = [];
			if ( s && style )
			{
				ends.push( styleKey );
				style = false;
			}
			else if ( !s && !style )
			{
				begins.push( protocol.ProtocolDocumentOperation.newKeyValueUpdate( styleKey, format ? format[ styleKey ] : null , styleValue ) );
				style = true;
			}
			ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( ends, begins ) );
		}
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( 1 ) );
	}
	if ( style )
		ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ styleKey ], [  ] ) );
	if ( this.doc.itemCount() - docpos2 > 0 )
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - docpos2 ) );
	this.submit( ops );
	
	this.showSelection();
};

JSOT.Editor.prototype.deleteSelection = function()
{
	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	
	if ( sel.isCollapsed )
		return;

	this.markSelection();
	
	var pos1 = this.getLinePosition( sel.anchorNode, sel.anchorOffset );
	var pos2 = this.getLinePosition( sel.focusNode, sel.focusOffset );
	
	if ( pos2.lineno < pos1.lineno || ( pos2.lineno == pos1.lineno && pos2.charCount < pos1.charCount ) )
	{
		var tmp = pos2;
		pos2 = pos1;
		pos1 = tmp;
	}
	
	var element1 = this.getElementByLineNo( pos1.lineno );
	var docpos1 = element1.itemCountBefore() + 2 + pos1.charCount;
	var element2 = this.getElementByLineNo( pos2.lineno );
	var docpos2 = element2.itemCountBefore() + 2 + pos2.charCount;

	var ops = new protocol.ProtocolDocumentOperation();
	ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos1 ) );
	for( var i = docpos1; i < docpos2; ++i )
	{
		var c = this.doc.getItemAt(i);
		if ( typeof(c) == "string" )
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( c ) );
		else if ( c.element_start )
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementStartFromElement( c ) );
		else
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementEnd() );
	}
	ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - docpos2 ) );
	this.submit( ops );
	
	this.showSelection();
};

/**
 * Determines the line number and character position inside the line
 * from a given HTML node and offset. This helps in mapping HTML cursor positions
 * to a position in JSOT.Doc.
 */
JSOT.Editor.prototype.getLinePosition = function(selDom, selOffset)
{
	var charCount = selDom.nodeType == 3 ? selOffset : 0;
	var lineno = 0;
	// Count the number of characters
	var line = selDom;
	while( line.nodeType != 1 || line.nodeName != "DIV" )
	{
		var p = line.previousSibling;
		while( p )
		{
			charCount += this.charCount(p);
			p = p.previousSibling;
		}
		line = line.parentNode;
	}
	// Find the line number
	var l = line.previousSibling;
	while( l )
	{
		lineno++;
		l = l.previousSibling;
	}
	
	return { line : line, lineno : lineno, charCount : charCount };
};

JSOT.Editor.prototype.getDocPosition = function( lineno, charCount )
{
	var element = this.getElementByLineNo( lineno );
	return element.itemCountBefore() + 2 + charCount;
};

JSOT.Editor.prototype.getDomPosition = function( line, lineno, charCount )
{
	var it = new JSOT.DomIterator(this.dom);
	it.line = line;
	it.current = line;
	it.lineno = lineno;
	it.skipChars( charCount );
	return { node : it.current, offset : it.index };
};

JSOT.Editor.prototype.markSelection = function()
{
	this.listener.setSuspend( true );
	
	// Delete the old selection
	if ( this.listener.cursor )
	{
		var cursorpos = this.getDocPosition( this.listener.cursor.lineno, this.listener.cursor.charCount );
		var itemcount = this.doc.itemCount();
		// The cursor is currently at the end of the document -> it has no annotationBoundary -> nothing to do. Otherwise remove the annoation of the cursor
		if ( cursorpos < itemcount )
		{
		  var ops = new protocol.ProtocolDocumentOperation();
		  ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( cursorpos ) );
		  ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ ], 
				[ protocol.ProtocolDocumentOperation.newKeyValueUpdate( this.listener.cursorAnnoKey, this.listener.cursorAnnoValue, null ) ] ) );
		  ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( itemcount - cursorpos ) );
		  // ops.applyTo( this.doc );
		  this.submit( ops );
		}
	}
	
	var sel = window.getSelection();
	var selDom = sel.focusNode;
	var selOffset = sel.focusOffset;
	
	// Is the current selection in this editor?
	var found = false;
	var x = selDom;
	while( x )
	{
		if ( x == this.dom )
		{
			found = true;
			break;
		}
		x = x.parentNode;
	}
	
	if ( found )
	{
		var pos = this.getLinePosition( selDom, selOffset );
		var cursorpos = this.getDocPosition( pos.lineno, pos.charCount );
		var ops = new protocol.ProtocolDocumentOperation();
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( cursorpos ) );
	  	ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ ],
				[ protocol.ProtocolDocumentOperation.newKeyValueUpdate( this.listener.cursorAnnoKey, null, this.listener.cursorAnnoValue ) ] ) );
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - cursorpos ) );
		// ops.applyTo( this.doc );
 		this.submit( ops );
	}
	
	this.listener.setSuspend( false );
};

JSOT.Editor.prototype.showSelection = function()
{
	var sel = window.getSelection();
	var cursorPos = this.getDomPosition( this.listener.cursor.line, this.listener.cursor.lineno, this.listener.cursor.charCount );
	sel.collapse( cursorPos.node, cursorPos.offset );
};

JSOT.Editor.prototype.submit = function(docOp)
{
	var wavelet = this.doc.wavelet;
	
	var m = new protocol.ProtocolWaveletOperation_MutateDocument();
    m.document_id = this.doc.docId;
    m.document_operation = docOp;
	
	// Apply locally and send to the server
	wavelet.submitMutations( [m] );
};
