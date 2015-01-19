var com = require("serialport");
require("colors");
var repl = require("repl");
var events = require('events');
var util = require("util");
var yargs = require("yargs")
//repl.start(">>");

// global singleton func that sets and determines the current state of the
// arduino board
function State() {
  events.EventEmitter.call(this);
  this.things = ["all", "motors", "lasers"];
  this.states = ["on", "off"];
}

util.inherits(State, events.EventEmitter);

State.prototype.set = function(event) {
  var thing = event.split("_")[0];
  var state = event.split("_")[1];
  if ((this.things.indexOf(thing) <= -1)
      && (this.states.indexOf(state) <= -1)) {
    throw new Error(
      'unrecognized state event.'
      + " You should pass a <thing>_<state>, but you passed: "
      + event);
  }
  var _this = this;
  if (thing === "all") {
    _this.things.slice(1).forEach(function(thing) {
      var ev = thing + "_" + state;
      _this.emit(ev);
      log("set state: " + ev);
    });
  }
}

var state = new State();

// show available ports
var list_ports_and_exit = function() {
  com.list(function (err, ports) {
    console.log("--------");
    ports.forEach(function(port) {
      console.log(port.comName);
      console.log("    ---> " + port.pnpId);
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

var init_serial = function(sp) {
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
        log_serial('something related to arduino failed!');
        throw err;
      }});
    sp.on('close', function(err) {
      state.set("all_off");
      if (err) {
        log_serial('arduino connection closed with failure!');
        throw err;
      }});
  });
}


var handle_incoming_data = function(rawmsg, sp) {
  var msg = rawmsg.toString('utf-8').trim()
  if (msg) {
    log_serial(msg);
  }

  //respond to various messages from the arduino firmware
  if (msg.match(
    "Please pass exactly 1 byte specifying the number"
      + " of microsteps per turn:"))
  {
    state.set("all_off");
    microstep_listener(sp);
    // TODO: the arduino might choose to reset itself for whatever reason.  If
    // it does, I should save what's currently in the pipe and then, after
    // microstep listener, perhaps submit it again?
  } else if (msg.match("Please pass 13 bytes at a time in Big Endian order")) {
    state.set("motors_on");
    send_serial(sp);
  }

}

var microstep_listener = function(sp) {
  var message = new Buffer(1);
  message[0] = argv.microstepping;
  // clear the pipe
  sp.drain(function(err) {
    if (err) {
      set.state("all_off");
      throw err;
    }});
  sp.write(message, function(err) {
    if (err) {
      set.state("all_off");
      throw err;
    }
    log('setting microsteps to: ' + argv.microstepping);
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
  s(200*argv.microstepping*2, 6400, 1000000, 0<<7);
  s(200*argv.microstepping*2, 6400, 1000000, 1<<7);
  }
  repl.start("repl> ");
}

var main = function() {
  if (!argv.fp) {
    list_ports_and_exit();
  } else {
    // Connect to Arduino
    var sp = new com.SerialPort(argv.fp, {
      baudRate: argv.baudrate,
      parser: com.parsers.readline("\n")
    });

    // init serial and then starts reading/writing data
    init_serial(sp);
  }
}

parse_argv = function() {
  return yargs
  .options('f', {
    alias: 'fp',
    required: false,  // handled in main
    describe: "device path to arduino serial port."
    + " ie: --fp /dev/ttyACM0",
  })
  .options('m', {
    alias: 'microstepping',
    default: 16
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
