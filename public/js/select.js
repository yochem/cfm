var checkboxen = [];

for (var i = 0; i < voorInstellingen.length; i++) {
  document.querySelector('ul[id=namen]').innerHTML += '<li class=naam id="'+i+'">'+ voorInstellingen[i].Name + '</li>';
  document.querySelector('ul[id=checkboxen]').innerHTML += '<li> <input class="check" id="'+i+'"type="checkbox"> </li>';
}

var lijstVanNamen = document.querySelectorAll('li[class=naam]');

var lijstCheckboxen = document.querySelectorAll('input[class=check]');

for (var i = 0; i < lijstVanNamen.length; i++) {
  lijstVanNamen[i].addEventListener('mouseover',toonVoorbeeld);
  lijstVanNamen[i].addEventListener('click',toonVoorbeeld);
  lijstCheckboxen[i].addEventListener('mouseover',toonVoorbeeld);
  lijstCheckboxen[i].addEventListener('click',toonVoorbeeld);
}

function toonVoorbeeld(){
  this.style.cursor = 'default';
  for (var i = 0; i < voorInstellingen[this.id].TL.length; i++) {
    if (voorInstellingen[this.id].TL[i] == true) {
      document.querySelector('div[id=T'+i+']').classList.replace('off2', 'on');
    } else {
      document.querySelector('div[id=T'+i+']').classList.replace('on', 'off2');
    }
  }
}



document.querySelector('button[id=createNew]').addEventListener('mouseover', mouseOverKnop1);
document.querySelector('button[id=createNew]').addEventListener('mouseout', mouseOutKnop1);
document.querySelector('button[id=createNew]').addEventListener('mouseover', pointerMuis);

function mouseOverKnop1(){
  this.style.background = '#D6D6D6';
}

function mouseOutKnop1(){
  this.style.background = '#FFFFFF';
}

function pointerMuis(){
  this.style.cursor = 'pointer';
}
