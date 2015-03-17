// Configure logging
require("colors");


function log(msg) {
  console.log(">>> ".green + msg);
}


log.debug = function(msg) {
  console.log("DEBUG ".gray + msg.gray);
}

log.warn = function(msg) {
  console.log("WARN ".yellow + msg);
}

log.error = function(msg) {
  console.error("ERROR ".red + msg);
}


log.serial = function(msg) {
  console.log("<<< ".blue + msg);
}


log.gcode = function(msg) {
  console.log("GCode: ".magenta + msg);
}


module.exports = log;
