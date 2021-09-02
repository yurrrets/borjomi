const ar = require('./arduino')
import { sleep } from '../common/utils'


let stopRequest = false
const MainNode = 8

const sec = 1000 // in milliseconds
const min = 60 * sec

// interruptible sleep
async function delay(ms) {
    const finalMoment = Date.now() + ms
    while (true) {
        let tmDiff = finalMoment - Date.now()
        if (tmDiff <= 0) {
            return
        }
        if (stopRequest) {
            throw new Error('Stopped by User')
        }
        await sleep(Math.min(tmDiff, 100))
    }
}

async function turnNodes(availNodes, targetNodes, ms) {
    let intersection = targetNodes.filter(x => availNodes.includes(x));
    for (node of targetNodes) {
        if (!intersection.includes(node)) {
            // log node
            continue;
        }
        await ar.water(node, 0, true)
    }
    if (intersection.length > 0) {
        await delay(ms)
    }
    for (node of targetNodes) {
        if (!intersection.includes(node)) {
            continue;
        }
        await ar.water(node, 0, false)
    }
}

export async function start() {
    stopRequest = false

    // turn on power and see who's there
    await ar.dcadapter(MainNode, 0, true)
    await delay(5*sec)
    availNodes = await ar.pingAll()

    // turn on pump and wait for pressure
    await ar.pump(MainNode, 0, true)
    await delay(10*sec)

    await turnNodes(availNodes, [50], 5*min)
    await turnNodes(availNodes, [40,90], 5*min)
    await turnNodes(availNodes, [30], 5*min)
    await turnNodes(availNodes, [60], 5*min)
    await turnNodes(availNodes, [70], 5*min)
    await turnNodes(availNodes, [80], 5*min)
    await turnNodes(availNodes, [10], 5*min)
    await turnNodes(availNodes, [20], 5*min)

    // turn off pump and wait for zero pressure
    await ar.pump(MainNode, 0, false)
    await turnNodes(availNodes, [50], 10*sec)
}

export async function finish() {
    await ar.pump(MainNode, 0, false)
    await ar.dcadapter(MainNode, 0, false)
}

export function stop() {
    stopRequest = true
}