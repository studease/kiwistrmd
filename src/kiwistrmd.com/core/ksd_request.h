/*
 * ksd_request.h
 *
 *  Created on: 2018年1月30日
 *      Author: Tony Lau
 */

#ifndef KIWISTRMD_COM_CORE_KSD_REQUEST_H_
#define KIWISTRMD_COM_CORE_KSD_REQUEST_H_

#include "ksd_core.h"

#define KSD_REQUEST_DEFAULT_SIZE  1024

stu_int32_t  ksd_request_init();

void  ksd_request_read_handler(stu_event_t *ev);

void  ksd_free_request(stu_websocket_request_t *r);
void  ksd_close_request(stu_websocket_request_t *r);
void  ksd_close_connection(stu_connection_t *c);

stu_int32_t  ksd_add_push_stat_timer(stu_msec_t timer);

#endif /* KIWISTRMD_COM_CORE_KSD_REQUEST_H_ */
