let tlIsOn = [];
let elements = document.getElementsByClassName('tl');

var naam = '';

for (element of elements) {
    element.addEventListener('mousedown', switchTL);
    element.addEventListener('mousepressed', tlMouseOver);
    tlIsOn.push(false);
}

klikEnMouseOver()

function tlMouseOver(){
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

function klikEnMouseOver(){
  document.querySelector('button[id=send]').addEventListener('click', klikSend);
  document.querySelector('button[id=none]').addEventListener('click', klikNone);
  document.querySelector('button[id=all]').addEventListener('click', klikAll);
  document.querySelector('button[id=negative]').addEventListener('click', klikNegative);
  document.querySelector('button[id=send]').addEventListener('mouseover', pointerMuis);
  document.querySelector('button[id=none]').addEventListener('mouseover', pointerMuis);
  document.querySelector('button[id=all]').addEventListener('mouseover', pointerMuis);
  document.querySelector('button[id=send]').addEventListener('mouseover', mouseOverKnop1);
  document.querySelector('button[id=send]').addEventListener('mouseout', mouseOutKnop1);
  document.querySelector('button[id=none]').addEventListener('mouseover', mouseOverKnop2);
  document.querySelector('button[id=none]').addEventListener('mouseout', mouseOutKnop2);
  document.querySelector('button[id=all]').addEventListener('mouseover', mouseOverKnop2);
  document.querySelector('button[id=all]').addEventListener('mouseout', mouseOutKnop2);
  document.querySelector('button[id=negative]').addEventListener('mouseover', mouseOverKnop2);
  document.querySelector('button[id=negative]').addEventListener('mouseout', mouseOutKnop2);
  document.querySelector('a[id=back]').addEventListener('mouseover', mouseOverKnop3);
  document.querySelector('a[id=back]').addEventListener('mouseout', mouseOutKnop3);

  function pointerMuis(){
    this.style.cursor = 'pointer';
  }

  function mouseOverKnop1(){
    this.style.background = '#D6D6D6';
  }

  function mouseOutKnop1(){
    this.style.background = '#FFFFFF';
  }

  function mouseOverKnop2(){
    this.style.background = '#FFFFFF';
    this.style.color = '#000000'
  }

  function mouseOutKnop2(){
    this.style.background = '#000000';
    this.style.color = '#FFFFFF'
  }

  function mouseOverKnop3(){
    this.style.color = '#7A7A7A';
  }

  function mouseOutKnop3(){
    this.style.color = '#555555';
  }

  function klikNone(){
    let tlIsOn = [];
    for (element of elements) {
      element.classList.replace('on', 'off');
      tlIsOn.push(false);
    }
    document.querySelector('input').value = '';
  }

  function klikAll(){
    let tlIsOn = [];
    for (element of elements) {
      element.classList.replace('off', 'on');
      tlIsOn.push(true);
    }
    document.querySelector('input').value = '';
  }

  function klikSend(){
    naam = document.querySelector('input').value;
    while (naam.length < 2 || naam.length > 15) {
      var naam = prompt('Tussen 2 en 15 tekens');
    }
    sendSettings({Name: naam, TL: tlIsOn});
    document.querySelector('input').value = '';
  }

  function klikNegative(){
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
