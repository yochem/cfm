// function is immediately run
(function fillListWithArtworks() {
    // This function fills the list item (#artworks element on select.html)
    // with artworks in the artworks global variable. This variable is filled
    // in by the backend, see select.html > html > head > script element.

    // first we get the list element with id #artworks
    let artList = document.getElementById('artworks');

    // then we loop over the artworks array. Remember an array item looks like
    // this: {Name: "naam", TL: [true, false, ...], InRandom: true}
    for (let artwork of artworks) {
        // create a new list item and fill it with a label that is either checked
        // or not, depending on the InRandom key. The label name is gotten from
        // the Name key
        let li = document.createElement('li');
        li.innerHTML = `<label>
            <input
                type="checkbox"
                name="${artwork.Name}"
                ${artwork.InRandom ? 'checked' : ''}
                onchange="sendCheckboxClick(this)">
            ${artwork.Name}
        </label>`;
        li.addEventListener('mouseover', toonVoorbeeld);
        li.addEventListener('click', toonVoorbeeld);
        // include the TL setup in the list item parameters
        // (https://stackoverflow.com/a/11986895/7442167)
        li.TL = artwork.TL;

        // and lastly append list item to the list
        artList.appendChild(li);
    }
})();

function toonVoorbeeld(evt) {
    // This function shows how the artwork looks when the mouse is hovered
    // over the name of the artwork.
    TLsetup = evt.currentTarget.TL;

    // loop over TLsetup like this: i=0, tlOn=true, i=1, tlOn=false, i=2...
    // this way we have the index and if the TL must be on or off
    for (const [i, tlOn] of TLsetup.entries()) {
        if (tlOn) {
            document.getElementById('T' + i).classList.replace('off2', 'on');
        } else {
            document.getElementById('T' + i).classList.replace('on', 'off2');
        }
    }
}

function sendCheckboxClick(e) {
    // this function lets the backend know the checkbox is clicked and the
    // artwork should be removed from being random selected (aka inRandom: false)

    // this.checked is acting weird, this makes sure it works as expected
    if (this.checked === undefined) {
        this.checked = e.getAttribute('checked') === '';
    }
    this.checked = !this.checked;

    let data = {
        Name: e.getAttribute('name'),
        TL: e.parentElement.parentElement.TL,
        InRandom: this.checked
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
