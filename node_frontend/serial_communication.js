var com = require("serialport");
var stream = require("stream");

var log = require("./log.js");
var state = require("./printer_state.js");
module.exports.msg_pack = require("./msg_pack.js")

var SERIAL_CACHE = {};  // stores current position data
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
    if (SERIAL_CACHE.last_msg) {
      SERIAL_CACHE.please_resend = true;
    }
    state.set("all_on");
  }
}


function send(msg) {
  /* Write an n-byte buffer to the serial port, and flush (ie drain) it to
  * ensure it fully sends.
  *
  * Resend messages if SERIAL_CACHE.please_resend is set,
  * and maintain a count of num consecutive resends.
  * If resend count >3, stop and throw an exception.
  * If successfully sent 2 messages in a row, assume printer is healthy
  * and reset the resend count.
  */
  log.debug("running: " + JSON.stringify(msg));
  if (SERIAL_CACHE.n_resends > 3) {
    state.set("all_off");
    throw "Just resent the same message 3x in a row.  msg: " + msg.toJSON();
  } else if (SERIAL_CACHE.please_resend) {
    log.warn("Resending last serial message: " + SERIAL_CACHE.last_msg.toJSON());
    SERIAL_CACHE.please_resend = false;
    inc("n_resends", SERIAL_CACHE);
    send(SERIAL_CACHE.last_msg);
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
    SERIAL_CACHE.please_resend = true;
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
        if (inc("comm_succ", SERIAL_CACHE) >= 2) {
          SERIAL_CACHE.comm_succ = 0;
          SERIAL_CACHE.n_resends = 0;
        }
      }
    });
  });
  SP.drain();
  SERIAL_CACHE.last_msg = msg;
}


function inc(varname, object) {
  if (!object[varname]) {
    return object[varname] = 1;
  } else {
    return ++object[varname];
  }
}
