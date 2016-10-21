//Credits to https://github.com/fulldecent/system-bus-radio
//As well as Jordan Harband for the nodejs simd library
//Tested to be working on Chrome at 1560khz

function now() {
 return performance.now()*1000000;
}

var NSEC_PER_SEC = 1000000000;
var register = 3.1415

var logs = document.getElementById('logs');

function square_am_signal(time,freq) {
    logs.value += "Playing / "+time+" seconds / "+freq+"Hz\n";
    var period = NSEC_PER_SEC/freq;
    var start = now();
    var end = now()+time*NSEC_PER_SEC;
    while (now() < end) {
        var mid = start+period/2;
        var reset = start+period;
        while (now()<mid) {
            for (i = 0; i < 100; i++) {
                register = 1 - Math.log(register) / 1.7193
            }
        }
        while(now() < reset){
        }
        start = reset
    }
}

function start() {
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2093);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2673);
    square_am_signal(0.790, 2673);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2349);
    square_am_signal(0.790, 2349);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 3136);
    square_am_signal(0.790, 3136);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2093);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2349);
    square_am_signal(0.400, 2673);
    square_am_signal(0.400, 2349);
    square_am_signal(0.790, 2093);
}
