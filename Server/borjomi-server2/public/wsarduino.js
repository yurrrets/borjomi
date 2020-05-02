
// define execRemoteFunction in the place of usage
// this could be wsCallFunction or sendMsgLikeWsCallFunction or smth else

async function arduino_ping(address) {
    return await execRemoteFunction({
        'function': "arduino.ping",
        'address': address
    })
}

async function arduino_version(address) {
    return await execRemoteFunction({
        'function': "arduino.version",
        'address': address
    })
}

async function arduino_capabilities(address) {
    return await execRemoteFunction({
        'function': "arduino.capabilities",
        'address': address
    })
}

/*
* @param {state} optional: if provided, sets water switch;
* otherwise gets current state of water switch
*/
async function arduino_water(address, num, state) {
    let ret = await execRemoteFunction({
        'function': "arduino.water",
        'address': address,
        'num': num,
        'state': state == undefined ? undefined : state ? 1 : 0
    })
    if (ret.state !== undefined) {
        ret.state = ret.state ? true : false
    }
    return ret
}

async function arduino_soil(address, num) {
    return await execRemoteFunction({
        'function': "arduino.soil",
        'address': address,
        'num': num
    })
}

async function arduino_pressure(address, num) {
    return await execRemoteFunction({
        'function': "arduino.pressure",
        'address': address,
        'num': num
    })
}

async function arduino_current(address, num) {
    return await execRemoteFunction({
        'function': "arduino.current",
        'address': address,
        'num': num
    })
}

async function arduino_voltage(address, num) {
    return await execRemoteFunction({
        'function': "arduino.voltage",
        'address': address,
        'num': num
    })
}

async function arduino_dcadapter(address, num, state) {
    let ret = await execRemoteFunction({
        'function': "arduino.dcadapter",
        'address': address,
        'num': num,
        'state': state == undefined ? undefined : state ? 1 : 0
    })
    if (ret.state !== undefined) {
        ret.state = ret.state ? true : false
    }
    return ret
}

async function arduino_pump(address, num, state) {
    let ret = await execRemoteFunction({
        'function': "arduino.pump",
        'address': address,
        'num': num,
        'state': state == undefined ? undefined : state ? 1 : 0
    })
    if (ret.state !== undefined) {
        ret.state = ret.state ? true : false
    }
    return ret
}
