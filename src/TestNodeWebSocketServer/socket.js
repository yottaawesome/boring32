const https = require('https');
const fs = require('fs');
const WebSocket = require('ws');

const options = {
	key: fs.readFileSync('key.pem'),
	cert: fs.readFileSync('cert.pem'),
	passphrase: 'aaaa'
};
const server = https.createServer(options);
const wss = new WebSocket.Server({ server });

function stringToUint8Array(str) {
	let result = [];
	for (let i = 0; i < str.length; i++) {
	  result.push(str.charCodeAt(i));
	}
	return Uint8Array.from(result);
}

function binArrayToString(array) {
	let result = "";
	for (let i = 0; i < array.length; i++) {
	  result += String.fromCharCode(array[i]);
	}
	return result;
  }

wss.on('message', 
  function incoming(message) {
	  console.log(data);
  }
);

wss.on('connection', 
	function connection(ws) {
		console.log('Connection established');

		ws.on('message', function incoming(message) {
			// setTimeout(() => {
			// 	ws.send('something1');
			// }, 1000);
			// setTimeout(() => {
			// 	ws.send('something2');
			// }, 1000);
			setTimeout(() => ws.close(), 5000);
		});
		
	}
);

const port = 51935;
server.listen(
	port,
	'127.0.0.1', 
	() => {
		const host = server.address().address;
		const port = server.address().port;
		console.log("Server listening at https://%s:%s", host, port);
	});
