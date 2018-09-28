import React from 'react'
import styled from 'styled-components'

class VideoPlayer extends React.Component {
    constructor(props) {
        super(props);
        this.video=React.createRef();
        this.src=this.props.src;
    }
    
    render() {
        return this.creatVideoCover();
    }

    creatVideoCover() {
        return (
            <div style={{position: 'relative', left:'15px'}}>
                <video ref={this.video} controls='controls' width="1280" height="720"></video>
            </div>
        );
    }

    componentDidMount() {
        if(flvjs.isSupported()) {
            //let fileurl = 'http://192.168.200.100:9980/1_480p.flv';
            let fileurl=this.props.src;
            this.player = flvjs.createPlayer({
            type: 'flv',
            url: fileurl,     
            });

            this.player.attachMediaElement(this.video.current);
            this.player.load();
        }
    }
  
    componentWillUnmount() {
        if(this.player != null) {
			this.player.pause();
			this.player.unload();
			this.player.detachMediaElement();
			this.player.destroy();
            this.player = null;
        }
    }
  
}

export default VideoPlayer;