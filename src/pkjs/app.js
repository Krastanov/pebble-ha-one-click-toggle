var url = localStorage.getItem("url");
var pwd = localStorage.getItem("pwd");

var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig, null, { autoHandleEvents: false });

Pebble.addEventListener('showConfiguration', function(e) {
    Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
    if (e && !e.response) {return;}
    var json_resp = JSON.parse(e.response);
    url = json_resp.HAurl.value;
    localStorage.setItem("url", url);
    pwd = json_resp.HApwd.value;
    localStorage.setItem("pwd", pwd);
});

Pebble.addEventListener('ready', function(e) {
    Pebble.sendAppMessage({'APP_READY': true});
});

Pebble.addEventListener('appmessage', function(dict) {
    if(dict.payload['TOGGLE']) {
        toggle();
    }
});

function toggle(lock_uuid, access_token) {
    var api = url+"/api/services/switch/toggle";
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        if(xhr.status == 200) {
            var state = JSON.parse(xhr.response)[0].state;
            if (state==="on") {
                Pebble.sendAppMessage({'SWITCH_STATE': 1});
            } else {
                Pebble.sendAppMessage({'SWITCH_STATE': 0});
            }
        }
    };
    xhr.open('POST', api);
    xhr.setRequestHeader('x-ha-access', pwd);
    xhr.send();    
}