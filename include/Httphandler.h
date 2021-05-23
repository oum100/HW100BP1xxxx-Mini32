

//Web Layout
const char index_html[] PROGMEM = R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <!-- Required meta tags -->
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

        <!-- Bootstrap CSS -->
        <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css"
        integrity="sha384-Vkoo8x4CGsO3+Hhxv8T/Q5PaXtkKtu6ug5TOeNV6gBiFeWPGFN9MuhOf23Q9Ifjh" crossorigin="anonymous">

        <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css">
        <title>Kiosk System</title>
    </head>

    <body>
        <div></div>
        <div class="container-fluid">
            <!-- Jumbotron -->
            <br>
            <div class="jumbotron text-center" style="margin-bottom:0">
            <div class="container" style="height:40px;">
            <h1><span class="fa fa-television"></span> Kiosk System</h1>
            <p>This is main page system page.!</p>   
            </div>
            </div>
            <nav class="navbar navbar-expand-sm bg-light navbar-light">
            <!-- Brand/logo -->
            <a class="navbar-brand" href="https://www.washcoin.net">
                <img src="https://www.washcoin.net/wp-content/uploads/2017/11/logo_washcoin_retina.png" alt="logo" style="width:100px;">
            </a>

            <!-- Links -->
            <ul class="navbar-nav">
                <li class="nav-item active"><a class="nav-link" href="/index.html">Status</a></li>
                <li class="nav-item dropdown">
                <a class="nav-link dropdown-toggle" href="#" id="configdropdown" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">Configuration</a>
                <div class="dropdown-menu" aria-labelledby="navbarDropdown">
                    <a class="dropdown-item" href="/cfgWiFi.html">WiFi</a>
                    <a class="dropdown-item" href="/cfgDevice.html">Price</a>
                </div>
                </li>
                <li class="nav-item"><a class="nav-link" href="#">Reboot</a></li>
                <li class="nav-item"><a class="nav-link" href="javascript:logoutButton();">Logout</a></li>
            </ul>
            </nav>
        </div>
        <br>
        <div class="container-sm">
            <div class="row">
                <div class="col-sm-6">
                <table class="table table-sm table-hover" style="width:400px">
                    <tbody id="myTable">
                      %PAGEBODY%
                    </tbody>
                </table>
                </div>
                <div class="col-sm-6 "></div>
            </div>
        </div>
        

        <!-- Optional JavaScript -->
        <!-- jQuery first, then Popper.js, then Bootstrap JS -->
        <script src="https://code.jquery.com/jquery-3.4.1.slim.min.js" integrity="sha384-J6qa4849blE2+poT4WnyKhv5vZF5SrPo0iEjwBvKU7imGFAV0wwj1yYfoRSJoZ+n" crossorigin="anonymous"></script>
        <script src="https://cdn.jsdelivr.net/npm/popper.js@1.16.0/dist/umd/popper.min.js" integrity="sha384-Q6E9RHvbIyZFJoft+2mJbHaEWldlvI9IOYy5n3zV9zzTtmI3UksdQRVvoxMfooAo" crossorigin="anonymous"></script>
        <script src="https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/js/bootstrap.min.js" integrity="sha384-wfSDF2E50Y2D1uUdj0O3uMBJnjuUD4Ih7YwaYd1iqfktj0Uod8GCExl3Og8ifwB6" crossorigin="anonymous"></script>
        <script>
            function logoutButton() {
            var xhr = new XMLHttpRequest();
            xhr.open("GET", "/logout", true);
            xhr.send();
            setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
            }
        </script>
    </body>
    </html>
)rawliteral";


const char logout_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <body>
    <p>Logged out or <a href="/">return to homepage</a>.</p>
    <p><strong>Note:</strong> close all web browser tabs to complete the logout process.</p>
  </body>
  </html>
)rawliteral";

const char reboot_html[] PROGMEM = R"rawliteral(
)rawliteral";