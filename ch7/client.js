
var name,
    connectedUser;

var connection = new WebSocket('ws://localhost:8888');

connection.onopen = function () {
  console.log("Connected");
};

// Handle all messages through this callback
connection.onmessage = function (message) {
  console.log("Got message", message.data);

  var data = JSON.parse(message.data);

  switch(data.type) {
    case "login":
      onLogin(data.success);
      break;
    case "offer":
      onOffer(data.offer, data.name);
      break;
    case "answer":
      onAnswer(data.answer);
      break;
    case "candidate":
      onCandidate(data.candidate);
      break;
    case "leave":
      onLeave();
      break;
    default:
      break;
  }
};

connection.onerror = function (err) {
  console.log("Got error", err);
};

// Alias for sending messages in JSON format
function send(message) {
  if (connectedUser) {
    message.name = connectedUser;
  }

  connection.send(JSON.stringify(message));
};

var loginPage = document.querySelector('#login-page'),
    usernameInput = document.querySelector('#username'),
    loginButton = document.querySelector('#login'),
    theirUsernameInput = document.querySelector('#their-username'),
    connectButton = document.querySelector('#connect'),
    sharePage = document.querySelector('#share-page'),
    sendButton = document.querySelector('#send'),
    readyText = document.querySelector('#ready'),
    statusText = document.querySelector('#status');

sharePage.style.display = "none";
readyText.style.display = "none";

// Login when the user clicks the button
loginButton.addEventListener("click", function (event) {
  name = usernameInput.value;

  if (name.length > 0) {
    send({
      type: "login",
      name: name
    });
  }
});

function onLogin(success) {
  if (success === false) {
    alert("Login unsuccessful, please try a different name.");
  } else {
    loginPage.style.display = "none";
    sharePage.style.display = "block";

    // Get the plumbing ready for a call
    startConnection();
  }
};

var yourConnection, connectedUser, dataChannel, currentFile, currentFileSize, currentFileMeta;

function startConnection() {
  if (hasRTCPeerConnection()) {
    setupPeerConnection();
  } else {
    alert("Sorry, your browser does not support WebRTC.");
  }
}

function setupPeerConnection() {
  var configuration = {
    "iceServers": [{ "url": "stun:127.0.0.1:9876" }]
  };
  yourConnection = new RTCPeerConnection(configuration, {optional: []});

  // Setup ice handling
  yourConnection.onicecandidate = function (event) {
    if (event.candidate) {
      send({
        type: "candidate",
        candidate: event.candidate
      });
    }
  };

  openDataChannel();
}

function openDataChannel() {
  var dataChannelOptions = {
    ordered: true,
    reliable: true,
    negotiated: true,
    id: "myChannel"
  };
  dataChannel = yourConnection.createDataChannel("myLabel", dataChannelOptions);

  dataChannel.onerror = function (error) {
    console.log("Data Channel Error:", error);
  };

  dataChannel.onmessage = function (event) {
    try {
      var message = JSON.parse(event.data);

      switch (message.type) {
        case "start":
          currentFile = [];
          currentFileSize = 0;
          currentFileMeta = message.data;
          console.log("Receiving file", currentFileMeta);
          break;
        case "end":
          saveFile(currentFileMeta, currentFile);
          break;
      }
    } catch (e) {
      // Assume this is file content
      currentFile.push(atob(event.data));

      currentFileSize += currentFile[currentFile.length - 1].length;

      var percentage = Math.floor((currentFileSize / currentFileMeta.size) * 100);
      statusText.innerHTML = "Receiving... " + percentage + "%";
    }
  };

  dataChannel.onopen = function () {
    readyText.style.display = "inline-block";
  };

  dataChannel.onclose = function () {
    readyText.style.display = "none";
  };
}

// Alias for sending messages in JSON format
function dataChannelSend(message) {
  dataChannel.send(JSON.stringify(message));
}

function saveFile(meta, data) {
  var blob = base64ToBlob(data, meta.type);

  var link = document.createElement('a');
  link.href = window.URL.createObjectURL(blob);
  link.download = meta.name;
  link.click();
}

function hasUserMedia() {
  navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;
  return !!navigator.getUserMedia;
}

function hasRTCPeerConnection() {
  window.RTCPeerConnection = window.RTCPeerConnection || window.webkitRTCPeerConnection || window.mozRTCPeerConnection;
  window.RTCSessionDescription = window.RTCSessionDescription || window.webkitRTCSessionDescription || window.mozRTCSessionDescription;
  window.RTCIceCandidate = window.RTCIceCandidate || window.webkitRTCIceCandidate || window.mozRTCIceCandidate;
  return !!window.RTCPeerConnection;
}

function hasFileApi() {
  return window.File && window.FileReader && window.FileList && window.Blob;
}

connectButton.addEventListener("click", function () {
  var theirUsername = theirUsernameInput.value;

  if (theirUsername.length > 0) {
    startPeerConnection(theirUsername);
  }
});

function startPeerConnection(user) {
  connectedUser = user;

  // Begin the offer
  yourConnection.createOffer(function (offer) {
    send({
      type: "offer",
      offer: offer
    });
    yourConnection.setLocalDescription(offer);
  }, function (error) {
    alert("An error has occurred.");
  });
};

function onOffer(offer, name) {
  connectedUser = name;
  yourConnection.setRemoteDescription(new RTCSessionDescription(offer));

  yourConnection.createAnswer(function (answer) {
    yourConnection.setLocalDescription(answer);

    send({
      type: "answer",
      answer: answer
    });
  }, function (error) {
    alert("An error has occurred");
  });
};

function onAnswer(answer) {
  yourConnection.setRemoteDescription(new RTCSessionDescription(answer));
};

function onCandidate(candidate) {
  yourConnection.addIceCandidate(new RTCIceCandidate(candidate));
};

function onLeave() {
  connectedUser = null;
  yourConnection.close();
  yourConnection.onicecandidate = null;
  setupPeerConnection();
};

function arrayBufferToBase64(buffer) {
    var binary = '';
    var bytes = new Uint8Array( buffer );
    var len = bytes.byteLength;
    for (var i = 0; i < len; i++) {
        binary += String.fromCharCode( bytes[ i ] );
    }
    return btoa(binary);
}

function base64ToBlob(b64Data, contentType) {
    contentType = contentType || '';

    var byteArrays = [], byteNumbers, slice;

    for (var i = 0; i < b64Data.length; i++) {
      slice = b64Data[i];

      byteNumbers = new Array(slice.length);
      for (var n = 0; n < slice.length; n++) {
          byteNumbers[n] = slice.charCodeAt(n);
      }

      var byteArray = new Uint8Array(byteNumbers);

      byteArrays.push(byteArray);
    }

    var blob = new Blob(byteArrays, {type: contentType});
    return blob;
}

var CHUNK_MAX = 16000;
function sendFile(file) {
  var reader = new FileReader();

  reader.onloadend = function(evt) {
    if (evt.target.readyState == FileReader.DONE) {
      var buffer = reader.result,
          start = 0,
          end = 0,
          last = false;

      function sendChunk() {
        end = start + CHUNK_MAX;

        if (end > file.size) {
          end = file.size;
          last = true;
        }

        var percentage = Math.floor((end / file.size) * 100);
        statusText.innerHTML = "Sending... " + percentage + "%";

        dataChannel.send(arrayBufferToBase64(buffer.slice(start, end)));

        // If this is the last chunk send our end message, otherwise keep sending
        if (last === true) {
          dataChannelSend({
            type: "end"
          });
        } else {
          start = end;
          // Throttle the sending to avoid flooding
          setTimeout(function () {
            sendChunk();
          }, 100);
        }
      }

      sendChunk();
    }
  };

  reader.readAsArrayBuffer(file);
}

sendButton.addEventListener("click", function (event) {
  var files = document.querySelector('#files').files;

  if (files.length > 0) {
    dataChannelSend({
      type: "start",
      data: files[0]
    });

    sendFile(files[0]);
  }
});
