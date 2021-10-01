# Wio_Terminal_IoTHub_Communication

# Connect your IoT device to the cloud - Wio Terminal

In this part of the lesson, you will connect your Wio Terminal to your IoT Hub, to send telemetry.

## Connect your device to IoT Hub

The next step is to connect your device to IoT Hub.

### Task - connect to Wifi

1. Add the following library dependencies:
    ```
        seeed-studio/Seeed Arduino rpcWiFi @ 1.0.5
        seeed-studio/Seeed Arduino FS @ 2.1.1
        seeed-studio/Seeed Arduino SFUD @ 2.0.2
        seeed-studio/Seeed Arduino rpcUnified @ 2.1.3
        seeed-studio/Seeed_Arduino_mbedtls @ 3.0.1
    ```

2. Add below method in setup defination and add wifi ssid and password info in config.h
    ```
    void connectWiFi()
    {
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Connecting to WiFi..");
            WiFi.begin(SSID, PASSWORD);
            delay(500);
        }

        Serial.println("Connected!");
    }
    ```

### Task - connect to IoT Hub

1. Open the project in VS Code

2. Add the following library dependencies:

    ```ini
    seeed-studio/Seeed Arduino RTC @ 2.0.0
    arduino-libraries/AzureIoTHub @ 1.6.0
    azure/AzureIoTUtility @ 1.6.1
    azure/AzureIoTProtocol_MQTT @ 1.6.0
    azure/AzureIoTProtocol_HTTP @ 1.6.0
    azure/AzureIoTSocket_WiFi @ 1.0.2
    ```

    The `Seeed Arduino RTC` library provides code to interact with a real-time clock in the Wio Terminal, used to track the time. The remaining libraries allow your IoT device to connect to IoT Hub.

3. Add the following to the bottom of the `platformio.ini` file:

    ```ini
    build_flags =
        -DDONT_USE_UPLOADTOBLOB
    ```

    This sets a compiler flag that is needed when compiling the Arduino IoT Hub code.

4 . Open the `config.h` header file and add the following constant for the device connection string:

    ```cpp
    // IoT Hub settings
    const char *CONNECTION_STRING = "<connection string>";
    ```

    Replace `<connection string>` with the connection string for your device you copied earlier.

5. The connection to IoT Hub uses a time-based token. This means the IoT device needs to know the current time. Unlike operating systems like Windows, macOS or Linux, microcontrollers don't automatically synchronize the current time over the Internet. This means you will need to add code to get the current time from an [NTP](https://wikipedia.org/wiki/Network_Time_Protocol) server. Once the time has been retrieved, it can be stored in a real-time clock in the Wio Terminal, allowing the correct time to be requested at a later date, assuming the device doesn't lose power. Add a new file called `ntp.h` with the existing code.

### Connection

1. Add the following `#include` directives to the top of the `main.cpp` file to include header files for the IoT Hub libraries, and for setting the time:

    ```cpp
    #include <AzureIoTHub.h>
    #include <AzureIoTProtocol_MQTT.h>
    #include <iothubtransportmqtt.h>
    #include "ntp.h"
    ```

1. Add the following call to the end of the `setup` function to set the current time:

    ```cpp
    initTime();
    ```

1. Add the following variable declaration to the top of the file, just below the include directives:

    ```cpp
    IOTHUB_DEVICE_CLIENT_LL_HANDLE _device_ll_handle;
    ```

    This declares an `IOTHUB_DEVICE_CLIENT_LL_HANDLE`, a handle to a connection to the IoT Hub.

1. Below this, add the following code:

    ```cpp
    static void connectionStatusCallback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
    {
        if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
        {
            Serial.println("The device client is connected to iothub");
        }
        else
        {
            Serial.println("The device client has been disconnected");
        }
    }
    ```

    This declares a callback function that will be called when the connection to the IoT Hub changes status, such as connecting or disconnecting. The status is sent to the serial port.

1. Below this, add a function to connect to IoT Hub:

    ```cpp
    void connectIoTHub()
    {
        IoTHub_Init();
    
        _device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(CONNECTION_STRING, MQTT_Protocol);
        
        if (_device_ll_handle == NULL)
        {
            Serial.println("Failure creating Iothub device. Hint: Check your connection string.");
            return;
        }
    
        IoTHubDeviceClient_LL_SetConnectionStatusCallback(_device_ll_handle, connectionStatusCallback, NULL);
    }
    ```

    This code initializes the IoT Hub library code, then creates a connection using the connection string in the `config.h` header file. This connection is based on MQTT. If the connection fails, this is sent to the serial port - if you see this in the output, check the connection string. Finally the connection status callback is set up.

1. Call this function in the `setup` function below the call to `initTime`:

    ```cpp
    connectIoTHub();
    ```

1. Just like with the MQTT client, this code runs on a single thread so needs time to process messages being sent by the hub, and sent to the hub. Add the following to the top of the `loop` function to do this:

    ```cpp
    IoTHubDeviceClient_LL_DoWork(_device_ll_handle);
    ```

1. Build and upload this code. You will see the connection in the serial monitor:

    ```output
    Connecting to WiFi..
    Connected!
    Fetched NTP epoch time is: 1619983687
    The device client is connected to iothub
    ```

## Send telemetry

Now that your device is connected, you can send telemetry to the IoT Hub.

### Task - send telemetry

1. Add the following function above the `setup` function:

    ```cpp
    void sendTelemetry(const char *telemetry)
    {
        IOTHUB_MESSAGE_HANDLE message_handle = IoTHubMessage_CreateFromString(telemetry);
        IoTHubDeviceClient_LL_SendEventAsync(_device_ll_handle, message_handle, NULL, NULL);
        IoTHubMessage_Destroy(message_handle);
    }
    ```

    This code creates an IoT Hub message from a string passed as a parameter, sends it to the hub, then cleans up the message object.

2. Call this code in the `loop` function, just after the line where the telemetry is sent to the serial port:

    ```cpp
    sendTelemetry("telemetry-string");
    ```