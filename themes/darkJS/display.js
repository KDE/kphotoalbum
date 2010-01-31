// SlideShow speed in milliseconds
var speed = 2500

// Image that is being displayed in ImageViewer
var viewerImage = 0

// Image that is being displayed on main page loadarea
var image

// SlideShow off
var ss = 0

// Pause SlideShow off
var pause = 0

// ImageViewer off
var viewer = 0

// SlideShow timer
var timer

var thumbsW
var areaHeight
var areaWidth

// Registering keyboard listener
function listenKey ()
{
	document.onkeydown = getKey
}

// Actions for key presses
function getKey(e)
{
	if(!e) {
		//ie
		e = window.event
	}

	switch(e.keyCode) {
	case 37: //left
		prevImage()
		break
	case 39: //right
		nextImage()
		break
	case 32: //space
		if (ss) {
			if (!pause) {
				pause = 1
			} else {
				pause = 0
				nextImage()
			}
		} else if (viewer)
			nextImage()
		break
	case 27: //escape
		hideImageViewer()
		break
	case 36: //home
		break
	}
}

// Viewing next image on ImageViewer
function nextImage()
{
	if (ss) {
		clearTimeout(timer)
		if (pause) return
		timer = setTimeout("nextImage()",speed)
	}
	viewerImage = ++viewerImage > gallery.length - 1 ? 0 : viewerImage
	showImage(viewerImage)
}

// Viewing previous image in ImageViewer
function prevImage()
{
	if (ss) {
		clearTimeout(timer)
		if (pause) return
		timer = setTimeout("nextImage()",speed)
	}
	viewerImage = --viewerImage < 0 ? gallery.length - 1 : viewerImage
	showImage(viewerImage)
}

// Checking window width
function windowWidth(pad)
{
	var myWidth = 0

	if( typeof( window.innerWidth ) == 'number' ) {
		//Non-IE
		myWidth = window.innerWidth
	} else if( document.documentElement &&
		document.documentElement.clientWidth ) {
			//IE 6+ in 'standards compliant mode'
			myWidth = document.documentElement.clientWidth
	} else if( document.body && document.body.clientWidth ) {
		//IE 4 compatible
		myWidth = document.body.clientWidth
	}
	return myWidth - pad
}

// Calculate appropriate height for thumbnails area
function thumbsHeight(imgHeight)
{
	var myHeight = 0

	if( typeof( window.innerWidth ) == 'number' ) {
		//Non-IE
		myHeight = window.innerHeight
	} else if( document.documentElement &&
		document.documentElement.clientHeight ) {
			//IE 6+ in 'standards compliant mode'
			myHeight = document.documentElement.clientHeight
	} else if( document.body && document.body.clientHeight ) {
		//IE 4 compatible
		myHeight = document.body.clientHeight
	}
	return (myHeight - 220) > imgHeight ? (myHeight - 220) : imgHeight
}

// Calculate appropriate width of thumbnails area
function thumbsWidth(pad)
{
	var myWidth = windowWidth(10)
	var thumbOverhead = 18
	var thumbs

	if( typeof( window.innerWidth ) == 'number' )
		thumbOverhead = 10

	thumbs = Math.floor((myWidth - width - pad) / (tsize + thumbOverhead))

	return thumbs * ( tsize + thumbOverhead )
}

function fullSizeLoaded()
{
	width = this.width > this.height ? this.width : this.height
	height = this.width < this.height ? this.width: this.height

	this.destroy

	thumbsW = thumbsWidth(40)
	areaHeight = thumbsHeight(width + 45)
	areaWidth = width + 20

	setSize()
}

// Main function to barf out the HTML for thumbnails and image area
function image()
{
	var slash
	var doc

	if (location.pathname.match(/\//))
		slash = '/'
	else
		slash = '\\'

	// Redirect browser to smallest sized images by default
	doc = location.pathname.substring(location.pathname.lastIndexOf(slash)+1)
	if ((doc == "index.html") || (doc == "")) {
		document.location=minPage
	}

	// If we are dealing with full sized images we have to figure out image
	// dimentions for size calculations
	if (width < 0) {
		// Just set some positive size and resize it once a sample
		// image has been loaded
		width=windowWidth()

		var pic = new Image()
		pic.onload=fullSizeLoaded
		pic.src=gallery[0][0]
	}

	thumbsW = thumbsWidth(40)

	// Currently using max portrait image height + some for the
	// load area of full size images
	areaHeight = thumbsHeight(width + 45)
	areaWidth = width + 20

	// Registering resize function
	window.onresize= widthTimer

	document.write('<div id="imagearea">')
	document.write('<div class="thumbnails" id="thumbnails" style="height:' +
		areaHeight + 'px;">')

	if ( typeof( window.innerWidth ) == 'number' )
		ts = (tsize + 4) + 'px'
	else
		ts = (tsize + 16) + 'px'

	for (image in gallery) {
		document.write('<div id="thumb-div-' + image +'" class="thumb-div"' +
			' style="width:' + ts + ';height:' + ts +
			';">' +
			'<img id="thumb-' + image + '" alt="' + gallery[image][1] + '" src="' +
			gallery[image][1] + '" onmouseover' +
			'=showImage("' + image + '") onclick=\'showImageViewer(' + image + ')\'' +
			'/></div>')
	}
	document.write('</div>') // thumbnails

	// opaque area
	document.write('<div id="opa"></div>')

	// viewer area that is shown on slideshow / full size view
	document.write('<div id="pad"><div id="fullarea" onclick=hideImageViewer()>' +
		'<img id="fullImg" class="fullImg" src="' + gallery[0][2] + '" onload=viewerSize() />' +
		'</div></div>')

	// creating the loadarea for the big images
	document.write('<div id="loadarea" style="width:' + areaWidth +
		'px; min-height:' + areaHeight + 'px;">' +
		'<img id="fsImg" src="' + gallery[0][0] + '" onclick=showImageViewer(-1) />')
	document.write('<div class="filename" id="filename"><b>' + gallery[0][2] + '</b></div>')
	document.write('<div class="keywords" id="keywords">' + gallery[0][3] +
			'</div>')
	document.write('</div>') // loadarea
	document.write('</div>') // imagearea

	document.getElementById("thumb-div-0").className = "thumb-div-selected"

	image=0
	setSize()

	document.body.height = '100%'
}

// Show image either on loadarea or ImageViewer
function showImage(img)
{
	if (gallery[img]) {
		if (!viewer) {
			document.getElementById("fsImg").src = gallery[img][0]
			document.getElementById("keywords").innerHTML = gallery[img][3]
			document.getElementById("filename").innerHTML = '<b>' + gallery[img][2] + '</b>'
			document.getElementById("thumb-div-" + img).className = "thumb-div-selected"
			document.getElementById("thumb-div-" + image).className = "thumb-div"
			image = img
		} else
			document.getElementById("fullImg").src = gallery[img][2]
		viewerImage=img
	}
}

// Resize the ImageViewer to match each image
function viewerSize()
{
	document.getElementById("fullarea").style.width =
		document.getElementById("fullImg").offsetWidth ?
		document.getElementById("fullImg").offsetWidth + 'px' :
		document.getElementById("fullImg").width + 'px'
}

// Display ImageViewer
function showImageViewer(img)
{
	if (img == -1) {
		document.getElementById("fullImg").src = gallery[image][2]
	} else
		document.getElementById("fullImg").src = gallery[img][2]

	document.getElementById("fullarea").style.display="block"
	document.getElementById("opa").style.display="block"
	document.getElementById("pad").style.display="block"
	viewer=1
}

// Hide ImageViewer
function hideImageViewer()
{
	document.getElementById("fullarea").style.display="none"
	document.getElementById("opa").style.display="none"
	document.getElementById("pad").style.display="none"
	if (ss) {
		clearTimeout(timer)
		ss=0
		pause=0
	}
	viewer=0
}

// Set timer for showing next image on slideShow
// This function is called from the HTML page!
function slideShow()
{
	if (!ss) {
		ss = 1
		showImageViewer(viewerImage)
		timer = setTimeout("nextImage()",speed)
	}
}

// Setting help text when JavaScript is supported
function helpIndex()
{
	if ( typeof( window.innerWidth ) == 'number' ) {
		document.getElementById('help-span').innerHTML = 'Move mouse over the thumbnail if you want to see it enlarged. Scroll through images with left and right keys.'
		document.getElementById('slide-help').innerHTML = 'Slideshow'
	} else {
		document.getElementById('help').innerHTML = ''
	}
}

// Resizing key elements to reflect new window size
function setSize()
{
	var thumbPad
	var thumbWidth

	// Resizing thumbnail are to match new width
	// Repositioning image area to match new width
	if ( typeof( window.innerWidth ) == 'number' ) {
		thumbPad = 50
		thumbWidth = thumbsWidth(thumbPad)
		// FF et. al
		if (!document.getElementById('thumbnails'))
			return
		document.getElementById('thumbnails').style.width =
			(thumbWidth + 24) + 'px'
		document.getElementById('thumbnails').style.height =
			thumbsHeight(width + 45) + 'px'
		document.getElementById("fsImg").style.paddingLeft =
			( thumbWidth + 28 ) + 'px'
		document.getElementById("keywords").style.paddingLeft =
			thumbWidth + 'px'
		document.getElementById("filename").style.paddingLeft =
			thumbWidth + 'px'
		document.getElementById("loadarea").style.width =
			windowWidth(40) + 'px'
		document.getElementById('loadarea').style.minHeight =
			thumbsHeight(width + 45) + 'px'
	} else {
		thumbPad = 40
		thumbWidth = thumbsWidth(thumbPad)
		if (!document.getElementById('thumbnails'))
			return
		document.getElementById('thumbnails').style.width =
			(thumbWidth + 38) + 'px'
		document.getElementById('thumbnails').style.height =
			thumbsHeight(width + 45) + 'px'
		document.getElementById("loadarea").style.paddingLeft =
			( windowWidth(width + 10) ) + 'px'
		document.getElementById("loadarea").style.width =
			windowWidth(20) + 'px'
		document.getElementById('loadarea').style.height =
			thumbsHeight(width + 45) + 'px'
		document.getElementById('loadarea').style.minHeight =
			thumbsHeight(width + 45) + 'px'
	}
}

// Setting timer to change width...hack to prevent resize loops
function widthTimer()
{
	setTimeout("setSize()",1)
}
