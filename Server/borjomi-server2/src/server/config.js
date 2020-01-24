let config = {
    db: {
        host     : "localhost",
        user     : 'borjomi',
        password : 'borjomi_pass',
        database : 'borjomi',
        waitForConnections : true,
        connectionLimit    : 10,
        queueLimit         : 0
    }
}


  function isObject(item) {
    return (item && typeof item === 'object' && !Array.isArray(item));
  }
  
  /**
   * Deep merge two objects.
   * @param target
   * @param ...sources
   */
  function mergeDeep(target, ...sources) {
    if (!sources.length) return target;
    const source = sources.shift();
  
    if (isObject(target) && isObject(source)) {
      for (const key in source) {
        if (isObject(source[key])) {
          if (!target[key]) Object.assign(target, { [key]: {} });
          mergeDeep(target[key], source[key]);
        } else {
          Object.assign(target, { [key]: source[key] });
        }
      }
    }
  
    return mergeDeep(target, ...sources);
  }


try {
    let localCfg = require('../../config.json')
    config = mergeDeep({}, config, localCfg)
}
catch(err) {
    console.log(`load config.json failed: ${err}`)
}

module.exports = config