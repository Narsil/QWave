if (!Array.prototype.indexOf)
{
  Array.prototype.indexOf = function(elt /*, from*/)
  {
    var len = this.length;

    var from = Number(arguments[1]) || 0;
    from = (from < 0)
         ? Math.ceil(from)
         : Math.floor(from);
    if (from < 0)
      from += len;

    for (; from < len; from++)
    {
      if (from in this &&
          this[from] === elt)
        return from;
    }
    return -1;
  };
}

JSOT.Path = { };
JSOT.Path.Axis = { };
JSOT.Path.Filter = { };
JSOT.Template = { };

JSOT.Path.Axis.Child = function()
{
  this.nodes = [ ];
};

JSOT.Path.Axis.Child.prototype.getEvent = function( event_type )
{
  if ( event_type == "change" )
  {
	if ( !this.change_event )
	  this.change_event = new JSOT.Event(this);
	return this.change_event;
  }
  return null;
};

JSOT.Path.Axis.Child.prototype.evaluate = function( context, init )
{
  this.context = context;

  if ( init )
	context.getEvent("newChild").addListener( this.onNewChild, this );
  
  // Find the first child
  var child = context.firstChild;
  this.nodes = [ ];
    
  while( child )
  {
	// if ( !this.nodeTest || this.nodeTest.evaluate( child ) )
	this.nodes.push(child);
	child = child.nextSibling;
  }  
  
  var t = this.nodeTest;
  while( t )
  {
	this.nodes = t.evaluate( this.nodes );
	t = t.nextTest;
  }
  
  if ( this.nextPath )
  {	
	var result = this.nodes;
	this.nodes = [ ];
	for( var i = 0; i < result.length; ++i )
	{
	  this.nextPath.evaluate( result[i], init );
	  this.nodes = this.nodes.concat( this.nextPath.nodes );
	}
  }
  else if ( init )
  {
	// Find the root of this path
	var a = this;
	while( a.parentPath )
	  a = a.parentPath;
	
	for( var i = 0; i < this.nodes.length; ++i )
	{
	  var n = this.nodes[i];
	  if ( n.element_start )
		n.getEvent("remove").addListener( a.onRemove, a );
	  else
	  {
		if ( n.parentNode )
		  n.parentNode.getEvent("textChange").addListener( a.onRemove, a );
		else
		  n.doc.getEvent("textChange").addListener( a.onRemove, a );
	  }
	}
  }
};

JSOT.Path.Axis.Child.prototype.onNewChild = function( eventArgs, sender )
{
  window.console.log("onNewChild in path. Node is " + eventArgs.element.type + " sender is " + sender.owner.type );
  
  var element = eventArgs.element;
  // Is it a direct child?
  if ( (element.parentNode || sender.owner != element.doc) && sender.owner != element.parentNode )
	return;

  this.nodes = [ element ];
  var t = this.nodeTest;
  while( t )
  {
	this.nodes = t.evaluate( this.nodes );
	t = t.nextTest;
  }

  if ( this.nodes.length > 0 )
  {
	// Evaluate the remaining path tail (if there is any)
	if ( this.nextPath )
	{
	  window.console.log("...recursion");
	  for( var i = 0; i < this.nodes.length; ++i )
		this.nextPath.evaluate( this.nodes[i], true );
	}
	
	// Find the root of this path
	var a = this;
	while( a.parentPath )
	  a = a.parentPath;

	a.evaluate( a.context );
	if ( a.change_event )
	  a.change_event.emit();
  }
};

JSOT.Path.Axis.Child.prototype.onRemove = function( eventArgs, sender )
{
  window.console.log("REMOVE");
  var index = this.nodes.indexOf( eventArgs.element );
  if ( index == -1 )
	return;
  window.console.log("REMOVE 2");
  this.nodes.splice( index, 1 );
  
  if ( this.change_event )
	this.change_event.emit();  
};

JSOT.Path.Filter.NodeName = function( nodeName )
{
  this.nodeName = nodeName;
};

JSOT.Path.Filter.NodeName.prototype.evaluate = function( nodes )
{
  var i = 0;
  while( i < nodes.length )
  {	
	if ( nodes[i].type != this.nodeName )
	  nodes.splice( i, 1 );
	else
	  ++i;
  }
  return nodes;
};

JSOT.Path.Filter.Index = function( index )
{
  this.index = index;
};

JSOT.Path.Filter.Index.prototype.evaluate = function( nodes )
{
  if ( nodes.length <= this.index )
	return [ ];
  return [ nodes[index] ];
};

JSOT.Path.Filter.Text = function()
{
};

JSOT.Path.Filter.Text.prototype.evaluate = function( nodes )
{
  var i = 0;
  while( i < nodes.length )  
  {
	var t = nodes[i].getText();
  window.console.log("Text is " + t );
	if ( !t || t == "" )
	  nodes.splice( i, 1 );
	else
	  i++;
  }
  return nodes;
};

JSOT.Template.RootTemplate = function( dom, subTemplate )
{
  this.dom = dom;
  this.subTemplate = subTemplate;
};

JSOT.Template.RootTemplate.prototype.createInstance = function( context )
{
  var obj = new JSOT.Template.RootTemplateInstance( context, this );
  obj.evaluate();
  return obj;
};

JSOT.Template.RootTemplateInstance = function( context, template )
{
  this.dirty = true;
  this.context = context;
  this.template = template;
  this.subTemplateInstance = this.template.subTemplate.createInstance( context, this );
  this.domNodes = [ ];
};

JSOT.Template.RootTemplateInstance.prototype.evaluate = function()
{
  if ( !this.dirty )
	return this.domNodes;

  var nodes = this.subTemplateInstance.evaluate();
  var old = this.template.dom.firstChild;
  for( var i = 0; i < nodes.length; ++i )
  {
	if ( old == nodes[i] )
	{
	  old = old.nextSibling;
	  continue;
	}
	
	if ( old )
	{
	  var tmp = old;
	  old = old.nextSibling;
	  this.template.dom.replaceChild( nodes[i], tmp );
	}
	else
	  this.template.dom.appendChild( nodes[i] );
  }

  while( old )
  {
	var tmp = old;
	old = old.nextSibling;
	this.template.dom.removeChild( tmp );
  }
  
  this.domNodes = nodes;
  this.dirty = false;
};

JSOT.Template.StaticTemplate = function( initFunc )
{
  this.initFunc = initFunc;
  this.subTemplates = { };
};

JSOT.Template.StaticTemplate.prototype.addTemplate = function( name, subTemplate )
{
  this.subTemplates[ name ] = subTemplate;
};

JSOT.Template.StaticTemplate.prototype.createInstance = function( context, parentInstance )
{
  var obj = new JSOT.Template.StaticTemplateInstance( context, this, parentInstance );
  this.initFunc( obj );
  return obj;
};

JSOT.Template.StaticTemplateInstance = function( context, template, parentInstance )
{
  this.parentInstance = parentInstance;
  this.dirty = true;
  this.context = context;
  this.template = template;
  this.domNodes = [ ];
  this.subTemplateInstances = [ ];
};

JSOT.Template.StaticTemplateInstance.prototype.instantiateTemplate = function( subTemplateName, parentDomNode, beforeDomNode )
{
  this.subTemplateInstances.push( { templateInstance : this.template.subTemplates[subTemplateName].createInstance( this.context, this ), parentDomNode : parentDomNode, beforeDomNode : beforeDomNode, domNodes : [] } );
};

JSOT.Template.StaticTemplateInstance.prototype.evaluate = function()
{
  if ( !this.dirty )
	return this.domNodes;
  
  window.console.log("Static Eval");
  for( var key in this.subTemplateInstances )
  {
	window.console.log("Static Eval Loop");
	var descriptor = this.subTemplateInstances[key];
	var nodes = descriptor.templateInstance.evaluate();
	var parentDomNode = descriptor.parentDomNode;
	
	var end;
	if ( descriptor.domNodes.length > 0 )
	  end = descriptor.domNodes[ descriptor.domNodes.length - 1 ];
	
	var mismatch = 0;
	var before = descriptor.beforeDomNode;
	for( var i = 0; i < nodes.length; ++i )
	{
	  if ( descriptor.domNodes.length > i && descriptor.domNodes[i] == nodes[i] )
		continue;
	  if ( descriptor.domNodes.length > i )
		parentDomNode.replaceChild( nodes[i], descriptor.domNodes[i] );
	  else
		parentDomNode.insertBefore( nodes[i], before );
	}
	
	while( i < descriptor.domNodes.length )
	  parentDomNode.removeChild( descriptor.domNodes[i++] );
	descriptor.domNodes = nodes;
  }
  
  this.dirty = false;
  return this.domNodes;
};

JSOT.Template.ForeachTemplate = function( path, subTemplate )
{
  this.path = path;
  this.subTemplate = subTemplate;
};

JSOT.Template.ForeachTemplate.prototype.createInstance = function( context, parentInstance )
{
  var obj = new JSOT.Template.ForeachTemplateInstance( context, this, parentInstance );
  return obj;
};

JSOT.Template.ForeachTemplateInstance = function( context, template, parentInstance )
{  
  this.parentInstance = parentInstance;
  this.dirty = true;
  this.context = context;
  this.template = template;
  // TODO: clone the path
  this.path = template.path;
  this.domNodes = [ ];
  this.otNodes = [ ];
  this.subTemplateInstances = [ ];
  
  this.path.getEvent("change").addListener( this.onPathChange, this );
  this.path.evaluate( this.context );  
};

JSOT.Template.ForeachTemplateInstance.prototype.evaluate = function()
{
  if ( !this.dirty )
	return this.domNodes;
  
  window.console.log("FOREACH");
    
  var dom_result = [ ];
  var dom_list = [ ];
  var instances = [ ];
  
  for( var i = 0; i < this.path.nodes.length; ++i )
  {
	window.console.log("FOREACH loop");
	var node = this.path.nodes[i];
	// Did we instantiate a template for this node before?
	var index = this.otNodes.indexOf( node );
	if ( index == -1 )
	{
	  var tinstance = this.template.subTemplate.createInstance( node, this );
	  var doms = tinstance.evaluate();
	  dom_result = dom_result.concat( doms );
	  instances[i] = tinstance;
	}
	else
	{
	  var tinstance = this.subTemplateInstances[index];
	  var nodes = tinstance.evaluate();
	  dom_result = dom_result.concat( nodes );
	  instances[i] = tinstance;
	}
  }
  
  this.subTemplateInstances = instances;
  this.otNodes = this.path.nodes;
  this.dirty = false;
  this.domNodes = dom_result;
  return this.domNodes;
};

JSOT.Template.ForeachTemplateInstance.prototype.onPathChange = function()
{
  this.dirty = true;
  var p = this.parentInstance;
  while( p )
  {
	p.dirty = true;
	p = p.parentInstance;
  }
};

JSOT.Template.SwitchTemplate = function()
{
  this.cases = [ null ];
};

JSOT.Template.SwitchTemplate.prototype.addDefaultCase = function( subTemplate )
{
  this.cases[0] = { template : subTemplate };
};

JSOT.Template.SwitchTemplate.prototype.addCase = function( path, subTemplate )
{
  this.cases.push( { path : path, template : subTemplate } );
};

JSOT.Template.SwitchTemplate.prototype.createInstance = function( context, parentInstance )
{
  var obj = new JSOT.Template.SwitchTemplateInstance( context, this, parentInstance );
  return obj;
};

JSOT.Template.SwitchTemplateInstance = function( context, template, parentInstance )
{  
  this.parentInstance = parentInstance;
  this.dirty = true;
  this.context = context;
  this.template = template;
  this.currentCaseIndex = -1;
  this.currentTemplateInstance = null;

  this.paths = [ ];
  for( var i = 1; i < this.template.cases.length; ++i )
  {
	var c = this.template.cases[i];
	if ( c.path )
	{
	  // TODO: Copy the path
	  this.paths[i] = c.path;
	  this.paths[i].getEvent("change").addListener( this.onPathChange, this );
	}
  }
};

JSOT.Template.SwitchTemplateInstance.prototype.onPathChange = function()
{
  window.console.log("SWITCH path changed");
  
  this.dirty = true;
  var p = this.parentInstance;
  while( p )
  {
	p.dirty = true;
	p = p.parentInstance;
  }
};

JSOT.Template.SwitchTemplateInstance.prototype.evaluate = function()
{
  if ( !this.dirty )
	return this.domNodes;
  
  window.console.log("SWITCH");

  // Default case
  var index = 0;
  // Determine which case applies
  for( var i = 1; i < this.template.cases.length; ++i )
  {
	var c = this.template.cases[i];
	var result = c.evaluate( [this.context] );
	if ( result.length == 1 )
	{
	  index = i;
	  break;
	}
  }
  
  // No change to before?
  if ( index == this.currentCaseIndex )
	return this.domNodes;
  
  // Instantiate the corresponding sub-template (if there is one)
  this.currentCaseIndex = index;
  var subTemplate = this.template.cases[ this.currentCaseIndex ].template;
  if ( subTemplate )
  {
	this.currentTemplateInstance = subTemplate.createInstance( this.context, this );
	this.domNodes = this.currentTemplateInstance.evaluate();
  }
  else
	this.domNodes = [ ];
  return this.domNodes;
};