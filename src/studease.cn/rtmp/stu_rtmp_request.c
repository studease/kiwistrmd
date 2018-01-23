/*
 * stu_rtmp_request.c
 *
 *  Created on: 2018年1月22日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


void
stu_rtmp_request_read_handler(stu_event_t *ev) {

}


void
stu_rtmp_free_request(stu_rtmp_request_t *r) {
	r->nc->connection->request = NULL;
}

void
stu_rtmp_close_request(stu_rtmp_request_t *r) {
	stu_connection_t *c;

	c = r->nc->connection;

	stu_rtmp_free_request(r);
	stu_rtmp_close_connection(c);
}

void
stu_rtmp_close_connection(stu_connection_t *c) {
	stu_connection_close(c);
}
