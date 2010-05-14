function getElementByClass( root, className )
{
	if ( !root )
		window.console.log("SHIT");
	for( var i = 0; i < root.childNodes.length; ++i )
	{
		var child = root.childNodes[i];
		if ( child.className == className )
			return child;
		var result = getElementByClass( child, className );
		if ( result )
			return result;
	}
	return null;
}

function getElementsByClass( root, className, result )
{
	if ( !result )
		result = [];
	for( var i = 0; i < root.childNodes.length; ++i )
	{
		var child = root.childNodes[i];
		if ( child.className == className )
			result.push( child );
		getElementsByClass( child, className, result );
	}
	return result;
}

function assert(ok, assertString)
{
  if ( ok )
	return true;
  
  try {
     throw new Error("ECMAScript assertion failed:  (" + assertString + ")");    
  }
  catch(e) {
/* For Mozilla, use
      throw new Error(e.message + " stack:\n" + e.stack);
*/
      window.console.log(e.stack);
      throw e;
/* For Mozilla, use
      dump ("Warning: " + e.message + " stack:\n" + e.stack);
      for (property in argsObj) {
        dump(property + " = " + argsObj[property] + "\n");
      }
      dump("\n");
*/
  }
  return false;
}

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
