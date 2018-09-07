var player = document.getElementById('videoElement');
        if (flvjs.isSupported()) {
            var flvPlayer = flvjs.createPlayer({
                type: 'flv',
                url: player.getAttribute("url"),
                isLive: true
                
            });
            flvPlayer.attachMediaElement(videoElement);
            flvPlayer.load(); //╪сть
        }

        function flv_start() {
            player.play();
        }

        function flv_pause() {
            player.pause();
        }

        function flv_destroy() {
            player.pause();
            player.unload();
            player.detachMediaElement();
            player.destroy();
            player = null;
        }

        function flv_seekto() {
            player.currentTime = parseFloat(document.getElementsByName('seekpoint')[0].value);
        }