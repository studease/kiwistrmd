/*
 * stu_event.c
 *
 *  Created on: 2017骞�11鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_event.h"

stu_event_actions_t  stu_event_actions;

stu_queue_t          stu_posted_accept_events;
stu_queue_t          stu_posted_events;

static stu_uint32_t  stu_timer_resolution = 0;


stu_int32_t
stu_event_init() {
#if (STU_WIN32)
	stu_event_actions.create = stu_event_iocp_create;
	stu_event_actions.add = stu_event_iocp_add;
	stu_event_actions.del = stu_event_iocp_del;
	stu_event_actions.add_conn = stu_event_iocp_add_connection;
	stu_event_actions.del_conn = stu_event_iocp_del_connection;
	stu_event_actions.process_events = stu_event_iocp_process_events;
#elif (STU_HAVE_KQUEUE)
	stu_event_actions.create = stu_event_kqueue_create;
	stu_event_actions.add = stu_event_kqueue_add;
	stu_event_actions.del = stu_event_kqueue_del;
	stu_event_actions.add_conn = stu_event_kqueue_add_connection;
	stu_event_actions.del_conn = stu_event_kqueue_del_connection;
	stu_event_actions.process_events = stu_event_kqueue_process_events;
#else
	stu_event_actions.create = stu_event_epoll_create;
	stu_event_actions.add = stu_event_epoll_add;
	stu_event_actions.del = stu_event_epoll_del;
	stu_event_actions.add_conn = stu_event_epoll_add_connection;
	stu_event_actions.del_conn = stu_event_epoll_del_connection;
	stu_event_actions.process_events = stu_event_epoll_process_events;
#endif

	return STU_OK;
}

void
stu_event_process_events_and_timers(stu_fd_t evfd) {
	stu_uint8_t  flags;
	stu_msec_t   timer, delta;

	if (stu_timer_resolution) {
		timer = stu_timer_resolution;
		flags = STU_EVENT_FLAGS_NONE;
	} else {
		timer = stu_timer_find();
		flags = STU_EVENT_FLAGS_UPDATE_TIME;

#if (STU_WIN32)
		/* handle signals from master in case of network inactivity */
		if (timer == STU_TIMER_INFINITE || timer > 500) {
			//timer = 500;
		}
#endif
	}

	delta = stu_current_msec;

	(void) stu_event_process_events(evfd, timer, flags);

	delta = stu_current_msec - delta;

	stu_log_debug(2, "timer delta: %lu.", delta);

	if (delta) {
		stu_timer_expire();
	}
}
