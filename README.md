# Fm tuner (si4703)

TODO: Description

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

## License

GPLv3 © [GNU General Public License](http://www.gnu.org/licenses/gpl-3.0.en.html)
