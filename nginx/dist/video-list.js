function check_mouse_time() {
	if(typeof(this.player) == "undefined" || this.player == null) {
		if(flvjs.isSupported()) {
			var fileurl = this.fileurl;
			this.player = flvjs.createPlayer({
          type: 'flv',
          url: fileurl,     
      });
      
      this.player.attachMediaElement(this);
      this.player.load();
    }
	}
	else if(this.player != null) {
		this.player.pause();
    this.player.unload();
    this.player.detachMediaElement();
    this.player.destroy();
    this.player = null;
    
    var fileurl = this.fileurl;
		this.player = flvjs.createPlayer({
        type: 'flv',
        url: fileurl,     
    });
    
    this.player.attachMediaElement(this);
    this.player.load();
	}
}
function video_onmouseover() {
	console.log(this.className);
	this.timer = setTimeout(check_mouse_time.bind(this), 500);
} 

function video_onmouseout() {
	console.log("step2");
	clearTimeout(this.timer);
	if(typeof this.player != "undefined" && this.player != null) {
			this.player.pause();
      this.player.unload();
      this.player.detachMediaElement();
      this.player.destroy();
      this.player = null;
	}

}

function creatH5VideoLabel(parent, media) {
		var video = document.createElement("video");
		video.id = parent.id+"h5 player";
		video.autoplay = true;
		video.fileurl = media.videofileurl;
		
		video.onmouseover = video_onmouseover.bind(video);
		video.onmouseout  = video_onmouseout.bind(video);
		
		parent.appendChild(video);
}

function creatVideoHref(parent, media) {
	var a = document.createElement("a");
	a.href = "/room?fileno="+parent.id;
	creatVideoCover(a, media);
	creatVideoInfo(a,media);
	parent.appendChild(a);
}

function creatVideoImage(parent, media) {
	var img = document.createElement("img");
	img.className = "video-image";
	img.src=media.imagefileurl;
	
	
	img.parent=parent;
	parent.image=img;
	parent.appendChild(img);
}
function creatVideoOverLayer(parent, media) {
	var div = document.createElement("div");
	div.className = "video-over-layer";
	parent.appendChild(div);
}
function creatVideoPlayer(parent, media) {
	var div = document.createElement("div");
	div.className="video-player";
	div.parent=parent;
	parent.videoplayer=div;
	creatH5VideoLabel(div, media);
	parent.appendChild(div);
}
function creatVideoCover(parent, media) {
	var div = document.createElement("div");
	div.className = "video-cover";
	creatVideoImage(div,media);
	creatVideoOverLayer(div, media);
	creatVideoPlayer(div, media);
	parent.appendChild(div);
}

function creatVideoInfoTitle(parent, media) {
	var span = document.createElement("span");
	span.className="video-info-title";
	span.title=media.title;
	span.innerHTML=media.title;
	
	parent.appendChild(span);
}

function creatVideoInfoAuther(parent, media) {
	var span = document.createElement("span");
	span.className="video-info-auth";
	span.title=media.auth;
	span.innerHTML=media.auth;
	
	parent.appendChild(span);
}

function creatVideoInfoLable(parent, media) {
	var span = document.createElement("span");
	span.className="video-info-lable";
	if(media.id%3==0)
			span.className+=" video-info-lable-0";
	else if(media.id%3==1) {
		span.className+=" video-info-lable-1";
	}
	else
		span.className+=" video-info-lable-2";
		
	parent.appendChild(span);
}

function creatVideoInfoCounter(parent, media) {
	var span = document.createElement("span");
	span.className="video-info-counter";
	span.innerHTML=media.people;
	
	parent.appendChild(span);
}

function creatVideoInfo(parent, media) {
	var div = document.createElement("div");
	div.className = "video-info";
	creatVideoInfoTitle(div, media);
	creatVideoInfoAuther(div, media);
	creatVideoInfoLable(div, media);
	creatVideoInfoCounter(div, media);
	
	parent.appendChild(div);
}

function creatLiLabel(parent, media) {
	var li = document.createElement("li");
	li.id = media.id;
	creatVideoHref(li, media);
	
	parent.appendChild(li);
	
	
}

function creatVideoList(parent, list) {
	var ul = document.createElement("ul");
	ul.className="video-list";
	
	var media = new Object();
	for(var i=0; i<list.list.length; i++) {
		if(list.list[i].videourl.length > 0) {
			media.id=list.list[i].id;
			media.imagefileurl=list.list[i].imageurl;
			media.videofileurl=list.list[i].videourl;
			media.title=list.list[i].title;
			media.auth=list.list[i].auth;
			media.people=list.list[i].people;
			creatLiLabel(ul, media);
		}
	}
	parent.appendChild(ul);
}

