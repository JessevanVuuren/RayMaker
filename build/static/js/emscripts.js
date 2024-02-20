function saveFileFromMEMFSToDisk(memoryFSname, localFSname)
{
  var isSafari = false; // Not supported, navigator.userAgent access is being restricted
  //var isSafari = /^((?!chrome|android).)*safari/i.test(navigator.userAgent);
  var data = FS.readFile(memoryFSname);
  var blob;

  if (isSafari) blob = new Blob([data.buffer], { type: "application/octet-stream" });
  else blob = new Blob([data.buffer], { type: "application/octet-binary" });

  // NOTE: SaveAsDialog is a browser setting. For example, in Google Chrome,
  // in Settings/Advanced/Downloads section you have a setting:
  // 'Ask where to save each file before downloading' - which you can set true/false.
  // If you enable this setting it would always ask you and bring the SaveAsDialog
  saveAs(blob, localFSname);
}

var Module = {
  onRuntimeInitialized: function() {
    c_upload_model = Module.cwrap('set_upload_model', null, ["string", "string", "string"]);
    c_get_model_list = Module.cwrap('get_object_list', "string", []);
  },
  canvas: (function () {
    var canvas = document.getElementById('canvas');
    canvas.width = window.innerWidth
    canvas.height = window.innerHeight
    return canvas;
  })()
};