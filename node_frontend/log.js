// Configure logging
require("colors");


function log(msg) {
  console.log(">>> ".green + msg);
}


log.serial = function(msg) {
  console.log("<<< ".blue + msg);
}


log.gcode = function(msg) {
  console.log("--- ".yellow + msg);
}


module.exports = log;
