String rootHTML = R"=====(
  <!DOCTYPE HTML>
  <html>
    <link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'>
    <link rel='manifest' href='/manifest.json'>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="apple-mobile-web-app-capable" content="yes">
    <head>
      <title>Garage Door Opener</title>
      <style>
        #status {
          font-style: italic;
          color: grey;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <div class="row">
          <div class="col-md-12">
            <h1 class="display-1">Garage Door Opener</h1>
          </div>
        </div>
        <hr>
        <div class="row">
          <div class="col-md-4 col-md-offset-4">
            <button id="togglebutton" class="btn btn-block btn-lg btn-danger disabled">Open/close</button>
          </div>
        </div>
        <div class="row">
          <div class="col-md-4 col-md-offset-4">
            <button id="refreshbutton" class="btn btn-block btn-lg" style="margin-top: 10px">Refresh</button>
          </div>
        </div>
      </div>

      <div class="navbar navbar-fixed-bottom text-center">
        <div class="container">
          <p id="status">Disconnected</p>
        </div>
      </div>

      <script>
        var ws;
        $(document).ready(function() {
          connectWebSocket();
        });

        $("#togglebutton").click(function () {
          ws.send("t");
        });

        $("#refreshbutton").click(function () {
          setConnected(false);
          ws.close();
          connectWebSocket();
        });

        function setConnected(isConnected) {
          if(isConnected)
          {
            $("#togglebutton").removeClass("disabled");
            $("#status").text("Connected");
          }
          else
          {
            $("#togglebutton").addClass("disabled");
            $("#status").text("Disconnected");
          }
        }

        function connectWebSocket() {
          ws = new WebSocket('ws://'+location.hostname+':81', ['arduino']);
          ws.onopen = function () {
            setConnected(true);
          };
          ws.onclose = function(message) {
            setConnected(false);
          };
          ws.onerror = function(error) {
            setConnected(false);
          };
          ws.onmessage = function(message) {
            console.log(message.data);
          };
        }
      </script>
    </body>
  </html>
)=====";


String manifest = R"=====(
  {
    "name": "Garage Door Opener",
    "short_name": "Garage Door",
    "display": "standalone"
  }
)=====";
