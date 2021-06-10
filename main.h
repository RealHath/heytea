#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/event_compat.h>
#include <event2/http_compat.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>
#include <event2/keyvalq_struct.h>

short http_port = 8084;
std::string http_addr = "0.0.0.0";
const size_t BUF_MAX = 1024 * 16;

void get_post_message(std::string &buf, struct evhttp_request *req);
void page404(evhttp_request *req, void *arg);
void login(evhttp_request *req, void *arg);
void resetPasswod(evhttp_request *req, void *arg);

#endif