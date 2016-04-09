# Fm tuner (si4703)

FM-tuner is a server working on BeagleBoard. It uses the [si4703 tuner] (https://www.sparkfun.com/products/12938) to listen radio stations, and transmits RDS data to clients as well as information of the volume or the current channel. 

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

A client is given in this repository. It is programmed in `nodeJS`, this code is not complicated and can transmit radio names given by the service to a `MQTT broker`. This client supports all messages currently sent by the service and can working on a BeagleBone or on another machine. However, beware ! You need a recent version of ` nodeJS` to use this client because it works with the `ES7 features`.

## License

GPLv3 © [GNU General Public License](http://www.gnu.org/licenses/gpl-3.0.en.html)
