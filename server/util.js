module.exports = {
    assertReading: (reading) => {
        if (!reading) {
            throw 'Invalid request body'
        }

        const { sensor, data } = reading

        if (!sensor || typeof sensor !== 'string' || sensor.length < 3) {
            throw 'Invalid or missing parameter "sensor"'
        }
        else if (!data) {
            throw 'Missing parameter data'
        }

        const { temperature, humidity, ligthlevel } = data

        if (!temperature || typeof temperature !== 'number') {
            throw 'Invalid or missing parameter "temperature"'
        }
        else if (!humidity || typeof humidity !== 'number') {
            throw 'Invalid or missing parameter "humidity"'
        }
        else if (!ligthlevel || typeof ligthlevel !== 'number') {
            throw 'Invalid or missing parameter "ligthlevel"'
        }
    }
}
