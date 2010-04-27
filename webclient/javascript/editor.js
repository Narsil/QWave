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
	this.cursorAnnoKey = "user/e/" + JSOT.Rpc.sessionId;
	this.cursorAnnoValue = JSOT.Rpc.jid;
	this.hasSelection = false;
	this.format = null;
	this.carets = { };
};

/**
 * Tells where the cursor should be according to document and its annotations.
 * Therefore, this function inspects the annotations and looks for keys of the format user/e/xxxx".
 */
JSOT.OTListener.prototype.getDocCursor = function()
{
	if ( this.it )
		return this.it;
	
	var lineno = -1;
	var charCount = 0;
	
	for( var i = 0; i < this.editor.doc.content.length; ++i )
	{
		var f = this.editor.doc.format[i];
		// Found the first item that has the cursor annotation
		if ( f && f[this.cursorAnnoKey] == this.cursorAnnoValue )
			return { lineno : lineno, charCount : charCount, isEndOfDocument : false };
		
		var c = this.editor.doc.content[i];
		if ( typeof(c) == "string" )
		{
			charCount += c.length;
		}
		else if ( c.element_start )
		{
			if ( c.type == "line" )
				lineno++;
			charCount = 0;
		}
	}
	
	// End of document
	return { lineno : lineno, charCount : charCount, isEndOfDocument : true };
};

JSOT.OTListener.prototype.begin = function( doc )
{
	delete this.cursor;
	this.doc = doc;
	if ( this.suspend )
		return;
	
	// Delete all carets
	for( var key in this.carets )
	{
		var d = this.carets[key].dom;
		window.console.log("Remove CARET " + d.id );
		d.parentNode.removeChild( d );
	}
		
	this.it = new JSOT.DomIterator( this.dom );
}

JSOT.OTListener.prototype.selectCarets_ = function()
{
	// Tell the iterator which carets we want to see
	var showCarets = { };
	if ( !this.doc.isEmpty() )
	{
		var current = new Date().getTime();
		var format = this.doc.getFormatAt(0);
		for( var a in format )
		{
			if ( a.substr(0,7) != "user/d/" )
			  continue;
			var val = format[a].split(",");
			var user = val[0];
			if ( user == JSOT.Rpc.jid )
				continue;
			try {
				var time = parseInt(val[1]);
			} catch( e ) { continue; }
			if ( current - time > 30 * 60 * 10000 )
				continue;
			var otherTime = showCarets[ user ];
			if ( otherTime && otherTime > time )
				continue;
			showCarets[ user ] = "user/e/" + a.substring(7, a.length );
		}
	}
	this.it.showCarets = showCarets;
};

JSOT.OTListener.prototype.retainElementStart = function( element, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.format = format;
	if ( element.type == "line" )
	{
		if ( this.it.lineno == -1 )
			this.selectCarets_();
		this.it.skipLineBreak();
	}
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
	this.checkForCursor( format );
	this.format = format;
	if ( element.type == "line" )
	{
		if ( this.it.lineno == -1 )
			this.selectCarets_();
		this.it.insertLineBreak( element );
	}
};

JSOT.OTListener.prototype.insertElementEnd = function( element, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.format = format;
};

JSOT.OTListener.prototype.deleteElementStart = function( element, format )
{
	if ( this.suspend )
		return;
	this.format = format;
	if ( element.type == "line" )
		this.it.deleteLineBreak();
};

JSOT.OTListener.prototype.deleteElementEnd = function(format)
{
};

JSOT.OTListener.prototype.insertCharacters = function( chars, format )
{
	if ( this.suspend )
		return;
	this.checkForCursor( format );
	this.format = format;
	this.it.insertChars( chars, format );
};

JSOT.OTListener.prototype.deleteCharacters = function( chars, format )
{
	if ( this.suspend )
		return;
	this.format = format;
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
	this.it.setStyle( this.format, update );
};

JSOT.OTListener.prototype.end = function()
{
	if ( this.suspend )
		return;
	
  	if ( !this.cursor )
		this.cursor = { line : this.it.line, lineno : this.it.lineno, charCount : this.it.charCount };
	this.carets = this.it.carets;
	
	this.it.finalizeLine();
	this.it.end();
	
	if ( this.hasSelection )
	{
		this.editor.cursor = this.cursor;
		this.editor.showCursor();
	}
	
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
		this.cursor = { line : this.it.line, lineno : this.it.lineno, charCount : this.it.charCount };
}


/**
  * @constructor
  */
JSOT.Editor = function(doc, dom)
{
	/**
	 * The HTML div element that is being edited.
	 */
	this.dom = dom;
	/**
	 * The wavelet document that is being edited.
	 */
	this.doc = doc;
	/**
	 * The current cursor position expressed as {line, lineno, charCount} or null if no cursor is set.
	 */
	this.cursor = null;
	this.cursorFormat = null;
	/**
	 * Callback function that is being invoked when the cursor changes or the format at
	 * the cursor changes.
	 */
	this.onCursorChange = null;
	
	var self = this;
	dom.onkeypress = function(e) { self.keypress(e); }
	dom.onkeydown = function(e) { self.keydown(e); }
	dom.onkeyup = function(e) { self.keyup(e); }
	
	this.listener = new JSOT.OTListener( this );
	this.doc.addListener( this.listener );
}

/**
 * Handle non-printable keys, i.e. backspace or delete.
 */
JSOT.Editor.prototype.keydown = function(e)
{
	window.console.log("KeyDown = " + e.keyIdentifier.toString() + " code=" + e.keyCode.toString());

	// Only backspace and delete are handled here. Everything else passes.
	if ( e.keyCode != 8 && e.keyCode != 46 )
		return;
	
	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	var pos = this.getLinePosition( selDom, selOffset );
  
	// Delete selection?
	if ( !sel.isCollapsed )
	{
		window.console.log("DeleteSelection");
		e.stopPropagation();
		e.preventDefault();

		this.deleteSelection();
		return;
	}

	// The cursor has moved by means of the cursor keys, mouse click, home key, ... ?
	this.updateCursor( pos );

	// Backspace?
	if ( e.keyCode == 8 )
	{
		var element = this.getElementByLineNo( pos.lineno );

		// Delete a linebreak?
		if ( pos.charCount == 0 )
		{
			e.stopPropagation();
			e.preventDefault();

			// At the beginning of the document? -> Do nothing
			if ( !pos.line.previousSibling )
				return;

			// Put the iterator at the end of the previous line
			var iter = new JSOT.DomIterator( this.dom );
			iter.line = pos.line.previousSibling;
			iter.lineno = pos.lineno - 1;
			iter.current = null;
			iter.index = 0;
			iter.deleteLineBreak();
			iter.finalizeLine();
			
			// Position the cursor
			if ( !iter.current )
			{
				if ( iter.line.lastChild.lastChild.nodeType == 3 )
					sel.collapse( iter.line.lastChild, iter.line.lastChild.lastChild.data.length );
				else
					sel.collapse( iter.line.lastChild, 0 );
			}
			else if ( iter.current.nodeType == 1 )
				sel.collapse( iter.current, 0 );
			else
				sel.collapse( iter.current, iter.index );
			this.cursor = { line : iter.line, lineno : iter.lineno, charCount : iter.charCount };
			var docpos = element.itemCountBefore();
			this.cursorFormat = this.doc.getFormatAt(docpos + 1);
			if ( this.onCursorChange )
				this.onCursorChange();
			
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			this.newUserAnnotation(ops);
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementStart("line") );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementEnd() );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - docpos - 2 ) );
			this.submit( ops );
			// ops.applyTo( this.doc );
			this.listener.setSuspend(false);
			return;
		}
		else
		{
			this.cursor.charCount--;
			
			var docpos = element.itemCountBefore() + 2 + pos.charCount - 1;
			var ch = this.doc.getCharAt(docpos);
			this.cursorFormat = this.doc.getFormatAt(docpos);
			if ( this.onCursorChange )
				this.onCursorChange();
			
			// window.console.log("Backspace " + ch);
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			this.newUserAnnotation(ops);
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteCharacters( ch ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - docpos - 1 ) );
			this.submit( ops );
			// ops.applyTo( this.doc );
			this.listener.setSuspend(false);
			
			// Chrome inserts some tags when a line becomes empty. Don't want this.
			// Thus, I do the deletion myself and end up with a nice <div><br><div>
			if ( this.charCount( pos.line ) == 1 )
			{
				e.stopPropagation();
				e.preventDefault();
				
				line.innerHTML = "<span><br></span>";
				sel.collapse( pos.line, 0 );
			}
			return;
		}
	}
	// Delete?
	else if ( e.keyCode == 46 )
	{
		var element = this.getElementByLineNo( pos.lineno );
		var docpos = element.itemCountBefore() + 2 + pos.charCount;

		var iter = new JSOT.DomIterator( this.dom );
		iter.line = pos.line;
		iter.lineno = pos.lineno;
		iter.current = selDom;
		iter.index = selOffset;
		
		// Delete a line break?
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
			this.newUserAnnotation(ops);
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos ) );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementStart("line") );
			ops.component.push( protocol.ProtocolDocumentOperation.newDeleteElementEnd() );
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( this.doc.itemCount() - docpos - 2 ) );
			this.submit( ops );
			this.listener.setSuspend(false);
			return;
		}
		else
		{
			var ch = this.doc.getCharAt(pos);
			window.console.log("Delete " + ch);
			this.listener.setSuspend(true);
			var ops = new protocol.ProtocolDocumentOperation();
			this.newUserAnnotation(ops);
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

/**
 * Handle printable keys, i.e. letters, numbers, enter/return
 */
JSOT.Editor.prototype.keypress = function(e)
{
	window.console.log("Key Press = " + e.keyIdentifier);

	// The document is currently empty?
	if ( this.doc.isEmpty() )
	{
		this.dom.innerHTML = '<div class="jsot_line"><span><br></span></div>';
		var sel = window.getSelection();
		sel.collapse( this.dom.firstChild.firstChild, 0 );

		this.cursor = { line: this.dom.firstChild, lineno : 0, charCount : 1 };

		this.listener.setSuspend(true);
		var ops = new protocol.ProtocolDocumentOperation();
		this.newUserAnnotation(ops);
		ops.component.push( protocol.ProtocolDocumentOperation.newElementStart("line") );
		ops.component.push( protocol.ProtocolDocumentOperation.newElementEnd() );
		ops.component.push( protocol.ProtocolDocumentOperation.newCharacters( String.fromCharCode(e.keyCode) ) );
		this.submit( ops );
		this.listener.setSuspend(false);
		return;
	}

	var sel = window.getSelection();
	
	// Delete selection?
	if ( !sel.isCollapsed )
	{
		this.deleteSelection();
	}

	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	var pos = this.getLinePosition( selDom, selOffset );

	// The cursor has moved by means of the cursor keys, mouse click, home key, ... ?
	this.updateCursor( pos );

	if ( e.keyIdentifier == "Enter" || e.keyCode == 13)
	{
		window.console.log("Return");
		e.stopPropagation();
		e.preventDefault();

		var iter = new JSOT.DomIterator( this.dom );
		iter.line = pos.line;
		iter.lineno = pos.lineno;
		iter.current = selDom;
		iter.index = selOffset;
		
		// Insert a line break
		iter.insertLineBreak();
		iter.finalizeLine();
		
		// Position the cursor
		if ( !iter.current )
			sel.collapse( iter.line.lastChild, 1 );
		else if ( iter.current.nodeType == 1 )
			sel.collapse( iter.current, 0 );
		else
			sel.collapse( iter.current, iter.index );
		this.cursor = { line : iter.line, lineno : iter.lineno, charCount : iter.charCount };
		
		var element = this.getElementByLineNo( pos.lineno );
		var docpos = element.itemCountBefore() + 2 + pos.charCount;
		var count = this.doc.itemCount();

		this.cursorFormat = this.doc.getFormatAt(docpos - 1);
		if ( this.onCursorChange )
			this.onCursorChange();

		this.listener.setSuspend(true);
		var ops = new protocol.ProtocolDocumentOperation();
		this.newUserAnnotation(ops);
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos ) );
		ops.component.push( protocol.ProtocolDocumentOperation.newElementStart("line") );
		ops.component.push( protocol.ProtocolDocumentOperation.newElementEnd() );
		if ( count - docpos > 0 )
			ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( count - docpos ) );
		this.submit( ops );
		this.listener.setSuspend(false);
		return;
	}

	var element = this.getElementByLineNo( pos.lineno );
	var docpos = element.itemCountBefore() + 2 + pos.charCount;
	var count = this.doc.itemCount();
	
	// First character? Create a span and put the cursor inside.
	if ( pos.charCount == 0 )
	{
		e.stopPropagation();
		e.preventDefault();
		
		var span = document.createElement("span");
		var format = this.doc.getFormatAt( docpos - 1 );
		JSOT.DomIterator.setSpanStyle( span, format );
		var t = document.createTextNode( String.fromCharCode(e.keyCode) );
		span.appendChild(t);
		pos.line.insertBefore(span, pos.line.firstChild);
		sel.collapse( t, 1 );
	}
	
	this.cursor.charCount++;
	if ( this.onCursorChange )
		this.onCursorChange();
	
	this.listener.setSuspend(true);
	// window.console.log("Before = " + element.itemCountBefore().toString() + " charCount=" + charCount.toString() );
	var ops = new protocol.ProtocolDocumentOperation();
	this.newUserAnnotation(ops);
	ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( docpos ) );
	ops.component.push( protocol.ProtocolDocumentOperation.newCharacters( String.fromCharCode(e.keyCode) ) );
	if ( count - docpos > 0 )
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( count - docpos ) );
	this.submit( ops );
	this.listener.setSuspend(false);
};

JSOT.Editor.prototype.keyup = function(e)
{
	// window.console.log("KeyUp = " + e.keyIdentifier.toString() + " code=" + e.keyCode.toString());
	
	// Cursor keys are handled here. Everything else passes.
	if ( e.keyCode < 33 || e.keyCode > 40 )
		return;
	
	var sel = window.getSelection();
	var selDom = sel.focusNode;
	var selOffset = sel.focusOffset;
	
	// Is the cursor inside a caret?
	var caret;
	if ( selDom.nodeType == 1 && selDom.className == "jsot_caret" )
		caret = selDom;
	else if ( selDom.nodeType == 3 && selDom.parentNode.className == "jsot_caret" )
		caret = selDom.parentNode;
	
	if ( caret )
	{
		// Left, Home, PageUp, Up
		if ( e.keyCode == 37 || e.keyCode == 36 || e.keyCode == 33 || e.keyCode == 38 )
		{
			// Skip all carets to the left
			while ( caret.previousSibling && caret.previousSibling.className == "jsot_caret" )
				caret = caret.previousSibling;
			// Position the cursor left of it.
			if ( caret.previousSibling )
			{
				// Left of the caret there is a span with a text node
				selDom = caret.previousSibling.lastChild;
				selOffset = caret.previousSibling.lastChild.data.length;
			}
			else
			{
				selDom = caret;
				selOffset = 0;
			}
		}
		else
		{
			// Skip all carets to the right
			while ( caret.nextSibling && caret.nextSibling.className == "jsot_caret" )
				caret = caret.nextSibling;
			// Position the cursor left of it.
			if ( caret.nextSibling )
			{
				// Right of the caret there is a span with a text node
				selDom = caret.nextSibling.firstChild;
				selOffset = 1;
			}
			else
			{
			  // TODO: Move the cursor to the left of the caret instead!
				selDom = caret;
				selOffset = 0;
			}
		}

		sel.collapse( selDom, selOffset );
	}	
};

/**
 * @return {JOST.Doc.ElementStart} of type 'line' for the given line number or null.
 */
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
 *
 * This helper function is required to map the cursor position from inside the dom
 * to a line/charCount representation.
 */
JSOT.Editor.prototype.charCount = function( node )
{
	if ( node.nodeType == 3 )
		return node.data.length;
	if ( node.className == "jsot_caret" )
		return 0;
	var result = 0;
	var c = node.firstChild;
	while( c )
	{
		result += this.charCount(c);
		c = c.nextSibling;
	}
	return result;
};

/*
JSOT.Editor.prototype.isEmptyElement = function(node)
{
	if ( node.nodeType == 3 )
		return node.data.length == 0;
	for( var i = 0; i < node.childNodes.length; ++i )
		if ( !this.isEmptyElement( node.childNodes[i] ) )
			return false;
	return true;
};
*/

/**
 * Changes the style of the current selection.
 *
 * @param {string} styleKey defines the style, e.g. "style/fontWeight"
 * @param {string} styleValue defines the value for the style, e.g. "bold".
 */
JSOT.Editor.prototype.setStyle = function(styleKey, styleValue)
{
	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	
	if ( sel.isCollapsed )
		return;

	var pos1 = this.getLinePosition( sel.anchorNode, sel.anchorOffset );
	var pos2 = this.getLinePosition( sel.focusNode, sel.focusOffset );

	// The cursor has moved by means of the cursor keys, mouse click, home key, ... ?
	this.updateCursor( pos2 );

	var switched = false;
	if ( pos2.lineno < pos1.lineno || ( pos2.lineno == pos1.lineno && pos2.charCount < pos1.charCount ) )
	{
		var tmp = pos2;
		pos2 = pos1;
		pos1 = tmp;
		switched = true;
	}
	
	var docpos1 = this.getDocPosition( pos1.lineno, pos1.charCount );
	var docpos2 = this.getDocPosition( pos2.lineno, pos2.charCount );
	
	var style = false;

	var ops = new protocol.ProtocolDocumentOperation();
	this.newUserAnnotation(ops);
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

	this.cursorFormat = this.doc.getFormatAt( switched ? docpos1 - 1 : docpos2 - 1 );
	if ( this.onCursorChange )
		this.onCursorChange();

	this.showCursor();
};

JSOT.Editor.prototype.deleteSelection = function()
{
	var sel = window.getSelection();
	var selDom = sel.anchorNode;
	var selOffset = sel.anchorOffset;
	
	// Paranoia
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

	// The cursor has moved by means of the cursor keys, mouse click, home key, ... ?
	this.updateCursor( pos1 );

	var element1 = this.getElementByLineNo( pos1.lineno );
	var docpos1 = element1.itemCountBefore() + 2 + pos1.charCount;
	var element2 = this.getElementByLineNo( pos2.lineno );
	var docpos2 = element2.itemCountBefore() + 2 + pos2.charCount;

	var ops = new protocol.ProtocolDocumentOperation();
	this.newUserAnnotation(ops);
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
	
	this.showCursor();
};

/**
 * Internal helper function.
 *
 * Determines the line number and character position inside the line
 * from a given HTML node and offset. This helps in mapping HTML cursor positions
 * to a position in JSOT.Doc.
 *
 * The counterpart is getDomPosition
 */
JSOT.Editor.prototype.getLinePosition = function(selDom, selOffset)
{
	var charCount = 0;
	// In a text node?
	if ( selDom.nodeType == 3 )
		charCount = selOffset;
	// At the end of a HTML node?
	else if ( selOffset == 1 )
		charCount = this.charCount(selDom);
	// Count the number of characters in front of selDom and find the line DIV
	var line = selDom;
	while( line.nodeType == 3 || line.className != "jsot_line" )
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
	var lineno = 0;
	var l = line.previousSibling;
	while( l )
	{
		lineno++;
		l = l.previousSibling;
	}
	
	return { line : line, lineno : lineno, charCount : charCount };
};

/**
 * Internal helper function.
 *
 * Maps from a line number and a character offset inside the line to a
 * position in the wavelet document.
 */
JSOT.Editor.prototype.getDocPosition = function( lineno, charCount )
{
	var element = this.getElementByLineNo( lineno );
	return element.itemCountBefore() + 2 + charCount;
};

/**
 * Internal helper function.
 *
 * Determines the HTML element and offset from a given line, line number and character count inside the line.
 *
 * The counterpart is getLinePosition
 */
JSOT.Editor.prototype.getDomPosition = function( line, lineno, charCount )
{
	var it = new JSOT.DomIterator(this.dom);
	it.line = line;
	it.current = line.firstChild;
	it.lineno = lineno;
	// TODO: This could change the DOM.
	it.skipChars( charCount );
	return { node : it.current, offset : it.index };
};

/**
 * Internal helper function.
 *
 * Updates the cursor annotation when the cursor changes. The new cursor (given as line, lineno, charCount)
 * is passed as an argument. The old cursor is stored in 'this.cursor'.
 * If the new and old cursor are equal, the function does nothing.
 */
JSOT.Editor.prototype.updateCursor = function( newCursor )
{
	if ( this.cursor && this.cursor.lineno == newCursor.lineno && this.cursor.charCount == newCursor.charCount )
		return;

	var count = this.doc.itemCount();

	if ( !this.cursor )
		var oldCursorPos = count;
	else
		var oldCursorPos = this.getDocPosition( this.cursor.lineno, this.cursor.charCount );
	var newCursorPos = this.getDocPosition( newCursor.lineno, newCursor.charCount );
	this.cursor = { line : newCursor.line, lineno : newCursor.lineno, charCount : newCursor.charCount };

	// Just paranoia
	if ( oldCursorPos == newCursorPos )
	  return;
  
	var ops = new protocol.ProtocolDocumentOperation();
	var start = Math.min( oldCursorPos, newCursorPos );
	if ( start > 0 )
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( start ) );
	if ( oldCursorPos < newCursorPos )
	{
		ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ ], 
			[ protocol.ProtocolDocumentOperation.newKeyValueUpdate( this.listener.cursorAnnoKey, this.listener.cursorAnnoValue, null ) ] ) );
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( newCursorPos - oldCursorPos ) );
	}
	else
	{
		ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ ], 
			[ protocol.ProtocolDocumentOperation.newKeyValueUpdate( this.listener.cursorAnnoKey, null, this.listener.cursorAnnoValue ) ] ) );
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( oldCursorPos - newCursorPos ) );
	}
	var end = Math.max( oldCursorPos, newCursorPos );
	if ( end < count )	  
	{
		ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ this.listener.cursorAnnoKey ], [ ] ) );
		ops.component.push( protocol.ProtocolDocumentOperation.newRetainItemCount( count - end ) );
	}
	this.submit( ops );
};

/**
 * Internal helper function.
 *
 * Sets the HTML cursor according to 'this.cursor'.
 */
JSOT.Editor.prototype.showCursor = function()
{
	var sel = window.getSelection();
	var cursorPos = this.getDomPosition( this.cursor.line, this.cursor.lineno, this.cursor.charCount );
	sel.collapse( cursorPos.node, cursorPos.offset );
};

/**
 * Internal helper function.
 *
 * Adds an annotation boundary to the ops which sets the
 * time and JID for the current session. This allows others to understand the age
 * of the current session and the user associated with the session.
 *
 * @param {ProtocolDocumentOperation} ops
 */
JSOT.Editor.prototype.newUserAnnotation = function(ops)
{
	var key = "user/d/" + JSOT.Rpc.sessionId;
	var newValue = JSOT.Rpc.jid + "," + (new Date()).getTime().toString();
	var oldValue = null;
	if ( this.doc.content.length > 0 )
		oldValue = this.doc.getFormatAt(0)[ key ];
	
	// Add an annotation to the ProtocolDocumentOperation
	ops.component.push( protocol.ProtocolDocumentOperation.newAnnotationBoundary( [ ], [ 
		protocol.ProtocolDocumentOperation.newKeyValueUpdate( key, oldValue, newValue) ] ) );
};

/**
 * Internal helper function.
 *
 * Sends the document operation to the wavelet which will apply it and forward it to the server.
 */
JSOT.Editor.prototype.submit = function(docOp)
{
	var wavelet = this.doc.wavelet;
	
	var m = new protocol.ProtocolWaveletOperation_MutateDocument();
    m.document_id = this.doc.docId;
    m.document_operation = docOp;
	
	// Apply locally and send to the server
	wavelet.submitMutations( [m] );
};

