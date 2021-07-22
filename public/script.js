let tlIsOn = [];
let elements = document.getElementsByClassName('tl')

for (element of elements) {
    element.addEventListener('click', switchTL);
    tlIsOn.push(false);
}

function switchTL() {
    if (this.classList.contains('off')) {
        this.classList.replace('off', 'on');
        var id = this.id.substring(1, 3);
        tlIsOn[id] = true;
    } else {
        this.classList.replace('on', 'off');
        var id = this.id.substring(1, 3);
        tlIsOn[id] = false;
    }
    sendSettings({Name: 'Custom', TL: tlIsOn});
}

function sendSettings(data) {
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "/ajax");
    xhr.onreadystatechange = function() {
        if (xhr.readyState > 3 && xhr.status == 200) {
            console.log('new settings received by server');
        }
    };
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.send(JSON.stringify(data));
}
