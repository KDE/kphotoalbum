// Calculate appropriate height for thumbnails area
function thumbsHeight(imgHeight) {
	var myHeight = 0;

	if( typeof( window.innerWidth ) == 'number' ) {
		//Non-IE
		myHeight = window.innerHeight;
	} else if( document.documentElement &&
		document.documentElement.clientHeight ) {
			//IE 6+ in 'standards compliant mode'
			myHeight = document.documentElement.clientHeight;
	} else if( document.body && document.body.clientHeight ) {
		//IE 4 compatible
		myHeight = document.body.clientHeight;
	}
	return (myHeight - 220) > imgHeight ? (myHeight - 220) : imgHeight;
}

function windowWidth(pad) {
	var myWidth = 0;

	if( typeof( window.innerWidth ) == 'number' ) {
		//Non-IE
		myWidth = window.innerWidth;
	} else if( document.documentElement &&
		document.documentElement.clientWidth ) {
			//IE 6+ in 'standards compliant mode'
			myWidth = document.documentElement.clientWidth;
	} else if( document.body && document.body.clientWidth ) {
		//IE 4 compatible
		myWidth = document.body.clientWidth;
	}
	return myWidth - pad;
}

// Calculate appropriate width of thumbnails area
function thumbsWidth(pad) {
	var myWidth = windowWidth(10);
	var thumbOverhead = 18;

	if( typeof( window.innerWidth ) == 'number' )
		thumbOverhead = 10;

	var thumbs = Math.floor((myWidth - width - pad) / (tsize + thumbOverhead));

	return thumbs * ( tsize + thumbOverhead )
}

// Main function to barf out the HTML for thumbnails and image area
function image()
{
	// Redirect browser to smallest sized images by default
	var slash = '/';
	if (typeof(window.innerWidth) != 'number')
		slash = '\\';
	if (location.pathname.substring(location.pathname.lastIndexOf(slash)+1)
			== "index.html") {
		document.location=minPage;
	} else if (location.pathname.substring(location.pathname.lastIndexOf(
		slash)+1) == "") {
		document.location=minPage;
	}

	var thumbsW = thumbsWidth(40);
	var image;
	var type="onmouseover"; //onclick or onmouseover

	// Currently using max portrait image height + some for the
	// load area of full size images
	var areaHeight = thumbsHeight(width + 45);
	var areaWidth = width + 20;
	var iets = (tsize + 16) + 'px';
	var ffts = (tsize + 4) + 'px';

	// Registering resize function
	window.onresize= widthTimer;

	document.write('<div id="imagearea">')

	document.write('<div class="thumbnails" id="thumbnails" style="height:' +
		areaHeight + 'px;">')

	for (image in gallery) {
		if ( typeof( window.innerWidth ) == 'number' ) {
			document.write('<div id="thumb-div" class="thumb-div"' +
				' style="width:' + ffts + ';height:' + ffts +
				';">' +
				'<a id="thumbA" class="thumbA" href="#" ' +
				'onclick="window.open (\'' + gallery[image][2] +
				'\', \'child\'); return false">' +
				'<img alt="' + gallery[image][1] + '" src="' +
				gallery[image][1] + '" ' + type +
				'=showImage("' + image + '") '
			)
			document.write('/></a></div>')
		} else {
		document.write('<div id ="thumb-div" class="thumb-div" ' +
			'style="padding:0px; width:' + iets + ';height:' +
			iets + ';">' +
			'<a id="thumbA" class="thumbA" href="#" ' +
			'onclick="window.open (\'' + gallery[image][2] +
			'\', \'child\'); return false">' +
			'<img alt="' +
			gallery[image][1] + '" src="' + gallery[image][1] +
			'" ' + type + '=showImage("' + image +
			'") ')
			document.write('/></a></div>')
		}
	}
	document.write('</div>') // thumbnails

	// creating the loadarea for the full size images
	document.write('<div id="loadarea" style="width:' + areaWidth +
		'px; min-height:' + areaHeight + 'px;">' +
		'<a id="fsImgA" class="fsImgA" href="#" onclick="window.open (this.href, \'child\'); return false">' +
		'<img id="fsImg" src="' + gallery[0][0] + '" />' +
		'</a>')
	if ( typeof( window.innerWidth ) == 'number' ) {
	document.write('<div class="keywords" id="keywords">' + gallery[image][3] +
			'</div>')
	} else {
	document.write('<div class="keywords" id="keywords" style="margin-left:1.5em">' + gallery[image][3] +
			'</div>')
	}
	document.write('</div>') // loadarea
	document.write('</div>') // imagearea

	setSize()
	document.body.height = '100%';
}

function showImage(image)
{
	if (gallery[image]) {
		document.getElementById("fsImgA").href= gallery[image][2];
		document.getElementById("fsImg").src = gallery[image][0];
		document.getElementById("keywords").innerHTML = gallery[image][3];
	}
}

// Setting help text when JavaScript is supported
function helpIndex()
{
	if ( typeof( window.innerWidth ) == 'number' ) {
		document.getElementById('help-span').innerHTML = 'Move mouse over the thumbnail if you want to see it enlarged.';
	} else {
		document.getElementById('help').innerHTML = '';
	}
}


function setSize() {
	// Resizing thumbnail are to match new width
	// Repositioning image area to match new width
	if ( typeof( window.innerWidth ) == 'number' ) {
		var thumbPad = 50;
		var thumbWidth = thumbsWidth(thumbPad);
		// FF et. al
		if (!document.getElementById('thumbnails'))
			return;
		document.getElementById('thumbnails').style.width =
			(thumbWidth + 16) + 'px';
		document.getElementById('thumbnails').style.height =
			thumbsHeight(width + 45) + 'px';
		document.getElementById("fsImg").style.paddingLeft =
			( thumbWidth + 28 ) + 'px';
		document.getElementById("keywords").style.paddingLeft =
			thumbWidth + 'px';
		document.getElementById("loadarea").style.width =
			windowWidth(40) + 'px';
		document.getElementById('loadarea').style.minHeight =
			thumbsHeight(width + 45) + 'px';
	} else {
		var thumbPad = 40;
		var thumbWidth = thumbsWidth(thumbPad);
		if (!document.getElementById('thumbnails'))
			return;
		document.getElementById('thumbnails').style.width =
			(thumbWidth + 38) + 'px';
		document.getElementById('thumbnails').style.height =
			thumbsHeight(width + 45) + 'px';
		document.getElementById("loadarea").style.paddingLeft =
			( windowWidth(width + 10) ) + 'px';
		document.getElementById("loadarea").style.width =
			windowWidth(20) + 'px';
		document.getElementById('loadarea').style.minHeight =
			thumbsHeight(width + 45) + 'px';
	}
}

// Setting timer to change width...hack to prevent resize loops
function widthTimer(){
	setTimeout("setSize()",1);
}
