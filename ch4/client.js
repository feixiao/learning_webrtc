var connection = new WebSocket('ws://localhost:8888');

connection.onopen = function () {
  console.log("Connected");
};

connection.onmessage = function (message) {
  console.log("Got message", message.data);
};

connection.onerror = function (err) {
  console.log("Got error", err);
};
