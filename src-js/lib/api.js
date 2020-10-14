// const api = (url) => 'http://localhost:4010' + url;
const api = (url) => url;

export const URL_REBOOT = api('/reboot');

export const URL_WIFI = api('/wifi');
export const URL_WIFI_SCAN = api('/wifi/scan');
export const URL_WIFI_CONNECT = api('/wifi/connect');
export const URL_WIFI_DISCONNECT = api('/wifi/disconnect');

export const URL_SETTINGS = api('/settings');
