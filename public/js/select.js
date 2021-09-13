let artList = document.getElementById('artworks');

for (let artwork of artworks) {
    let li = document.createElement('li');
    li.innerHTML = `<label>
            <input type="checkbox" name="${artwork.Name}"
            ${artwork.InRandom ? 'checked' : ''} value="value">
            ${artwork.Name}
        </label>`;
    li.addEventListener('mouseover', toonVoorbeeld);
    li.addEventListener('click', toonVoorbeeld);
    artList.appendChild(li);
}

function toonVoorbeeld() {
  let selectedArtwork = this.querySelector('input').name;
  for (let artwork of artworks) {
      if (artwork.Name == selectedArtwork) {
          var TLsetup = artwork.TL;
          break;
      }
  }
  for (const [i, tlOn] of TLsetup.entries()) {
    if (tlOn) {
      document.querySelector('div[id=T'+i+']').classList.replace('off2', 'on');
    } else {
      document.querySelector('div[id=T'+i+']').classList.replace('on', 'off2');
    }
  }
}

function sendCheckboxClick() {
  let selectedArtwork = this.querySelector('input').name;
  for (let artwork of artworks) {
      if (artwork.Name == selectedArtwork) {
          var TLsetup = artwork.TL;
          break;
      }
  }
    if (this.checked) {
        alert("i am checked");
    } else {
        alert("i am not checked")
    }
}
