#ifndef __NGX_HTTP_MEMPIPE_MODULE_H__
#define __NGX_HTTP_MEMPIPE_MODULE_H__


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


#endif
