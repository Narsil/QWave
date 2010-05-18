JSOT.Path = { };
JSOT.Path.Axis = { };
JSOT.Path.Filter = { };
JSOT.Path.Mapping = { };
JSOT.Template = { };

/*******************************************************
 *
 * Path.PathTemplate
 *
 *******************************************************/

JSOT.Path.PathTemplate = function( axis )
{
  this.axis = axis;
};

JSOT.Path.PathTemplate.prototype.setMapping = function( mapping )
{
  this.mapping = mapping;
};

JSOT.Path.PathTemplate.prototype.createInstance = function( context )
{
  var obj = new JSOT.Path.Root( this.axis, context );
  if ( this.mapping )
	obj.mapping = this.mapping;
  return obj;
};

/*******************************************************
 *
 * Path.Root
 *
 *******************************************************/

JSOT.Path.Root = function( axis, context )
{
  this.nodes = [ ];
  this.dirty = true;
  this.valuesDirty = true;
  this.isInitialized = false;
  this.axis = axis;
  this.context = context;
};

JSOT.Path.Root.prototype.getEvent = function( event_type )
{
  if ( event_type == "change" )
  {
	if ( !this.change_event )
	  this.change_event = new JSOT.Event(this);
	return this.change_event;
  }
  return null;
};

JSOT.Path.Root.prototype.getNodes = function()
{
  if ( !this.dirty )
	return this.nodes;
  this.dirty = false;
  if ( this.isInitialized )
  {
	window.console.log("getNodes 1");
	this.nodes = this.axis.evaluate( this, this.context, false );
  }
  else
  {
	window.console.log("getNodes 2");
	this.nodes = this.axis.evaluate( this, this.context, true );
	this.isInitialized = true;
  }
  return this.nodes;
};

JSOT.Path.Root.prototype.getValues = function()
{
  if ( !this.mapping )
	return this.getNodes();
  if ( !this.valuesDirty )
	return this.values;
  var nodes = this.getNodes();
  this.values = [ ];  
  for( var i = 0; i < nodes.length; ++i )
	this.values.push( this.mapping.map( this, nodes[i] ) );
  this.valuesDirty = false;
  return this.values;
};

JSOT.Path.Root.prototype.setDirty = function()
{
  if ( this.dirty )
	return;
  window.console.log("Root is set dirty");
  this.dirty = true;
  this.valuesDirty = true;
  if ( this.change_event )
	this.change_event.emit();
};

JSOT.Path.Root.prototype.containsNode = function( node )
{
  return this.nodes.indexOf( node ) != -1;
};

JSOT.Path.Root.prototype.removeNode = function( node )
{
  var index = this.nodes.indexOf( node );
  if ( index == -1 )
	return;
  this.nodes.splice( index, 1 );
  if ( this.values )
	this.values.splice( index, 1 );
  
  if ( this.change_event )
	this.change_event.emit();  
};

JSOT.Path.Root.prototype.onRemove = function( eventArgs, sender )
{
  window.console.log("REMOVE");
  this.removeNode( eventArgs.element );
};

JSOT.Path.Root.prototype.onMappingChange = function( eventArgs, sender )
{
  var index = this.nodes.indexOf( eventArgs.element );
  if ( index == -1 )
	return;
  if ( this.mapping )
	this.values[i] = this.mapping.map( this, this.nodes[i] );
  
  if ( this.change_event )
	this.change_event.emit();  
};

/*******************************************************
 *
 * Mapping.Text
 *
 *******************************************************/

JSOT.Path.Mapping.Text = function()
{
};

JSOT.Path.Mapping.Text.prototype.map = function( root, node )
{
  var result = node.getText();

  if ( this.nextMapping )
	return this.nextMapping( result );  
  return result;
};

/*******************************************************
 *
 * Axis.Child
 *
 *******************************************************/

JSOT.Path.Axis.Child = function()
{
};

JSOT.Path.Axis.Child.prototype.evaluate = function( root, context, init )
{
  if ( init )
	context.getEvent("newChild").addListenerOnce( this.onNewChild, this, root );
  
  // Find the first child
  var child = context.firstChild;
  var nodes = [ ];
    
  while( child )
  {
	nodes.push(child);
	child = child.nextSibling;
  }  
  
  if( this.filter )
	nodes = this.filter.evaluate( root, this, nodes );
  
  if ( this.nextPath )
  {	
	var result = [ ];
	for( var i = 0; i < nodes.length; ++i )
	  result = result.concat( this.nextPath.evaluate( root, nodes[i], init ) );
	return result;
  }
  
  if ( init )
  {
	for( var i = 0; i < nodes.length; ++i )
	{
	  nodes[i].getEvent("remove").addListenerOnce( root.onRemove, root );
	}
  }
  
  return nodes;
};

JSOT.Path.Axis.Child.prototype.onNewChild = function( eventArgs, sender, root )
{  
  var element = eventArgs.element;
  // Is it a direct child? If not -> not interested
  if ( (element.parentNode || sender.owner != element.doc) && sender.owner != element.parentNode )
	return;

  if ( !this.testNode_( root, eventArgs.element ) )
	return;
    
  element.getEvent("remove").addListenerOnce( root.onRemove, root );
  
  root.setDirty();
};

JSOT.Path.Axis.Child.prototype.onFilterChange = function( eventArgs, sender, root )
{
  if ( !this.testNode_( root, eventArgs.element ) )
  {
	root.removeNode( eventArgs.element );
  }
  else
  {
	if ( !root.containsNode( eventArgs.element ) )
	  root.setDirty();
  }
};

JSOT.Path.Axis.Child.prototype.testNode_ = function( root, node )
{  
  // Filter the node
  var nodes = [ node ];  
  if( this.filter )
	nodes = this.filter.evaluate( root, this, nodes );

  // Did not pass the filter?
  if ( nodes.length == 0 )
	return false;
  
  // Evaluate the remaining path tail (if there is any)
  if ( this.nextPath )
  {
	window.console.log("...recursion");
	for( var i = 0; i < nodes.length; ++i )
	  nodes = this.nextPath.evaluate( root, node, true );

	// There is no new result node? Nothing changed -> good bye
	if ( nodes.length == 0 )
	  return false;
  }
	
  return true;
};

/*******************************************************
 *
 * Axis.Self
 *
 *******************************************************/

JSOT.Path.Axis.Self = function()
{
};

JSOT.Path.Axis.Self.prototype.evaluate = function( root, context, init )
{
  var nodes = [ context ];
  
  if( this.filter )
	nodes = this.filter.evaluate( root, this, nodes );
  
  if ( this.nextPath )
  {	
	var result = [ ];
	for( var i = 0; i < nodes.length; ++i )
	  result = result.concat( this.nextPath.evaluate( root, nodes[i], init ) );
	return result;
  }  
  else if ( init )
  {
	context.getEvent("remove").addListenerOnce( root.onRemove, root );
  }
  
  return nodes;
};

JSOT.Path.Axis.Self.prototype.onFilterChange = function( eventArgs, sender, root )
{
  if ( !this.testNode_( root, eventArgs.element ) )
  {
	root.removeNode( eventArgs.element );
  }
  else
  {
	if ( !root.containsNode( eventArgs.element ) )
	  root.setDirty();
  }
};

JSOT.Path.Axis.Self.prototype.testNode_ = function( root, node )
{  
  // Filter the node
  var nodes = [ node ];  
  if( this.filter )
	nodes = this.filter.evaluate( root, this, nodes );

  // Did not pass the filter?
  if ( nodes.length == 0 )
	return false;
  
  // Evaluate the remaining path tail (if there is any)
  if ( this.nextPath )
  {
	window.console.log("...recursion");
	for( var i = 0; i < nodes.length; ++i )
	  nodes = this.nextPath.evaluate( root, node, true );

	// There is no new result node? Nothing changed -> good bye
	if ( nodes.length == 0 )
	  return false;
  }
	
  return true;
};

/*******************************************************
 *
 * Filter.NodeName
 *
 *******************************************************/

JSOT.Path.Filter.NodeName = function( nodeName )
{
  this.nodeName = nodeName;
};

JSOT.Path.Filter.NodeName.prototype.evaluate = function( root, axis, nodes )
{
  var i = 0;
  while( i < nodes.length )
  {	
	if ( nodes[i].type != this.nodeName )
	  nodes.splice( i, 1 );
	else
	  ++i;
  }
  
  if ( this.nextFilter )
	return this.nextFilter.evaluate( root, axis, nodes );
  return nodes;
};

/*******************************************************
 *
 * Filter.Index
 *
 *******************************************************/

JSOT.Path.Filter.Index = function( index )
{
  this.index = index;
};

JSOT.Path.Filter.Index.prototype.evaluate = function( root, axis, nodes )
{
  if ( nodes.length <= this.index )
	return [ ];
  
  if ( this.nextFilter )
	return this.nextFilter.evaluate( root, axis, nodes[index] );
  return [ nodes[index] ];
};

/*******************************************************
 *
 * Filter.HasText
 *
 *******************************************************/

JSOT.Path.Filter.HasText = function()
{
};

JSOT.Path.Filter.HasText.prototype.evaluate = function( root, axis, nodes )
{
  var i = 0;
  while( i < nodes.length )  
  {
	var node = nodes[i];
	node.getEvent("textChange").addListenerOnce( axis.onFilterChange, axis, root );
	
	var t = nodes[i].getText();
	if ( !t || t == "" )
	  nodes.splice( i, 1 );
	else
	  i++;
  }
  
  if ( this.nextFilter )
	return this.nextFilter.evaluate( root, axis, nodes );  
  return nodes;
};

/*******************************************************
 *
 * Filter.LocalAttribute
 *
 *******************************************************/

JSOT.Path.Filter.LocalAttribute = function(attributeName, value)
{
  this.attributeName = attributeName;
  this.value = value;
};

JSOT.Path.Filter.LocalAttribute.prototype.evaluate = function( root, axis, nodes )
{
  var i = 0;
  while( i < nodes.length )  
  {
	var node = nodes[i];
	node.getEvent("attributeChange").addListenerOnce( axis.onFilterChange, axis, root );
	
	if ( node.getLocalAttribute( this.attributeName ) == this.value )
	{
	  window.console.log("ATTRIB match");
	  i++;
	}
	else
	{
	  window.console.log("ATTRIB no match");
	  nodes.splice( i, 1 );
	}
  }
  
  if ( this.nextFilter )
	return this.nextFilter.evaluate( root, axis, nodes );  
  return nodes;
};

/*******************************************************
 *
 * RootTemplate
 *
 *******************************************************/

JSOT.Template.RootTemplate = function( subTemplate )
{
  this.subTemplate = subTemplate;
};

JSOT.Template.RootTemplate.prototype.createInstance = function( context, dom )
{
  return new JSOT.Template.RootTemplateInstance( context, this, dom );
};

JSOT.Template.RootTemplateInstance = function( context, template, dom )
{
  this.dom = dom;  
  this.context = context;
  this.template = template;
  this.subTemplateInstance = this.template.subTemplate.createInstance( context, this );
  this.domNodes = [ ];
  this.setDirty( true );
};

JSOT.Template.RootTemplateInstance.prototype.evaluate_ = function()
{
  if ( !this.dirty )
	return;

  window.console.log("evaluate_");
  
  var nodes = this.subTemplateInstance.evaluate();
  var old = this.dom.firstChild;
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
	  this.dom.replaceChild( nodes[i], tmp );
	}
	else
	  this.dom.appendChild( nodes[i] );
  }

  while( old )
  {
	var tmp = old;
	old = old.nextSibling;
	this.dom.removeChild( tmp );
  }
  
  this.domNodes = nodes;
  this.dirty = false;
};

JSOT.Template.RootTemplateInstance.prototype.setDirty = function()
{
  if ( this.dirty )
	return;
  this.dirty = true;
  var self = this;
  window.setTimeout( function() { self.evaluate_(); }, 1 );
};

/*******************************************************
 *
 * StaticTemplate
 *
 *******************************************************/

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
//  window.console.log("Instantiate with " + context );
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
  
//  window.console.log("Static Eval");
  for( var key in this.subTemplateInstances )
  {
//	window.console.log("Static Eval Loop");
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

/*******************************************************
 *
 * ForeachTemplate
 *
 *******************************************************/

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
  this.path = template.path.createInstance( this.context );
  this.domNodes = [ ];
  this.otNodes = [ ];
  this.otValues = [ ];
  this.subTemplateInstances = [ ];
  
  this.path.getEvent("change").addListener( this.onPathChange, this );
};

JSOT.Template.ForeachTemplateInstance.prototype.evaluate = function()
{
  if ( !this.dirty )
	return this.domNodes;
      
  var dom_result = [ ];
  var dom_list = [ ];
  var instances = [ ];
  
  var nodes = this.path.getNodes();  
  var values = this.path.getValues();
  
  window.console.log("FOREACH " + nodes.length);

  for( var i = 0; i < nodes.length; ++i )
  {	
	var node = nodes[i];
	var value = values[i];
	// Did we instantiate a template for this node before?
	var index = this.otNodes.indexOf( node );
	// No -> instantiate it
	if ( index == -1 || value != this.otValues[index] )
	{
	  // window.console.log("FOREACH loop x1: index = " + index + ", value=" + value + ", otValue is " + this.otValues[i] );
	  var tinstance = this.template.subTemplate.createInstance( value, this );
	  var doms = tinstance.evaluate();
	  dom_result = dom_result.concat( doms );
	  instances[i] = tinstance;
	}
	// Yes -> update it
	else
	{
	  window.console.log("FOREACH loop x2");
	  var tinstance = this.subTemplateInstances[index];
	  dom_result = dom_result.concat( tinstance.evaluate() );
	  instances[i] = tinstance;
	}
  }
  
  this.subTemplateInstances = instances;
  this.otNodes = nodes.slice( 0, nodes.length );
  this.otValues = values.slice( 0, values.length );
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
	if ( !p.parentInstance )
	{
	  p.setDirty();
	  break;
	}
	else
	{
	  p.dirty = true;
	  p = p.parentInstance;
	}
  }
};

/*******************************************************
 *
 * SwitchTemplate
 *
 *******************************************************/

JSOT.Template.SwitchTemplate = function()
{
  this.cases = [ null ];
};

/**
 * @param {JSOT.Path.PathTemplate} path is optional.
 */
JSOT.Template.SwitchTemplate.prototype.addDefaultCase = function( subTemplate, path )
{
  this.cases[0] = { template : subTemplate };
};

/**
 * @param {JSOT.Path.PathTemplate} path is mandatory.
 */
JSOT.Template.SwitchTemplate.prototype.addCase = function( subTemplate, path )
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
	  this.paths[i] = c.path.createInstance( this.context );
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
	if ( !p.parentInstance )
	{
	  p.setDirty();
	  break;
	}
	else
	{
	  p.dirty = true;
	  p = p.parentInstance;
	}
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
  for( var i = 1; i < this.paths.length; ++i )
  {
	var result = this.paths[i].getNodes();
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