function hasUserMedia() {
  return !!(navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia);
}

if (hasUserMedia()) {
  navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;

  navigator.getUserMedia({
    video: {
      mandatory: {
        maxWidth: 320,
        maxHeight: 480
      },
      optional: [
        { maxWidth: 640 },
        { maxHeigth: 480 }
      ]
    },
    audio: false
  }, function (stream) {
    var video = document.querySelector('video');
    video.src = window.URL.createObjectURL(stream);
  }, function (error) {
    console.log("Raised an error when capturing:", error);
  });
} else {
  alert("Sorry, your browser does not support getUserMedia.");
}
