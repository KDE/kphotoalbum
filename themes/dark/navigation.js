function listenKey () { document.onkeydown = getKey; }

function getKey(e){
  if(!e) {
    //ie
    e = window.event;
  }

  switch(e.keyCode) {
  case 37: //left
    if (prev != "prev") {
	window.location.href = prev;
    } else {
	window.location.href = index;
    };
    return false;
    break;
  case 39: //right
  //case 32: //space
    if (next != "next") {
	window.location.href = next;
    } else {
	window.location.href = index;
    };
    return false;
    break;
  case 36: //home
    if (index != "index") {
	window.location.href = index;
    };
    break;
  }
}
