var WebSocketServer = require('ws').Server,
    wss = new WebSocketServer({ port: 8888 }),
    users = {};

wss.on('connection', function (connection) {
  connection.on('message', function (message) {
    var data;

    try {
      data = JSON.parse(message);
    } catch (e) {
      console.log("Error parsing JSON");
      data = {};
    }

    switch (data.type) {
      case "login":
        console.log("User logged in as", data.name);
        if (users[data.name]) {
          sendTo(connection, {
            type: "login",
            success: false
          });
        } else {
          users[data.name] = connection;
          connection.name = data.name;
          sendTo(connection, {
            type: "login",
            success: true
          });
        }

        break;
      default:
        sendTo(connection, {
          type: "error",
          message: "Unrecognized command: " + data.type
        });

        break;
    }
  });

  connection.on('close', function () {
    if (connection.name) {
      delete users[connection.name];
    }
  });
});

function sendTo(conn, message) {
  conn.send(JSON.stringify(message));
}

wss.on('listening', function () {
    console.log("Server started...");
});
