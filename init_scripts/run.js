var wait_time = 10;

setTimeout(function(){
	console.log("start audio app");
	var child_process = require('child_process');

	child_process.exec('/usr/bin/amixer -c 2 set PCM 90%');

var spawn = child_process.spawn,
    p = spawn('/home/root/main', [], {'cwd': "/home/root/"});

p.stdout.on('data', function (data) {
  console.log('stdout: ' + data);
});

p.stderr.on('data', function (data) {
  console.log('stderr: ' + data);
});

p.on('close', function (code) {
  console.log('child process exited with code ' + code);
});

}, wait_time * 1000);

