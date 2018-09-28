/***********************************************************************************
 * 文 件 名   : ngx_http_mempipe_module.c
 * 负 责 人   : yechaoqun
 * 创建日期   : 2018年8月27日
 * 文件描述   : 提供nignx与其他程序交互数据的功能
 * 版权说明   : Copyright (c) 2008-2018   叶超群
 * 其    他   : 
 * 修改日志   : 
***********************************************************************************/



#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#include "ngx_http_mempipe_module.h"

static ngx_str_t ngx_http_html_text_type = ngx_string("text/html");

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

typedef struct {
    ngx_http_upstream_conf_t   upstream;
} ngx_http_mempipe_loc_conf_t;


typedef struct {
    off_t        start;
    off_t        end;
    ngx_str_t    content_range;
} ngx_http_range_t;


typedef struct {
    off_t        offset;
    ngx_str_t    boundary_header;
    ngx_array_t  ranges;
} ngx_http_range_filter_ctx_t;


static char *ngx_http_mempipe_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_mempipe_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_mempipe_merge_loc_conf(ngx_conf_t * cf, void * parent, void * child);
static ngx_http_range_t *ngx_http_range_header_parse(ngx_http_request_t *r);




static ngx_command_t  ngx_http_mempipe_commands[] = {
	{ ngx_string("mempipe"),
      NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF|NGX_CONF_TAKE1,
      ngx_http_mempipe_pass,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
	ngx_null_command
};


static ngx_http_module_t  ngx_http_mempipe_module_ctx = {
    NULL,                                  /* preconfiguration */
    NULL,                                  /* postconfiguration */

    NULL,                                  /* create main configuration */
    NULL,                                  /* init main configuration */

    NULL,                                  /* create server configuration */
    NULL,                                  /* merge server configuration */

    ngx_http_mempipe_create_loc_conf,    	/* create location configuration */
    ngx_http_mempipe_merge_loc_conf			/* merge location configuration */
};



ngx_module_t  ngx_http_mempipe_module = {
    NGX_MODULE_V1,
    &ngx_http_mempipe_module_ctx,        /* module context */
    ngx_http_mempipe_commands,           /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};



static ngx_int_t ngx_http_mempipe_create_request(ngx_http_request_t *r)
{
    size_t                          len;
    ngx_buf_t                      *b;
    ngx_chain_t                    *cl;
	mempipe_request_hdr 		   *hdr;
	mempipe_html_request 		   *html_request;
	mempipe_file_request		   *flv_request;
	ngx_http_range_t			   *range;

	if(r->exten.len == 4 && ngx_strncmp(r->exten.data, "html", 4) == 0) {
		len = sizeof(mempipe_request_hdr)+sizeof(mempipe_html_request)+r->uri.len;
		
		b = ngx_create_temp_buf(r->pool, len);
		if (b == NULL) {
		   return NGX_ERROR;
		}

		hdr = (mempipe_request_hdr *)b->last;
		hdr->tag  = MEMPIPE_TAG;
		hdr->code = MEMPIPE_GET_HTML;
		
		html_request = (mempipe_html_request*)hdr->request;
		html_request->code = 0;
		html_request->str_len = r->uri.len;
		ngx_memcpy(html_request->str, r->uri.data, r->uri.len);

		if(r->args.len > 7 && ngx_strncmp(r->args.data, "fileno=", 7) == 0) {
			int rlen  = r->args.len - 7;
			u_char *p = r->args.data + 7;
			int fileid=0;
			while(rlen > 0 && *p >= '0' && *p <= '9') {
				fileid = fileid*10 + *p - '0';
				p++;
				rlen--;
			}
			printf("fileno %d\n", fileid);
			html_request->code=fileid;
		}
		
		printf("request:%s\n", html_request->str);
	}
	else if(r->exten.len == 3 && ngx_strncmp(r->exten.data, "flv", 3) == 0) {
		len = sizeof(mempipe_request_hdr)+sizeof(mempipe_file_request);
		
		b = ngx_create_temp_buf(r->pool, len);
		if (b == NULL) {
		   return NGX_ERROR;
		}

		hdr = (mempipe_request_hdr *)b->last;
		hdr->tag  = MEMPIPE_TAG;
		hdr->code = MEMPIPE_GET_FLV;
		
		flv_request = (mempipe_file_request*)hdr->request;
		flv_request->code = MEMPIPE_GET_FLV;
		flv_request->fileid = 0;
		if(r->args.len > 7 && ngx_strncmp(r->args.data, "fileno=", 7) == 0) {
			int rlen  = r->args.len - 7;
			u_char *p = r->args.data + 7;
			while(rlen > 0 && *p >= '0' && *p <= '9') {
				flv_request->fileid = flv_request->fileid*10 + *p - '0';
				p++;
				rlen--;
			}
			printf("fileno %d\n", flv_request->fileid);
		}
		else
			return NGX_ERROR;
		
		range = ngx_http_range_header_parse(r);
		if(range) {
			flv_request->start = range->start;
			flv_request->size  = range->end - range->start > 0 ? range->end - range->start : 0;

			printf("start %d, %d\n", flv_request->start, flv_request->size);
		}
		else {
			flv_request->start = 0;
			flv_request->size  = 0;
		}
	}
	else
		return NGX_ERROR;
	
    
   	b->last += len;
	
    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = b;
    cl->next = NULL;

    r->upstream->request_bufs = cl;
	r->allow_ranges = 1;
	
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "http mempipe request: \"%d\"", hdr->fileno);


    return NGX_OK;
}

static ngx_int_t ngx_http_mempipe_reinit_request(ngx_http_request_t *r)
{
    return NGX_OK;
}


static ngx_int_t ngx_http_mempipe_process_header(ngx_http_request_t *r)
{
    ngx_http_upstream_t            *u;
	mempipe_response_hdr 		   *hdr;
    u = r->upstream;
	ngx_uint_t len = u->buffer.last - u->buffer.pos;


	if(len >= sizeof(mempipe_response_hdr))
		goto found;

    return NGX_AGAIN;

found:

	hdr = (mempipe_response_hdr *)u->buffer.pos;
	if(hdr->tag != MEMPIPE_TAG) {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
			"mempipe sent invalid tag in response %x, %d", hdr->tag, hdr->code);
		return NGX_HTTP_UPSTREAM_INVALID_HEADER;
	}

	if(hdr->code == MEMPIPE_GET_HTML) {
		mempipe_html_response *html_response = (mempipe_html_response *)hdr->response;
		if(html_response->length > 0)
			u->headers_in.content_length_n = html_response->length;
		else
			u->headers_in.content_length_n = -1;

		u->buffer.pos += sizeof(mempipe_response_hdr)+sizeof(mempipe_html_response);
		r->headers_out.last_modified_time = 1535816100;
	}
	else if(hdr->code == MEMPIPE_GET_FLV) {
		mempipe_file_resopnse *file_response = (mempipe_file_resopnse *)hdr->response;
		if(file_response->length > 0)
			u->headers_in.content_length_n = file_response->length;
		else
			u->headers_in.content_length_n = -1;
		
		r->headers_out.content_offset  = file_response->start;
		u->buffer.pos += sizeof(mempipe_response_hdr)+sizeof(mempipe_file_resopnse);
	}
	else {
		ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
			"mempipe sent invalid tag in response %x, %d", hdr->tag, hdr->code);
		return NGX_HTTP_UPSTREAM_INVALID_HEADER;
	}

	
	u->headers_in.status_n = 200;
    u->state->status = 200;
    
	u->keepalive = 0;

    return NGX_OK;

}


static void ngx_http_mempipe_abort_request(ngx_http_request_t *r)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "abort http mempipe request");
    return;
}


static void ngx_http_mempipe_finalize_request(ngx_http_request_t *r, ngx_int_t rc)
{
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                   "finalize http mempipe request");
    return;
}

static ngx_int_t ngx_http_mempipe_handler(ngx_http_request_t *r)
{
    ngx_int_t                       rc;
    ngx_http_upstream_t            *u;
    ngx_http_mempipe_loc_conf_t    *mlcf;

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }
	printf("ngx_http_mempipe_handler\n");
	

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    r->headers_out.content_type_len = ngx_http_html_text_type.len;
    r->headers_out.content_type = ngx_http_html_text_type;


    if (ngx_http_upstream_create(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    u = r->upstream;

    ngx_str_set(&u->schema, "mempipe://");
    u->output.tag = (ngx_buf_tag_t) &ngx_http_mempipe_module;

    mlcf = ngx_http_get_module_loc_conf(r, ngx_http_mempipe_module);

    u->conf = &mlcf->upstream;

    u->create_request = ngx_http_mempipe_create_request;
    u->reinit_request = ngx_http_mempipe_reinit_request;
    u->process_header = ngx_http_mempipe_process_header;
    u->abort_request = ngx_http_mempipe_abort_request;
    u->finalize_request = ngx_http_mempipe_finalize_request;

    r->main->count++;

    ngx_http_upstream_init(r);

    return NGX_DONE;
}


static void *ngx_http_mempipe_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_mempipe_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_mempipe_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    /*
     * set by ngx_pcalloc():
     *
     *     conf->upstream.bufs.num = 0;
     *     conf->upstream.next_upstream = 0;
     *     conf->upstream.temp_path = NULL;
     */
		printf("ngx_http_mempipe_create_loc_conf\n");

    conf->upstream.local = NGX_CONF_UNSET_PTR;
    conf->upstream.next_upstream_tries = NGX_CONF_UNSET_UINT;
    conf->upstream.connect_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.send_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.read_timeout = NGX_CONF_UNSET_MSEC;
    conf->upstream.next_upstream_timeout = NGX_CONF_UNSET_MSEC;

    conf->upstream.buffer_size = NGX_CONF_UNSET_SIZE;

    /* the hardcoded values */
    conf->upstream.cyclic_temp_file = 0;
    conf->upstream.buffering = 0;
    conf->upstream.ignore_client_abort = 0;
    conf->upstream.send_lowat = 0;
    conf->upstream.bufs.num = 0;
    conf->upstream.busy_buffers_size = 0;
    conf->upstream.max_temp_file_size = 0;
    conf->upstream.temp_file_write_size = 0;
    conf->upstream.intercept_errors = 1;
    conf->upstream.intercept_404 = 1;
    conf->upstream.pass_request_headers = 0;
    conf->upstream.pass_request_body = 0;
    conf->upstream.force_ranges = 1;


    return conf;
}

static char *ngx_http_mempipe_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_mempipe_loc_conf_t *prev = parent;
    ngx_http_mempipe_loc_conf_t *conf = child;

    ngx_conf_merge_ptr_value(conf->upstream.local,
                              prev->upstream.local, NULL);

    ngx_conf_merge_uint_value(conf->upstream.next_upstream_tries,
                              prev->upstream.next_upstream_tries, 0);

    ngx_conf_merge_msec_value(conf->upstream.connect_timeout,
                              prev->upstream.connect_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.send_timeout,
                              prev->upstream.send_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.read_timeout,
                              prev->upstream.read_timeout, 60000);

    ngx_conf_merge_msec_value(conf->upstream.next_upstream_timeout,
                              prev->upstream.next_upstream_timeout, 0);

    ngx_conf_merge_size_value(conf->upstream.buffer_size,
                              prev->upstream.buffer_size,
                              (size_t) ngx_pagesize);

    ngx_conf_merge_bitmask_value(conf->upstream.next_upstream,
                              prev->upstream.next_upstream,
                              (NGX_CONF_BITMASK_SET
                               |NGX_HTTP_UPSTREAM_FT_ERROR
                               |NGX_HTTP_UPSTREAM_FT_TIMEOUT));

    if (conf->upstream.next_upstream & NGX_HTTP_UPSTREAM_FT_OFF) {
        conf->upstream.next_upstream = NGX_CONF_BITMASK_SET
                                       |NGX_HTTP_UPSTREAM_FT_OFF;
    }

    if (conf->upstream.upstream == NULL) {
        conf->upstream.upstream = prev->upstream.upstream;
    }


    return NGX_CONF_OK;
}


static char *ngx_http_mempipe_pass(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_mempipe_loc_conf_t *mlcf = conf;

    ngx_str_t                 *value;
    ngx_url_t                  u;
    ngx_http_core_loc_conf_t  *clcf;

    if (mlcf->upstream.upstream) {
        return "is duplicate";
    }
printf("ngx_http_mempipe_pass\n");
    value = cf->args->elts;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = value[1];
    u.no_resolve = 1;

    mlcf->upstream.upstream = ngx_http_upstream_add(cf, &u, 0);
    if (mlcf->upstream.upstream == NULL) {
        return NGX_CONF_ERROR;
    }

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    clcf->handler = ngx_http_mempipe_handler;

    if (clcf->name.data[clcf->name.len - 1] == '/') {
        clcf->auto_redirect = 1;
    }


    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_range_parse2(ngx_http_request_t *r, ngx_http_range_filter_ctx_t *ctx,
    ngx_uint_t ranges)
{
    u_char                       *p;
    off_t                         start, end, size, content_length, cutoff,
                                  cutlim;
    ngx_uint_t                    suffix;
    ngx_http_range_t             *range;


    if (ngx_array_init(&ctx->ranges, r->pool, 1, sizeof(ngx_http_range_t))
        != NGX_OK)
    {
    	 printf("%s:%d\n", __func__, __LINE__);
        return NGX_ERROR;
    }

    p = r->headers_in.range->value.data + 6;
    size = 0;
    content_length = 0;

    cutoff = NGX_MAX_OFF_T_VALUE / 10;
    cutlim = NGX_MAX_OFF_T_VALUE % 10;

	printf("%s\n", p);

    for ( ;; ) {
        start = 0;
        end = 0;
        suffix = 0;

        while (*p == ' ') { p++; }

        if (*p != '-') {
            if (*p < '0' || *p > '9') {
                return NGX_HTTP_RANGE_NOT_SATISFIABLE;
            }

            while (*p >= '0' && *p <= '9') {
                if (start >= cutoff && (start > cutoff || *p - '0' > cutlim)) {
                    return NGX_HTTP_RANGE_NOT_SATISFIABLE;
                }

                start = start * 10 + (*p++ - '0');
            }

            while (*p == ' ') { p++; }

            if (*p++ != '-') {
				return NGX_HTTP_RANGE_NOT_SATISFIABLE;
            }

            while (*p == ' ') { p++; }

            if (*p == ',' || *p == '\0') {
                end = 0;
				
				range = ngx_array_push(&ctx->ranges);
				if (range == NULL) {
					return NGX_ERROR;
				}
				
				range->start = start;
				range->end = end;
                return NGX_OK;
            }

        } else {
            suffix = 1;
            p++;
        }

        if (*p < '0' || *p > '9') {
            return NGX_HTTP_RANGE_NOT_SATISFIABLE;
        }

        while (*p >= '0' && *p <= '9') {
            if (end >= cutoff && (end > cutoff || *p - '0' > cutlim)) {
                return NGX_HTTP_RANGE_NOT_SATISFIABLE;
            }

            end = end * 10 + (*p++ - '0');
        }

        while (*p == ' ') { p++; }

        if (*p != ',' && *p != '\0') {
            return NGX_HTTP_RANGE_NOT_SATISFIABLE;
        }

        if (suffix) {
            start = (end < content_length) ? content_length - end : 0;
            end = content_length - 1;
        }

        
        end++;
		
	printf("%s:%d: start %ld, end %ld\n", __func__, __LINE__, start, end);

        if (start < end) {
            range = ngx_array_push(&ctx->ranges);
            if (range == NULL) {
                return NGX_ERROR;
            }

            range->start = start;
            range->end = end;

            if (size > NGX_MAX_OFF_T_VALUE - (end - start)) {
                return NGX_HTTP_RANGE_NOT_SATISFIABLE;
            }

            size += end - start;

            if (ranges-- == 0) {
                return NGX_DECLINED;
            }

        } else if (start == 0) {
            return NGX_DECLINED;
        }

        if (*p++ != ',') {
            break;
        }
    }

    if (ctx->ranges.nelts == 0) {
        return NGX_HTTP_RANGE_NOT_SATISFIABLE;
    }

    return NGX_OK;
}


static ngx_http_range_t *ngx_http_range_header_parse(ngx_http_request_t *r)
{
    time_t                        if_range_time;
    ngx_str_t                    *if_range, *etag;
    ngx_http_core_loc_conf_t     *clcf;
    ngx_http_range_filter_ctx_t  *ctx;

    if (r->http_version < NGX_HTTP_VERSION_10
        || (r != r->main && !r->subrequest_ranges)
        || !r->allow_ranges)
    {
    printf("%s:%d\n", __func__, __LINE__);
        return NULL;
    }

    clcf = ngx_http_get_module_loc_conf(r, ngx_http_core_module);

    if (clcf->max_ranges == 0) {
		  printf("%s:%d\n", __func__, __LINE__);
        return NULL;
    }

    if (r->headers_in.range == NULL
        || r->headers_in.range->value.len < 7
        || ngx_strncasecmp(r->headers_in.range->value.data,
                           (u_char *) "bytes=", 6)
           != 0)
    {
      printf("%s:%d\n", __func__, __LINE__);
        goto next_filter;
    }

    if (r->headers_in.if_range) {

        if_range = &r->headers_in.if_range->value;

        if (if_range->len >= 2 && if_range->data[if_range->len - 1] == '"') {

            if (r->headers_out.etag == NULL) {
				  printf("%s:%d\n", __func__, __LINE__);
                goto next_filter;
            }

            etag = &r->headers_out.etag->value;

            ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                           "http ir:%V etag:%V", if_range, etag);

            if (if_range->len != etag->len
                || ngx_strncmp(if_range->data, etag->data, etag->len) != 0)
            {
              printf("%s:%d\n", __func__, __LINE__);
                goto next_filter;
            }

            goto parse;
        }

        if (r->headers_out.last_modified_time == (time_t) -1) {
			  printf("%s:%d\n", __func__, __LINE__);
            goto next_filter;
        }

        if_range_time = ngx_parse_http_time(if_range->data, if_range->len);

        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "http ir:%T lm:%T",
                       if_range_time, r->headers_out.last_modified_time);

        if (if_range_time != r->headers_out.last_modified_time) {
			  printf("%s:%d\n", __func__, __LINE__);
            goto next_filter;
        }
    }

parse:

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_range_filter_ctx_t));
    if (ctx == NULL) {
		  printf("%s:%d\n", __func__, __LINE__);
        return NULL;
    }

    ctx->offset = r->headers_out.content_offset;

    switch (ngx_http_range_parse2(r, ctx, 1)) {

    case NGX_OK:
		if(ctx->ranges.nelts == 1)
        	return ctx->ranges.elts;
		  printf("%s:%d\n", __func__, __LINE__);
		break;
    default: /* NGX_DECLINED */
		 printf("%s:%d\n", __func__, __LINE__);
        break;
    }

next_filter:

    return NULL;
}






