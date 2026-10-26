// Configure your device in config.local.js (ignored by Git).
const local = require('./config.local');
const apiBase = 'https://iot-api.heclouds.com';

module.exports = {
  authorization: local.authorization,
  product_id: local.product_id,
  device_name: local.device_name,
  getinfo_url: apiBase + '/thingmodel/query-device-property?product_id='
    + encodeURIComponent(local.product_id) + '&device_name='
    + encodeURIComponent(local.device_name),
  setinfo_url: apiBase + '/thingmodel/set-device-property'
};
