#ifndef __MEMPIPE_H__
#define __MEMPIPE_H__

#define ngx_string(str)     { sizeof(str) - 1, (unsigned char *) str }
#define ngx_null_string     { 0, NULL }
typedef struct {
    int      		   len;
    unsigned char     *data;
} ngx_str_t;


typedef struct sturl_list
{
    ngx_str_t str;
	int (*handle)(char *, int, ngx_str_t);

	ngx_str_t arg;
}url_list;


#define MEMPIPE_TAG		(0x50495045)

enum {
	MEMPIPE_GET_HTML = 0,
	MEMPIPE_GET_FLV  = 1
};

typedef struct {
	unsigned int tag;
	unsigned int code;
	unsigned char request[0];
}mempipe_request_hdr;

typedef struct {
	unsigned int tag;
	unsigned int code;
	unsigned char response[0];
}mempipe_response_hdr;

typedef struct stmempipe_html_request
{
	int code;
	
	int str_len;
	char str[0];
}mempipe_html_request;



typedef struct stmempipe_html_response
{
	int code;
	int status;
	int length;
}mempipe_html_response;


typedef struct stmempipe_file_request
{
    int code;

	int fileid;
	int start;
	int size;
}mempipe_file_request;

typedef struct stmempipe_file_resopnse
{
    int code;
	int status;

	int start;
	int length;
}mempipe_file_resopnse;


url_list *findhtml(char *url, int url_len);

#endif

