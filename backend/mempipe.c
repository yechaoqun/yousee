/*system*/
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>


#include "cJSON.h"
#include "mempipe.h"

enum {
	TCP_NO_USED = 0,
	TCP_WAIT_REQUEST,
	TCP_WAIT_HTML_REQUEST,
	TCP_SEND_RESPONSE,
	TCP_WAIT_DATA,
	TCP_SEND_DATA
};



static char g_filelist[100][128];


typedef struct stmempipe_connection
{
	int tag; //the mempipe point address
	int state;

	int fd;
    struct pollfd *poll_entry;

	char *buffer, *buffer_ptr, *buffer_end;
	int buffer_size;

	int rsize;
	int eof;

	char html[4096];
	char *html_prt, *html_end;
}mempipe_connection;

#if 1
static int socket_set_rwbuf(int socket, int wmem, int rmem)
{
	if(setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &rmem, sizeof(rmem)) != 0) {
		printf("Set Recv Buf Failed!\n");
	}
	if(setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &wmem, sizeof(wmem)) != 0) {
		printf("Set Send Buf Failed!\n");
	}

	return 0;
}

int socket_nonblock(int socket, int enable)
{
	if(enable)
      return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
	else
      return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
}


void socket_keepalive(int sock, int keep_alive, int keep_idle, 
		int keep_interval, int keep_count)
{
    if(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,(void*)&keep_alive, sizeof(keep_alive)) == -1)
    {
        printf("setsockopt SOL_SOCKET::SO_KEEPALIVE failed, %s\n",strerror(errno));
    }
    if(setsockopt(sock, SOL_TCP, TCP_KEEPIDLE,(void *)&keep_idle,sizeof(keep_idle)) == -1)
    {
        printf("setsockopt SOL_TCP::TCP_KEEPIDLE failed, %s\n", strerror(errno));
    }
    if(setsockopt(sock,SOL_TCP,TCP_KEEPINTVL,(void *)&keep_interval, sizeof(keep_interval)) == -1)
    {
        printf("setsockopt SOL_tcp::TCP_KEEPINTVL failed, %s\n", strerror(errno));
    }
    if(setsockopt(sock,SOL_TCP,TCP_KEEPCNT,(void *)&keep_count,sizeof(keep_count)) == -1)
    {
       printf("setsockopt SOL_TCP::TCP_KEEPCNT failed, %s\n", strerror(errno));
    }
}


static int socket_open_listen(struct sockaddr_in *my_addr)
{
    int server_fd;

    server_fd = socket(AF_INET,SOCK_STREAM,0);
    if (server_fd < 0) {
        perror ("socket");
        return -1;
    }

    my_addr->sin_family = AF_INET;
    if (bind (server_fd, (struct sockaddr *) my_addr, sizeof (*my_addr)) < 0) {
        char bindmsg[32];
        snprintf(bindmsg, sizeof(bindmsg), "bind(port %d)", 6666);
        perror (bindmsg);
        close(server_fd);
        return -1;
    }

    if (listen (server_fd, 5) < 0) {
		perror ("listen");
		close(server_fd);
		return -1;
    }

	socket_nonblock(server_fd, 1);
	
    return server_fd;
}




static void new_connection(int serverfd, mempipe_connection *table, int max_con)
{
	int i = 0, len;
	int fd = -1;
	struct sockaddr_in from_addr;
	mempipe_connection *con = NULL;

	for(i=0; i<max_con; i++) {
		if(table[i].state == TCP_NO_USED) {
			con = &table[i];
			break;
		}
	}

	if(con == NULL)
		return;

	len = sizeof(from_addr);
    fd = accept(serverfd, (struct sockaddr *)&from_addr, (socklen_t *)&len);
	if(-1 == fd) {
		perror("new_connection accept failed\n");
		return;
	}

	socket_keepalive(fd, 1, 5, 10, 5);
	socket_nonblock(fd, 1);

	con->fd = fd;
	con->state = TCP_WAIT_REQUEST;

	con->buffer_size = 1024*1024;
	con->buffer = malloc(con->buffer_size);
	con->buffer_ptr = con->buffer_end = con->buffer;


	con->rsize = 0;
	con->eof = 0;
	con->html_prt = con->html;
	con->html_end = con->html+sizeof(con->html)-1;

}

static void close_connection(mempipe_connection *c)
{
	if(c->fd > 0)
		close(c->fd);

	
	free(c->buffer);

	memset(c, 0, sizeof(mempipe_connection));
	c->tag = (int)c;

	printf("close connection\n");
}

static void rt_strcat(char *buf, int buf_size, const char *str)
{
	char *p = buf;
	
	if(NULL == buf || NULL == str || buf_size <= 0)
		return;
	
	while(*p) {
		if(p-buf >= buf_size-1)
			return;
		p++;
	}

	while(*str) {
		if((p - buf) < buf_size - 1)
			*p++ = *str;
		str++;
	}
	
	*p = '\0';
}

static int html_buffer_printf(mempipe_connection *c, const char *fmt, ...)
{
	va_list ap;
    char buf[4096];
    int ret;
	int len = 0;

    va_start(ap, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

	len = c->html_end  -c->html_prt;
	if(len <= 0) {
		c->html[0] = '\0';
		c->html_prt = c->html;
		printf("%s:%d: Too Much Data To Response\n", __func__, __LINE__);
		return 0;
	}
	
	len = len<strlen(buf) ? len : strlen(buf);
	
    rt_strcat(c->html_prt, c->html_end-c->html_prt, buf);
	c->html_prt += len;

    return c->html_prt-c->html;
}

static int creat_html(mempipe_connection *c, int id)
{
	html_buffer_printf(c, "<!DOCTYPE html><html><head><title>video room</title><style></style></head>");
	html_buffer_printf(c, "<body>");
	html_buffer_printf(c, "<div class=\"mainContainer\">");
	html_buffer_printf(c, "<video id=\"%s\" url=\"%s\" controls width=\"%d\" height=\"%d\">Your browser is too old which doesn't support HTML5 video.</video>", 
		"videoElement", g_filelist[id-1], 640, 480);
	html_buffer_printf(c, "<script src=\"http://192.168.200.100:9980/flv.js\"></script>");
	html_buffer_printf(c, "<script src=\"http://192.168.200.100:9980/player.js\"></script>");
	html_buffer_printf(c, "</body></html>");

	return c->html_prt-c->html;
}

static int handle_connection(mempipe_connection *c)
{
	int rlen = 0;
	
	
	switch(c->state)
	{
		case TCP_WAIT_REQUEST :
		{
			if(c->poll_entry->revents & (POLLERR | POLLHUP))
				return -1;
			if (!(c->poll_entry->revents & POLLIN))
            	return 0;

			mempipe_request_hdr  rqthdr;
			rlen = recv(c->fd, &rqthdr,  sizeof(rqthdr), 0);
			if(rlen == sizeof(rqthdr)) {
				if(rqthdr.code == MEMPIPE_GET_HTML) {
					c->state = TCP_WAIT_HTML_REQUEST;
				}
				else {
					printf("unsupport code(%d)\n", rqthdr.code);
					return -1;
				}
			}
			else if(rlen == 0) {
				printf("client close socket\n");
				return -1;
			}
			else {
				perror("recv failed");
				return -1;
			}
		}
		break;
		case TCP_WAIT_HTML_REQUEST:
		{
			if(c->poll_entry->revents & (POLLERR | POLLHUP))
				return -1;
			if (!(c->poll_entry->revents & POLLIN))
            	return 0;

			mempipe_html_request html_request;
			rlen = recv(c->fd, &html_request,  sizeof(html_request), 0);
			if(rlen == sizeof(html_request)) {
				if(html_request.str_len > 0) {
					char tmpbuf[256];
					rlen = recv(c->fd, tmpbuf, sizeof(tmpbuf), 0);
					url_list *html = findhtml(tmpbuf, html_request.str_len);
					if(!html) {
						printf("not found the %d, %d ", html_request.code, html_request.str_len);
						for(int j=0; j<html_request.str_len; j++)
							printf("%02x ", html_request.str[j]);
						printf("\n");
						return -1;
					}
					c->buffer_ptr = c->buffer+sizeof(mempipe_response_hdr)+sizeof(mempipe_html_response); 
					c->buffer_end = c->buffer+c->buffer_size-1;
					rlen = html->handle(c->buffer_ptr, c->buffer_end-c->buffer_ptr, html->arg);

					mempipe_response_hdr *response = (mempipe_response_hdr *)c->buffer;
					response->tag  = MEMPIPE_TAG;
					response->code = MEMPIPE_GET_HTML;

					mempipe_html_response *html_response = (mempipe_html_response *)response->response;
					html_response->code = MEMPIPE_GET_HTML;
					html_response->status = 0;
					html_response->length = rlen;

					c->buffer_end = c->buffer_ptr+rlen;
					c->buffer_ptr = c->buffer;
					c->state = TCP_SEND_RESPONSE;
					
				}
				else
					return -1;

				
			}
			else if(rlen == 0) {
				printf("client close socket\n");
				return -1;
			}
			else {
				perror("recv failed");
				return -1;
			}
		}
		break;
		case TCP_SEND_RESPONSE:
		{
			
			if(c->poll_entry->revents & (POLLERR | POLLHUP)) {
				printf("SEND DATA POLLER\n");
				return -1;
			}
			
			if (!(c->poll_entry->revents & POLLOUT))
				return 0;

			if(c->buffer_ptr < c->buffer_end) {
				rlen = send(c->fd, c->buffer_ptr, c->buffer_end-c->buffer_ptr, 0);
				if(rlen > 0) {
					c->buffer_ptr += rlen;
					if(c->buffer_ptr >= c->buffer_end)
						return -1;
					return 0;
				}
				else {
					perror("send reply error");
					return -1;
				}
			}
		}
		break;
		case TCP_WAIT_DATA:
		{
			if(c->poll_entry->revents & (POLLERR | POLLHUP))
				return -1;
			if (!(c->poll_entry->revents & POLLIN))
            	return 0;
			
		}
		break;
		case TCP_SEND_DATA :
		{
			if(c->poll_entry->revents & (POLLERR | POLLHUP)) {
				printf("SEND DATA POLLER\n");
				return -1;
			}
			
			if (!(c->poll_entry->revents & POLLOUT))
	            return 0;

			if(c->buffer_ptr < c->buffer_end) {
				rlen = send(c->fd, c->buffer_ptr, c->buffer_end-c->buffer_ptr, 0);
				if(rlen > 0) {
					c->buffer_ptr += rlen;
					if(c->buffer_ptr >= c->buffer_end) {
						c->state = TCP_WAIT_DATA;
					}
					return 0;
				}
				else {
					perror("rtsp send reply error");
					return -1;
				}
			}

			
		}
		break;
	}

	return 0;
}



static int mempipe_server(void)
{
	int i = 0, max_con = 10;
	int retry = 0, ret = 0;
	int serverfd = -1;
	struct sockaddr_in inet_addr;
	struct pollfd *poll_table, *poll_entry;
	mempipe_connection *con_table, *con;


	inet_addr.sin_family=AF_INET;
	inet_addr.sin_port=htons(6666);
	inet_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(inet_addr.sin_zero),8);

S_AGAIN:
	if ((serverfd = socket_open_listen(&inet_addr)) == -1) {
		printf("Create Socket failed!\n");
		poll(NULL, 0, 1000); //1s
		retry++;
		if(retry > 3) {
			printf("Create Socket failed!\n");
			return -1;
		}
		goto S_AGAIN;
	}

	int reuseaddr = 1;
	setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                           (const void *) &reuseaddr, sizeof(int));


	poll_table = malloc((max_con*2+1+1)*sizeof(struct pollfd));
	con_table  = malloc(max_con*sizeof(mempipe_connection));

	memset(con_table, 0, max_con*sizeof(mempipe_connection));
	for(i=0; i<max_con; i++)
		con_table[i].tag = &con_table[i];

	while(1) {
		poll_entry = poll_table;
		poll_entry->fd = serverfd;
		poll_entry->events = POLLIN;
        poll_entry++;

		for(i=0; i<max_con; i++) {
			switch(con_table[i].state) {
				case TCP_WAIT_REQUEST:
				case TCP_WAIT_HTML_REQUEST:
				{
					con_table[i].poll_entry = poll_entry;
					poll_entry->fd = con_table[i].fd;
					poll_entry->events = POLLIN;
        			poll_entry++;
				}
				break;
				case TCP_SEND_RESPONSE:
				case TCP_SEND_DATA:
				{
					con_table[i].poll_entry = poll_entry;
					poll_entry->fd = con_table[i].fd;
					poll_entry->events = POLLOUT;
        			poll_entry++;
				}
				break;
				default:
					con_table[i].poll_entry = NULL;
					break;
			}
		}

		ret = poll(poll_table, poll_entry-poll_table, 1000);
		if(ret<0) {
			perror("RtspServer:poll error");
			poll(NULL, 0, 1000);
			continue;
		}

		for(i=0; i<max_con; i++) {
			if(con_table[i].poll_entry) {
				if(handle_connection(&con_table[i]))
					close_connection(&con_table[i]);
				}
		}

		poll_entry = poll_table;
		if(poll_entry->revents & POLLIN) {
			new_connection(serverfd, con_table, max_con);
		}
	}
}

#endif

int json_init(const char *filename)
{
	char buf[4096];
	cJSON *root, *js_list, *item, *it, *name;
	int array_size, i;
	char *p;
	int fd = open(filename, O_RDONLY);

	if(fd < 0) {
		printf("open failed\n");
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	read(fd, buf, sizeof(buf));
	root  = cJSON_Parse(buf);
	if(!root) {
		printf("get root faild !\n");
		return -1;
	}

	js_list = cJSON_GetObjectItem(root, "list");
	if(!js_list) {
		printf("get js_list faild !\n");
		return -1;
	}

	array_size = cJSON_GetArraySize(js_list);
	printf("array size is %d\n",array_size);
	for(i=0; i< array_size; i++) {
		item = cJSON_GetArrayItem(js_list, i);
		if(!item)
			continue;
		p = cJSON_PrintUnformatted(item);
		it = cJSON_Parse(p);
		if(!it)
			continue;
		name = cJSON_GetObjectItem(it, "highvideourl");
		
		sprintf(g_filelist[i], name->valuestring);
	}

	if(root)
		cJSON_Delete(root);

	for(i=0; i< array_size; i++) {
		printf("%s\n", g_filelist[i]);
	}
	return 0;
}

int main(void)
{
	json_init("/mnt/hgfs/VMshare/OpenCode/nginx-1.14.0/html/dist/list.data");
	mempipe_server();
	return 0;
}


