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
  constructor ({
    actions
  } = {}) {
    this._socket = new Socket()
    this._buf = new Buffer(128)
    this._off = 0

    for (const attr of [ 'volume', 'channel', 'radioName', 'radioText' ]) {
      if (actions[attr] === undefined) {
        actions[attr] = val => { console.log(`${attr}: '${val}'`) }
      }
    }

    this._actions = actions
  }

  _parseMsg (buf) {
    let i = 0
    const { _actions: actions } = this

    while (i < buf.length) {
      const event = buf.readUInt8(i++)

      if (event === EVENT_VOLUME) {
        actions.volume(buf.readUInt8(i))
        i++
        continue
      }

      if (event === EVENT_CHANNEL) {
        actions.channel(buf.readUInt16BE(i))
        i += 2
        continue
      }

      if (event !== EVENT_RADIO_NAME && event !== EVENT_RADIO_TEXT) {
        throw Error('Unknown event.')
      }

      const len = buf.readUInt8(i)
      const start = i + 1
      const radioData = buf.slice(start, start + len)

      if (event === EVENT_RADIO_NAME) {
        actions.radioName(radioData)
      } else {
        actions.radioText(radioData)
      }

      i += len + 1
    }
  }

  _onData (data) {
    const { _buf: buf } = this

    data.copy(buf, this._off)
    this._off += data.length

    if (this._off >= buf.length) {
      throw new Error('The buffer is full.')
    }

    while (this._off > 0) {
      const len = buf.readUInt8(0)

      if (this._off >= len) {
        this._parseMsg(buf.slice(1, len))
        buf.copy(buf, 0, len)
        this._off -= len
      } else {
        break
      }
    }
  }

  async _send (buf) {
    return new Promise((resolve, reject) => {
      const { _socket: socket } = this
      socket.on('end', reject)
      socket.write(buf, () => {
        socket.removeListener('end', reject)
        resolve()
      })
    })
  }

  async connect (host, port) {
    const { _socket: socket } = this
    socket.connect({ host, port })
    socket.on('data', data => {
      try {
        this._onData(data)
      } catch (error) {
        console.error(error)
        process.exit(1)
      }
    })

    this.endConnection = eventToPromise(socket, 'end')
    await eventToPromise(socket, 'connect')
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

  async waitEndConnection () {
    return this.endConnection
  }
}
