console.log("Hello");

navigator.webkitGetUserMedia({ video: true, audio: true }, function (stream) {
  var video = document.querySelector('video');
  video.src = window.URL.createObjectURL(stream);
});
