import React from 'react'
import styled from 'styled-components'

class VideoInfo extends React.Component {
	constructor(props) {
		super(props);
	}

	render() {
		return this.creatVideoInfo(this.props.media);
	}

	creatVideoInfo(media) {
		const Div=styled.div`
			position: relative;
			height: 52px;
			font-size: 12px;
			overflow: hidden;
		`;
		return (
			<Div>
				{this.creatTitle(media)}
				{this.creatAuther(media)}
				{this.creatVipLogo(media)}
				{this.creatStat(media)}
			</Div>
		);
	}

	creatTitle(media) {
		const Span=styled.span`
			position: absolute;
			font-size: 16px;
			color: #333;
			top: 7px;
			right: 95px;
			left: 14px;
			overflow: hidden;
			text-overflow: ellipsis;
			white-space: nowrap;
		`;
		return (
			<Span title={media.title}>
				{media.title}
			</Span>
		);
	}

	creatAuther(media) {
		const Span=styled.span`
			position: absolute;
			left: 18px;
			right: 12px;
			bottom: 0px;
			color: #999;
			overflow: hidden;
			text-overflow: ellipsis;
			white-space: nowrap;
		`;
		return (
			<Span title={media.auth}>
				{media.auth}
			</Span>
		);
	}

	creatVipLogo(media) {
		const Span=styled.span`
			position: absolute;
			bottom: 0px;
			margin-right: 5px;
			display: inline-block;
			width: 16px;
			height: 16px;
			zoom: 1;
			background: url(http://192.168.200.100:9980/label.png) no-repeat;
		`;
		const Span_vip1=styled(Span)`
			background-position: 0px 0px;
		`;
		const Span_vip2=styled(Span)`
			background-position: -17px 0px;
		`;
		const Span_vip3=styled(Span)`
			background-position: 0px -16px;
		`;
		let SpanVip={};
		if(media.id%3==1)
			SpanVip=Span_vip1;
		else if(media.id%3==2)
			SpanVip=Span_vip2;
		else
			SpanVip=Span_vip3;
		return (
			<SpanVip></SpanVip>
		);
	}

	creatStat(media) {
		const Span=styled.span`
			position: absolute;
			right: 12px;
			bottom: 3px;
			color: #666;
			font-size: 12px;
			line-height: 12px;
		`;
		return (
			<Span>
				{media.people}
			</Span>
		);
	}
}

class VideoPlayer extends React.Component {
	constructor(props) {
		super(props);
		this.video=React.createRef();
	}

	render() {
		return this.creatH5VideoLabel(this.props.media);
	}


	creatH5VideoLabel(media) {
		return (
			
			<video ref={this.video} autoplay="true" onMouseOver={this.onmouseover.bind(this)} onMouseOut={this.onmouseout.bind(this)}>

			</video>
		);
	}

	check_mouse_time() {
		console.log("step1");
		if(typeof(this.player) == "undefined" || this.player == null) {
			if(flvjs.isSupported()) {
				let fileurl = this.props.media.videofileurl;
				this.player = flvjs.createPlayer({
				type: 'flv',
				url: fileurl,     
				});

				this.player.attachMediaElement(this.video.current);
				this.player.load();
			}
		}
		else if(this.player != null) {
			this.player.pause();
			this.player.unload();
			this.player.detachMediaElement();
			this.player.destroy();
			this.player = null;
			
			let fileurl = this.props.media.videofileurl;
			this.player = flvjs.createPlayer({
			type: 'flv',
			url: fileurl,     
			});
			
			this.player.attachMediaElement(this.video.current);
			this.player.load();
		}
	}
	
	onmouseover() {
		this.timer = setTimeout(this.check_mouse_time.bind(this), 500);
	} 
	
	onmouseout() {
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
}

class VideoLabel extends React.Component {
	constructor(props) {
		super(props);
	}

	render() {
		return this.creatVideoList(this.props.list);
	}

	creatVideoList(list) {
		let func=(list) => {
			let res=[];
			let media={};
			for(let i=0; i<list.length; i++) {
				media = new Object();
				if(list[i].videourl.length > 0) {
					media.id=list[i].id;
					media.imagefileurl=list[i].imageurl;
					media.videofileurl=list[i].videourl;
					media.title=list[i].title;
					media.auth=list[i].auth;
					media.people=list[i].people;
					res.push(this.creatLiLabel(media));
				}
			}

			return res;
		}

		const Ul=styled.ul`
			list-style-type: none;
		`;
		return (
			<Ul>
				{func(list)}
			</Ul>
		);
	}

	creatLiLabel(media) {
		const Li=styled.li`
			list-style-type: none;
			float: left;
			margin: 5px;
			width: 204px;
		`;
		return (
			<Li>
				{this.creatVideoHref(media)}
			</Li>
		);
	}

	creatVideoHref(media) {
		const A=styled.a`
			osition: relative;
			width: 100%;
		`;
		return (
			<A href={"/room.html?fileno="+media.id}>
				{this.creatVideoCover(media)}
				{this.creatVideoInfo(media)}
			</A>
		);
	}

	creatVideoInfo(media) {
		return (
			<VideoInfo media={media}></VideoInfo>
		);
	}

	creatVideoCover(media) {
		const Div=styled.div`
			position: relative;
			width: 100%;
		`;
		return (
			<Div>
				{this.creatVideoImage(media)}
				{this.creatVideoOverLayer(media)}
				{this.creatVideoPlayer(media)}
			</Div>
		);
	}

	creatVideoImage(media) {
		const Img=styled.img`
			position: relative;
		`;
		return (
			<Img src={media.imagefileurl}></Img>
		);
	}

	creatVideoOverLayer(media) {
		return (
			<div></div>
		);
	}

	creatVideoPlayer(media) {
		const Div=styled.div`
			position: absolute;
			overflow: hidden;
			width: 100%;
			height: 100%;
			z-index: 100;
			left: 0;
			bottom: 0;
			right: 0;
			top: 0;
		`;
		return (
			<Div>
				<VideoPlayer media={media}></VideoPlayer>
			</Div>
		);
	}
}


export default VideoLabel;
