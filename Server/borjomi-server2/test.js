// const db = require('./db')


// async function main() {
//     r = await db.login('test', 'test')
//     // r = 5
//     console.log(r)
// }

// main().then().catch(console.log)

function timeout(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function sleep() {
    await timeout(50)
    console.log(20)
}

var busy = false

setInterval(async () => {
    if (busy) {
        console.log("busy")
        return
    }
    busy = true
    console.log("in")
    await sleep()
    console.log("out")
    busy = false
}, 10);
console.log("0")