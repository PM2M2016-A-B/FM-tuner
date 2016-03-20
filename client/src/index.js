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

import ServiceClient from './service-client'

const HOST = '192.168.7.2'
const PORT = 9502

const DEFAULT_VOLUME = 1
const DEFAULT_CHANNEL = 931

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
