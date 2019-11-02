# ESP-Garage

ESP-Garage is a simple garage door opener using ESP8266.

The ESP8266 hosts a webpage with a button that when clicked will open or close your garage door. The ESP needs to be connected to the garage door opener via a relay or some other switching mechanism. Most garage door openers will open when the switch terminals are shorted, so I used a relay with the normally open connection wired to the switch terminals.

The relay can be activated from the button on the ESP's webpage, from a POST request with the parameter `toggle=true`, or through Thinger.io if enabled.

### How to install / configure
This project is built using PlatformIO. You will need the WebSockets and (optionally) Thinger.io libraries that can be installed from the PlatformIO library manager. Once you have the libraries installed, rename *keys.h.template* to *keys.h* and fill in your WiFi credentials as well as the Thinger credentials if you are using it. If you are not using Thinger, comment out the third line in *main.cpp* to disable the library.

Other configuration items:
* Change MDNS_NAME to change the network name this device uses. The default is "garage", so you can navigate to "http://garage.local" on a computer running Bonjour to visit the local webpage.
* Change BUTTON_DELAY to change the amount of time in milliseconds the relay is brought HIGH for in order to activate the garage door opener.
* Change relayPin to change which pin is used on the ESP8266 to control the relay.

### Thinger.io <-> IFTTT setup
You can set up Thinger.io to accept web requests from IFTTT in order to allow Google Home or Alexa to control the garage. Sign up for Thinger and create a new device. In *keys.h*, add your username as well as device ID and credential. When you upload the new firmware to the ESP8266, you can now toggle the relay via the Thinger API Explorer. If that's working, you can add a device token for IFTTT. In IFTTT, create a new applet. I set the event to Google Assistant and the action to a webhook POST request to "https://api.thinger.io/v2/users/Username/devices/DeviceID/toggle?authorization=DeviceToken". Replace Username, DeviceID, and DeviceToken with your own.

Thinger has provided much more detail in their own blog post here: https://community.thinger.io/t/make-your-iot-things-react-to-hundred-of-ifttt-events/41
