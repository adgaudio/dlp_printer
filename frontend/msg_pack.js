var MSG_CACHE = {}

module.exports = function(gcode) {
  /* Build a message to send to the Arduino */
  var move = _msg_pack_ismove(gcode);
  var msg = new Buffer(_msg_pack_get_buflen(move));
  msg.fill(0);
  var byte_offset = 0;
  msg.writeUInt8(_msg_pack_instructions(gcode, move), byte_offset++);

  if (move.motor || move.laser_galvo) {
    // feedrate: how fast to move in #steps per microsecond
    if (gcode.F) {
      MSG_CACHE.feedrate = Math.abs(gcode.F) | 0x0;
    }
    msg.writeUInt32BE(MSG_CACHE.feedrate, byte_offset);
    byte_offset += 4;
    // which direction to move motors and galvos
    msg.writeUInt8(_msg_pack_directions(gcode), byte_offset++);
  }
  if (move.motor) {
    // How many steps to move motors
    MSG_CACHE.Z += (Math.abs(gcode.Z) | 0x0);  // store last val
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


