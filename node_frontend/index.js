require("colors");
var byline = require("byline");
var com = require("serialport");
var log = require("./log.js");
var repl = require("repl");
var stream = require("stream");
var yargs = require("yargs");
// global singleton func that sets and determines the current state of the
// arduino board
var state = require("./printer_state.js").state;

var PRINTER_CACHE = function() {};  // stores current position data
var REPL_STARTED = false;

// show available ports
var list_ports_and_exit = function() {
  com.list(function (err, ports) {
    log("--------");
    ports.forEach(function(port) {
      log(port.comName);
      log("    ---> " + port.pnpId);
      port.manufacturer && log(port.manufacturer);
      log("");
    });
    log("========");
    process.exit();
  });
  log('Please specify one of these available device ports...');
}

var init_serial = function(sp) {
  /* Initialize the arduino.
  * Define how to handle messages from the serial port */
  sp.on('open', function(err) {
    if (err) {
      state.set("all_off");
      log('failed to open: '+ error);
      throw err;
    }
    log('opened Serial port');

    sp.on('data', function(rawmsg) {handle_incoming_data(rawmsg, sp);});
    sp.on('error', function(err) {
      state.set("all_off");
      if (err) {
        log.serial('something related to arduino failed!');
        throw err;
      }});
    sp.on('close', function(err) {
      state.set("all_off");
      if (err) {
        log.serial('arduino connection closed with failure!');
        throw err;
      }});
  });
}


var handle_incoming_data = function(rawmsg, sp) {
  var msg = rawmsg.toString('utf-8').trim()
  if (msg) {
    log.serial(msg);
  }

    // TODO: the arduino might choose to reset itself for whatever reason.  If
    // it does, I should save what's currently in the pipe and then, after
    // microstep listener, perhaps submit it again?
  if (msg.match("Hello!")) {
    state.set("all_off");
    state.set("motors_on");
  }
}


parse_line = function(gcode_str) {
  /* Parse a string of GCode into an object */
  var GCode = function() {};
  GCode.raw = gcode_str;
  var l = gcode_str.split(';');  // [gcode, comment]
  if (l[1]) {  // log comments in gcode
    GCode.comment = l[1];
    gcode.log(GCode.comment);
  }
  var _gcode = l[0].split(/\s+/);
  GCode.instruction = _gcode.shift();  // G0 G92 etc

  var regexp = /([A-z]+)([0-9]+)/
  _gcode.forEach(function(code) {
    var m = regexp.exec(code);
    GCode[m[1]] = parseInt(m[2], 10);
  });
  return GCode
}


var msg_pack = function(gcode) {
  // TODO: incomplete function
  var msg = new Buffer(17);
  var instructions = 0;  // TODO
  if (gcode.X or gcode.Y) {  // move galvos
    instructions |= 1;
  }
  if (gcode.Z or "the side_to_side motion") {  // move motors
    instructions |= 2;
  }
  if ("laser on") {  // laser power on during move
    instructions |= 4;
  }
  if ("galvos on") {  // galvos powered on
    instructions |= 8;
  }
  if ("motor power on") {  // stepper motors powered on
    instructions |= 16;
  }
  if (gcode.F) {
    PRINTER_CACHE.F = gcode.F;
  }
  var feedrate = PRINTER_CACHE.F;
  msg.writeUInt8(instructions);
  msg.writeUInt32BE(feedrate);

  if (gcode.X or gcode.Y or gcode.Z or "side_to_side") {
    var directions = 0;
    if (gcode.Z >= 0) {
      directions |= 1;  // Z moves up
    }
    if ("side_to_side forward") {
      directions |= 2;
    }
    if (gcode.X >= 0) {
      directions |= 4;
    }
    if (gcode.Y >= 0) {
      directions |= 8;
    }
    msg.writeUInt8(directions);
  }

  if (gcode.Z or "side_to_side") {
    PRINTER_CACHE.Z = gcode.Z || PRINTER_CACHE.Z || 0
    msg.writeUInt32BE(PRINTER_CACHE.Z);
    PRINTER_CACHE.side_to_side_steps =  // TODO side_to_side?? (see next line)
      gcode.side_to_side_steps || PRINTER_CACHE.side_to_side_steps || 0;
    msg.writeUInt32BE(PRINTER_CACHE.side_to_side_steps);
  }

  if (gcode.X or gcode.Y) {
    // TODO: is this way of controlling galvos appropriate?
    var galvo_y_steps = 10;  // 12 bit number?
    var galvo_x_steps = 20;  // 12 bit number?
    msg.writeUInt16BE(
      ((galvo_y_steps & 0x0FFF) << 4) | ((galvo_x_steps & 0x0F00) >> 8));  // y
    msg.writeUInt8(galvo_xy & 0x00FF);  // x
  }
  return msg;
}


var send_serial = function(sp) {
  /*
   * 1. open gcode file
   * 2. for line in file, convert to printer instructions
   * 3. send to printer
   *
   * -- if unrecognized instruction, raise warning
   */

  byline(fs.createReadStream(argv.fp)).on('data', function(line) {
    // if (comment) { log.gcode(comment); }
    var gcode = parse_line(line);
    log.gcode(line.comment);

    msg = msg_pack(gcode)
    sp.write(msg, function() {
      sp.drain(function() {
        log('sent bytes');
      });
    });
    // TODO: handle what happens when state emits motors_on|off,  etc
  });

  // TODO: make this queue up on nodejs's end to ensure these all go through!
  // ex = function() {
  // s(1*argv.microstepping, 1*argv.microstepping, 1000, 0<<7);
  // s(1*argv.microstepping, 1*argv.microstepping, 1000, 1<<7);
  // }
  if (!REPL_STARTED) {
    log("Attaching repl for interactive use");
    var rs = repl.start("repl> ").on('exit', function() {
      log("EXIT");
      process.exit();
    });
    REPL_STARTED = true;
  }
}

var main = function() {
  if (!argv.serialport) {
    list_ports_and_exit();
    return 1
  }
  // Connect to Arduino
  var sp = new com.SerialPort(argv.serialport, {
    baudRate: argv.baudrate,
    parser: com.parsers.readline("\n")
  });

  // init serial and then starts reading/writing data
  init_serial(sp);

  state.on('motors_on', function(stream) {
    send_serial(sp);
  });
}

parse_argv = function() {
  return yargs
  .options('f', {
    alias: 'fp',
    required: true,
    describe: "filepath to a gcode file"
    + " ie: --fp ./myprint.gcode",
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
