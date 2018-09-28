import React from 'react'
import ReactDOM from 'react-dom';

import TitleImage from './titleimage/TitleImage.js';
import SideBar from './sidebar/sidebar.js';
import VideoPlayer from './player/VideoPlayer.js';


var main_title=document.getElementById("main-title");
var main_video = document.getElementById("main-video");
var main_bar = document.getElementById("main-bar");


ReactDOM.render(
	<TitleImage src="/summer_header.gif" />,
	main_title
);

var config = new Object();
config.privatedata=main_video.getAttribute("privatedata");

var labels=[
	{tag:"all", href:"all.html", text:'全部', sublabel:[{href:"game",text:"游戏"}, {href:"dongman",text:"动漫"}]},
	{tag:"dianying", href:"dianying.html", text:"电影",sublabel:[]},
	{tag:"dianshiju", href:"dianshiju.html", text:"电视剧", sublabel:[]}
];
config.labels=labels;

ReactDOM.render(
	<SideBar config={config} />,
	main_bar
);

ReactDOM.render(
    <VideoPlayer src={config.privatedata}></VideoPlayer>,
    main_video
);

