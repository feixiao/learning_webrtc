var mobile = {
  video: {
    mandatory: {
      maxWidth: 640,
      maxHeight: 360
    }
  }
};

var desktop = {
  video: {
    mandatory: {
      minWidth: 1280,
      minHeight: 720
    }
  }
};

var constraints;

if(/Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent)) {
  constraints = mobile;
} else {
  constraints = desktop;
}

navigator.getUserMedia(constraints, success, function (error) {
  // Possibly try again at a different resolution
});
