
sendButton.addEventListener("click", function (event) {
  var files = document.querySelector('#files').files;

  if (files.length > 0) {
    var reader = new FileReader(),
        file = files[0];

    reader.onloadend = function(evt) {
      if (evt.target.readyState == FileReader.DONE) {
        var buffer = reader.result,
            start = 0,
            end = file.size,
            last = false;

        var data = buffer.slice(start, end);

        // Now use the data in some way
      }
    };

    reader.readAsArrayBuffer(file);
  }
});
