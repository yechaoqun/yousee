function creatTopTitle(parent, config) {
	var img = document.createElement("img");
	img.className = "toptitle-image";
	img.src="/summer_header.gif";
	
	parent.appendChild(img);
}

function creatBarCateLiLabelHref(parent, href, text) {
	var a = document.createElement("a");
	a.href=href;
	a.innerHTML=text;
	parent.appendChild(a);
}

function openLable() {
	if(this.openlabel==false) {
		this.openlabel=true;
		this.className+=" bar-cates-label-open";
		this.parentNode.getElementsByClassName("bar-cates-sub-label")[0].style.display="block";
	}
	else {
		this.openlabel=false;
		this.className="bar-cates-label";
		this.parentNode.getElementsByClassName("bar-cates-sub-label")[0].style.display="none";
	}
}

function creatBarCateLiLabel(parent, href, text) {
	var div = document.createElement("div");
	div.className="bar-cates-label";
	creatBarCateLiLabelHref(div, href, text);
	
	div.openlabel=false;
	var right=document.createElement("i");
	var down=document.createElement("i");
	right.className="ricon ricon-right";
	down.className="ricon ricon-down";
	
	right.onclick=openLable.bind(div);
	down.onclick=openLable.bind(div);
	div.appendChild(right);
	div.appendChild(down);
	
	var div_sub=document.createElement("div");
	div_sub.className="bar-cates-sub-label"
	creatBarCateLiLabelHref(div_sub, "", "喜剧");
	creatBarCateLiLabelHref(div_sub, "", "动作");
	
	parent.appendChild(div);
	parent.appendChild(div_sub);
}

function creatBarCateLi(parent, active, href, text) {
	var li = document.createElement("li");
	li.className="bar-cates-li";
	if(active)
		li.className+=" bar-content-active";
	creatBarCateLiLabel(li, href, text);
	parent.appendChild(li);
	
	return li;
}

function creatBarCates(parent, config) {
	var active=0;
	var dom;
	var ul=document.createElement("ul");
	ul.id="bar-cates";
	
	if(config.privatedata=="all")
		active=1;
	if(config.privatedata=="dianying")
		active=2;
	if(config.privatedata=="dianshiju")
		active=3;
	if(config.privatedata=="zongyi")
		active=4;
		
	dom=creatBarCateLi(ul, active==1, "/all.html", "全部");
		
	dom=creatBarCateLi(ul, active==2, "/dianying.html", "电影");
	dom=creatBarCateLi(ul, active==3, "/dianshiju.html", "电视剧");
	dom=creatBarCateLi(ul, active==4, "/zongyi.html", "综艺");
	parent.appendChild(ul);
}

var main_title=document.getElementById("main-title");
creatTopTitle(main_title, null);

var main_video = document.getElementById("main-video");
var config = new Object();
config.privatedata=main_video.getAttribute("privatedata");

var main_bar = document.getElementById("main-bar");
main_bar.innerHTML="";
creatBarCates(main_bar, config);



var filelist;
var jq = $.noConflict();
jq.ajax({
	url:'list.data',
	async:false,
	cache:false,
	success:function(data,status,jqXHR) {
		filelist = JSON.parse(data);
	}
});

creatVideoList(main_video, filelist);
