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

import mqtt from 'mqtt'
import eventToPromise from 'event-to-promise'
import ServiceClient from './service-client'

const HOST = '192.168.7.2'
const PORT = 9502

const DEFAULT_VOLUME = 1
const DEFAULT_CHANNEL = 978

const MQTT_URL = ''
const MQTT_TOPIC = ''
const ENABLE_MQTT = 0

// ===================================================================

async function run () {
  let mqttClient

  if (ENABLE_MQTT) {
    mqttClient = mqtt.connect(MQTT_URL, {
      protocolId: 'MQIsdp',
      protocolVersion: 3
    })

    await eventToPromise(mqttClient, 'connect')
    await new Promise((resolve, reject) => {
      mqttClient.subscribe(MQTT_TOPIC, (err, granted) => {
        if (err) reject(err)
        resolve(granted)
      })
    })
  }

  const client = new ServiceClient({
    actions: {
      radioName: ENABLE_MQTT && (name => {
        console.log(`radioName: '${name}'`)
        // mqttClient.publish(MQTT_TOPIC, name)
      }) || undefined,
      radioText: ENABLE_MQTT && (text => {
        console.log(`radioText: '${text}'`)
        mqttClient.publish(MQTT_TOPIC, text)
      }) || undefined
    }
  })

  await client.connect(HOST, PORT)
  await Promise.all([
    client.setVolume(DEFAULT_VOLUME),
    client.setChannel(DEFAULT_CHANNEL)
  ])
  await client.waitEndConnection()

  throw new Error('Disconnected!')
}

// ===================================================================

run()
  .catch(::console.log)
