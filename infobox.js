//
// Bubblehelp infoboxes, (C) 2002 Klaus Knopper <infobox@knopper.net>
// You can copy/modify and distribute this code under the conditions
// of the GNU GENERAL PUBLIC LICENSE Version 2.
//
var IWIDTH=250  // Tip box width
var ns4         // Are we using Netscape4?
var ie4         // Are we using Internet Explorer Version 4?
var ie5         // Are we using Internet Explorer Version 5 and up?
var kon         // Are we using KDE Konqueror?
var x,y,winW,winH  // Current help position and main window size
var idiv=null   // Pointer to infodiv container
var px="px"     // position suffix with "px" in some cases

function nsfix(){setTimeout("window.onresize = rebrowse", 2000);}

function rebrowse(){window.location.reload();}

function hascss(){ return gettip('infodiv')?true:false }

function infoinit(){
 ns4=(document.layers)?true:false, ie4=(document.all)?true:false;
 ie5=((ie4)&&((navigator.userAgent.indexOf('MSIE 5')>0)||(navigator.userAgent.indexOf('MSIE 6')>0)))?true:false;
 kon=(navigator.userAgent.indexOf('konqueror')>0)?true:false;
 x=0;y=0;winW=800;winH=600;
 idiv=null;
 document.onmousemove = mousemove;
 if(ns4&&document.captureEvents) document.captureEvents(Event.MOUSEMOVE);
 // Workaround for just another netscape bug: Fix browser confusion on resize
 // obviously conqueror has a similar problem :-(
 if(ns4||kon){ nsfix() }
 if(ns4) { px=""; }
}

function untip(){
 if(idiv) idiv.visibility=ns4?"hide":"hidden";
 idiv=null;
}

function gettip(name){return (document.layers&&document.layers[name])?document.layers[name]:(document.all&&document.all[name]&&document.all[name].style)?document.all[name].style:document[name]?document[name]:(document.getElementById(name)?document.getElementById(name).style:0);}

// Prepare tip boxes, but don't show them yet
function maketip(name,title,text){
 if(hascss()) document.write('<div id="'+name+'" name="'+name+'" style="position:absolute; visibility:hidden; z-index:20; top:0'+px+'; left:0'+px+';"><table width='+IWIDTH+' border=0 cellpadding=2 cellspacing=0 bgcolor="#333399"><tr><td class="tiptd"><table width="100%" border=0 cellpadding=0 cellspacing=0><tr><th><span class="ptt"><b><font color="#FFFFFF">'+title+'</font></b></span></th></tr></table><table width="100%" border=0 cellpadding=2 cellspacing=0 bgcolor="#CCCCFF"><tr><td><span class="pst"><font color="#000000">'+text+'</font></span></td></tr></table></td></tr></table></div>'+"\n");
}

function tip(name){
 if(hascss()){
  if(idiv) untip();
  idiv=gettip(name);
  if(idiv){
   winW=(window.innerWidth)? window.innerWidth+window.pageXOffset-16:document.body.offsetWidth-20;
   winH=(window.innerHeight)?window.innerHeight+window.pageYOffset  :document.body.offsetHeight;
   if(x<=0||y<=0){ // konqueror can't get mouse position
    x=(winW-IWIDTH)/2+(window.pageXOffset?window.pageXOffset:0); y=(winH-50)/2+(window.pageYOffset?window.pageYOffset:0); // middle of window
   }
   showtip();
  }
 }
}

function showtip(){
  idiv.left=(((x+260)<winW)?x+12:x-255)+px; idiv.top=(((y+90)<winH)?y+12:y-90)+px;
  idiv.visibility=ns4?"show":"visible";
//  window.status="idiv="+idiv+"X:"+(idiv.left?idiv.left:"NAN")+", Y:"+(idiv.top?idiv.top:"NAN")+", x:"+x+", y:"+y;
}

function mousemove(e){
 if(e)   {x=e.pageX?e.pageX:e.clientX?e.clientX:0; y=e.pageY?e.pageY:e.clientY?e.clientY:0;}
 else if(event) {x=event.clientX; y=event.clientY;}
 else {x=0; y=0;}
 if(document.documentElement) // Workaround for scroll offset of IE
  {
    x+=document.documentElement.scrollLeft;
    y+=document.documentElement.scrollTop;
  }
 if(idiv) showtip();
}

// Initialize after loading the page
window.onload=infoinit;

// EOF infobox.js
