// Selectable options BEGIN
// Konqueror tends to crash with videos. If set to 1 videos are disabled with Konqueror
var konquerorPlaySafe = 1

// Set to 1 to display alerts when video support is disabled
var annoyingAlerts = 0

// SlideShow speed in milliseconds
var speed = 2500

// Default video format for downloads (if browser supports HTML5 video the browser supported format used)
var videoExt = ".mp4"

// Selectable options END

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

var browser

// Videos are scaled to this size
var videoWidth = 640
var videoHeight = 480
var videoPlaying = 0

// 1 if browser supports video
var browserVideo = 0
var konqueror = 0

var thumbsW
var areaHeight
var areaWidth
var width


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
		} else if (viewer) {
			nextImage()
		}
		break
	case 27: //escape
		hideImageViewer()
		break
	case 83: //s
		slideShow()
		break
	case 36: //home
		break
	case 13: //return
		showImageViewer(-1)
		break
	default:
		break
	}
}

// Viewing next image on ImageViewer
function nextImage()
{
	if (videoPlaying && browserVideo) {
		document.getElementById("fullVideo").pause()
	}

	if (ss) {
		clearTimeout(timer)
		if (pause) { return }
		timer = setTimeout("nextImage()",speed)
	}

	if (browserVideo) {
		viewerImage = ++viewerImage > gallery.length - 1 ? 0 : viewerImage
	} else {
		var counter = 0
		viewerImage = ++viewerImage > gallery.length - 1 ? 0 : viewerImage
		while (gallery[viewerImage][3].match(/video/i) == "video") {
			viewerImage = ++viewerImage > gallery.length - 1 ? 0 : viewerImage
			if (++counter > gallery.length) {
				return
			}
		}
	}
	showImage(viewerImage)
}

// Viewing previous image in ImageViewer
function prevImage()
{
	if (videoPlaying && browserVideo) {
		document.getElementById("fullVideo").pause()
	}

	if (ss) {
		clearTimeout(timer)
		if (pause) { return }
		timer = setTimeout("nextImage()",speed)
	}

	if (browserVideo) {
		viewerImage = --viewerImage < 0 ? gallery.length - 1 : viewerImage
	} else {
		var counter = 0
		viewerImage = --viewerImage < 0 ? gallery.length - 1 : viewerImage
		while (gallery[viewerImage][3].match(/video/i) == "video") {
			viewerImage = --viewerImage < 0 ? gallery.length - 1 : viewerImage
			if (--counter > gallery.length) {
				return
			}
		}
	}
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

	switch(browser) {
		case 0:
			myHeight = window.innerHeight
			break
		case 1:
			myHeight = document.documentElement.clientHeight
			break
		case 2:
			myHeight = document.body.clientHeight
			break
		default:
			break
	}
	return (myHeight - 220) > imgHeight ? (myHeight - 220) : imgHeight
}

function detectBrowser()
{
	if( typeof( window.innerWidth ) == 'number' ) {
		//Non-IE
		browser = 0
	} else if( document.documentElement &&
		document.documentElement.clientHeight ) {
			//IE 6+ in 'standards compliant mode'
			browser = 1
	} else if( document.body && document.body.clientHeight ) {
		//IE 4 compatible
		browser = 2
	}

	if (typeof enableVideo == 'undefined' || !enableVideo) {
		return
	}

	if (!generatedVideo && inlineVideo) {
		videoInfo('Videos displayed using original format, if browser supports')
		return
	}
	if (inlineVideo) {
		if (!!document.createElement('video').canPlayType) {
			if (!!document.createElement("video").canPlayType('video/ogg; codecs="theora, vorbis"')) {
				browserVideo = 1
				videoFormat = "'video/ogg; codecs=\"theora, vorbis\"'"
				videoExt = ".ogg"
			} if (!!document.createElement("video").canPlayType('video/mp4; codecs="avc1.42E01E, mp4a.40.2"')) {
				browserVideo = 1
				videoFormat = "'video/mp4; codecs=\"avc1.42E01E, mp4a.40.2\"'"
				videoExt = ".mp4"
			}
			if (navigator.userAgent.indexOf("Konqueror") > -1 || navigator.userAgent.indexOf("rekonq") > -1) {
				if (konquerorPlaySafe == 1) {
					konqueror = 0
					browserVideo = 0
					videoInfo('Konqueror keeps crashing if video support is enabled - thus videos must be viewed manually<br />')
					return
				} else {
					konqueror = 1
				}
			}
		}
		videoInfo('')
	} else {
		videoInfo('Inline videos are disabled')
	}
}

function videoInfo(msg)
{
	if (msg === '') {
		msg = 'Emedded videos are disabled due to poor or missing HTML5 video support. However, clicking video thumbnail allows downloading or viewing with plugins.'
	}
	// Displaying info on video support
	document.write('<div class="videoInfo" id="videoInfo" onclick=closeVideoInfo()><p>')
	if (browserVideo) {
		document.write('Videos displayed using ' + videoFormat.replace(/;.*/, '') + "'")
	} else {
		document.write(msg)
	}
	document.write('</p></div>') // videoInfo
	setTimeout("closeVideoInfo()",7000)
}

function closeVideoInfo()
{
	document.getElementById("videoInfo").style.display="none"
}

// Calculate appropriate width of thumbnails area
function thumbsWidth(pad)
{
	var myWidth = windowWidth(10)
	var thumbOverhead = 18
	var thumbs

	if( browser === 0 ) {
		thumbOverhead = 10
	}

	thumbs = Math.floor((myWidth - width - pad) / (tsize + thumbOverhead))

	return thumbs * ( tsize + thumbOverhead )
}

function fullSizeLoaded()
{
	width = self.width > self.height ? self.width : self.height
	height = self.width < self.height ? self.width: self.height

	thumbsW = thumbsWidth(40)
	areaHeight = thumbsHeight(width + 45)
	areaWidth = width + 20

	setSize()
}

// Main function to barf out the HTML for thumbnails and image area
function imageInit()
{
	var slash
	var doc

	detectBrowser()

	if (location.pathname.match(/\//)) {
		slash = '/'
	} else {
		slash = '\\'
	}

	// Redirect browser to smallest sized images by default
	doc = location.pathname.substring(location.pathname.lastIndexOf(slash)+1)
	if ((doc == "index.html") || (doc === "")) {
		window.location=minPage
	}

	// If we are dealing with full sized images we have to figure out image
	// dimentions for size calculations
	if (width < 0) {
		// Just set some positive size and resize it once a sample
		// image has been loaded
		width=windowWidth(0)

		var pic = new Image()
		pic.onload=fullSizeLoaded

		var iter = 0
		while (gallery[iter][3].match(/video/i) == "video") {
			if (++iter > gallery.length) {
				iter = 0
				break
			}
		}
		pic.src=gallery[iter][0]
	}

	thumbsW = thumbsWidth(40)

	// Currently using max portrait image height + some for the
	// load area of full size images
	areaHeight = thumbsHeight(width + 45)
	areaWidth = width + 20

	// Registering resize function
	window.onresize= widthTimer

	// SlideShow speed control
	document.write('<div id="slideSpeed" class="slideSpeed"')
	document.write('<p>Delay</p>')
	document.write('<a href="#" onclick=changeSpeed(1)>+</a>')
	document.write('<p id="currentSpeed">' + speed / 1000 + 's</p>')
	document.write('<a href="#" onclick=changeSpeed(0)>-</a>')
	document.write('</div>')

	document.write('<div id="imagearea">')
	document.write('<div class="thumbnails" id="thumbnails" style="height:' +
		areaHeight + 'px;">')

	var ts
	if ( browser === 0 ) {
		ts = (tsize + 4) + 'px'
	} else {
		ts = (tsize + 16) + 'px'
	}

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
	document.write(
		'<div id="navi" class="navi">' +
		'<img id="arrow-left" class="arrow-left" onmouseover="this.src=\'arrow-left2.png\'" onmouseout="this.src=\'arrow-left.png\'' +
			'" onclick=prevImage() alt="prev" src="arrow-left.png" />' +
		'<img id="arrow-right" class="arrow-right" onmouseover="this.src=\'arrow-right2.png\'" onmouseout="this.src=\'arrow-right.png\'' +
			'" onclick=nextImage() alt="next" src="arrow-right.png" /></div>' +
		'<div id="pad"><div id="fullarea" onclick=nextImage()>' +
		'<div id="X" class="X" onclick=hideImageViewer()><img alt="close" src="close.png" /></div>' +
		'<img id="fullImg" class="fullImg" src="' + gallery[0][2] +
		'" onload=viewerSize() />' +
		'<span id="fullImgD" class="fullImgD">' + '<i>' + gallery[image][2] + '</i> &nbsp;' + gallery[0][5] + '</span>')
	if (browserVideo == 1) {
		document.write('<video id="fullVideo" class="fullVideo" width="' + videoWidth + '" height="' +
			videoHeight + '" onloadstart=viewerVideoSize() onended=videoEnd() >' +
			'<source type=' + videoFormat + ' src="')
		if (generatedVideo) {
			document.write(gallery[0][2].replace(/\..*/, videoExt))
		} else {
			document.write(gallery[0][2])
		}
		document.write('" />"</video>')
		document.write('<span id="fullVidD" class="fullVidD">' + '<i>' + gallery[image][2] + '</i> &nbsp;' + gallery[0][5] + '</span>')
			//'<source type=\'video/mp4; codecs="avc1.42E01E, mp4a.40.2"\' src="' + gallery[0][2].replace(/\.*/, "mp4") + '" />"' +
		if (konqueror) {
			document.getElementById("fullVideo").setAttribute("autoplay", 1)
			document.getElementById("fullVideo").setAttribute("controls", 1)
		}
	}
	document.write('<div class="dl" id="dl" onclick="event.cancelBubble = true;"><a id="videoDL" href="')
	if (generatedVideo) {
		document.write(gallery[0][2].replace(/\..*/, videoExt))
	} else {
		document.write(gallery[0][2])
	}
	document.write('"><img alt="download" src="download.png" /></a></div>' +
		'</div></div>')

	// creating the loadarea for the big images
	document.write('<div id="loadarea" style="width:' + areaWidth +
		'px; min-height:' + areaHeight + 'px;">' +
		'<img id="fsImg" src="' + gallery[0][0] + '" onclick=showImageViewer(-1) />')
	document.write('<div class="filename" id="filename"><b>' + gallery[0][2] + '</b></div>')
	document.write('<div class="keywords" id="keywords">' + gallery[0][5] +
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
			document.getElementById("keywords").innerHTML = gallery[img][5]
			document.getElementById("filename").innerHTML = '<b>' + gallery[img][2] + '</b>'
			document.getElementById("thumb-div-" + img).className = "thumb-div-selected"
			document.getElementById("thumb-div-" + image).className = "thumb-div"
			image = img
		} else {
			var mimetype = gallery[img][3].split('/', 2)
			if (mimetype[0] == "video") {
				if (browserVideo) {
					document.getElementById("fullVideo").style.display="block"
					document.getElementById("dl").style.display="block"
					document.getElementById("fullImg").style.display="none"
					document.getElementById("fullImgD").style.display="none"
					if (generatedVideo) {
						document.getElementById("videoDL").href = gallery[img][2].replace(/\..*/, videoExt)
						document.getElementById("fullVideo").setAttribute("src", gallery[img][2].replace(/\..*/, videoExt))
					} else {
						document.getElementById("videoDL").href = gallery[img][2]
						document.getElementById("fullVideo").setAttribute("src", gallery[img][2])
					}

					document.getElementById("fullVideo").load()
					document.getElementById("fullVidD").innerHTML = '<i>' + gallery[img][2] + '</i> &nbsp;' + gallery[img][5]
					document.getElementById("fullVidD").style.display="block"

					if (ss) {
						clearTimeout(timer)
					}
					document.getElementById("fullVideo").play()
					videoPlaying = 1
				}
			} else {
				if (browserVideo) {
					document.getElementById("fullVideo").style.display="none"
					document.getElementById("dl").style.display="none"
					document.getElementById("fullVidD").style.display="none"
				}
				document.getElementById("fullImg").src = gallery[img][2]
				document.getElementById("fullImgD").innerHTML = '<i>' + gallery[img][2] + '</i> &nbsp;' + gallery[img][5]
				document.getElementById("fullImg").style.display="block"
				document.getElementById("fullImgD").style.display="block"
			}
			viewerSize()
		}
		viewerImage=img
	}
}

function videoEnd()
{
	videoPlaying = 0
	if (ss) {
		nextImage()
	}
}

// Resize the ImageViewer to match each image
function viewerSize()
{
	var mimetype = gallery[viewerImage][3].split('/', 2)
	if (mimetype[0] == "video") {
		document.getElementById("fullarea").style.width = width + 'px'
	} else  {
		document.getElementById("fullarea").style.width =
			document.getElementById("fullImg").offsetWidth ?
			document.getElementById("fullImg").offsetWidth + 'px' :
			document.getElementById("fullImg").width + 'px'
	}
}

function viewerVideoSize()
{
	document.getElementById("fullarea").style.width = videoWidth + 'px'

}

// Display ImageViewer
function showImageViewer(img)
{
	var mimetype
	if (img == -1) {
		mimetype = gallery[image][3].split('/', 2)
	} else {
		mimetype = gallery[img][3].split('/', 2)
	}
	if (mimetype[0] == "video") {
		if (browserVideo) {
			document.getElementById("fullVideo").style.display="block"
			document.getElementById("dl").style.display="block"
			document.getElementById("fullImg").style.display="none"
			document.getElementById("fullImgD").style.display="none"
	
			if (img == -1) {
				if (generatedVideo) {
					document.getElementById("fullVideo").setAttribute("src", gallery[image][2].replace(/\..*/, videoExt))
					document.getElementById("videoDL").href = gallery[image][2].replace(/\..*/, videoExt)
				} else {
					document.getElementById("fullVideo").setAttribute("src", gallery[image][2])
					document.getElementById("videoDL").href = gallery[image][2]
				}
				document.getElementById("fullVidD").innerHTML = '<i>' + gallery[image][2] + '</i> &nbsp;' + gallery[image][5]
			} else {
				if (generatedVideo) {
					document.getElementById("fullVideo").setAttribute("src", gallery[img][2].replace(/\..*/, videoExt))
					document.getElementById("videoDL").href = gallery[img][2].replace(/\..*/, videoExt)
				} else {
					document.getElementById("fullVideo").setAttribute("src", gallery[img][2])
					document.getElementById("videoDL").href = gallery[img][2]
				}
				document.getElementById("fullVidD").innerHTML = '<i>' + gallery[image][2] + '</i> &nbsp;' + gallery[image][5]
			}
			document.getElementById("fullVideo").load()
			document.getElementById("fullVidD").style.display="block"
			document.getElementById("fullVideo").play()
			videoPlaying = 1
		} else {
			if (generatedVideo) {
				document.location = gallery[img][2].replace(/\..*/, videoExt)
			} else {
				document.location = gallery[img][2]
			}
			return
		}
	} else {
		if (browserVideo) {
			document.getElementById("fullVideo").style.display="none"
			document.getElementById("fullVidD").style.display="none"
			document.getElementById("dl").style.display="none"
		}

		if (img == -1) {
			document.getElementById("fullImg").src = gallery[image][2]
			document.getElementById("fullImgD").innerHTML = '<i>' + gallery[image][2] + '</i> &nbsp;' + gallery[image][5]
		} else {
			document.getElementById("fullImg").src = gallery[img][2]
			document.getElementById("fullImgD").innerHTML = '<i>' + gallery[img][2] + '</i> &nbsp;' + gallery[img][5]
		}
		document.getElementById("fullImg").style.display="block"
		document.getElementById("fullImgD").style.display="block"
	}

	document.getElementById("fullarea").style.display="block"
	document.getElementById("opa").style.display="block"
	document.getElementById("pad").style.display="block"
	if (!ss) {
		document.getElementById("navi").style.display="block"
	}
	viewer=1
}

// Hide ImageViewer
function hideImageViewer()
{
	document.getElementById("fullarea").style.display="none"
	document.getElementById("opa").style.display="none"
	document.getElementById("pad").style.display="none"
	document.getElementById("dl").style.display="none"
	document.getElementById("navi").style.display="none"
	if (videoPlaying) {
		document.getElementById("fullVideo").pause()
	}
	if (ss) {
		clearTimeout(timer)
		ss=0
		pause=0
		document.getElementById("slideSpeed").style.display = "none"
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

		// SlideShow speed config
		document.getElementById("slideSpeed").style.display = "block"
	}
}

function changeSpeed(change)
{
	if (change) {
		if (speed < 10000) {
			speed += 500
		}
	} else {
		if (speed >= 1000) {
			speed -= 500
		}
	}
	document.getElementById("currentSpeed").innerHTML = speed / 1000 + 's'
}

// Setting help text when JavaScript is supported
function helpIndex()
{
	if ( typeof( window.innerWidth ) == 'number' ) {
		document.getElementById('help-span').innerHTML = 'Move mouse over the thumbnail if you want to see it enlarged, full image by clicking the thumbnail or preview image. Scroll through images with left and right keys in either mode.'
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
	if ( browser === 0 ) {
		thumbPad = 50
		thumbWidth = thumbsWidth(thumbPad)
		// FF et. al
		if (!document.getElementById('thumbnails')) {
			return
		}
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
		document.getElementById("navi").style.paddingLeft =
			parseInt(windowWidth(0) / 2, 10) - 22  + 'px'
	} else {
		thumbPad = 50
		thumbWidth = thumbsWidth(thumbPad)
		if (!document.getElementById('thumbnails')) {
			return
		}
		document.getElementById('thumbnails').style.width =
			(thumbWidth + 38) + 'px'
		document.getElementById('thumbnails').style.height =
			thumbsHeight(width + 45) + 'px'
		document.getElementById("loadarea").style.paddingLeft =
			(thumbWidth + 20) + 'px'
		document.getElementById("loadarea").style.width =
			windowWidth(thumbWidth + 20) + 'px'
		document.getElementById('loadarea').style.height =
			thumbsHeight(width + 45) + 'px'
		document.getElementById('loadarea').style.minHeight =
			thumbsHeight(width + 45) + 'px'
		document.getElementById("navi").style.paddingLeft =
			parseInt(windowWidth(0) / 2, 10) - 22  + 'px'
	}
}

// Setting timer to change width...hack to prevent resize loops
function widthTimer()
{
	setTimeout("setSize()",1)
}

// vi:noexpandtab:tabstop=4 shiftwidth=4:
