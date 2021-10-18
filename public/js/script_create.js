let tlIsOn = [];
let elements = document.getElementsByClassName('tl');

var naam = '';

for (element of elements) {
    element.addEventListener('mousedown', switchTL);
    element.addEventListener('mousepressed', tlMouseOver);
    tlIsOn.push(false);
}

function tlMouseOver() {
    if (this.classList.contains('off')) {
        this.classList.replace('off', 'on');
        var tl_id = this.id.substring(1, 3);
        tlIsOn[tl_id] = true;
    } else {
        this.classList.replace('on', 'off');
        var tl_id = this.id.substring(1, 3);
        tlIsOn[tl_id] = false;
    }
}

function switchTL() {
    if (this.classList.contains('off')) {
        this.classList.replace('off', 'on');
        var tl_id = this.id.substring(1, 3);
        tlIsOn[tl_id] = true;
    } else {
        this.classList.replace('on', 'off');
        var tl_id = this.id.substring(1, 3);
        tlIsOn[tl_id] = false;
    }
}

document.addEventListener("keyup", function(event) {
    if (event.code === 'Enter') {
        klikSend();
    }
});


// These klik* functions are run when the buttons in create.html are clicked
// They bind by using the onclick attribute in the html (create.html:75)
function klikNone() {
    let tlIsOn = [];
    for (element of elements) {
        element.classList.replace('on', 'off');
        tlIsOn.push(false);
    }
    document.querySelector('input').value = '';
}

function klikAll() {
    let tlIsOn = [];
    for (element of elements) {
        element.classList.replace('off', 'on');
        tlIsOn.push(true);
    }
    document.querySelector('input').value = '';
}

function klikSend() {
    naam = document.querySelector('input').value;
    while (naam.length < 2 || naam.length > 15) {
        var naam = prompt('Tussen 2 en 15 tekens');
    }
    sendSettings({
        Name: naam,
        TL: tlIsOn
    });
    document.querySelector('input').value = '';
}

function klikNegative() {
    let tlIsOn = [];
    for (element of elements) {
        if (element.classList.contains('off')) {
            element.classList.replace('off', 'on');
            tlIsOn.push(true);
        } else {
            element.classList.replace('on', 'off');
            tlIsOn.push(false);
        }
    }
}

function sendSettings(data) {
    // data currently consists of the following keys:
    // Name (str): A displayname for the current setting
    // TL (array[bool]): A list of on/off settings for every TL
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
