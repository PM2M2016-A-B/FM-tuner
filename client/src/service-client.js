/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

import eventToPromise from 'event-to-promise'
import { Socket } from 'net'

const EVENT_VOLUME = 0x01
const EVENT_CHANNEL = 0x02
const EVENT_RADIO_NAME = 0x05
const EVENT_RADIO_TEXT = 0x06

// ===================================================================

export default class ServiceClient {
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
        continue
      }

      if (event === EVENT_CHANNEL) {
        console.log(`Channel: ${buf.readUInt16BE(i)}.`)
        i += 2
        continue
      }

      let attr

      if (event === EVENT_RADIO_NAME) {
        attr = 'name'
      } else if (event === EVENT_RADIO_TEXT) {
        attr = 'text'
      } else {
        throw Error('Unknown event.')
      }

      const len = buf.readUInt8(i)
      console.log(`Radio ${attr}: ${buf.slice(i + 1, i + 1 + len)}`)
      i += len + 1
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
      const { _socket } = this
      _socket.on('end', reject)
      _socket.write(buf, () => {
        _socket.removeListener('end', reject)
        resolve()
      })
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
