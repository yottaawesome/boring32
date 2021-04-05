# TestNodeWebSocketServer

## Introduction

This is a simple server to test out the behaviour of WinHttp WebSockets when testing Boring32 locally.

## Running

You must first generate a self-signed certificate or key that the node app will use to secure the connection. If you have Git Bash on Windows, you can simply use the following command `winpty openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365` and use `aaaa` as your passphrase (or use no passphrase and remove it from `socket.js` server options). The you simply run `npm run start` to start the server.

## Updating NPM dependencies

* Install NCU globally: `npm i -g npm-check-updates`;
* Update package.json: `ncu -u`;
* Install updated dependencies: `npm i`.
