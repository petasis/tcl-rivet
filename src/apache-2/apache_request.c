/* Copyright 2000-2004 The Apache Software Foundation

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   	http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <apr_lib.h>

#include "apache_request.h"
#include "apache_multipart_buffer.h"
int fill_buffer(multipart_buffer *self); /* needed for mozilla hack */

static void req_plustospace(char *str)
{
    register int x;
    for(x=0;str[x];x++) if(str[x] == '+') str[x] = ' ';
}

static int
util_read(ApacheRequest *req, const char **rbuf)
{
    request_rec *r = req->r;
    int rc = OK;

    if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR))) {
        return rc;
    }

    if (ap_should_client_block(r)) {
        char buff[HUGE_STRING_LEN];
        int rsize, len_read, rpos=0;
        long length = r->remaining;

        if (length > req->post_max && req->post_max > 0) {
            //TODO: fix logging apr_log_rerror
            //ap_log_error(REQ_ERROR, "[libapreq] entity too large (%d, max=%d)",
            //        (int)length, req->post_max);
            //apr_log_rerror(REQ_ERROR, "[libapreq] entity too large (%d, max=%d)",
            //        (int)length, req->post_max);
            return HTTP_REQUEST_ENTITY_TOO_LARGE;
        }

        *rbuf = apr_pcalloc(r->pool, length + 1);

        //TODO: fix ap_hard_timeout
        //ap_hard_timeout("[libapreq] util_read", r);

        while ((len_read =
                    ap_get_client_block(r, buff, sizeof(buff))) > 0) {
            if ((rpos + len_read) > length) {
                rsize = length - rpos;
            }
            else {
                rsize = len_read;
            }
            memcpy((char*)*rbuf + rpos, buff, rsize);
            rpos += rsize;
        }

        //TODO: fix ap_kill_timeout()
        //ap_kill_timeout(r);
    }

    return rc;
}

char *ApacheRequest_script_name(ApacheRequest *req)
{
    request_rec *r = req->r;
    char *tmp;

    if (r->path_info && *r->path_info) {
        int path_info_start = ap_find_path_info(r->uri, r->path_info);
        tmp = (char*) apr_pstrndup(r->pool, r->uri, path_info_start);
    }
    else {
        tmp = r->uri;
    }

    return tmp;
}

char *ApacheRequest_script_path(ApacheRequest *req)
{
    return ap_make_dirstr_parent(req->r->pool, ApacheRequest_script_name(req));
}

const char *ApacheRequest_param(ApacheRequest *req, const char *key)
{
    ApacheRequest_parse(req);
    return (char*) apr_table_get(req->parms, key);
}

static int make_params(void *data, const char *key, const char *val)
{
    //array_header *arr = (array_header *)data;
    apr_array_header_t *arr = (apr_array_header_t *)data;
    //*(char **)apr_push_array(arr) = (char *)val;
    *(char **)apr_array_push(arr) = (char *)val;
    return 1;
}

//array_header *ApacheRequest_params(ApacheRequest *req, const char *key)
apr_array_header_t *ApacheRequest_params(ApacheRequest *req, const char *key)
{
    //array_header *values = ap_make_array(req->r->pool, 4, sizeof(char *));
    apr_array_header_t *values = apr_array_make(req->r->pool, 4, sizeof(char *));
    ApacheRequest_parse(req);
    apr_table_do(make_params, (void*)values, req->parms, key, NULL);
    return values;
}

char *ApacheRequest_params_as_string(ApacheRequest *req, const char *key)
{
    char *retval = NULL;
    //array_header *values = ApacheRequest_params(req, key);
    apr_array_header_t *values = ApacheRequest_params(req, key);
    int i;

    for (i=0; i<values->nelts; i++) {
	retval = (char*) apr_pstrcat(req->r->pool,
			    retval ? retval : "",
			    ((char **)values->elts)[i],
			    (i == (values->nelts - 1)) ? NULL : ", ",
			    NULL);
    }

    return retval;
}

//table *ApacheRequest_query_params(ApacheRequest *req, ap_pool *p)
apr_table_t *ApacheRequest_query_params(ApacheRequest *req, /*ap_pool*/apr_pool_t *p)
{
    //array_header *a = ap_palloc(p, sizeof *a);
    //array_header *b = (array_header *)req->parms;
    apr_array_header_t *a = apr_palloc(p, sizeof *a);
    apr_array_header_t *b = (apr_array_header_t *)req->parms;

    a->elts     = b->elts;
    a->nelts    = req->nargs;

    a->nalloc   = a->nelts; /* COW hack: array push will induce copying */
    //a->elt_size = sizeof(table_entry);
    //return (table *)a;
    a->elt_size = sizeof(apr_table_entry_t);
    return (apr_table_t *)a;
}

apr_table_t *ApacheRequest_post_params(ApacheRequest *req, apr_pool_t *p)
{
    apr_array_header_t *a = apr_palloc(p, sizeof *a);
    apr_array_header_t *b = (apr_array_header_t *)req->parms;

    a->elts     = (void *)( (apr_table_entry_t *)b->elts + req->nargs );
    a->nelts    = b->nelts - req->nargs;

    a->nalloc   = a->nelts; /* COW hack: array push will induce copying */
    a->elt_size = sizeof(apr_table_entry_t);
    return (apr_table_t *)a;
}

ApacheUpload *ApacheUpload_new(ApacheRequest *req)
{
    ApacheUpload *upload = (ApacheUpload *)
	apr_pcalloc(req->r->pool, sizeof(ApacheUpload));

    upload->next = NULL;
    upload->name = NULL;
    upload->info = NULL;
    upload->fp   = NULL;
    upload->size = 0;
    upload->req  = req;

    return upload;
}

ApacheUpload *ApacheUpload_find(ApacheUpload *upload, char *name)
{
    ApacheUpload *uptr;

    for (uptr = upload; uptr; uptr = uptr->next) {
	if (strEQ(uptr->name, name)) {
	    return uptr;
	}
    }

    return NULL;
}

ApacheRequest *ApacheRequest_new(request_rec *r)
{
    ApacheRequest *req = (ApacheRequest *)
	apr_pcalloc(r->pool, sizeof(ApacheRequest));

    req->status = OK;
    req->parms = apr_table_make(r->pool, DEFAULT_TABLE_NELTS);
    req->upload = NULL;
    req->post_max = -1;
    req->disable_uploads = 0;
    req->upload_hook = NULL;
    req->hook_data = NULL;
    req->temp_dir = NULL;
    req->raw_post = NULL;
    req->parsed = 0;
    req->r = r;
    req->nargs = 0;

    return req;
}
static char x2c(const char *what)
{
    register char digit;

#ifndef CHARSET_EBCDIC
    digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'))
;
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));
#else /*CHARSET_EBCDIC*/
    char xstr[5];
    xstr[0]='0';
    xstr[1]='x';
    xstr[2]=what[0];
    xstr[3]=what[1];
    xstr[4]='\0';
    digit = os_toebcdic[0xFF & ap_strtol(xstr, NULL, 16)];
#endif /*CHARSET_EBCDIC*/
    return (digit);
}


static unsigned int utf8_convert(char *str) {
    long x = 0;
    int i = 0;
    while (i < 4 ) {
	if ( apr_isxdigit(str[i]) != 0 ) {
	    if( apr_isdigit(str[i]) != 0 ) {
		x = x * 16 + str[i] - '0';
	    }
	    else {
		str[i] = tolower( str[i] );
		x = x * 16 + str[i] - 'a' + 10;
	    }
	}
	else {
	    return 0;
	}
	i++;
    }
    if(i < 3)
	return 0;
    return (x);
}

static int ap_unescape_url_u(char *url)
{
    register int x, y, badesc, badpath;

    badesc = 0;
    badpath = 0;
    for (x = 0, y = 0; url[y]; ++x, ++y) {
	if (url[y] != '%'){
	    url[x] = url[y];
	}
	else {
	    if(url[y + 1] == 'u' || url[y + 1] == 'U'){
		unsigned int c = utf8_convert(&url[y + 2]);
		y += 5;
		if(c < 0x80){
		    url[x] = c;
		}
                else if(c < 0x800) {
                    url[x] = 0xc0 | (c >> 6);
                    url[++x] = 0x80 | (c & 0x3f);
                }
		else if(c < 0x10000){
		    url[x] = (0xe0 | (c >> 12));
		    url[++x] = (0x80 | ((c >> 6) & 0x3f));
		    url[++x] = (0x80 | (c & 0x3f));
		}
		else if(c < 0x200000){
		    url[x] = 0xf0 | (c >> 18);
		    url[++x] = 0x80 | ((c >> 12) & 0x3f);
		    url[++x] = 0x80 | ((c >> 6) & 0x3f);
		    url[++x] = 0x80 | (c & 0x3f);
		}
		else if(c < 0x4000000){
		    url[x] = 0xf8 | (c >> 24);
		    url[++x] = 0x80 | ((c >> 18) & 0x3f);
		    url[++x] = 0x80 | ((c >> 12) & 0x3f);
		    url[++x] = 0x80 | ((c >> 6) & 0x3f);
		    url[++x] = 0x80 | (c & 0x3f);
		}
		else if(c < 0x8000000){
		    url[x] = 0xfe | (c >> 30);
		    url[++x] = 0x80 | ((c >> 24) & 0x3f);
		    url[++x] = 0x80 | ((c >> 18) & 0x3f);
		    url[++x] = 0x80 | ((c >> 12) & 0x3f);
		    url[++x] = 0x80 | ((c >> 6) & 0x3f);
		    url[++x] = 0x80 | (c & 0x3f);
		}
	    }
	    else {
		if (!apr_isxdigit(url[y + 1]) || !apr_isxdigit(url[y + 2])) {
		    badesc = 1;
		    url[x] = '%';
		}
		else {
		    url[x] = x2c(&url[y + 1]);
		    y += 2;
		    if (url[x] == '/' || url[x] == '\0')
			badpath = 1;
		}
	    }
	}
    }
    url[x] = '\0';
    if (badesc)
	   return HTTP_BAD_REQUEST;
    else if (badpath)
	   return HTTP_NOT_FOUND;
    else
	   return OK;
}

static int urlword_dlm[] = {'&', ';', 0};

static char *my_urlword(apr_pool_t *p, const char **line)
{
    char *res = NULL;
    const char *pos = *line;
    char ch;

    while ( (ch = *pos) != '\0' && ch != ';' && ch != '&') {
	   ++pos;
    }

    res = (char*) apr_pstrndup(p, *line, pos - *line);

    while (ch == ';' || ch == '&') {
	   ++pos;
	   ch = *pos;
    }

    *line = pos;

    return res;
}


static void split_to_parms(ApacheRequest *req, const char *data)
{
    request_rec *r = req->r;
    const char *val;

    while (*data && (val = my_urlword(r->pool, &data))) {
        const char *key = ap_getword(r->pool, &val, '=');

        req_plustospace((char*)key);
        ap_unescape_url_u((char*)key);
        req_plustospace((char*)val);
        ap_unescape_url_u((char*)val);
        apr_table_add(req->parms, key, val);
    }

}

int ApacheRequest___parse(ApacheRequest *req)
{
    request_rec *r = req->r;
    int result;

    if (r->args) {
        split_to_parms(req, r->args);
        req->nargs = ((apr_array_header_t *)req->parms)->nelts;
    }

    if (r->method_number == M_POST) {
        const char *ct = apr_table_get(r->headers_in, "Content-type");
        if (ct && strncaseEQ(ct, DEFAULT_ENCTYPE, DEFAULT_ENCTYPE_LENGTH)) {
            result = ApacheRequest_parse_urlencoded(req);
        } else if (ct && strncaseEQ(ct, TEXT_XML_ENCTYPE, TEXT_XML_ENCTYPE_LENGTH)) {
            result = ApacheRequest_parse_urlencoded(req);
        } else if (ct && strncaseEQ(ct, MULTIPART_ENCTYPE, MULTIPART_ENCTYPE_LENGTH)) {
            result = ApacheRequest_parse_multipart(req);
        } else {
            //TODO: fix logging apr_log_rerror
            //apr_log_rerror(REQ_ERROR,"[libapreq] unknown content-type: `%s'", ct);
            result = HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    else {
        result = ApacheRequest_parse_urlencoded(req);
    }

    req->parsed = 1;
    return result;
}

int ApacheRequest_parse_urlencoded(ApacheRequest *req)
{
    request_rec *r = req->r;
    int rc = OK;

    if (r->method_number == M_POST) {
    	const char *data = NULL, *type;
    
    	type = apr_table_get(r->headers_in, "Content-Type");
    
    	if (!strncaseEQ(type, DEFAULT_ENCTYPE, DEFAULT_ENCTYPE_LENGTH) &&
    	    !strncaseEQ(type, TEXT_XML_ENCTYPE, TEXT_XML_ENCTYPE_LENGTH)) {
    	    return DECLINED;
    	}
    	if ((rc = util_read(req, &data)) != OK) {
    	    return rc;
    	}
    	if (data) {
    	    req->raw_post = (char*) data; /* Give people a way of getting at the raw data. */
    	    split_to_parms(req, data);
    	}
    }

    return OK;
}

static void remove_tmpfile(void *data)
{
    ApacheUpload *upload = (ApacheUpload *) data;
    ApacheRequest *req = upload->req;

    //TODO: fix ap_pfclose
    //if( ap_pfclose(req->r->pool, upload->fp) )
        //TODO: fix logging apr_log_rerror
        //apr_log_rerror(REQ_ERROR,"[libapreq] close error on '%s'", upload->tempname);
#ifndef DEBUG
    if( remove(upload->tempname) )
        //TODO: fix logging apr_log_rerror
        //apr_log_rerror(REQ_ERROR,"[libapreq] remove error on '%s'", upload->tempname);
#endif

    free(upload->tempname);
}

FILE *ApacheRequest_tmpfile(ApacheRequest *req, ApacheUpload *upload)
{
    request_rec *r = req->r;
    FILE *fp = 0;
    char prefix[] = "apreq";
    char *name = NULL;
    int fd = 0; 
    int tries = 100;

    //TODO: port it
    while (--tries > 0) {
    	if ( (name = tempnam(req->temp_dir, prefix)) == NULL )
    	    continue;
    	//fd = apr_popenf(r->pool, name, O_CREAT|O_EXCL|O_RDWR|O_BINARY, 0600);
        if ( fd >= 0 )
    	    break; /* success */
    	else
    	    free(name);
    }
    
    //if ( tries == 0  || (fp = ap_pfdopen(r->pool, fd, "w+" "b") ) == NULL ) {
        //ap_log_rerror(REQ_ERROR, 
        //                  "[libapreq] could not create/open temp file: %s",
        //                  strerror(errno));
    	//if ( fd >= 0 ) { remove(name); free(name); }
    	//return NULL;
    //}

    upload->fp = fp;
    upload->tempname = name;
    apr_pool_cleanup_register (r->pool, (void *)upload,
			remove_tmpfile, apr_pool_cleanup_null);
    return fp;

}

int
ApacheRequest_parse_multipart(ApacheRequest *req)
{
    request_rec *r = req->r;
    int rc = OK;
    const char *ct = apr_table_get(r->headers_in, "Content-Type");
    long length;
    char *boundary;
    multipart_buffer *mbuff;
    ApacheUpload *upload = NULL;

    if (!ct) {
        //TODO: fix logging apr_log_rerror
        //apr_log_rerror(REQ_ERROR, "[libapreq] no Content-type header!");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR))) {
        return rc;
    }

    if (!ap_should_client_block(r)) {
        return rc;
    }

    if ((length = r->remaining) > req->post_max && req->post_max > 0) {
        //TODO: fix logging apr_log_rerror
        //apr_log_rerror(REQ_ERROR, "[libapreq] entity too large (%d, max=%d)",
        //        (int)length, req->post_max);
        return HTTP_REQUEST_ENTITY_TOO_LARGE;
    }

    do {
        int blen;
        boundary = ap_getword(r->pool, &ct, '=');
        if (boundary == NULL)
            return DECLINED;
        blen = strlen(boundary);
        if (blen == 0 || blen < strlen("boundary"))
            return DECLINED;
        boundary += blen - strlen("boundary");
    } while (strcasecmp(boundary,"boundary") != 0);

    boundary = ap_getword_conf(r->pool, &ct);

    if (!(mbuff = multipart_buffer_new(boundary, length, r))) {
        return DECLINED;
    }

    while (!multipart_buffer_eof(mbuff)) {
        apr_table_t *header = (apr_table_t*) multipart_buffer_headers(mbuff);
        const char *cd, *param=NULL, *filename=NULL;
        char buff[FILLUNIT];
        int blen, wlen;

        if (!header) {
#ifdef DEBUG
            //TODO: fix logging apr_log_rerror
            //apr_log_rerror(REQ_ERROR,
            //   "[libapreq] silently drop remaining '%ld' bytes", r->remaining);
#endif
            //TODO: fix ap_hard_timeout
            //ap_hard_timeout("[libapreq] parse_multipart", r);
            while ( ap_get_client_block(r, buff, sizeof(buff)) > 0 )
                /* wait for more input to ignore */ ;
            //TODO: fix ap_kill_timeout()     
            //ap_kill_timeout(r);
            return OK;
        }

        if ((cd = apr_table_get(header, "Content-Disposition"))) {
            const char *pair;

            while (*cd && (pair = ap_getword(r->pool, &cd, ';'))) {
                const char *key;

                while (apr_isspace(*cd)) {
                    ++cd;
                }
                if (ap_ind(pair, '=')) {
                    key = ap_getword(r->pool, &pair, '=');
                    if(strcaseEQ(key, "name")) {
                        param = ap_getword_conf(r->pool, &pair);
                    }
                    else if(strcaseEQ(key, "filename")) {
                        filename = ap_getword_conf(r->pool, &pair);
                    }
                }
            }
            if (!filename) {
                char *value = multipart_buffer_read_body(mbuff);
                apr_table_add(req->parms, param, value);
                continue;
            }
            if (!param) continue; /* shouldn't happen, but just in case. */

            if (req->disable_uploads) {
                //TODO: fix logging apr_log_rerror
                //apr_log_rerror(REQ_ERROR, "[libapreq] file upload forbidden");
                return HTTP_FORBIDDEN;
            }

            apr_table_add(req->parms, param, filename);

            if (upload) {
                upload->next = ApacheUpload_new(req);
                upload = upload->next;
            }
            else {
                upload = ApacheUpload_new(req);
                req->upload = upload;
            }

            if (! req->upload_hook && ! ApacheRequest_tmpfile(req, upload) ) {
                return HTTP_INTERNAL_SERVER_ERROR;
            }

            upload->info = header;
            upload->filename = (char*)apr_pstrdup(req->r->pool, filename);
            upload->name = (char*)apr_pstrdup(req->r->pool, param);

            /* mozilla empty-file (missing CRLF) hack */
            fill_buffer(mbuff);
            if( strEQN(mbuff->buf_begin, mbuff->boundary, 
                        strlen(mbuff->boundary)) ) {
                r->remaining -= 2;
                continue; 
            }

            while ((blen = multipart_buffer_read(mbuff, buff, sizeof(buff)))) {
                if (req->upload_hook != NULL) {
                    wlen = req->upload_hook(req->hook_data, buff, blen, upload);
                } else {
                    wlen = fwrite(buff, 1, blen, upload->fp);
                }
                if (wlen != blen) {
                    return HTTP_INTERNAL_SERVER_ERROR;
                }
                upload->size += wlen;
            }

            if (upload->size > 0 && (upload->fp != NULL)) {
                fseek(upload->fp, 0, 0);
            }
        }
    }

    return OK;
}

#define Mult_s 1
#define Mult_m 60
#define Mult_h (60*60)
#define Mult_d (60*60*24)
#define Mult_M (60*60*24*30)
#define Mult_y (60*60*24*365)

static int expire_mult(char s)
{
    switch (s) {
    case 's':
	return Mult_s;
    case 'm':
	return Mult_m;
    case 'h':
	return Mult_h;
    case 'd':
	return Mult_d;
    case 'M':
	return Mult_M;
    case 'y':
	return Mult_y;
    default:
	return 1;
    };
}

static time_t expire_calc(char *time_str)
{
    int is_neg = 0, offset = 0;
    char buf[256];
    int ix = 0;

    if (*time_str == '-') {
        is_neg = 1;
        ++time_str;
    }
    else if (*time_str == '+') {
        ++time_str;
    }
    else if (strcaseEQ(time_str, "now")) {
        /*ok*/
    }
    else {
        return 0;
    }

    /* wtf, ap_isdigit() returns false for '1' !? */
    while (*time_str && (apr_isdigit(*time_str) || (*time_str == '1'))) {
        buf[ix++] = *time_str++;
    }
    buf[ix] = '\0';
    offset = atoi(buf);

    return time(NULL) +
        (expire_mult(*time_str) * (is_neg ? (0 - offset) : offset));
}

char *ApacheUtil_expires(apr_pool_t *p, char *time_str, int type)
{
    time_t when;
    struct tm *tms;
    int sep = (type == EXPIRES_HTTP) ? ' ' : '-';

    if (!time_str) {
	return NULL;
    }

    when = expire_calc(time_str);

    if (!when) {
	   return (char*) apr_pstrdup(p, time_str);
    }

    tms = gmtime(&when);
    return (char*) apr_psprintf(p,
		       "%s, %.2d%c%s%c%.2d %.2d:%.2d:%.2d GMT",
		       apr_day_snames[tms->tm_wday],
		       tms->tm_mday, sep, apr_month_snames[tms->tm_mon], sep,
		       tms->tm_year + 1900,
		       tms->tm_hour, tms->tm_min, tms->tm_sec);
}

char *ApacheRequest_expires(ApacheRequest *req, char *time_str)
{
    return ApacheUtil_expires(req->r->pool, time_str, EXPIRES_HTTP);
}
