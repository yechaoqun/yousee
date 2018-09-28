#ifndef __HTML_H__
#define __HTML_H__



static char html_main_head_block_start[] = {
"<!DOCTYPE html><html><head>"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
"<title>demo</title>"
"<meta name=\"keywords\" content=\"YouSee视频点播平台̨\"></>"
"<meta name=\"description\" content=\"YouSee视频点播平台\"></>"
"<meta name=\"author\" content=\"yechaoqun1006@163.com\"></>"
"<style type=\"text/css\">"
".toptitle {position: absolute;left: 210px; right: 0;height: 72px;}"
".bar-content {position: fixed;left: 0; top: 0; bottom: 0;width: 210px;background:#191b2e;}"
".video-content {position: absolute;left:210px; top: 72px; right: 0; bottom: 0;}"
"</style>"
"<script src=\"jquery-3.1.1.min.js\"></script>"
"<script src=\"flv.js\"></script>"
"</head>"
};

static char html_main_body_block_start[] = {
"<body>"
"<div class=\"toptitle\" id=\"main-title\">"
"</div>"
"<div class=\"bar-content\" id=\"main-bar\">"
"</div>"
};

static char html_main_video_block_start[] = {
	"<div class=\"video-content\" id=\"main-video\" "
};


static char html_main_video_block_end[] = {
	"></div>"
};

static char html_body_block_end[] = {
    "<script src=\"video-list.js\"></script>"
    "<script src=\"main.js\"></script></body>"
};
static char html_head_block_end[] = {"</html>"};

#endif

