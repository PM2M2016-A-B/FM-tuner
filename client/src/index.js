import eventToPromise from 'event-to-promise'
import { Socket } from 'net'

const HOST = '192.168.7.2'
const PORT = 9502

const EVENT_VOLUME = 0x01
const EVENT_CHANNEL = 0x02

const DEFAULT_VOLUME = 1
const DEFAULT_CHANNEL = 931

// ===================================================================

class ServiceClient {
  constructor () {
    this._socket = new Socket()
    this._buf = new Buffer(128)
    this._off = 0
  }

  _parseServiceMsg (buf) {
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
        throw Error('Unknown event.')
      }
    }
  }

  _onData (data) {
    const { _buf: buf } = this

    data.copy(buf, this._off)
    this._off += data.length

    try {
      if (this._off >= buf.length) {
        throw new Error('The buffer is full.')
      }

      while (this._off > 0) {
        const len = buf.readUInt8(0)

        if (this._off >= len) {
          this._parseServiceMsg(buf.slice(1, len))
          buf.copy(buf, len, this._off)
          this._off -= len
        } else {
          break
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

    this._end = eventToPromise(_socket, 'end')
    await eventToPromise(_socket, 'connect')
  }

  async _send (buf) {
    return new Promise((resolve, reject) => {
      this._socket.on('end', reject)
      this._socket.write(buf, resolve)
    })
  }

  async setVolume (volume) {
    return this._send(
      new Buffer([ 0x03, EVENT_VOLUME, volume & 0xFF ])
    )
  }

  async setChannel (channel) {
    const buf = new Buffer(4)

    buf.writeUInt8(0x04, 0)
    buf.writeUInt8(EVENT_CHANNEL, 1)
    buf.writeUInt16BE(channel & 0xFFFF, 2)

    return this._send(buf)
  }

  async waitEnd () {
    return this._end
  }
}

async function run () {
  const client = new ServiceClient()

  await client.connect(HOST, PORT)
  await Promise.all([
    client.setVolume(DEFAULT_VOLUME),
    client.setChannel(DEFAULT_CHANNEL)
  ])
  await client.waitEnd().then(() => {
    throw new Error('Disconnected!')
  })
}

// ===================================================================

run()
  .catch(::console.log)
