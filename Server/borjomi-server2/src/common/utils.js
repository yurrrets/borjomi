export function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function _wrapDate(obj){

  // walk trhoug all object and replcace Date with json { _dtype:'date', value:val.toISOString() }
  var objToSee=[ obj ]

  while (objToSee.length>0){

    obj=objToSee.pop()

    for (var key in obj)
    {
      var val=obj[key]

      if (val instanceof Date) {
        if (isNaN(val.getTime())) // invalid date case
          obj[key]={ _dtype:'date', value:'' }
        else
          obj[key]={ _dtype:'date', value: val.toISOString() }
      }if ( typeof val==='object' && val!==null ){
        objToSee.push(val)
      }
  
    }
    
  }
}

export function customJSONStringify(obj){
  _wrapDate(obj)
  return JSON.stringify(obj, (k,val)=>{
    if (typeof val === 'number'){
      if (isFinite(val)) return val;
      return {_dtype:'number', value: val.toString() }
    }
    return val
  })
}

export function customJSONparse(str,blobs=undefined){
  return JSON.parse(str,function (k, v) {
    if (k === '') return v
    if (v===null) return v
    if (typeof (v) == 'object' ) {

      if ('_dtype' in v){
        switch (v._dtype){
          case 'number': return Number(v.value)
          case 'date': return new Date(v.value)
        }  
      }else if ('_blobN' in v){
        return blobs[v._blobN]
      }
    }
    
    return v
  })
}


function isObject(item) {
  return (item && typeof item === 'object' && !Array.isArray(item));
}

/**
 * Deep merge two objects.
 * @param target
 * @param ...sources
 */
export function mergeDeep(target, ...sources) {
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