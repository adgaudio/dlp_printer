var byline = require("byline");
var com = require("serialport");
var stream = require("stream");

var log = require("./log.js");
var state = require("./printer_state.js");

var PRINTER_CACHE = function() {};  // stores current position data


module.exports.get_connection = function(serialport, baudrate) {
  if (!serialport) {
    list_ports_and_exit();
    process.exit(1);
  }
  // Connect to Arduino
  var sp = new com.SerialPort(serialport, {
    baudRate: baudrate,
    parser: com.parsers.readline("\n")
  });

  // init serial and then starts reading/writing data
  init_serial(sp);
  return sp;
}


function list_ports_and_exit() {
  /* show available serial ports */
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


function init_serial(sp) {
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


function handle_incoming_data(rawmsg, sp) {
  /* Define what to do when the Arduino sends a Serial message */
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


module.exports.msg_pack = function(gcode) {
  /* Build a message to send to the Arduino */
  var move = msg_pack_ismove(gcode);
  var msg = new Buffer(msg_pack_get_buflen(move));
  var byte_offset = 0;
  msg.writeUInt8(msg_pack_instructions(move), byte_offset++);

  if (move.motor || move.laser_galvo) {
    // feedrate: how fast to move in #steps per microsecond
    PRINTER_CACHE.feedrate = gcode.F | 0x0;
    msg.writeUInt32BE(PRINTER_CACHE.feedrate, byte_offset);
    byte_offset += 4;
    // which direction to move motors and galvos
    msg.writeUInt8(msg_pack_directions(gcode), byte_offset++);
  }
  if (move.motor) {
    // How many steps to move motors
    msg.writeUInt32BE(gcode.S | 0x0, byte_offset);  // slides vat side to side
    byte_offset += 4;
    PRINTER_CACHE.Z += (gcode.Z | 0x0);  // store last val
    msg.writeUInt32BE(gcode.Z | 0x0, byte_offset);
    byte_offset += 4;
  }
  if (move.laser_galvo) {
    // How many steps to move laser galvos
    // TODO: is this way of controlling galvos appropriate?
    // assumes X and Y must be 12 bit numbers
    msg.writeUInt16BE(
      ((gcode.Y & 0x0FFF) << 4) | ((gcode.X & 0x0F00) >> 8), byte_offset);
    byte_offset += 2;
    msg.writeUInt8(gcode.X & 0x00FF, byte_offset++);
  }
  PRINTER_CACHE.last_msg = msg;
  return msg;
}


function msg_pack_ismove(gcode) {
  /* Is this gcode line attempting to move the motors or laser galvos? */
  var move_instruction = /^(G0|G1|M100)$/;
  var motor_move = false;
  var laser_move = false;
  if (gcode.instruction.match(move_instruction)) {
    if (gcode.Z || gcode.S) {
      motor_move = true;
    }
    if (gcode.X || gcode.Y) {
      laser_move = true;
    }
  }
  return {motor: motor_move, laser_galvo: laser_move};
}


function msg_pack_get_buflen(move) {
  /* Calculate the number of bytes to send to the Arduino */
  var _buflen = 1;
  if (move.motor || move.laser_galvo) {
    _buflen += 5;
  }
  if (move.motor) {
    _buflen += 8;
  }
  if (move.laser_galvo) {
    _buflen += 3;
  }
  return _buflen;
}


function msg_pack_instructions(move) {
  // build the instruction byte to inform printer what to receive and do
  var instructions = 0;
  // this sub-section useful to limit num bytes sent to arduino.
  // TODO: necessary?
  if (move.laser_galvo) {
    instructions |= 1;
  }
  if (move.motor) {
    instructions |= 2;
  }
  if (gcode.E) {  // laser power on
    instructions |= 4;
  }
  if ("galvos on") {  // galvos powered on
    // TODO: really, always on?
    instructions |= 8;
  }
  if ("motor power on") {  // stepper motors powered on
    // TODO: really, always on?
    instructions |= 16;
  }
  return instructions;
}

function msg_pack_directions(gcode) {
  // which direction to move motors or galvos
  var directions = 0;
  if (gcode.S > 0) {
    directions |= 1;  // tell Arduino to slide vat side to side after moves
  }
  if (gcode.Z >= 0) {
    directions |= 2;  // Z moves up
  }
  if (gcode.X >= 0) {
    directions |= 4;
  }
  if (gcode.Y >= 0) {
    directions |= 8;
  }
  return directions;
}
