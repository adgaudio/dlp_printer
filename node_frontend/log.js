// Configure logging


function log(msg) {
  console.log(">>> ".green + msg);
}


log.serial = function(msg) {
  console.log("<<< ".blue + msg);
}

module.exports = log;
