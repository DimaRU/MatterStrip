# Matter Power Strip controls 4 outlets and USB chager.

## Power strip configuration
Five default on-off plugin units have been created. You can create similar plugin units.

To update the existing CONFIG_GPIO_PLUG values, follow these steps:

1. Open a terminal.
1. Run the following command to access the configuration menu: 
`idf.py menuconfig`
1. Navigate to the "Power strip configuration" menu.
1. Update the GPIO pin number used for the factory reset button and plug output (**Use only available GPIO pins as per the target chip**).

You can update the number of plugin units from same menu.

The following table defines the default GPIO pin numbers for each supported target device.

| IO Function          | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S3 |
|----------------------|-------|----------|----------|----------|----------|----------|
| Factory Reset Button | 0     | 9        | 9        | 9        | 9        | 0        |
| Outlet 1             | 2     | 2        | 2        | 2        | 2        | 2        |
| Outlet 2             | 4     | 4        | 4        | 4        | 4        | 4        |
| Outlet 3             | 5     | 5        | 5        | 5        | 5        | 5        |
| Outlet 4             | 12    | 6        | 6        | 12       | 12       | 12       |
| Outlet 5             | 13    | 7        | 7        | 13       | 13       | 13       |
| Outlet 6             | 14    | 8        | 8        | 15       | 14       | 14       |



See the [docs](https://docs.espressif.com/projects/esp-matter/en/latest/esp32/developing.html) for more information about building and flashing the firmware.

Applications that do not require BLE post commissioning, can disable it using app_ble_disable() once commissioning is complete. 
It is not done explicitly because of a known issue with esp32c3 and will be fixed with the next IDF release (v4.4.2).
