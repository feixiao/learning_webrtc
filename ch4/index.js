var WebSocketServer = require('ws').Server,
    wss = new WebSocketServer({ port: 8888 }),
    users = {};

wss.on('connection', function (connection) {
  console.log("User connected");

  connection.on('message', function (message) {
    console.log("Got message:", message);

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
          sendTo(connection, {
            type: "login",
            success: true
          });
        }

        break;
      case "offer":
        console.log("Sending offer to", data.name);
        var conn = users[data.name];

        if (conn != null) {
          sendTo(conn, {
            type: "offer",
            offer: data.offer
          });
        }

        break;
      case "answer":
        console.log("Sending answer to", data.name);
        var conn = users[data.name];

        if (conn != null) {
          sendTo(conn, {
            type: "answer",
            answer: data.answer
          });
        }

        break;
      case "candidate":
        console.log("Sending candidate to", data.name);
        var conn = users[data.name];

        if (conn != null) {
          sendTo(conn, {
            type: "candidate",
            candidate: data.candidate
          });
        }

        break;
      case "leave":
        console.log("Disconnecting user from", data.name);
        var conn = users[data.name];

        if (conn != null) {
          sendTo(conn, {
            type: "leave"
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

  sendTo(connection, "Hello World");
});

function sendTo(conn, message) {
  conn.send(JSON.stringify(message));
}
