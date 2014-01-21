var port_fp = process.argv[2];
var microstepping = process.argv[3] || 16;
var baudrate = process.argv[4] || 9600;

var com = require("serialport");
require("colors");
var repl = require("repl");
//repl.start(">>");

// show available ports
var list_ports_and_exit = function() {
  com.list(function (err, ports) {
    console.log("--------");
    ports.forEach(function(port) {
      console.log(port.comName);
      console.log(port.pnpId);
      port.manufacturer && console.log(port.manufacturer);
      console.log("");
    });
    console.log("========");
    process.exit();
  });
  console.log('Please specify one of these available device ports...');
}

var log = function(msg) {
  console.log(">>> ".green + msg);
}
var log_serial = function(msg) {
  console.log("<<< ".blue + msg);
}

var _init_serial_called_next = false;
var init_serial = function(sp, next) {
  sp.on('open', function() {
    log('opened Serial port');
    // read data as it arrives
    sp.on('data', function(data) {
      var data2 = data.toString('utf-8')
      if (data2) {
      log_serial(data2);
      }
      //set microstepping
      if (data2.trim().match("Please pass the number of microsteps per turn:"))
      {
        var message = new Buffer(1);
        message[0] = microstepping;
        sp.write(message, function() {
          sp.drain(function() {
            if (!_init_serial_called_next) {
              _init_serial_called_next = true;
              next(sp);
            }
          });
        });
      }
    });
  });
}

var send_serial = function(sp) {
  s = function(a, b, c, d) {
  log('sending data to Serial');
  msg = new Buffer(13);
  msg.writeInt32BE(a, 0, false); // num steps on motor 1
  msg.writeInt32BE(b, 4, false); // num steps on motor 2
  msg.writeInt32BE(c, 8, false); // num microsecs to move for
  msg.writeUInt8(d, 12, false); // bitmap of motor directions
  sp.write(msg, function() {
    sp.drain(function() {
      log('sent bytes');
    });
  })
  }
  
  // TODO: make this queue up on nodejs's end to ensure these all go through!
  ex = function() {
  s(200*microstepping*2, 6400, 1000000, 0<<7);
  s(200*microstepping*2, 6400, 1000000, 1<<7);
  }
  repl.start("repl> ");
}

var main = function() {
  if (!port_fp) {
    list_ports_and_exit();
  } else {
    // Connect to Arduino
    var sp = new com.SerialPort(port_fp, {
      baudRate: baudrate,
      parser: com.parsers.readline("\n")
    });

    // init serial and then starts reading/writing data
    init_serial(sp, send_serial);
  }
}

// execute main function
main();
