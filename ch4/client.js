var connection = new WebSocket('ws://localhost:8888');

var name = "User" + Math.floor(Math.random() * 100);

connection.onopen = function () {
  console.log("Connected");

  connection.send(JSON.stringify({
    type: "login",
    name: name
  }));
};

connection.onmessage = function (message) {
  console.log("Got message", message.data);

  var data = JSON.parse(message.data);

  switch(data.type) {
    case "login":
      onLogin(data.success);
      break;
    default:
      break;
  }
};

connection.onerror = function (err) {
  console.log("Got error", err);
};

var timeout = 500;
function onLogin(success) {
  if (success === false) {
    // Try to login again after a timeout
    setTimeout(function () {
        var name = "User" + Math.floor(Math.random() * 100);
        connection.send(JSON.stringify({
          type: "login",
          name: name
        }));
    }, timeout);
    timout = timeout * 2;
  } else {
    timeout = 500;
    console.log("READY!");
  }
}
