const log4js = require('log4js')
const logger = log4js.getLogger('script_prog1')

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
    let intersection = targetNodes.filter(x => availNodes.includes(x))
    
    const notFoundNodes = targetNodes.filter(x => !intersection.includes(x))
    if (notFoundNodes.length > 0)
        logger.warn("nodes are offline:", notFoundNodes)

    for (const node of targetNodes) {
        if (!intersection.includes(node)) {
            // log node
            continue;
        }
        logger.debug("turning on node", node)
        await ar.water(node, 0, true)
    }
    if (intersection.length > 0) {
        await delay(ms)
    }
    for (const node of targetNodes) {
        if (!intersection.includes(node)) {
            continue;
        }
        logger.debug("turning off node", node)
        await ar.water(node, 0, false)
    }
}

export async function start() {
    logger.debug("start")
    stopRequest = false

    // logger.debug("turning power on")
    // await ar.dcadapter(MainNode, 0, true)
    // await delay(5*sec)

    logger.debug("see who's there")
    const availNodes = await ar.pingAll()
    logger.debug("answered: ", availNodes)

    logger.debug("turn on pump and wait for pressure")
    await ar.pump(MainNode, 0, true)
    await delay(10*sec)

    logger.debug("starting main cycle")
    await turnNodes(availNodes, [50], 5*min)
    await turnNodes(availNodes, [40,90], 5*min)
    await turnNodes(availNodes, [30], 5*min)
    await turnNodes(availNodes, [60], 5*min)
    await turnNodes(availNodes, [70], 5*min)
    await turnNodes(availNodes, [80], 5*min)
    await turnNodes(availNodes, [10], 5*min)
    await turnNodes(availNodes, [20], 5*min)

    await turnNodes(availNodes, [120], 5*sec)
    await turnNodes(availNodes, [170], 5*sec)

    logger.debug("turn off pump and wait for zero pressure")
    await ar.pump(MainNode, 0, false)
    await turnNodes(availNodes, [50], 10*sec)

    logger.debug("done")
}

export async function finish() {
    await ar.pump(MainNode, 0, false)
    await ar.dcadapter(MainNode, 0, false)
}

export function stop() {
    stopRequest = true
}