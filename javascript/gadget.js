/**
  * Wave
  */

wave = {};
wave.getMode = gadgetAPI.wave_getMode;
wave.log = gadgetAPI.wave_log;
wave.getState = function() { return wave.state_; };
wave.isInWaveContainer = function() { return true; }
wave.getTime = function() { return(new Date).getTime() };
wave.isPlayback = function() {
  var b = wave.getMode();
  return b == wave.Mode.PLAYBACK || b == wave.Mode.UNKNOWN
};
wave.Mode = {UNKNOWN:0, VIEW:1, EDIT:2, DIFF_ON_OPEN:3, PLAYBACK:4};

/**
  * Callback
  */

wave.Callback = function(b, c) {
  this.callback_ = b;
  this.context_ = c || null;
};
wave.Callback.prototype.invoke = function() {
  if(this.callback_) {
    var b = Array.prototype.slice.call(wave.Callback.prototype.invoke.arguments, 0);
    this.callback_.apply(this.context_, b);
  }
};

/**
  * State
  */

wave.currentState_ = {};
wave.state_ = {};
wave.state_.get = function( key, def )
{
        if (def)
            return gadgetAPI.state_get( key, def );
        return gadgetAPI.state_get( key, null );
};
wave.state_.getKeys = gadgetAPI.state_getKeys;
wave.state_.submitDelta = gadgetAPI.state_submitDelta;
wave.state_.submitValue = gadgetAPI.state_submitValue;
wave.state_.toString = gadgetAPI.state_toString;
wave.state_.reset = function() {
  var b = {};
  for(var c in this.state_)b[c] = null;
  gadgetAPI.state_submitDelta(b)
};

/**
  * Util
  */

wave.util = { };
wave.util.SPACES = "                                                 ";
wave.util.toSpaces_ = function(b)
{
  return wave.util.SPACES.substring(0, b * 2);
};
wave.util.isArray_ = function(b)
{
  try {
    return b && typeof b.length == "number"
  }catch(c) {
    return false;
  }
};
wave.util.printJson = function(b, c, e)
{
  if(!b || typeof b.valueOf() != "object") {
    if(typeof b == "string")return"'" + b + "'";
    else if(b instanceof Function)return"[function]";
    return"" + b
  }var d = [], f = wave.util.isArray_(b), g = f ? "[]" : "{}", h = c ? "\n" : "", k = c ? " " : "", l = 0;
  e = e || 1;
  c || (e = 0);
  d.push(g.charAt(0));
  for(var i in b) {
    var j = b[i];
    l++ > 0 && d.push(", ");
    if(f)d.push(wave.util.printJson(j, c, e + 1));
    else {
      d.push(h);
      d.push(wave.util.toSpaces_(e));
      d.push(i + ": ");
      d.push(k);
      d.push(wave.util.printJson(j, c, e + 1))
    }
  }if(!f) {
    d.push(h);
    d.push(wave.util.toSpaces_(e - 1))
  }d.push(g.charAt(1));
  return d.join("")
};

/**
  * Participant
  */

wave.participants_ = [];
wave.participantMap_ = {};
wave.getParticipants = function() {
  return wave.participants_
};
wave.getParticipantById = function(b) {
  return wave.participantMap_[b]
};
wave.Participant = function(b, c, e)
{
  this.id_ = b || "";
  this.displayName_ = c || "";
  this.thumbnailUrl_ = e || ""
};
wave.Participant.prototype.getId = function()
{
  return this.id_
};
wave.Participant.prototype.getDisplayName = function()
{
  return this.displayName_
};
wave.Participant.prototype.getThumbnailUrl = function()
{
  return this.thumbnailUrl_
};
wave.Participant.fromJson_ = function(b) {
  var c = new wave.Participant;
  c.id_ = b.id;
  c.displayName_ = b.displayName;
  c.thumbnailUrl_ = b.thumbnailUrl;
  return c
};

/**
  * Callbacks
  */

wave.setStateCallback = function(b, c) {
  wave.stateCallback_ = new wave.Callback(b, c);
  wave.currentState_ && wave.stateCallback_.invoke(wave.currentState_)
};
wave.setParticipantCallback = function(b, c) {
  wave.participantCallback_ = new wave.Callback(b, c);
  wave.participants_ && wave.participantCallback_.invoke(wave.participants_)
};
wave.setModeCallback = function(b, c) {
  wave.modeCallback_ = new wave.Callback(b, c);
  wave.mode_ && wave.modeCallback_.invoke(wave.getMode())
};
wave.updateWaveParticipants_ = function()
{
  b = gadgetAPI.participants_getAll();
  wave.viewer_ = null;
  wave.host_ = null;
  wave.participants_ = [];
  wave.participantMap_ = {};
  var c = b.myId, e = b.authorId;
  b = b.participants;
  for(var d in b) {
    var f = wave.Participant.fromJson_(b[d]);
    if(d == c)wave.viewer_ = f;
    if(d == e)wave.host_ = f;
    wave.participants_.push(f);
    wave.participantMap_[d] = f
  }if(!wave.viewer_ && c) {
    f = new wave.Participant(c, c);
    wave.viewer_ = f;
    wave.participants_.push(f);
    wave.participantMap_[c] = f
  }wave.participantCallback_.invoke(wave.participants_)
};
wave.updateState_ = function()
{
        wave.currentState_ = {};
        var s = gadgetAPI.state_getAll();
        for( key in s )
                wave.currentState_[key] = s[key];
        wave.stateCallback_.invoke(wave.currentState_)
};
wave.receiveMode_ = function(b) {
  wave.mode_ = b || {};
  wave.modeCallback_.invoke(wave.getMode())
};
wave.getViewer = function() {
  return wave.viewer_;
};
wave.getHost = function() {
  return wave.host_;
};

gadgets = { };
gadgets.util = { };
gadgets.util.onLoadHandlers_ = [];
gadgets.util.registerOnLoadHandler = function( handler )
{
        gadgetAPI.testme({'Hallo': 'RegisterInit'});
        gadgets.util.onLoadHandlers_.push( handler );
};
gadgets.util.callOnLoadHandlers_ = function()
{
        for( var i = 0; i < gadgets.util.onLoadHandlers_.length; ++i )
        {
                gadgetAPI.testme({'Hallo': 'Init'});
                try {
                gadgets.util.onLoadHandlers_[i]();
        } catch( e ) {
                gadgetAPI.testme({'Err': e.toString()});
        }
                gadgetAPI.testme({'Hallo': 'AfterInit'});
        }
};
gadgets.json = {};
gadgets.json.stringify = wave.util.printJson;
gadgets.json.parse = function(str)
{
        // TODO
        return {};
};
gadgets.window = {};
gadgets.window.adjustHeight = function()
{
        gadgetAPI.gadgets_adjustHeight( document.getElementById("__os__container").offsetHeight );
};
JSON = {};
JSON.stringify = gadgets.json.stringify;
JSON.parse = gadgets.json.parse;
gadgetAPI.testme({'Hallo': 'Wave API'})
