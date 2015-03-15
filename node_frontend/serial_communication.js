var com = require("serialport");
var stream = require("stream");

var log = require("./log.js");
var state = require("./printer_state.js");

var PRINTER_CACHE = function() {};  // stores current position data
var SP;  // serialport connection to arduino


module.exports.close_connection = function() {
  if (SP) {
    log('closing connection to arduino')
    SP.close();
  } else {
    throw "You must open the connection before you can close it!"
  }

}


module.exports.open_connection = function(serialport, baudrate) {
  if (!serialport) {
    list_ports_and_exit();
    process.exit(1);
  }
  // Connect to Arduino
  SP = new com.SerialPort(serialport, {
    baudRate: baudrate,
    parser: com.parsers.readline("\n")
  });

  // init serial and then starts reading/writing data
  init_serial();
  return SP;
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


function init_serial() {
  /* Initialize the arduino.
  * Define how to handle messages from the serial port */
  SP.on('open', function(err) {
    if (err) {
      state.set("all_off");
      log.error('failed to open serial port: '+ err);
      throw err;
    }
    log('opened Serial port');

    SP.on('data', function(rawmsg) {handle_incoming_data(rawmsg);});
    SP.on('error', function(err) {
      state.set("all_off");
      if (err) {
        log.error('something related to arduino failed!');
        throw err;
      }});
    SP.on('close', function(err) {
      state.set("all_off");
      if (err) {
        log.error('arduino connection closed with failure!');
        throw err;
      }});
  });
}


function handle_incoming_data(rawmsg) {
  /* Define what to do when the Arduino sends a Serial message */
  var msg = rawmsg.toString('utf-8').trim()
  if (msg) {
    log.serial(msg);
  }
  if (msg.match("Hello!")) {
    if (PRINTER_CACHE.last_msg) {
      PRINTER_CACHE.please_resend = true;
    }
    state.set("all_on");
  }
}


module.exports.msg_pack = function(gcode) {
  /* Build a message to send to the Arduino */
  var move = _msg_pack_ismove(gcode);
  var msg = new Buffer(_msg_pack_get_buflen(move));
  msg.fill(0);
  var byte_offset = 0;
  msg.writeUInt8(_msg_pack_instructions(gcode, move), byte_offset++);

  if (move.motor || move.laser_galvo) {
    // feedrate: how fast to move in #steps per microsecond
    if (gcode.F) {
      PRINTER_CACHE.feedrate = Math.abs(gcode.F) | 0x0;
    }
    log("feedrate " + PRINTER_CACHE.feedrate);
    msg.writeUInt32BE(PRINTER_CACHE.feedrate, byte_offset);
    byte_offset += 4;
    // which direction to move motors and galvos
    msg.writeUInt8(_msg_pack_directions(gcode), byte_offset++);
  }
  if (move.motor) {
    // How many steps to move motors
    PRINTER_CACHE.Z += (Math.abs(gcode.Z) | 0x0);  // store last val
    msg.writeUInt32BE(Math.abs(gcode.Z) | 0x0, byte_offset);
    byte_offset += 4;
    msg.writeUInt32BE(Math.abs(gcode.S) | 0x0, byte_offset);  // slides vat side to side
    byte_offset += 4;
  }
  if (move.laser_galvo) {
    // How many steps to move laser galvos
    // TODO: is this way of controlling galvos appropriate?
    // assumes X and Y must be 12 bit numbers
    msg.writeUInt16BE(
      ((Math.abs(gcode.X) & 0x0FFF) << 4)
        | ((Math.abs(gcode.Y) & 0x0F00) >> 8), byte_offset);
    byte_offset += 2;
    msg.writeUInt8(Math.abs(gcode.Y) & 0x00FF, byte_offset++);
  }
  return msg;
}


function _msg_pack_ismove(gcode) {
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


function _msg_pack_get_buflen(move) {
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


function _msg_pack_instructions(gcode, move) {
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


function _msg_pack_directions(gcode) {
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


function send(msg) {
  /* Write an n-byte buffer to the serial port, and flush (ie drain) it to
  * ensure it fully sends.
  *
  * Resend messages if PRINTER_CACHE.please_resend is set,
  * and maintain a count of num consecutive resends.
  * If resend count >3, stop and throw an exception.
  * If successfully sent 2 messages in a row, assume printer is healthy
  * and reset the resend count.
  */
  log.debug("running: " + JSON.stringify(msg));
  if (PRINTER_CACHE.n_resends > 3) {
    state.set("all_off");
    throw "Just resent the same message 3x in a row.  msg: " + msg.toJSON();
  } else if (PRINTER_CACHE.please_resend) {
    log.warn("Resending last serial message: " + PRINTER_CACHE.last_msg.toJSON());
    PRINTER_CACHE.please_resend = false;
    inc("n_resends", PRINTER_CACHE);
    send(PRINTER_CACHE.last_msg);
  }
  if (msg == "end of stream") {
    return;
  } else {
    _send(msg);
  }
}
module.exports.send = send;


var _send_handle_err = function(err) {
  if (err) {
    log.error(err);
    PRINTER_CACHE.please_resend = true;
    // assume an external process will call send(...)
  }
}


function _send(msg) {
  // actually send msg to serial and drain (flush) the serial port buffer
  SP.write(msg, function(err) {
    _send_handle_err(err);
    SP.drain(function(err) {
      if (err) {
      _send_handle_err(err);
      } else {
        log('sent bytes: ' + JSON.stringify(msg));
        if (inc("comm_succ", PRINTER_CACHE) >= 2) {
          PRINTER_CACHE.comm_succ = 0;
          PRINTER_CACHE.n_resends = 0;
        }
      }
    });
  });
  SP.drain();
  PRINTER_CACHE.last_msg = msg;
}


function inc(varname, object) {
  if (!object[varname]) {
    return object[varname] = 1;
  } else {
    return ++object[varname];
  }
}
