const util = require('./util')

const express = require('express')
const ULID = require('ulid')

const app = express()
app.use(express.json())

let readings = [
    {
        id: 1,
        date: "2020-01-10T17:30:31.098Z",
        sensor: "fake_sensor",
        data: {
            temperature: 20.0,
            pressure: 990.0,
            humidity: 25.0,
            ligthlevel: 100.0
        }
    },
    {
        id: 2,
        date: "2020-01-10T17:30:31.098Z",
        sensor: "fake_sensor",
        data: {
            temperature: 21.0,
            pressure: 991.0,
            humidity: 30.0,
            ligthlevel: 100.0
        }
    }
]

app.get('/api/readings/:limit', (req, res) => {
    console.log('Recieved readings get request')
    const limit = req.params.limit
    res.json(readings.slice(-limit))
})

app.post('/api/readings', (req, res) => {
    console.log('Recieved readings post request')
    const body = req.body

    try {
        util.assertReading(body)
    }
    catch (err) {
        console.log(err)
        return res.status(400).send(err)
    }

    const reading = {
        id: ULID.ulid(),
        date: new Date(),
        sensor: body.sensor,
        data: body.data
    }

    readings = readings.concat(reading)

    return res.json(reading)
})

const PORT = 3001
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`)
})