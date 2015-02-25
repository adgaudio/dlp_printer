var byline = require("byline");
var fs = require('fs');
var repl = require("repl");
var yargs = require("yargs");
// global singleton func that sets and determines the current state of the
// arduino board
var log = require("./log.js");
var state = require("./printer_state.js");
var serial = require("./serial_communication.js");

var REPL_STARTED = false;
var PRINT_STARTED = false;


function parse_line(gcode_str) {
  /* Parse a string of GCode into an object */
  var GCode = {};
  GCode.raw = gcode_str;
  var l = gcode_str.split(';');  // [gcode, comment]
  if (l[1]) {  // log comments in gcode
    GCode.comment = l[1];
    gcode.log(GCode.comment);
  }
  var _gcode = l[0].split(/\s+/);
  GCode.instruction = _gcode.shift();  // G0 G1 M100 etc

  var regexp = /([A-z]+)([0-9]+)/
  _gcode.forEach(function(code) {
    var m = regexp.exec(code);
    GCode[m[1]] = parseInt(m[2], 10);
  });
  log.gcode(JSON.stringify(GCode));
  return GCode
}


function send_serial(sp) {
  /*
   * 1. open gcode file
   * 2. for line in file, convert to printer instructions
   * 3. send to printer
   *
   * -- if unrecognized instruction, raise warning
   */

  var stream = byline(fs.createReadStream(argv.fp));
  stream.on('end', function() {
    // send 3 times to guarantee no errors
    serial.send(sp, "end of stream");
    serial.send(sp, "end of stream");
    serial.send(sp, "end of stream");
    state.set('all_off');
  });
  stream.on('data', function(line) {
    // if (comment) { log.gcode(comment); }
    var gcode = parse_line(line.toString());
    if (!gcode) {
      return;
    }
    msg = serial.msg_pack(gcode)
    serial.send(sp, msg)
    // TODO: handle what happens when state emits motors_on|off,  etc
  });
}

function main() {
  var sp = serial.get_connection(argv.serialport, argv.baudrate);
  state.on('all_on', function() {
    if (argv.fp == "repl") {
      if (!REPL_STARTED) {
        log("Attaching repl for interactive use");
        var rs = repl.start("repl> ").on('exit', function() {
          log("EXIT");
          process.exit();
        });
        REPL_STARTED = true;
      }
    } else if (!PRINT_STARTED) {
      log("Starting print");
      PRINT_STARTED = true;
      send_serial(sp);
    } else {
      log("Arduino reset while print in progress.  Will hopefully recover.");
    }
  });
  // TODO: state.on('all_off', function() {});
}

function parse_argv() {
  return yargs
  .options('f', {
    alias: 'fp',
    required: true,
    describe: "filepath to a gcode file.  "
    + " ie: --fp ./myprint.gcode   ..."
    + " if you pass --fp repl, load a repl instead.",
  })
  .options('p', {
    alias: 'serialport',
    required: false,  // handled in main
    describe: "device path to arduino serial port."
    + " ie: --sp /dev/ttyACM0",
  })
  .options('m', {
    alias: 'microstepping',
    default: 32
  })
  .options('b', {
    alias: 'baudrate',
    default: 9600
  })
  .strict()
  .argv;
}

// execute main function
var argv = parse_argv();
main(argv);
