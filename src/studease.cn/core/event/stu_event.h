/*
 * stu_event.h
 *
 *  Created on: 2017骞�10鏈�23鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_CORE_EVENT_STU_EVENT_H_
#define STUDEASE_CN_CORE_EVENT_STU_EVENT_H_

#include "../../stu_config.h"
#include "../stu_core.h"

#define STU_EVENT_FLAGS_NONE         0x00
#define STU_EVENT_FLAGS_UPDATE_TIME  0x01

#if (STU_HAVE_IOCP)

typedef struct {
    WSAOVERLAPPED         ovlp;
    stu_event_t          *event;
    int                   error;
} stu_event_ovlp_t;

#endif

struct stu_event_s {
	unsigned              active:1;
    unsigned              error:1;
	unsigned              timedout:1;
	unsigned              timer_set:1;
	unsigned              cancelable:1;

#if (STU_HAVE_KQUEUE)
	unsigned              kq_vnode:1;

	/* the pending errno reported by kqueue */
	int                   kq_errno;
#endif

#if (STU_HAVE_KQUEUE) || (STU_HAVE_IOCP)
	int                   available;
#else
	unsigned              available:1;
#endif

#if (STU_HAVE_IOCP)
	stu_event_ovlp_t      ovlp;
#endif

	stu_fd_t              evfd;
	void                 *data;
	stu_event_handler_pt  handler;

	stu_rbtree_node_t     timer;
	stu_uint32_t          fails;
};

typedef struct {
	stu_fd_t            (*create)();

	stu_int32_t         (*add)(stu_event_t *ev, uint32_t event, stu_uint32_t flags);
	stu_int32_t         (*del)(stu_event_t *ev, uint32_t event, stu_uint32_t flags);
	stu_int32_t         (*add_conn)(stu_connection_t *c);
	stu_int32_t         (*del_conn)(stu_connection_t *c, stu_uint32_t flags);

	stu_int32_t         (*process_events)(stu_fd_t evfd, stu_msec_t timer, stu_uint32_t flags);
} stu_event_actions_t;

extern stu_event_actions_t        stu_event_actions;

#define stu_event_create          stu_event_actions.create
#define stu_event_add             stu_event_actions.add
#define stu_event_del             stu_event_actions.del
#define stu_event_add_conn        stu_event_actions.add_conn
#define stu_event_del_conn        stu_event_actions.del_conn
#define stu_event_process_events  stu_event_actions.process_events


/*
 * The event filter is deleted just before the closing file.
 * Has no meaning for select and poll.
 * kqueue, epoll, rtsig, eventport:  allows to avoid explicit delete,
 *                                   because filter automatically is deleted
 *                                   on file close,
 *
 * /dev/poll:                        we need to flush POLLREMOVE event
 *                                   before closing file.
 */
#define STU_CLOSE_EVENT    1

/*
 * disable temporarily event filter, this may avoid locks
 * in kernel malloc()/free(): kqueue.
 */
#define STU_DISABLE_EVENT  2

/*
 * event must be passed to kernel right now, do not wait until batch processing.
 */
#define STU_FLUSH_EVENT    4


/* these flags have a meaning only for kqueue */
#define STU_LOWAT_EVENT    0
#define STU_VNODE_EVENT    0

#if (STU_HAVE_KQUEUE)

#define STU_READ_EVENT     EVFILT_READ
#define STU_WRITE_EVENT    EVFILT_WRITE

#undef  STU_VNODE_EVENT
#define STU_VNODE_EVENT    EVFILT_VNODE

/*
 * STU_CLOSE_EVENT, STU_LOWAT_EVENT, and STU_FLUSH_EVENT are the module flags
 * and they must not go into a kernel so we need to choose the value
 * that must not interfere with any existent and future kqueue flags.
 * kqueue has such values - EV_FLAG1, EV_EOF, and EV_ERROR:
 * they are reserved and cleared on a kernel entrance.
 */
#undef  STU_CLOSE_EVENT
#define STU_CLOSE_EVENT    EV_EOF

#undef  STU_LOWAT_EVENT
#define STU_LOWAT_EVENT    EV_FLAG1

#undef  STU_FLUSH_EVENT
#define STU_FLUSH_EVENT    EV_ERROR

#define STU_LEVEL_EVENT    0
#define STU_ONESHOT_EVENT  EV_ONESHOT
#define STU_CLEAR_EVENT    EV_CLEAR

#undef  STU_DISABLE_EVENT
#define STU_DISABLE_EVENT  EV_DISABLE

#elif (STU_HAVE_EPOLL)

#define STU_READ_EVENT     (EPOLLIN|EPOLLRDHUP)
#define STU_WRITE_EVENT    EPOLLOUT

#define STU_CLEAR_EVENT    EPOLLET

#elif (STU_HAVE_IOCP)

#define STU_READ_EVENT     0
#define STU_WRITE_EVENT    0

#define STU_IOCP_ACCEPT    0
#define STU_IOCP_IO        1
#define STU_IOCP_CONNECT   2

#endif

#ifndef STU_CLEAR_EVENT
#define STU_CLEAR_EVENT    0    /* dummy declaration */
#endif

#define STU_UPDATE_TIME    1
#define STU_POST_EVENTS    2


stu_int32_t  stu_event_init();
void         stu_event_process_events_and_timers(stu_fd_t evfd);


#if (STU_WIN32)
#include "stu_event_iocp.h"
#elif (STU_HAVE_KQUEUE)
#include "stu_event_kqueue.h"
#else
#include "stu_event_epoll.h"
#endif

#endif /* STUDEASE_CN_CORE_EVENT_STU_EVENT_H_ */
