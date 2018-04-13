function hasUserMedia() {
  navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;
  return !!navigator.getUserMedia;
}

function hasRTCPeerConnection() {
  window.RTCPeerConnection = window.RTCPeerConnection || window.webkitRTCPeerConnection || window.mozRTCPeerConnection;
  return !!window.RTCPeerConnection;
}

var yourVideo = document.querySelector('#yours'),
    theirVideo = document.querySelector('#theirs'),
    yourConnection, theirConnection;

if (hasUserMedia()) {
  navigator.getUserMedia({ video: true, audio: false }, function (stream) {
    yourVideo.src = window.URL.createObjectURL(stream);

    if (hasRTCPeerConnection()) {
      startPeerConnection(stream);
    } else {
      alert("Sorry, your browser does not support WebRTC.");
    }
  }, function (error) {
    console.log(error);
  });
} else {
  alert("Sorry, your browser does not support WebRTC.");
}

function startPeerConnection(stream) {
  var configuration = {
    "iceServers": [{ "url": "stun:127.0.0.1:9876" }]
  };
  yourConnection = new webkitRTCPeerConnection(configuration);
  theirConnection = new webkitRTCPeerConnection(configuration);

  // Setup stream listening
  yourConnection.addStream(stream);
  theirConnection.onaddstream = function (e) {
    theirVideo.src = window.URL.createObjectURL(e.stream);
  };

  // Setup ice handling
  yourConnection.onicecandidate = function (event) {
    if (event.candidate) {
      theirConnection.addIceCandidate(new RTCIceCandidate(event.candidate));
    }
  };

  theirConnection.onicecandidate = function (event) {
    if (event.candidate) {
      yourConnection.addIceCandidate(new RTCIceCandidate(event.candidate));
    }
  };

  // Begin the offer
  yourConnection.createOffer(function (offer) {
    yourConnection.setLocalDescription(offer);
    theirConnection.setRemoteDescription(offer);

    theirConnection.createAnswer(function (offer) {
      theirConnection.setLocalDescription(offer);
      yourConnection.setRemoteDescription(offer);
    });
  });
};
