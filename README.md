# Fm tuner (si4703)

FM-tuner is a TCP server working on BeagleBoard. It uses the [si4703 tuner] (https://www.sparkfun.com/products/12938) to listen radio stations, and transmits RDS data to clients as well as information of the volume or the current channel. 

It works on the principle of subscription, It only transmits different data to each message. Consequently two successive messages cannot contain the same data.

__Example:__ The following RDS message: `FLASH_FM` is parsed by the service, it is broadcasted to the possible clients. If, it is again parsed by the service, it is not returned.

## Install

Copy this repository content in your BeagleBoard. Go inside the `service` folder and install it:

```
> make install
```

The default program path is: `/bin/fmtuner`.

## Usage

You can get a list of useful options in this way:

```
> /bin/fmtuner --help
Usage: ./bin/fmtuner [OPTION]...
  -h, --help           Print this helper.
  -i, --i2c-id=ID      Set the i2c bus id. Default: 1.
  -m, --max-clients=N  Set the number max of server clients. Default: 10.
  -p, --port=PORT      Set the server port. Default: 9502.
  -r, --reset-pin=PIN  Set the reset pin number of the fm tuner. Default: 45.
  -s, --sdio-pin=PIN   Set the sdio pin number of the fm tuner. Default: 12.
      --seek           Seek to locate radio stations.
```

You can use this program with systemd, you must define your BeagleBone pins in `fmtuner.service` using parameters before the installation.

## Client

A client is given in this repository. It is programmed in `nodeJS`, this code is not complicated and can transmit radio names given by the service to a `MQTT broker`. This client supports all messages currently sent by the service and can working on a BeagleBone or on another machine. However, beware ! You need a recent version of ` nodeJS` to use this client because it works with the `ES7 features`. See the results whith Kibana [here](https://github.com/PM2M2016-A-B/FM-tuner/blob/master/presentation/pres.pdf).

## Messages

A messages uses this structure: 
```
MSG_LENGTH(1 byte) TYPE_1(1 byte) VALUE_1(n bytes) [TYPE_N VALUE_N...]
```

### Service to client

Supported messages:

```
EVENT_MALFORMED_MESSAGE = 0x00 (no value)
EVENT_VOLUME            = 0x01 (value = 1 byte)
EVENT_CHANNEL           = 0x02 (value = 2 bytes)
EVENT_RADIO_NAME        = 0x05 (value = 1 for the length + n bytes of length)
EVENT_RADIO_TEXT        = 0x06 (value = 1 for the length + n bytes of length)
```

__Example:__ The server/service sends the volume 9 and channel 937 like this:

```
0x07 0x01 0x00 0x09 0x02 0x03 0xA9
```
__Note:__ The EVENT_MALFORMED_MESSAGE is sent to a client which made a bad request.

### Client to service

Supported messages:

```
EVENT_MALFORMED_MESSAGE = 0x00 (no value)
EVENT_VOLUME            = 0x01 (value = 1 byte)
EVENT_CHANNEL           = 0x02 (value = 2 bytes)
EVENT_SEEKUP            = 0x03 (no value)
EVENT_SEEKDOWN          = 0x04 (no value)
```

__Example:__ A client set the volume to 3 and seek up:

```
0x04 0x01 0x03 0x03
```

## License

GPLv3 © [GNU General Public License](http://www.gnu.org/licenses/gpl-3.0.en.html)
