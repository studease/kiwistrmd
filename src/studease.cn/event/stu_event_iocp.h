/*
 * stu_event_iocp.h
 *
 *  Created on: 2018Äê3ÔÂ30ÈÕ
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_EVENT_STU_EVENT_IOCP_H_
#define STUDEASE_CN_EVENT_STU_EVENT_IOCP_H_

#include "stu_event.h"

int32_t  stu_event_iocp_create();

int32_t  stu_event_iocp_add(stu_event_t *ev, uint32_t event, uint32_t key);
int32_t  stu_event_iocp_del(stu_event_t *ev, uint32_t event, uint32_t key);

int32_t  stu_event_iocp_process_events(HANDLE iocp, stu_msec_t timer, uint32_t flags);

#endif /* STUDEASE_CN_EVENT_STU_EVENT_IOCP_H_ */
