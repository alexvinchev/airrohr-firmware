const char TXT_CONTENT_TYPE_JSON[] PROGMEM = "application/json";
const char TXT_CONTENT_TYPE_INFLUXDB[] PROGMEM = "application/x-www-form-urlencoded";
const char TXT_CONTENT_TYPE_TEXT_HTML[] PROGMEM = "text/html; charset=utf-8";
const char TXT_CONTENT_TYPE_TEXT_PLAIN[] PROGMEM = "text/plain";
const char TXT_CONTENT_TYPE_IMAGE_SVG[] PROGMEM = "image/svg+xml";
const char TXT_CONTENT_TYPE_IMAGE_PNG[] PROGMEM = "image/png";

const char DBG_TXT_TEMPERATURE[] PROGMEM = "Temperature: ";
const char DBG_TXT_HUMIDITY[] PROGMEM = "Humidity: ";
const char DBG_TXT_PRESSURE[] PROGMEM = "Pressure: ";
const char DBG_TXT_START_READING[] PROGMEM = "Start reading ";
const char DBG_TXT_END_READING[] PROGMEM = "End reading ";

const uint8_t start_SDS_cmd[] PROGMEM = {0xAA, 0xB4, 0x06, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x06, 0xAB};
const uint8_t start_SDS_cmd_len = 19;
const uint8_t stop_SDS_cmd[] PROGMEM = {0xAA, 0xB4, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x05, 0xAB};
const uint8_t stop_SDS_cmd_len = 19;
const uint8_t version_SDS_cmd[] PROGMEM = {0xAA, 0xB4, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x05, 0xAB};
const uint8_t version_SDS_cmd_len = 19;

const char WEB_PAGE_HEADER[] PROGMEM = "<html>\
<head>\
<title>{t}</title>\
<meta name='viewport' content='width=device-width'><html lang='en'>\
<style type='text/css'>\
body{font-family:Arial;margin:0}\
.content{margin:10px}\
.r{text-align:right}\
td{vertical-align:top;}\
a{text-decoration:none;padding:10px;background:#a0ffa0;color:black;display:block;width:auto;border-radius:5px;}\
input[type='text']{width:100%;}\
input[type='password']{width:100%;}\
input[type='submit']{border-radius:5px;font-size:medium;padding:5px;}\
.submit_green{padding:9px !important;width:100%;border-style:none;background:#a0ffa0;color:black;text-align:left;}\
</style>\
</head><body>\
<div style='min-height:120px;background-color:#a0ffa0;margin-bottom:20px'>\
<h3 style='margin:0'>{tt}</h3>\
<small>ID: {id}<br/>MAC: {mac}<br/>{fwt}: {fw}</small></div><div class='content'><h4>{h} {n} {t}</h4>";

const char WEB_PAGE_FOOTER[] PROGMEM = "<br/><br/><a href='/' style='display:inline;'>{t}</a><br/><br/><br/>\
</div></body></html>\r\n";

const char WEB_ROOT_PAGE_CONTENT[] PROGMEM = "<a href='/values'>{t}</a><br/>\
<a href='/config'>{conf}</a><br/>\
<a href='/removeConfig'>{conf_delete}</a><br/>\
<a href='/reset'>{restart}</a><br/>\
<table style='width:100%;'>\
<tr><td style='width:33%;'><a href='/debug?lvl=0'>Debug null</a></td>\
<td style='width:33%;'><a href='/debug?lvl=1'>Debug Error</a></td>\
<td style='width:33%;'><a href='/debug?lvl=2'>Debug Warning</a></td>\
</tr><tr>\
<td><a href='/debug?lvl=3'>Debug Info low</a></td>\
<td><a href='/debug?lvl=4'>Debug Info medium</a></td>\
<td><a href='/debug?lvl=5'>Debug Info high</a></td>\
</tr>\
</table>\
";

const char WEB_CONFIG_SCRIPT[] PROGMEM = "<script>\
function setSSID(ssid){document.getElementById('wlanssid').value=ssid.innerText||ssid.textContent;document.getElementById('wlanpwd').focus();}\
function load_wifi_list(){var x=new XMLHttpRequest();x.open('GET','/wifi');x.onload = function(){if (x.status === 200) {document.getElementById('wifilist').innerHTML = x.responseText;}};x.send();}\
</script>";

const char WEB_REMOVE_CONFIG_CONTENT[] PROGMEM = "<h3>{t}</h3>\
<table><tr><td><form method='POST' action='/removeConfig'><input type='submit' class='submit_green' name='submit' value='{b}'/></form></td><td><a href='/'>{c}</a></td></tr></table>\
";

const char WEB_RESET_CONTENT[] PROGMEM = "<h3>{t}</h3>\
<table><tr><td><form method='POST' action'/reset'><input type='submit' class='submit_green' name='submit' value='{b}'/></form></td><td><a href='/'>{c}</a></td></tr></table>\
";
