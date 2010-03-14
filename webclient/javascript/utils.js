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