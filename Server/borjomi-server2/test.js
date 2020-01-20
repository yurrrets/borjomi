const db = require('./db')


async function main() {
    r = await db.login('test', 'test')
    // r = 5
    console.log(r)
}

main().then().catch(console.log)