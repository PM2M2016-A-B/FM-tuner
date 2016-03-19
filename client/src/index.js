import eventToPromise from 'event-to-promise'
import { Socket } from 'net'

const HOST = '192.168.7.2'
const PORT = 9502

const EVENT_VOLUME = 1
const EVENT_CHANNEL = 2

// ===================================================================

class Client {
  constructor () {
    this._socket = new Socket()
    this._buf = new Buffer(128)
    this._off = 0
  }

  _parseMsg (buf) {
    let i = 0

    while (i < buf.length) {
      const event = buf.readUInt8(i++)

      if (event === EVENT_VOLUME) {
        console.log(`Volume: ${buf.readUInt8(i)}.`)
        i++
      } else if (event === EVENT_CHANNEL) {
        console.log(`Channel: ${buf.readUInt16BE(i)}.`)
        i += 2
      } else {
        throw Error('Unknown event')
      }
    }
  }

  _onData (data) {
    const { _buf: buf } = this

    data.copy(buf, this._off)
    this._off += data.length

    try {
      while (this._off > 0) {
        const len = buf.readUInt8(0)

        if (this._off >= len) {
          this._parseMsg(buf.slice(1, len))
          buf.copy(buf, len, this._off)
          this._off -= len
        }
      }
    } catch (error) {
      this._off = 0
    }
  }

  async connect (host, port) {
    const { _socket } = this

    _socket.connect({ host, port })
    _socket.on('data', ::this._onData)
    await eventToPromise(_socket, 'connect')
  }
}

async function run () {
  const client = new Client()
  return client.connect(HOST, PORT)
}

// ===================================================================

run()
  .catch(::console.log)
