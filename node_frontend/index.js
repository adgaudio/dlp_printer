var port_fp = process.argv[2];
var baudrate = process.argv[3] || 9600;
var microstepping = 16;

var com = require("serialport")
//var repl = require("repl");
//repl.start(">>");

// show available ports
com.list(function (err, ports) {
  console.log("--------");
  ports.forEach(function(port) {
    console.log(port.comName);
    console.log(port.pnpId);
    port.manufacturer && console.log(port.manufacturer);
    console.log("");
  });
  console.log("========");
});

// Connect to Arduino
var sp = new com.SerialPort(port_fp, {
  baudRate: baudrate,
  parser: com.parsers.readline("\n")
});

sp.on('open',function() {
  // read data at regular intervals
  console.log('opened Serial port');

  sp.on('data', function(data) {
    var data2 = data.toString('utf-8')
    if (data2) {
    console.log("<<< " + data2);
    }
  });

  // set microstepping
  setTimeout(function() {
  console.log("writing data!");
  var message = new Buffer(1);
  message[0] = microstepping;
  sp.write(message, function() {
    sp.drain(function() {
      console.log('finished writing');
    });
  });
  }, 2000);

});
