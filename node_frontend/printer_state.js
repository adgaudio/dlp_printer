// An Event Emitter that sets and determines the current state of the key
// components of the Arduino board.
//
// All events have the form "<component>_<state>".
// For instance, "motors_on" or "lasers_off"

var util = require('util');
var events = require('events');
var log = require('./log.js');


//// global singleton func that sets and determines the current state of the
// arduino board
function PrinterState() {
  events.EventEmitter.call(this);
  this.components = ["all", "motors", "lasers"];
  this.states = ["on", "off"];
}

util.inherits(PrinterState, events.EventEmitter);

PrinterState.prototype.set = function(event) {
  var thing = event.split("_")[0];
  var state = event.split("_")[1];
  if ((this.components.indexOf(thing) <= -1)
      && (this.states.indexOf(state) <= -1)) {
    throw new Error(
      'unrecognized state event.'
      + " You should pass a <thing>_<state>, but you passed: "
      + event);
  }

  var _this = this;
  function _set(thing, state) {
    var ev = thing + "_" + state;
    _this.emit(ev);
    log("set state: " + ev);
  }
  if (thing === "all") {
    _this.components.forEach(function(thing) {
      _set(thing, state);
    });
  } else {
    _set(thing, state);
  }
}

module.exports = new PrinterState();
