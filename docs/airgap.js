var player; // Define "player" var to make my code linter happy

function start() { // Start Web Worker & send song data to player
	var logs = document.getElementById('progress'); // Define log element
	window.logs = logs; // Make variable Global
	window.logs.value = "";

	// Create Web Worker if it doesn't already exist
	if (window.Worker && typeof(player) == "undefined") {
		var player = new Worker("worker.js");
		window.player = player; // Make variable Global
		player.onmessage = function(event) {
			var data = event.data;
			console.log(data)
			window.logs.value += "x\n";
		};

		// Send song data to player
		var song = document.getElementById("tune").value;
		player.postMessage(song);
	}
}

function end() { // Stops the Web Worker
	window.logs.value = "";
	player.terminate();
}
