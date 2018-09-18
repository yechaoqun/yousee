#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "html.h"
#include "mempipe.h"



static ngx_str_t main_head_block_start = ngx_string(html_main_head_block_start);
static ngx_str_t main_head_block_end   = ngx_string(html_head_block_end);
static ngx_str_t main_body_block_start = ngx_string(html_main_body_block_start);
static ngx_str_t main_body_block_end   = ngx_string(html_body_block_end);


static int creatMainHtml(char *buf, int bufsize, ngx_str_t arg)
{
static ngx_str_t main_video_arg_start   = ngx_string("privateData=\"");
static ngx_str_t main_video_arg_end     = ngx_string("\"");
static ngx_str_t main_video_block_start = ngx_string(html_main_video_block_start);
static ngx_str_t main_video_block_end   = ngx_string(html_main_video_block_end);
	int total=0;
	char *p = buf;

	total  = main_head_block_start.len+main_head_block_end.len;
	total += main_body_block_start.len+main_body_block_start.len;
	total += main_video_block_start.len+main_video_block_end.len+arg.len;
	total += main_video_arg_start.len+main_video_arg_end.len;

	if(total > bufsize) {
		printf("bufsize is no enough(%d, %d)\n", bufsize, total);
		return 0;
	}
	
	memcpy(p, main_head_block_start.data, main_head_block_start.len);
	p += main_head_block_start.len;
	
	memcpy(p, main_body_block_start.data, main_body_block_start.len);
	p += main_body_block_start.len;

	
	memcpy(p, main_video_block_start.data, main_video_block_start.len);
	p += main_video_block_start.len;

	memcpy(p, main_video_arg_start.data, main_video_arg_start.len);
	p += main_video_arg_start.len;

	memcpy(p, arg.data, arg.len);
	p += arg.len;

	memcpy(p, main_video_arg_end.data, main_video_arg_end.len);
	p += main_video_arg_end.len;

	memcpy(p, main_video_block_end.data, main_video_block_end.len);
	p += main_video_block_end.len;

	memcpy(p, main_body_block_end.data, main_body_block_end.len);
	p += main_body_block_end.len;

	memcpy(p, main_head_block_end.data, main_head_block_end.len);
	p += main_head_block_end.len;

	return p-buf;
}


static url_list html_pages[] = {
	{ ngx_string("/"), creatMainHtml, ngx_string("all") },
	{ ngx_string("/index.html"), creatMainHtml, ngx_string("all") },
	{ ngx_string("/all.html"), creatMainHtml, ngx_string("all") },
	{ ngx_string("/dianying.html"), creatMainHtml, ngx_string("dianying") },
	{ ngx_string("/dianshiju.html"), creatMainHtml, ngx_string("dianshiju") },
	{ ngx_string("/zongyi.html"), creatMainHtml, ngx_string("zongyi") },

	
	{ ngx_null_string, NULL, ngx_string("")}
};


url_list *findhtml(char *url, int url_len)
{
	int i = 0;

	for(;;i++) {
		if(html_pages[i].str.len == 0)
			break;

		if(html_pages[i].str.len == url_len && strncmp(html_pages[i].str.data, url, url_len) == 0)
			return &html_pages[i];
	}

	return NULL;
}




