# EPS8266 plugin framework

## Getting this to work

Put your wifi credentials in `config.lua` and activate the plugins you want to use by uncommenting them.

Upload the files `init.lua`, `config.lua`, `wifi_connect.lua` and the plugins (and their dependencies) you activated to your ESP.

```
./luatool.py -f init.lua
./luatool.py -f config.lua
./luatool.py -f wifi_connect.lua
```

## Troubleshooting

If you get `Not enough memory` errors compile the lib you want to load (via console on the esp):

```
> node.compile('file.lua')
```
then restart and it should work.


# dht_reader.lua

Push DHT22 data with ESP8266 to api.dusti.xyz

## Dependencies

* [dht_lib.lua](https://github.com/nodemcu/nodemcu-firmware/tree/master/lua_modules/dht_lib/dht_lib.lua)
* `drf_api.lua`

## Instructions

Connect the DHT22 datapin to GPIO2
