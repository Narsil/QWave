/**
 * Code retrieved from: http://ostemplates-demo.appspot.com/#home
 */

/**
 * NOTE: This code is a minimized and concatenated version of jstemplates
 * library and an implementation of the OpenSocial Templates spec proposal.
 * All comments have been stripped off.
 */

function log() {
}
var TYPE_boolean = "boolean", TYPE_number = "number", TYPE_object = "object", TYPE_undefined = "undefined";
function copyProperties(to, from) {
  for(var p in from)to[p] = from[p]
}
function isArray(value) {
  return value != null && typeof value == TYPE_object && typeof value.length == TYPE_number
}
function arraySlice() {
  return Function.prototype.call.apply(Array.prototype.slice, arguments)
}
function arrayClear(array) {
  array.length = 0
}
function bindFully(object, method) {
  var args = arraySlice(arguments, 2);
  return function() {
    return method.apply(object, args)
  }
}
function domTraverseElements(node, callback) {
  var traverser = new DomTraverser(callback);
  traverser.run(node)
}
function DomTraverser(callback) {
  this.callback_ = callback
}
DomTraverser.prototype.run = function(root) {
  var me = this;
  me.queue_ = [root];
  while(me.queue_.length)me.process_(me.queue_.shift())
};
DomTraverser.prototype.process_ = function(node) {
  var me = this;
  me.callback_(node);
  for(var c = node.firstChild;c;c = c.nextSibling)c.nodeType == 1 && me.queue_.push(c)
};
function domSetAttribute(node, name, value) {
  node.setAttribute(name, value)
}
function domRemoveAttribute(node, name) {
  node.removeAttribute(name)
}
function displayDefault(node) {
  node.style.display = ""
}
function displayNone(node) {
  node.style.display = "none"
}
function positionAbsolute(node) {
  node.style.position = "absolute"
}
function domRemoveNode(node) {
  return node.parentNode.removeChild(node)
}
;var VAR_index = "$index", VAR_count = "$count", VAR_this = "$this", VAR_context = "$context", VAR_top = "$top", GLOB_default = "$default", CHAR_colon = ":", REGEXP_semicolon = /\s*;\s*/;
function JsEvalContext() {
  this.constructor_.apply(this, arguments)
}
JsEvalContext.prototype.constructor_ = function(opt_data, opt_parent) {
  var me = this;
  if(!me.vars_)me.vars_ = {};
  opt_parent ? copyProperties(me.vars_, opt_parent.vars_) : copyProperties(me.vars_, JsEvalContext.globals_);
  me.vars_[VAR_this] = opt_data;
  me.vars_[VAR_context] = me;
  me.data_ = typeof opt_data != TYPE_undefined && opt_data != null ? opt_data : "";
  if(!opt_parent)me.vars_[VAR_top] = me.data_
};
JsEvalContext.globals_ = {};
JsEvalContext.setGlobal = function(name, value) {
  JsEvalContext.globals_[name] = value
};
JsEvalContext.setGlobal(GLOB_default, null);
JsEvalContext.recycledInstances_ = [];
JsEvalContext.create = function(opt_data, opt_parent) {
  if(JsEvalContext.recycledInstances_.length > 0) {
    var instance = JsEvalContext.recycledInstances_.pop();
    JsEvalContext.call(instance, opt_data, opt_parent);
    return instance
  }else return new JsEvalContext(opt_data, opt_parent)
};
JsEvalContext.recycle = function(instance) {
  for(var i in instance.vars_)delete instance.vars_[i];
  instance.data_ = null;
  JsEvalContext.recycledInstances_.push(instance)
};
JsEvalContext.prototype.jsexec = function(exprFunction, template) {
  try {
    return exprFunction.call(template, this.vars_, this.data_)
  }catch(e) {
    log("jsexec EXCEPTION: " + e + " at " + template + " with " + exprFunction);
    return JsEvalContext.globals_[GLOB_default]
  }
};
JsEvalContext.prototype.clone = function(data, index, count) {
  var ret = JsEvalContext.create(data, this);
  ret.setVariable(VAR_index, index);
  ret.setVariable(VAR_count, count);
  return ret
};
JsEvalContext.prototype.setVariable = function(name, value) {
  this.vars_[name] = value
};
JsEvalContext.prototype.getVariable = function(name) {
  return this.vars_[name]
};
JsEvalContext.prototype.evalExpression = function(expr, opt_template) {
  var exprFunction = jsEvalToFunction(expr);
  return this.jsexec(exprFunction, opt_template)
};
var STRING_a = "a_", STRING_b = "b_", STRING_with = "with (a_) with (b_) return ";
JsEvalContext.evalToFunctionCache_ = {};
function jsEvalToFunction(expr) {
  if(!JsEvalContext.evalToFunctionCache_[expr])try {
    JsEvalContext.evalToFunctionCache_[expr] = new Function(STRING_a, STRING_b, STRING_with + expr)
  }catch(e) {
    log("jsEvalToFunction (" + expr + ") EXCEPTION " + e)
  }return JsEvalContext.evalToFunctionCache_[expr]
}
function jsEvalToSelf(expr) {
  return expr
}
function jsEvalToValues(expr) {
  var ret = [], values = expr.split(REGEXP_semicolon);
  for(var i = 0, I = values.length;i < I;++i) {
    var colon = values[i].indexOf(CHAR_colon);
    if(colon < 0)continue;
    var label = values[i].substr(0, colon).replace(/^\s+/, "").replace(/\s+$/, ""), value = jsEvalToFunction(values[i].substr(colon + 1));
    ret.push(label, value)
  }return ret
}
function jsEvalToExpressions(expr) {
  var ret = [], values = expr.split(REGEXP_semicolon);
  for(var i = 0, I = values.length;i < I;++i)if(values[i]) {
    var value = jsEvalToFunction(values[i]);
    ret.push(value)
  }return ret
}
;var ATT_select = "jsselect", ATT_instance = "jsinstance", ATT_display = "jsdisplay", ATT_values = "jsvalues", ATT_vars = "jsvars", ATT_eval = "jseval", ATT_transclude = "transclude", ATT_content = "jscontent", ATT_skip = "jsskip", ATT_jstcache = "jstcache", PROP_jstcache = "__jstcache", STRING_jsts = "jsts", CHAR_asterisk = "*", CHAR_dollar = "$", CHAR_period = ".", CHAR_ampersand = "&", STRING_div = "div", STRING_id = "id", STRING_asteriskzero = "*0", STRING_zero = "0";
function jstProcess(context, template) {
  var processor = new JstProcessor;
  JstProcessor.prepareTemplate_(template);
  processor.document_ = template ? template.nodeType == 9 ? template : template.ownerDocument || document : document;
  processor.run_(bindFully(processor, processor.jstProcessOuter_, context, template))
}
function JstProcessor() {
}
JstProcessor.jstid_ = 0;
JstProcessor.jstcache_ = {};
JstProcessor.jstcache_[0] = {};
JstProcessor.jstcacheattributes_ = {};
JstProcessor.attributeValues_ = {};
JstProcessor.attributeList_ = [];
JstProcessor.prepareTemplate_ = function(template) {
  template[PROP_jstcache] || domTraverseElements(template, function(node) {
    JstProcessor.prepareNode_(node)
  })
};
var JST_ATTRIBUTES = [[ATT_select, jsEvalToFunction], [ATT_display, jsEvalToFunction], [ATT_values, jsEvalToValues], [ATT_vars, jsEvalToValues], [ATT_eval, jsEvalToExpressions], [ATT_transclude, jsEvalToSelf], [ATT_content, jsEvalToFunction], [ATT_skip, jsEvalToFunction]];
JstProcessor.prepareNode_ = function(node) {
  if(node[PROP_jstcache])return node[PROP_jstcache];
  var jstid = node.getAttribute(ATT_jstcache);
  if(jstid != null)return node[PROP_jstcache] = JstProcessor.jstcache_[jstid];
  var attributeValues = JstProcessor.attributeValues_, attributeList = JstProcessor.attributeList_;
  attributeList.length = 0;
  for(var i = 0, I = JST_ATTRIBUTES.length;i < I;++i) {
    var name = JST_ATTRIBUTES[i][0], value = node.getAttribute(name);
    attributeValues[name] = value;
    value != null && attributeList.push(name + "=" + value)
  }if(attributeList.length == 0) {
    domSetAttribute(node, ATT_jstcache, STRING_zero);
    return node[PROP_jstcache] = JstProcessor.jstcache_[0]
  }var attstring = attributeList.join(CHAR_ampersand);
  if(jstid = JstProcessor.jstcacheattributes_[attstring]) {
    domSetAttribute(node, ATT_jstcache, jstid);
    return node[PROP_jstcache] = JstProcessor.jstcache_[jstid]
  }var jstcache = {};
  for(var i = 0, I = JST_ATTRIBUTES.length;i < I;++i) {
    var att = JST_ATTRIBUTES[i], name = att[0], parse = att[1], value = attributeValues[name];
    if(value != null)jstcache[name] = parse(value)
  }jstid = "" + ++JstProcessor.jstid_;
  domSetAttribute(node, ATT_jstcache, jstid);
  JstProcessor.jstcache_[jstid] = jstcache;
  JstProcessor.jstcacheattributes_[attstring] = jstid;
  return node[PROP_jstcache] = jstcache
};
JstProcessor.prototype.run_ = function(f) {
  var me = this, calls = me.calls_ = [], queueIndices = me.queueIndices_ = [];
  me.arrayPool_ = [];
  f();
  var queue, queueIndex, method, arg1, arg2;
  while(calls.length) {
    queue = calls[calls.length - 1];
    queueIndex = queueIndices[queueIndices.length - 1];
    if(queueIndex >= queue.length) {
      me.recycleArray_(calls.pop());
      queueIndices.pop();
      continue
    }method = queue[queueIndex++];
    arg1 = queue[queueIndex++];
    arg2 = queue[queueIndex++];
    queueIndices[queueIndices.length - 1] = queueIndex;
    method.call(me, arg1, arg2)
  }
};
JstProcessor.prototype.push_ = function(args) {
  this.calls_.push(args);
  this.queueIndices_.push(0)
};
JstProcessor.prototype.createArray_ = function() {
  return this.arrayPool_.length ? this.arrayPool_.pop() : []
};
JstProcessor.prototype.recycleArray_ = function(array) {
  arrayClear(array);
  this.arrayPool_.push(array)
};
JstProcessor.prototype.jstProcessOuter_ = function(context, template) {
  var me = this, jstAttributes = me.jstAttributes_(template), transclude = jstAttributes[ATT_transclude];
  if(transclude) {
    var tr = jstGetTemplate(transclude);
    if(tr) {
      template.parentNode.replaceChild(tr, template);
      var call = me.createArray_();
      call.push(me.jstProcessOuter_, context, tr);
      me.push_(call)
    }else domRemoveNode(template);
    return
  }var select = jstAttributes[ATT_select];
  select ? me.jstSelect_(context, template, select) : me.jstProcessInner_(context, template)
};
JstProcessor.prototype.jstProcessInner_ = function(context, template) {
  var me = this, jstAttributes = me.jstAttributes_(template), display = jstAttributes[ATT_display];
  if(display) {
    var shouldDisplay = context.jsexec(display, template);
    if(!shouldDisplay) {
      displayNone(template);
      return
    }displayDefault(template)
  }var values = jstAttributes[ATT_vars];
  values && me.jstVars_(context, template, values);
  values = jstAttributes[ATT_values];
  values && me.jstValues_(context, template, values);
  var expressions = jstAttributes[ATT_eval];
  if(expressions)for(var i = 0, I = expressions.length;i < I;++i)context.jsexec(expressions[i], template);
  var skip = jstAttributes[ATT_skip];
  if(skip) {
    var shouldSkip = context.jsexec(skip, template);
    if(shouldSkip)return
  }var content = jstAttributes[ATT_content];
  if(content)me.jstContent_(context, template, content);
  else {
    var queue = me.createArray_();
    for(var c = template.firstChild;c;c = c.nextSibling)c.nodeType == 1 && queue.push(me.jstProcessOuter_, context, c);
    queue.length && me.push_(queue)
  }
};
JstProcessor.prototype.jstSelect_ = function(context, template, select) {
  var me = this, value = context.jsexec(select, template), instance = template.getAttribute(ATT_instance), instanceLast = false;
  if(instance)if(instance.charAt(0) == CHAR_asterisk) {
    instance = parseInt(instance.substr(1), 10);
    instanceLast = true
  }else instance = parseInt(instance, 10);
  var multiple = isArray(value), count = multiple ? value.length : 1, multipleEmpty = multiple && count == 0;
  if(multiple)if(multipleEmpty)if(instance)domRemoveNode(template);
  else {
    domSetAttribute(template, ATT_instance, STRING_asteriskzero);
    displayNone(template)
  }else {
    displayDefault(template);
    if(instance === null || instance === "" || instanceLast && instance < count - 1) {
      var queue = me.createArray_(), instancesStart = instance || 0, i, I, clone;
      for(i = instancesStart, I = count - 1;i < I;++i) {
        var node = template.cloneNode(true);
        template.parentNode.insertBefore(node, template);
        jstSetInstance(node, value, i);
        clone = context.clone(value[i], i, count);
        queue.push(me.jstProcessInner_, clone, node, JsEvalContext.recycle, clone, null)
      }jstSetInstance(template, value, i);
      clone = context.clone(value[i], i, count);
      queue.push(me.jstProcessInner_, clone, template, JsEvalContext.recycle, clone, null);
      me.push_(queue)
    }else if(instance < count) {
      var v = value[instance];
      jstSetInstance(template, value, instance);
      var clone = context.clone(v, instance, count), queue = me.createArray_();
      queue.push(me.jstProcessInner_, clone, template, JsEvalContext.recycle, clone, null);
      me.push_(queue)
    }else domRemoveNode(template)
  }else if(value == null)displayNone(template);
  else {
    displayDefault(template);
    var clone = context.clone(value, 0, 1), queue = me.createArray_();
    queue.push(me.jstProcessInner_, clone, template, JsEvalContext.recycle, clone, null);
    me.push_(queue)
  }
};
JstProcessor.prototype.jstVars_ = function(context, template, values) {
  for(var i = 0, I = values.length;i < I;i += 2) {
    var label = values[i], value = context.jsexec(values[i + 1], template);
    context.setVariable(label, value)
  }
};
JstProcessor.prototype.jstValues_ = function(context, template, values) {
  for(var i = 0, I = values.length;i < I;i += 2) {
    var label = values[i], value = context.jsexec(values[i + 1], template);
    if(label.charAt(0) == CHAR_dollar)context.setVariable(label, value);
    else if(label.charAt(0) == CHAR_period) {
      var nameSpaceLabel = label.substr(1).split(CHAR_period), nameSpaceObject = template, nameSpaceDepth = nameSpaceLabel.length;
      for(var j = 0, J = nameSpaceDepth - 1;j < J;++j) {
        var jLabel = nameSpaceLabel[j];
        nameSpaceObject[jLabel] || (nameSpaceObject[jLabel] = {});
        nameSpaceObject = nameSpaceObject[jLabel]
      }nameSpaceObject[nameSpaceLabel[nameSpaceDepth - 1]] = value
    }else if(label)if(typeof value == TYPE_boolean)value ? domSetAttribute(template, label, label) : domRemoveAttribute(template, label);
    else domSetAttribute(template, label, "" + value)
  }
};
JstProcessor.prototype.jstContent_ = function(context, template, content) {
  var value = "" + context.jsexec(content, template);
  if(template.innerHTML == value)return;
  while(template.firstChild)domRemoveNode(template.firstChild);
  var t = this.document_.createTextNode(value);
  template.appendChild(t)
};
JstProcessor.prototype.jstAttributes_ = function(template) {
  if(template[PROP_jstcache])return template[PROP_jstcache];
  var jstid = template.getAttribute(ATT_jstcache);
  if(jstid)return template[PROP_jstcache] = JstProcessor.jstcache_[jstid];
  return JstProcessor.prepareNode_(template)
};
function jstGetTemplate(name, opt_loadHtmlFn) {
  var doc = document, section;
  section = opt_loadHtmlFn ? jstLoadTemplateIfNotPresent(doc, name, opt_loadHtmlFn) : doc.getElementById(name);
  if(section) {
    JstProcessor.prepareTemplate_(section);
    var ret = section.cloneNode(true);
    domRemoveAttribute(ret, STRING_id);
    return ret
  }else return null
}
function jstLoadTemplateIfNotPresent(doc, name, loadHtmlFn, opt_target) {
  var section = doc.getElementById(name);
  if(section)return section;
  jstLoadTemplate_(doc, loadHtmlFn(), opt_target || STRING_jsts);
  var section = doc.getElementById(name);
  section || log("Error: jstGetTemplate was provided with opt_loadHtmlFn, but that function did not provide the id '" + name + "'.");
  return section
}
function jstLoadTemplate_(doc, html, targetId) {
  var existing_target = doc.getElementById(targetId), target;
  if(existing_target)target = existing_target;
  else {
    target = doc.createElement(STRING_div);
    target.id = targetId;
    displayNone(target);
    positionAbsolute(target);
    doc.body.appendChild(target)
  }var div = doc.createElement(STRING_div);
  target.appendChild(div);
  div.innerHTML = html
}
function jstSetInstance(template, values, index) {
  index == values.length - 1 ? domSetAttribute(template, ATT_instance, CHAR_asterisk + index) : domSetAttribute(template, ATT_instance, "" + index)
}
;function Profiler() {
}
Profiler.monitor = function() {
};
Profiler.monitorAll = function() {
};
Profiler.dump = function() {
};var opensocial = opensocial || {};
opensocial.template = opensocial.template || {};
var os = opensocial.template;
os.log = function(msg) {
  var console = window.console;
  console && console.log && console.log(msg)
};
if(typeof log != "undefined")log = os.log;
else window.log = os.log;
os.warn = function(msg) {
  os.log("WARNING: " + msg)
};
os.ATT_customtag = "customtag";
os.VAR_my = "$my";
os.VAR_cur = "$cur";
os.VAR_node = "$node";
os.VAR_msg = "Msg";
os.VAR_parentnode = "$parentnode";
os.VAR_uniqueId = "$uniqueId";
os.VAR_identifierresolver = "$_ir";
os.VAR_callbacks = "$callbacks_";
os.regExps_ = {onlyWhitespace:/^[ \t\n]*$/};
os.compileTemplate = function(node, opt_id) {
  if(typeof node == "string")return os.compileTemplateString(node, opt_id);
  opt_id = opt_id || node.id;
  var src = node.value || node.innerHTML;
  src = os.trim(src);
  var template = os.compileTemplateString(src, opt_id);
  return template
};
os.compileTemplateString = function(src, opt_id) {
  src = os.prepareTemplateXML_(src);
  var doc = os.parseXML_(src);
  return os.compileXMLDoc(doc, opt_id)
};
os.renderTemplateNode_ = function(compiledNode, context) {
  var template = compiledNode.cloneNode(true);
  template.removeAttribute && template.removeAttribute(STRING_id);
  jstProcess(context, template);
  return template
};
os.elementIdCounter_ = 0;
os.createTemplateCustomTag = function(template) {
  return function(node, data, context) {
    context.setVariable(os.VAR_my, node);
    context.setVariable(os.VAR_node, node);
    context.setVariable(os.VAR_uniqueId, os.elementIdCounter_++);
    var ret = template.render(data, context);
    os.markNodeToSkip(ret);
    return ret
  }
};
os.createNodeAccessor_ = function(node) {
  return function(name) {
    return os.getValueFromNode_(node, name)
  }
};
os.gadgetPrefs_ = null;
if(window.gadgets && window.gadgets.Prefs)os.gadgetPrefs_ = new window.gadgets.Prefs;
os.getPrefMessage = function(key) {
  if(!os.gadgetPrefs_)return null;
  return os.gadgetPrefs_.getMsg(key)
};
os.globalDisallowedAttributes_ = {data:1};
os.customAttributes_ = {};
os.registerAttribute = function(attrName, functor) {
  os.customAttributes_[attrName] = functor
};
os.doAttribute = function(node, attrName, data, context) {
  var attrFunctor = os.customAttributes_[attrName];
  if(!attrFunctor)return;
  attrFunctor(node, node.getAttribute(attrName), data, context)
};
os.doTag = function(node, ns, tag, data, context) {
  var tagFunction = os.getCustomTag(ns, tag);
  if(!tagFunction) {
    os.warn("Custom tag <" + ns + ":" + tag + "> not defined.");
    return
  }for(var child = node.firstChild;child;child = child.nextSibling)if(child.nodeType == 1) {
    jstProcess(context, child);
    os.markNodeToSkip(child)
  }var result = tagFunction.call(null, node, data, context);
  if(!result && typeof result != "string")throw"Custom tag <" + ns + ":" + tag + "> failed to return anything.";if(typeof result == "string")node.innerHTML = result ? result : "";
  else if(isArray(result) && result.nodeType != 3) {
    os.removeChildren(node);
    for(var i = 0;i < result.length;i++)if(result[i].nodeType && (result[i].nodeType == 1 || result[i].nodeType == 3)) {
      node.appendChild(result[i]);
      result[i].nodeType == 1 && os.markNodeToSkip(result[i])
    }
  }else {
    var callbacks = context.getVariable(os.VAR_callbacks), resultNode = null;
    if(result.nodeType && result.nodeType == 1)resultNode = result;
    else if(result.root && result.root.nodeType && result.root.nodeType == 1)resultNode = result.root;
    if(resultNode && resultNode != node && (!resultNode.parentNode || result.parentNode.nodeType == 11)) {
      os.removeChildren(node);
      node.appendChild(resultNode);
      os.markNodeToSkip(resultNode)
    }result.onAttach && callbacks.push(result)
  }
};
os.setContextNode_ = function(data, context) {
  data.nodeType == 1 && context.setVariable(os.VAR_node, data)
};
os.markNodeToSkip = function(node) {
  node.setAttribute(ATT_skip, "true");
  node.removeAttribute(ATT_select);
  node.removeAttribute(ATT_eval);
  node.removeAttribute(ATT_values);
  node.removeAttribute(ATT_display);
  node[PROP_jstcache] = null;
  node.removeAttribute(ATT_jstcache)
};os.nsmap_ = {};
os.nsurls_ = {};
os.createNamespace = function(ns, url) {
  var tags = os.nsmap_[ns];
  if(!tags) {
    tags = {};
    os.nsmap_[ns] = tags;
    os.nsurls_[ns] = url
  }else if(os.nsurls_[ns] != url)throw"Namespace " + ns + " already defined with url " + os.nsurls_[ns];return tags
};
os.getNamespace = function(prefix) {
  return os.nsmap_[prefix]
};
os.addNamespace = function(ns, url, nsObj) {
  if(os.nsmap_[ns])throw"Namespace '" + ns + "' already exists!";os.nsmap_[ns] = nsObj;
  os.nsurls_[ns] = url
};
os.getCustomTag = function(ns, tag) {
  var nsObj = os.nsmap_[ns];
  if(!nsObj)return null;
  return nsObj.getTag ? nsObj.getTag(tag) : nsObj[tag]
};
os.getRequiredNamespaces = function(templateSrc) {
  var codeToInject = "";
  for(var ns in os.nsurls_)if(templateSrc.indexOf("<" + ns + ":") >= 0 && templateSrc.indexOf("xmlns:" + ns + ":") < 0)codeToInject += " xmlns:" + ns + '="' + os.nsurls_[ns] + '"';
  return codeToInject
};
os.defineBuiltinTags = function() {
  var osn = os.getNamespace("os") || os.createNamespace("os", "http://opensocial.com/#template");
  osn.Render = function(node, data, context) {
    var parent = context.getVariable(os.VAR_parentnode), exp = node.getAttribute("content") || "*", result = os.getValueFromNode_(parent, exp);
    if(!result)return"";
    else if(typeof result == "string") {
      var textNode = document.createTextNode(result);
      result = [];
      result.push(textNode)
    }else if(!isArray(result)) {
      var resultArray = [];
      for(var i = 0;i < result.childNodes.length;i++)resultArray.push(result.childNodes[i]);
      result = resultArray
    }else if(exp != "*" && result.length == 1 && result[0].nodeType == 1) {
      var resultArray = [];
      for(var i = 0;i < result[0].childNodes.length;i++)resultArray.push(result[0].childNodes[i]);
      result = resultArray
    }return result
  };
  osn.render = osn.RenderAll = osn.renderAll = osn.Render;
  osn.Html = function(node) {
    var html = node.code ? "" + node.code : node.getAttribute("code") || "";
    return html
  };
  function createClosure(object, method) {
    return function() {
      method.apply(object)
    }
  }
  function processOnAttach(node, code, data, context) {
    var callbacks = context.getVariable(os.VAR_callbacks), func = new Function(code);
    callbacks.push(createClosure(node, func))
  }
  os.registerAttribute("onAttach", processOnAttach)
};
os.defineBuiltinTags();os.trim = function(string) {
  return string.replace(/^\s+/, "").replace(/\s+$/, "")
};
os.isAlphaNum = function(ch) {
  return ch >= "a" && ch <= "z" || ch >= "A" && ch <= "Z" || ch >= "0" && ch <= "9" || ch == "_"
};
os.removeChildren = function(node) {
  while(node.firstChild)node.removeChild(node.firstChild)
};
os.appendChildren = function(sourceNode, targetNode) {
  while(sourceNode.firstChild)targetNode.appendChild(sourceNode.firstChild)
};
os.replaceNode = function(node, replacement) {
  var parent = node.parentNode;
  if(!parent)throw"Error in replaceNode() - Node has no parent: " + node;if(replacement.nodeType == 1 || replacement.nodeType == 3)parent.replaceChild(replacement, node);
  else if(isArray(replacement)) {
    for(var i = 0;i < replacement.length;i++)parent.insertBefore(replacement[i], node);
    parent.removeChild(node)
  }
};
os.getPropertyGetterName = function(propertyName) {
  var getter = "get" + propertyName.charAt(0).toUpperCase() + propertyName.substring(1);
  return getter
};
os.convertToCamelCase = function(str) {
  var words = str.toLowerCase().split("_"), out = [];
  out.push(words[0].toLowerCase());
  for(var i = 1;i < words.length;++i) {
    var piece = words[i].charAt(0).toUpperCase() + words[i].substring(1);
    out.push(piece)
  }return out.join("")
};os.Template = function(opt_id) {
  this.templateRoot_ = document.createElement("span");
  this.id = opt_id || "template_" + os.Template.idCounter_++
};
os.Template.idCounter_ = 0;
os.registeredTemplates_ = {};
os.registerTemplate = function(template) {
  os.registeredTemplates_[template.id] = template
};
os.unRegisterTemplate = function(template) {
  delete os.registeredTemplates_[template.id]
};
os.getTemplate = function(templateId) {
  return os.registeredTemplates_[templateId]
};
os.Template.prototype.setCompiledNodes_ = function(nodes) {
  os.removeChildren(this.templateRoot_);
  for(var i = 0;i < nodes.length;i++)this.templateRoot_.appendChild(nodes[i])
};
os.Template.prototype.render = function(opt_data, opt_context) {
  opt_context || (opt_context = os.createContext(opt_data));
  return os.renderTemplateNode_(this.templateRoot_, opt_context)
};
os.Template.prototype.renderInto = function(root, opt_data, opt_context) {
  opt_context || (opt_context = os.createContext(opt_data));
  var result = this.render(opt_data, opt_context);
  os.removeChildren(root);
  os.appendChildren(result, root);
  os.fireCallbacks(opt_context)
};os.SEMICOLON = ";";
os.isIe = navigator.userAgent.indexOf("Opera") != 0 && navigator.userAgent.indexOf("MSIE") != -1;
os.compileXMLNode = function(node, opt_id) {
  var nodes = [];
  for(var child = node.firstChild;child;child = child.nextSibling)if(child.nodeType == 1)nodes.push(os.compileNode_(child));
  else if(child.nodeType == 3)if(!child.nodeValue.match(os.regExps_.onlyWhitespace)) {
    var compiled = os.breakTextNode_(child);
    for(var i = 0;i < compiled.length;i++)nodes.push(compiled[i])
  }var template = new os.Template(opt_id);
  template.setCompiledNodes_(nodes);
  return template
};
os.compileXMLDoc = function(doc, opt_id) {
  var node = doc.firstChild;
  while(node.nodeType != 1)node = node.nextSibling;
  return os.compileXMLNode(node, opt_id)
};
os.parseXML_ = function(str) {
  if(typeof DOMParser != "undefined") {
    os.parser_ = os.parser_ || new DOMParser;
    var doc = os.parser_.parseFromString(str, "text/xml");
    if(doc.firstChild && doc.firstChild.tagName == "parsererror")throw doc.firstChild.firstChild.nodeValue;return doc
  }else {
    var doc = new ActiveXObject("MSXML2.DomDocument");
    doc.validateOnParse = false;
    doc.loadXML(str);
    if(doc.parseError && doc.parseError.errorCode)throw doc.parseError.reason;return doc
  }
};
os.operatorMap = {and:"&&", eq:"==", lte:"<=", lt:"<", gte:">=", gt:">", neq:"!=", or:"||", not:"!"};
os.remapOperators_ = function(src) {
  var out = "", sub = "";
  for(var i = 0;i < src.length;++i) {
    var c = src.charAt(i);
    if(os.isAlphaNum(c))sub += c;
    else {
      if(sub.length > 0) {
        if(sub.length < 4)sub = os.operatorMap[sub.toLowerCase()] || sub;
        out += sub;
        sub = ""
      }out += c
    }
  }out += sub;
  return out
};
os.transformVariables_ = function(expr) {
  expr = os.replaceTopLevelVars_(expr);
  return expr
};
os.variableMap_ = {my:os.VAR_my, My:os.VAR_my, cur:VAR_this, Cur:VAR_this, $cur:VAR_this, Top:VAR_top, Index:VAR_index, Count:VAR_count};
os.replaceTopLevelVars_ = function(text) {
  text = " " + text;
  var regex = /([^.$a-zA-Z0-9])([$a-zA-Z0-9]+)/g, results, dest = "", index = 0;
  while((results = regex.exec(text)) != null) {
    dest += text.substring(index, regex.lastIndex - results[0].length);
    dest += results[1];
    dest += results[2] in os.variableMap_ ? os.variableMap_[results[2]] : results[2];
    index = regex.lastIndex
  }dest += text.substring(index, text.length);
  return dest.substring(1)
};
os.identifierResolver_ = function(data, name) {
  return data[name]
};
os.setIdentifierResolver = function(resolver) {
  os.identifierResolver_ = resolver
};
os.getFromContext = function(context, name, opt_default) {
  var ret;
  if(context.vars_ && context.data_) {
    if(context.data_.nodeType == 1) {
      ret = os.getValueFromNode_(context.data_, name);
      if(ret == null) {
        var und;
        ret = und
      }
    }else ret = os.identifierResolver_(context.data_, name);
    if(typeof ret == "undefined")ret = os.identifierResolver_(context.vars_, name)
  }else ret = context.nodeType == 1 ? os.getValueFromNode_(context, name) : os.identifierResolver_(context, name);
  if(typeof ret == "undefined" || ret == null)ret = typeof opt_default != "undefined" ? opt_default : "";
  return ret
};
os.transformExpression_ = function(expr, opt_default) {
  expr = os.remapOperators_(expr);
  expr = os.transformVariables_(expr);
  if(os.identifierResolver_)expr = os.wrapIdentifiersInExpression(expr, opt_default);
  return expr
};
os.attributeMap_ = {"if":"jsdisplay", repeat:"jsselect", context:"jsselect"};
os.appendJSTAttribute_ = function(node, attrName, value) {
  var previousValue = node.getAttribute(attrName);
  if(previousValue)value = previousValue + ";" + value;
  node.setAttribute(attrName, value)
};
os.copyAttributes_ = function(from, to, opt_customTag) {
  var dynamicAttributes = null;
  for(var i = 0;i < from.attributes.length;i++) {
    var name = from.attributes[i].nodeName, value = from.getAttribute(name);
    if(name && value)if(name == "var")os.appendJSTAttribute_(to, ATT_vars, from.getAttribute(name) + ": $this");
    else if(name == "index")os.appendJSTAttribute_(to, ATT_vars, from.getAttribute(name) + ": $index");
    else if(name.length < 7 || name.substring(0, 6) != "xmlns:") {
      if(os.customAttributes_[name])os.appendJSTAttribute_(to, ATT_eval, "os.doAttribute(this, '" + name + "', $this, $context)");
      else name == "repeat" && os.appendJSTAttribute_(to, ATT_eval, "os.setContextNode_($this, $context)");
      var outName = os.attributeMap_[name] || name, substitution = os.attributeMap_[name] || opt_customTag && os.globalDisallowedAttributes_[outName] ? null : os.parseAttribute_(value);
      if(substitution) {
        if(outName == "class")outName = ".className";
        else if(outName == "style")outName = ".style.cssText";
        else if(to.getAttribute(os.ATT_customtag))outName = "." + outName;
        else if(os.isIe && !os.customAttributes_[outName] && outName.substring(0, 2).toLowerCase() == "on") {
          outName = "." + outName;
          substitution = "new Function(" + substitution + ")"
        }dynamicAttributes || (dynamicAttributes = []);
        dynamicAttributes.push(outName + ":" + substitution)
      }else {
        if(os.attributeMap_[name]) {
          if(value.length > 3 && value.substring(0, 2) == "${" && value.charAt(value.length - 1) == "}")value = value.substring(2, value.length - 1);
          value = os.transformExpression_(value, "null")
        }else if(outName == "class")to.setAttribute("className", value);
        else if(outName == "style")to.style.cssText = value;
        os.isIe && !os.customAttributes_[outName] && outName.substring(0, 2).toLowerCase() == "on" ? to.attachEvent(outName, new Function(value)) : to.setAttribute(outName, value)
      }
    }
  }dynamicAttributes && os.appendJSTAttribute_(to, ATT_values, dynamicAttributes.join(";"))
};
os.compileNode_ = function(node) {
  if(node.nodeType == 3) {
    var textNode = node.cloneNode(false);
    return os.breakTextNode_(textNode)
  }else if(node.nodeType == 1) {
    var output;
    if(node.tagName.indexOf(":") > 0) {
      output = document.createElement("span");
      output.setAttribute(os.ATT_customtag, node.tagName);
      var custom = node.tagName.split(":");
      os.appendJSTAttribute_(output, ATT_eval, 'os.doTag(this, "' + custom[0] + '", "' + custom[1] + '", $this, $context)');
      var context = node.getAttribute("context") || "$this||true";
      output.setAttribute(ATT_select, context);
      if(node.tagName == "os:render" || node.tagName == "os:Render" || node.tagName == "os:renderAll" || node.tagName == "os:RenderAll")os.appendJSTAttribute_(output, ATT_values, os.VAR_parentnode + ":" + os.VAR_node);
      os.copyAttributes_(node, output, node.tagName)
    }else output = os.xmlToHtml_(node);
    if(output && !os.processTextContent_(node, output))for(var child = node.firstChild;child;child = child.nextSibling) {
      var compiledChild = os.compileNode_(child);
      if(compiledChild)if(!compiledChild.tagName && typeof compiledChild.length == "number")for(var i = 0;i < compiledChild.length;i++)output.appendChild(compiledChild[i]);
      else if(compiledChild.tagName == "TR" && output.tagName == "TABLE") {
        var lastEl = output.lastChild;
        while(lastEl && lastEl.nodeType != 1 && lastEl.previousSibling)lastEl = lastEl.previousSibling;
        if(!lastEl || lastEl.tagName != "TBODY") {
          lastEl = document.createElement("tbody");
          output.appendChild(lastEl)
        }lastEl.appendChild(compiledChild)
      }else output.appendChild(compiledChild)
    }return output
  }return null
};
os.ENTITIES = '<!ENTITY nbsp "&#160;">';
os.prepareTemplateXML_ = function(templateSrc) {
  var namespaces = os.getRequiredNamespaces(templateSrc);
  return"<!DOCTYPE root [" + os.ENTITIES + "]><root " + namespaces + ">" + templateSrc + "</root>"
};
os.xmlToHtml_ = function(xmlNode) {
  var htmlNode = document.createElement(xmlNode.tagName);
  os.copyAttributes_(xmlNode, htmlNode);
  return htmlNode
};
os.createContext = function(data, opt_globals) {
  var context = JsEvalContext.create(data);
  context.setVariable(os.VAR_callbacks, []);
  context.setVariable(os.VAR_identifierresolver, os.getFromContext);
  if(opt_globals)for(var global in opt_globals)context.setVariable(global, opt_globals[global]);
  return context
};
os.fireCallbacks = function(context) {
  var callbacks = context.getVariable(os.VAR_callbacks);
  while(callbacks.length > 0) {
    var callback = callbacks.pop();
    callback.onAttach ? callback.onAttach() : callback()
  }
};
os.processTextContent_ = function(fromNode, toNode) {
  if(fromNode.childNodes.length == 1 && !toNode.getAttribute(os.ATT_customtag) && fromNode.firstChild.nodeType == 3) {
    var substitution = os.parseAttribute_(fromNode.firstChild.data);
    substitution ? toNode.setAttribute(ATT_content, substitution) : toNode.appendChild(document.createTextNode(os.trimWhitespaceForIE_(fromNode.firstChild.data, true, true)));
    return true
  }return false
};
os.pushTextNode = function(array, text) {
  text.length > 0 && array.push(document.createTextNode(text))
};
os.trimWhitespaceForIE_ = function(string, opt_trimStart, opt_trimEnd) {
  if(os.isIe) {
    var ret = string.replace(/\n/g, " ").replace(/\s+/g, " ");
    if(opt_trimStart)ret = ret.replace(/^\s/, "");
    if(opt_trimEnd)ret = ret.replace(/\s$/, "");
    return ret
  }return string
};
os.breakTextNode_ = function(textNode) {
  var substRex = /^([^$]*)(\$\{[^\}]*\})([\w\W]*)$/, text = textNode.data, nodes = [], match = text.match(substRex);
  while(match) {
    match[1].length > 0 && os.pushTextNode(nodes, os.trimWhitespaceForIE_(match[1]));
    var token = match[2].substring(2, match[2].length - 1);
    token || (token = "$this");
    var tokenSpan = document.createElement("span");
    tokenSpan.setAttribute(ATT_content, os.transformExpression_(token));
    nodes.push(tokenSpan);
    match = text.match(substRex);
    text = match[3];
    match = text.match(substRex)
  }text.length > 0 && os.pushTextNode(nodes, os.trimWhitespaceForIE_(text));
  return nodes
};
os.transformLiteral_ = function(string) {
  return"'" + string.replace(/'/g, "\\'").replace(/\n/g, " ").replace(/;/g, "'+os.SEMICOLON+'") + "'"
};
os.parseAttribute_ = function(value) {
  if(!value.length)return null;
  var substRex = /^([^$]*)(\$\{[^\}]*\})([\w\W]*)$/, text = value, parts = [], match = text.match(substRex);
  if(!match)return null;
  while(match) {
    match[1].length > 0 && parts.push(os.transformLiteral_(os.trimWhitespaceForIE_(match[1], parts.length == 0)));
    var expr = match[2].substring(2, match[2].length - 1);
    parts.push("(" + os.transformExpression_(expr) + ")");
    text = match[3];
    match = text.match(substRex)
  }text.length > 0 && parts.push(os.transformLiteral_(os.trimWhitespaceForIE_(text, false, true)));
  return parts.join("+")
};
os.getValueFromObject_ = function(object, name) {
  return name ? object.nodeType == 1 ? os.getValueFromNode_(object, name) : object[name] : object
};
os.getValueFromNode_ = function(node, name) {
  var ret = node[name] || node.getAttribute(name);
  if(!ret) {
    if(name)name = name.toLowerCase();
    var allChildren = "*" == name;
    ret = [];
    for(var child = node.firstChild;child;child = child.nextSibling) {
      if(allChildren) {
        ret.push(child);
        continue
      }if(child.nodeType != 1)continue;
      var tagName = child.getAttribute(os.ATT_customtag);
      if(!tagName)tagName = child.tagName;
      tagName = tagName.toLowerCase();
      tagName == name && ret.push(child)
    }if(ret.length == 0)ret = null;
    var content;
    if(ret == null && allChildren && (content = node.getAttribute(ATT_content))) {
      var span = document.createElement("span");
      span.setAttribute(ATT_content, content);
      ret = [span]
    }
  }if(ret == "false")ret = false;
  else if(ret == "0")ret = 0;
  return ret
};
os.identifiersNotToWrap_ = {};
os.identifiersNotToWrap_["true"] = true;
os.identifiersNotToWrap_["false"] = true;
os.identifiersNotToWrap_["null"] = true;
os.identifiersNotToWrap_["var"] = true;
os.identifiersNotToWrap_[os.VAR_my] = true;
os.identifiersNotToWrap_[VAR_this] = true;
os.identifiersNotToWrap_[VAR_context] = true;
os.identifiersNotToWrap_[VAR_top] = true;
os.identifiersNotToWrap_[VAR_index] = true;
os.identifiersNotToWrap_[VAR_count] = true;
os.canStartIdentifier = function(ch) {
  return ch >= "a" && ch <= "z" || ch >= "A" && ch <= "Z" || ch == "_" || ch == "$"
};
os.canBeInIdentifier = function(ch) {
  return os.canStartIdentifier(ch) || ch >= "0" && ch <= "9" || ch == "-" || ch == ":"
};
os.canBeInToken = function(ch) {
  return os.canBeInIdentifier(ch) || ch == "(" || ch == ")" || ch == "[" || ch == "]" || ch == "."
};
os.wrapSingleIdentifier = function(iden, opt_context, opt_default) {
  if(os.identifiersNotToWrap_[iden])return iden;
  return os.VAR_identifierresolver + "(" + (opt_context || VAR_context) + ", '" + iden + "'" + (opt_default ? ", " + opt_default : "") + ")"
};
os.wrapIdentifiersInToken = function(token, opt_default) {
  if(!os.canStartIdentifier(token.charAt(0)))return token;
  if(token.substring(0, os.VAR_msg.length + 1) == os.VAR_msg + "." && os.gadgetPrefs_) {
    var key = token.split(".")[1], msg = os.getPrefMessage(key) || "";
    return os.parseAttribute_(msg) || os.transformLiteral_(msg)
  }var identifiers = os.tokenToIdentifiers(token), parts = false, buffer = [], output = null;
  for(var i = 0;i < identifiers.length;i++) {
    var iden = identifiers[i];
    parts = os.breakUpParens(iden);
    if(parts) {
      buffer.length = 0;
      buffer.push(os.wrapSingleIdentifier(parts[0], output));
      for(var j = 1;j < parts.length;j += 3) {
        buffer.push(parts[j]);
        parts[j + 1] && buffer.push(os.wrapIdentifiersInExpression(parts[j + 1]));
        buffer.push(parts[j + 2])
      }output = buffer.join("")
    }else output = os.wrapSingleIdentifier(iden, output, opt_default)
  }return output
};
os.wrapIdentifiersInExpression = function(expr, opt_default) {
  var out = [], tokens = os.expressionToTokens(expr);
  for(var i = 0;i < tokens.length;i++)out.push(os.wrapIdentifiersInToken(tokens[i], opt_default));
  return out.join("")
};
os.expressionToTokens = function(expr) {
  var tokens = [], inquotes = false, inidentifier = false, inparens = 0, escaped = false, quotestart = null, buffer = [];
  for(var i = 0;i < expr.length;i++) {
    var ch = expr.charAt(i);
    if(inquotes) {
      if(!escaped && ch == quotestart)inquotes = false;
      else escaped = ch == "\\" ? true : false;
      buffer.push(ch)
    }else {
      if(ch == "'" || ch == '"') {
        inquotes = true;
        quotestart = ch;
        buffer.push(ch);
        continue
      }if(!inquotes && ch == "(")inparens++;
      else!inquotes && ch == ")" && inparens > 0 && inparens--;
      if(inparens > 0) {
        buffer.push(ch);
        continue
      }if(!inidentifier && os.canStartIdentifier(ch)) {
        if(buffer.length > 0) {
          tokens.push(buffer.join(""));
          buffer.length = 0
        }inidentifier = true;
        buffer.push(ch);
        continue
      }if(inidentifier)if(os.canBeInToken(ch))buffer.push(ch);
      else {
        tokens.push(buffer.join(""));
        buffer.length = 0;
        inidentifier = false;
        buffer.push(ch)
      }else buffer.push(ch)
    }
  }tokens.push(buffer.join(""));
  return tokens
};
os.tokenToIdentifiers = function(token) {
  var inquotes = false, quotestart = null, escaped = false, buffer = [], identifiers = [];
  for(var i = 0;i < token.length;i++) {
    var ch = token.charAt(i);
    if(inquotes) {
      if(!escaped && ch == quotestart)inquotes = false;
      else escaped = ch == "\\" ? true : false;
      buffer.push(ch);
      continue
    }else if(ch == "'" || ch == '"') {
      buffer.push(ch);
      inquotes = true;
      quotestart = ch;
      continue
    }if(ch == "." && !inquotes) {
      identifiers.push(buffer.join(""));
      buffer.length = 0;
      continue
    }buffer.push(ch)
  }identifiers.push(buffer.join(""));
  return identifiers
};
os.breakUpParens = function(identifier) {
  var parenIndex = identifier.indexOf("("), bracketIndex = identifier.indexOf("[");
  if(parenIndex < 0 && bracketIndex < 0)return false;
  var parts = [];
  if(parenIndex < 0 || bracketIndex >= 0 && bracketIndex < parenIndex) {
    parenIndex = 0;
    parts.push(identifier.substring(0, bracketIndex))
  }else {
    bracketIndex = 0;
    parts.push(identifier.substring(0, parenIndex))
  }var parenstart = null, inquotes = false, quotestart = null, parenlevel = 0, escaped = false, buffer = [];
  for(var i = bracketIndex + parenIndex;i < identifier.length;i++) {
    var ch = identifier.charAt(i);
    if(inquotes) {
      if(!escaped && ch == quotestart)inquotes = false;
      else escaped = ch == "\\" ? true : false;
      buffer.push(ch)
    }else {
      if(ch == "'" || ch == '"') {
        inquotes = true;
        quotestart = ch;
        buffer.push(ch);
        continue
      }if(parenlevel == 0) {
        if(ch == "(" || ch == "[") {
          parenstart = ch;
          parenlevel++;
          parts.push(ch);
          buffer.length = 0
        }
      }else if(parenstart == "(" && ch == ")" || parenstart == "[" && ch == "]") {
        parenlevel--;
        if(parenlevel == 0) {
          parts.push(buffer.join(""));
          parts.push(ch)
        }else buffer.push(ch)
      }else {
        ch == parenstart && parenlevel++;
        buffer.push(ch)
      }
    }
  }return parts
};os.Loader = {};
os.Loader.loadedUrls_ = {};
os.Loader.loadUrl = function(url, callback) {
  typeof window.gadgets != "undefined" ? os.Loader.requestUrlGadgets_(url, callback) : os.Loader.requestUrlXHR_(url, callback)
};
os.Loader.requestUrlXHR_ = function(url, callback) {
  if(os.Loader.loadedUrls_[url]) {
    window.setTimeout(callback, 0);
    return
  }var req = null;
  req = typeof XMLHttpRequest != "undefined" ? new XMLHttpRequest : new ActiveXObject("MSXML2.XMLHTTP");
  req.open("GET", url, true);
  req.onreadystatechange = function() {
    if(req.readyState == 4) {
      os.Loader.loadContent(req.responseText);
      os.Loader.loadedUrls_[url] = true;
      callback()
    }
  };
  req.send(null)
};
os.Loader.requestUrlGadgets_ = function(url, callback) {
  var params = {}, gadgets = window.gadgets;
  if(os.Loader.loadedUrls_[url]) {
    window.setTimeout(callback, 0);
    return
  }params[gadgets.io.RequestParameters.CONTENT_TYPE] = gadgets.io.ContentType.TEXT;
  gadgets.io.makeRequest(url, function(obj) {
    os.Loader.loadContent(obj.data);
    os.Loader.loadedUrls_[url] = true;
    callback()
  }, params)
};
os.Loader.loadUrls = function(urls, callback) {
  var loadOne = function() {
    urls.length == 0 ? callback() : os.Loader.loadUrl(urls.pop(), loadOne)
  };
  loadOne()
};
os.Loader.loadContent = function(xmlString) {
  var doc = os.parseXML_(xmlString), templatesNode = doc.firstChild;
  os.Loader.processTemplatesNode(templatesNode)
};
os.Loader.getProcessorFunction_ = function(tagName) {
  return os.Loader["process" + tagName + "Node"] || null
};
os.Loader.processTemplatesNode = function(node) {
  for(var child = node.firstChild;child;child = child.nextSibling)if(child.nodeType == 1) {
    var handler = os.Loader.getProcessorFunction_(child.tagName);
    handler && handler(child)
  }
};
os.Loader.processNamespaceNode = function(node) {
  var prefix = node.getAttribute("prefix"), url = node.getAttribute("url");
  os.createNamespace(prefix, url)
};
os.Loader.processTemplateDefNode = function(node) {
  var tag = node.getAttribute("tag");
  for(var child = node.firstChild;child;child = child.nextSibling)if(child.nodeType == 1) {
    var handler = os.Loader.getProcessorFunction_(child.tagName);
    handler && handler(child, tag)
  }
};
os.Loader.processTemplateNode = function(node, opt_tag) {
  var tag = opt_tag || node.getAttribute("tag");
  if(tag) {
    var tagParts = tag.split(":");
    if(tagParts.length != 2)throw"Invalid tag name: " + tag;var nsObj = os.getNamespace(tagParts[0]);
    if(!nsObj)throw"Namespace not registered: " + tagParts[0] + " while trying to define " + tag;var template = os.compileXMLNode(node);
    nsObj[tagParts[1]] = os.createTemplateCustomTag(template)
  }
};
os.Loader.processJavaScriptNode = function(node) {
  for(var contentNode = node.firstChild;contentNode;contentNode = contentNode.nextSibling)os.Loader.injectJavaScript(contentNode.nodeValue)
};
os.Loader.processStyleNode = function(node) {
  for(var contentNode = node.firstChild;contentNode;contentNode = contentNode.nextSibling)os.Loader.injectStyle(contentNode.nodeValue)
};
os.Loader.headNode_ = document.getElementsByTagName("head")[0] || document.getElementsByTagName("*")[0];
os.Loader.injectJavaScript = function(jsCode) {
  var scriptNode = document.createElement("script");
  scriptNode.type = "text/javascript";
  scriptNode.text = jsCode;
  os.Loader.headNode_.appendChild(scriptNode)
};
os.Loader.injectStyle = function(cssCode) {
  var sheet;
  document.styleSheets.length == 0 && document.getElementsByTagName("head")[0].appendChild(document.createElement("style"));
  sheet = document.styleSheets[0];
  var rules = cssCode.split("}");
  for(var i = 0;i < rules.length;i++) {
    var rule = rules[i].replace(/\n/g, "").replace(/\s+/g, " ");
    if(rule.length > 2)if(sheet.insertRule) {
      rule = rule + "}";
      sheet.insertRule(rule, sheet.cssRules.length)
    }else {
      var ruleParts = rule.split("{");
      sheet.addRule(ruleParts[0], ruleParts[1])
    }
  }
};os.Container = {};
os.Container.inlineTemplates_ = [];
os.Container.domLoadCallbacks_ = null;
os.Container.domLoaded_ = false;
os.Container.registerDomLoadListener_ = function() {
  navigator.product == "Gecko" && window.addEventListener("DOMContentLoaded", os.Container.onDomLoad_, false);
  if(window.addEventListener)window.addEventListener("load", os.Container.onDomLoad_, false);
  else {
    if(!document.body) {
      setTimeout(arguments.callee, 0);
      return
    }var oldOnLoad = window.onload || function() {
    };
    window.onload = function() {
      oldOnLoad();
      os.Container.onDomLoad_()
    }
  }
};
os.Container.onDomLoad_ = function() {
  if(os.Container.domLoaded_)return;
  while(os.Container.domLoadCallbacks_.length)os.Container.domLoadCallbacks_.pop()();
  os.Container.domLoaded_ = true
};
os.Container.executeOnDomLoad = function(callback) {
  if(os.Container.domLoaded_)setTimeout(callback, 0);
  else {
    if(os.Container.domLoadCallbacks_ == null) {
      os.Container.domLoadCallbacks_ = [];
      os.Container.registerDomLoadListener_()
    }os.Container.domLoadCallbacks_.push(callback)
  }
};
os.Container.registerDocumentTemplates = function(opt_doc) {
  var doc = opt_doc || document, nodes = doc.getElementsByTagName(os.Container.TAG_script_);
  for(var i = 0;i < nodes.length;++i) {
    var node = nodes[i];
    if(os.Container.isTemplateType_(node.type)) {
      var tag = node.getAttribute("tag");
      if(tag)os.Container.registerTagElement_(node, tag);
      else node.getAttribute("name") && os.Container.registerTemplateElement_(node, node.getAttribute("name"))
    }
  }
};
os.Container.executeOnDomLoad(os.Container.registerDocumentTemplates);
os.Container.compileInlineTemplates = function(opt_data, opt_doc) {
  var doc = opt_doc || document, nodes = doc.getElementsByTagName(os.Container.TAG_script_);
  for(var i = 0;i < nodes.length;++i) {
    var node = nodes[i];
    if(os.Container.isTemplateType_(node.type)) {
      var name = node.getAttribute("name") || node.getAttribute("tag");
      if(!name || name.length < 0) {
        var template = os.compileTemplate(node);
        template ? os.Container.inlineTemplates_.push({template:template, node:node}) : os.warn("Failed compiling inline template.")
      }
    }
  }
};
os.Container.renderInlineTemplates = function(opt_data, opt_doc) {
  var doc = opt_doc || document, inlined = os.Container.inlineTemplates_;
  for(var i = 0;i < inlined.length;++i) {
    var template = inlined[i].template, node = inlined[i].node, id = "_T_" + template.id, el = doc.getElementById(id);
    if(!el) {
      el = doc.createElement("div");
      el.setAttribute("id", id);
      node.parentNode.insertBefore(el, node)
    }var beforeData = node.getAttribute("beforeData");
    if(beforeData) {
      var keys = beforeData.split(/[\, ]+/);
      os.data.DataContext.registerListener(keys, os.createHideElementClosure(el))
    }var requiredData = node.getAttribute("requireData");
    if(requiredData) {
      var keys = requiredData.split(/[\, ]+/);
      os.data.DataContext.registerListener(keys, os.createRenderClosure(template, el, os.data.DataContext))
    }else template.renderInto(el, opt_data)
  }
};
os.Container.registerTemplate = function(elementId) {
  var element = document.getElementById(elementId);
  return os.Container.registerTemplateElement_(element)
};
os.Container.registerTag = function(elementId) {
  var element = document.getElementById(elementId);
  os.Container.registerTagElement_(element, elementId)
};
os.Container.renderElement = function(elementId, templateId, opt_data) {
  var template = os.getTemplate(templateId);
  if(template) {
    var element = document.getElementById(elementId);
    element ? template.renderInto(element, opt_data) : os.warn("Element (" + elementId + ") not found to render into.")
  }else os.warn("Template (" + templateId + ") not registered.")
};
os.Container.loadDataRequests = function(opt_doc) {
  var doc = opt_doc || document, nodes = doc.getElementsByTagName(os.Container.TAG_script_);
  for(var i = 0;i < nodes.length;++i) {
    var node = nodes[i];
    node.type == os.Container.dataType_ && os.data.loadRequests(node)
  }os.data.executeRequests()
};
os.Container.processInlineTemplates = function(opt_data, opt_doc) {
  var data = opt_data || os.data.DataContext;
  os.Container.compileInlineTemplates(opt_doc);
  os.Container.renderInlineTemplates(data, opt_doc)
};
os.Container.processDocument = function(opt_data, opt_doc) {
  os.Container.loadDataRequests(opt_doc);
  os.Container.registerDocumentTemplates(opt_doc);
  os.Container.processInlineTemplates(opt_data, opt_doc)
};
os.Container.TAG_script_ = "script";
os.Container.templateTypes_ = {};
os.Container.templateTypes_["text/os-template"] = true;
os.Container.templateTypes_["text/template"] = true;
os.Container.dataType_ = "text/os-data";
os.Container.isTemplateType_ = function(typeName) {
  return os.Container.templateTypes_[typeName] != null
};
os.Container.registerTemplateElement_ = function(element, opt_id) {
  var template = os.compileTemplate(element, opt_id);
  template ? os.registerTemplate(template) : os.warn("Could not compile template (" + element.id + ")");
  return template
};
os.Container.registerTagElement_ = function(element, name) {
  var template = os.Container.registerTemplateElement_(element);
  if(template) {
    var tagParts = name.split(":"), nsObj = os.getNamespace(tagParts[0]);
    if(nsObj)nsObj[tagParts[1]] = os.createTemplateCustomTag(template);
    else os.warn("Namespace " + tagParts[0] + " is not registered.")
  }
};os.resolveOpenSocialIdentifier = function(object, name) {
  if(typeof object[name] != "undefined")return object[name];
  var functionName = os.getPropertyGetterName(name);
  if(object[functionName])return object[functionName]();
  if(object.getField) {
    var fieldData = object.getField(name);
    if(fieldData)return fieldData
  }if(object.get) {
    var responseItem = object.get(name);
    if(responseItem && responseItem.getData) {
      var data = responseItem.getData();
      return data.array_ || data
    }return responseItem
  }var und;
  return und
};
os.setIdentifierResolver(os.resolveOpenSocialIdentifier);
os.createOpenSocialGetMethods_ = function(object, fields) {
  if(object && fields)for(var key in fields) {
    var value = fields[key], getter = os.getPropertyGetterName(value);
    object.prototype[getter] = function() {
      this.getField(key)
    }
  }
};
os.registerOpenSocialFields_ = function() {
};
os.registerOpenSocialFields_();window.opensocial.data = window.opensocial.data || {};
var gadgets = window.gadgets;
os.data = window.opensocial.data;
os.data.requests_ = {};
os.data.registerRequestHandler = function(name, handler) {
  var tagParts = name.split(":"), ns = os.getNamespace(tagParts[0]);
  if(!ns)throw"Namespace " + tagParts[0] + " is undefined.";else if(ns[tagParts[1]])throw"Request handler " + tagParts[1] + " is already defined.";ns[tagParts[1]] = handler
};
os.data.loadRequests = function(xmlData) {
  if(typeof xmlData != "string") {
    var node = xmlData;
    xmlData = node.value || node.innerHTML
  }xmlData = os.prepareTemplateXML_(xmlData);
  var doc = os.parseXML_(xmlData), node = doc.firstChild;
  while(node.nodeType != 1)node = node.nextSibling;
  os.data.processDataNode_(node)
};
os.data.executeRequests = function() {
  for(var nsName in os.data.requests_) {
    var requestList = os.data.requests_[nsName];
    if(nsName == "os") {
      var req = opensocial.newDataRequest();
      for(var key in requestList) {
        var tag = requestList[key], callback = os.getCustomTag("os", tag.tagParts[1]);
        req.add(callback(req, tag), key)
      }req.send(function(response) {
        if(!response || response.hadError())throw"Unexpected error with OpenSocial data request.";else for(var key in requestList) {
          var responseItem = response.get(key);
          if(responseItem)if(responseItem.hadError())throw"Response error(" + responseItem.getErrorCode() + "): " + responseItem.getErrorMessage();else os.data.DataContext.putDataSet(key, response);
          else throw"Request for " + key + " could not be loaded.";
        }
      })
    }else for(var key in requestList) {
      var tag = requestList[key], callback = os.getCustomTag(nsName, tag.tagParts[1]);
      callback(key, tag)
    }
  }delete os.data.requests_;
  os.data.requests_ = {}
};
os.data.processDataNode_ = function(node) {
  for(var child = node.firstChild;child;child = child.nextSibling)child.nodeType == 1 && child.tagName == "os:dataSet" && os.data.processDataSet_(child)
};
os.data.processDataSet_ = function(node) {
  for(var child = node.firstChild;child;child = child.nextSibling)if(child.nodeType == 1) {
    var key = node.getAttribute("key"), tag = new os.data.DataRequestTag(child);
    os.data.requests_[tag.tagParts[0]] || (os.data.requests_[tag.tagParts[0]] = {});
    var requestList = os.data.requests_[tag.tagParts[0]];
    requestList[key] = tag;
    break
  }
};
os.data.listeners_ = [];
os.data.checkListener_ = function(listener) {
  for(var key in listener.keys)if(os.data.DataContext[key] == null)return false;
  return true
};
os.data.DataRequestTag = function(xmlNode) {
  this.tagName = xmlNode.tagName;
  this.tagParts = this.tagName.split(":");
  this.attributes = {};
  for(var i = 0;i < xmlNode.attributes.length;++i) {
    var name = xmlNode.attributes[i].nodeName;
    if(name) {
      var value = xmlNode.getAttribute(name);
      if(name && value)this.attributes[name] = value
    }
  }
};
os.data.DataRequestTag.prototype.hasAttribute = function(name) {
  return!!this.attributes[name]
};
os.data.DataRequestTag.prototype.getAttribute = function(name) {
  var attrExpression = this.attributes[name];
  if(!attrExpression)return attrExpression;
  var expression = os.parseAttribute_(attrExpression);
  if(!expression)return attrExpression;
  return os.data.DataContext.evalExpression(expression)
};
os.data.DataContext = {};
os.data.DataContext.registerListener = function(keys, callback) {
  var listener = {};
  listener.keys = {};
  if(typeof keys == "object")for(var i in keys)listener.keys[keys[i]] = true;
  else listener.keys[keys] = true;
  listener.callback = callback;
  os.data.listeners_.push(listener);
  os.data.checkListener_(listener) && window.setTimeout(function() {
    listener.callback()
  }, 1)
};
os.data.DataContext.getDataSet = function(key) {
  return os.data.DataContext[key]
};
os.data.DataContext.putDataSet = function(key, obj) {
  var data = obj;
  if(typeof data == "undefined" || data === null)return;
  if(obj.get) {
    var responseItem = obj.get(key);
    if(responseItem && responseItem.getData) {
      data = responseItem.getData();
      data = data.array_ || data
    }
  }os.data.DataContext[key] = data;
  os.data.fireCallbacks_(key)
};
os.data.DataContext.putDataResult = function(key, handler) {
  handler(function(obj) {
    os.data.DataContext.putDataSet(key, obj)
  })
};
os.data.DataContext.evalContext_ = os.createContext(os.data.DataContext);
os.data.DataContext.evalExpression = function(expr) {
  return os.data.DataContext.evalContext_.evalExpression(expr)
};
os.data.fireCallbacks_ = function(key) {
  for(var i = 0;i < os.data.listeners_.length;++i) {
    var listener = os.data.listeners_[i];
    listener.keys[key] != null && os.data.checkListener_(listener) && listener.callback()
  }
};
os.createRenderClosure = function(template, element, opt_data) {
  var closure = function() {
    template.renderInto(element, opt_data)
  };
  return closure
};
os.createHideElementClosure = function(element) {
  var closure = function() {
    displayNone(element)
  };
  return closure
};
os.data.newJsonPostRequestHandler = function(url, opt_postData) {
  var handler = function(callback) {
    if(!gadgets)return;
    var params = {};
    params[gadgets.io.RequestParameters.METHOD] = gadgets.io.MethodType.POST;
    if(opt_postData)params[gadgets.io.RequestParameters.POST_DATA] = opt_postData;
    params[gadgets.io.RequestParameters.CONTENT_TYPE] = gadgets.io.ContentType.JSON;
    gadgets.io.makeRequest(url, function(obj) {
      callback(obj.data)
    }, params)
  };
  return handler
};
(os.data.defineRequests_ = function() {
  os.data.registerRequestHandler("os:personRequest", function(req, tag) {
    return req.newFetchPersonRequest(tag.getAttribute("id"))
  });
  os.data.registerRequestHandler("os:peopleRequest", function(req, tag) {
    return req.newFetchPeopleRequest(tag.getAttribute("group"))
  });
  if(gadgets) {
    os.createNamespace("json", "http://json.org");
    os.data.registerRequestHandler("json:makeRequest", function(key, tag) {
      var url = tag.getAttribute("url"), params = {};
      params[gadgets.io.RequestParameters.CONTENT_TYPE] = gadgets.io.ContentType.JSON;
      params[gadgets.io.RequestParameters.METHOD] = gadgets.io.MethodType.GET;
      gadgets.io.makeRequest(url, function(obj) {
        os.data.DataContext.putDataSet(key, obj.data)
      }, params)
    })
  }
})();
(os.data.populateParams_ = function() {
  var params = {}, queryString = document.location.search;
  if(queryString) {
    queryString = queryString.substring(1);
    var queryParts = queryString.split("&");
    for(var i = 0;i < queryParts.length;i++) {
      var paramParts = queryParts[i].split("=");
      params[paramParts[0]] = paramParts[1]
    }
  }os.data.DataContext.putDataSet("params", params)
})();
