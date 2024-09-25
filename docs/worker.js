//Credits to https://github.com/fulldecent/system-bus-radio
//As well as Jordan Harband for the nodejs simd library
//Tested to be working on Chrome at 1560khz

var window = self; // Create "window" object

function now() {
	return window.performance.now() * 1000000;
}

var NSEC_PER_SEC = 1000000000;
var register = 3.1415;

function square_am_signal(freq, time) { // This function generates the radio waves
	postMessage("\nPlaying / " + time + " seconds / " + freq + "Hz");
	var period = NSEC_PER_SEC / freq;
	var start = now();
	var end = now() + time * NSEC_PER_SEC;
	while (now() < end) {
		var mid = start + period / 2;
		var reset = start + period;
		while (now() < mid) {
			for (var i = 0; i < 100; i++) {
				register = 1 - Math.log(register) / 1.7193;
			}
		}
		while (now() < reset) {}
		start = reset;
	}
}

function play(song) { // Parse song data, and call on required scripts to run it
	song = song.split("\n");
	var length = song.length;
	var line, freq, time;
	for (var i = 0; i < length; i++) {
		line = song[i].split(" ");
		if (+line[1] == 0) {
			pause(line[0]);
		}
		else {
			time = (+line[1])*.001;
			freq = +line[0];
			square_am_signal(freq, time);
		}
	}

	close(); // Close Web Worker
}

function pause(time) { // A useless function to run when there is no noise to play
	postMessage("\nPaused / " + time*.001 + " seconds");
	var dt = new Date();
	while ((new Date()) - dt <= time) { /* Do nothing */ }
}

onmessage = function(event) { // Recieve song data from main page
	var data = event.data;
	play(data);
};