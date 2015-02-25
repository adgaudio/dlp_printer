// Configure logging
require("colors");


function log(msg) {
  console.log(">>> ".green + msg);
}


log.error = function(msg) {
  console.error("ERROR ".red + msg);
}


log.serial = function(msg) {
  console.log("<<< ".blue + msg);
}


log.gcode = function(msg) {
  console.log("GCode: ".yellow + msg);
}


module.exports = log;
