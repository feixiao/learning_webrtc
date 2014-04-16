function hasUserMedia() {
  return !!(navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia);
}

if (hasUserMedia()) {
  navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;

  var constraints = {
    video: {
      mandatory: {
        minWidth: 640,
        minHeight: 480
      }
    },
    audio: true
  };

  if (/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)) {
    // The user is using a mobile device, lower our minimum resolution
    constraints = {
      video: {
        mandatory: {
          minWidth: 480,
          minHeight: 320,
          maxWidth: 1024,
          maxHeight: 768
        }
      },
      audio: true
    };
  }

  navigator.getUserMedia(constraints, function (stream) {
    var video = document.querySelector('video');
    video.src = window.URL.createObjectURL(stream);
  }, function (error) {
    console.log("Raised an error when capturing:", error);
  });
} else {
  alert("Sorry, your browser does not support getUserMedia.");
}
