/*
 * stu_event_iocp.c
 *
 *  Created on: 2018Äê3ÔÂ30ÈÕ
 *      Author: Tony Lau
 */

#include "stu_event.h"


stu_fd_t
stu_event_iocp_create() {
	stu_fd_t fd;

	fd = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (fd == NULL) {
		stu_log_error(stu_errno, "Failed to create iocp instance.");
	}

	return fd;
}


stu_int32_t
stu_event_iocp_add(stu_event_t *ev, uint32_t event, stu_uint32_t flags) {
	stu_connection_t  *c;

	c = (stu_connection_t *) ev->data;

	c->read->active = 1;
	c->write->active = 1;

	c->read->ovlp.event = c->read;
	c->write->ovlp.event = c->write;

	stu_log_debug(2, "iocp add: fd=%d, key=%u, ov=%p", c->fd, flags, &ev->ovlp);

	if (CreateIoCompletionPort((HANDLE) c->fd, ev->evfd, flags, 0) == NULL) {
		stu_log_error(stu_errno, "CreateIoCompletionPort() failed");
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_event_iocp_del(stu_event_t *ev, uint32_t event, stu_uint32_t flags) {
	return STU_OK;
}


stu_int32_t
stu_event_iocp_process_events(stu_fd_t evfd, stu_msec_t timer, stu_uint32_t flags) {
	stu_event_ovlp_t  *ovlp;
	stu_event_t       *ev;
	int                rc;
	u_long             bytes;
	stu_uint32_t       key;
	stu_err_t          err;
	stu_msec_t         delta;

	err = 0;

	if (timer == STU_TIMER_INFINITE) {
		timer = INFINITE;
	}

	stu_log_debug(2, "iocp timer: %M", timer);

	rc = GetQueuedCompletionStatus(evfd, &bytes, (PULONG_PTR) &key, (LPOVERLAPPED *) &ovlp, (u_long) timer);
	if (rc == 0) {
		err = stu_errno;
	}

	delta = stu_current_msec;

	if (flags & STU_UPDATE_TIME) {
		stu_time_update();
	}

	stu_log_debug(2, "iocp: %d b:%d k:%d ov:%p", rc, bytes, key, ovlp);

	if (timer != INFINITE) {
		delta = stu_current_msec - delta;
		stu_log_debug(2, "iocp timer: %M, delta: %M", timer, delta);
	}

	if (err) {
		if (ovlp == NULL) {
			if (err != WAIT_TIMEOUT) {
				stu_log_error(err, "GetQueuedCompletionStatus() failed");
				return STU_ERROR;
			}

			return STU_OK;
		}

		ovlp->error = err;
	}

	if (ovlp == NULL) {
		stu_log_error(0, "GetQueuedCompletionStatus() returned no operation");
		return STU_ERROR;
	}

	ev = ovlp->event;

	stu_log_debug(err, "iocp event:%p", ev);

	if (err == ERROR_NETNAME_DELETED /* the socket was closed */
		|| err == ERROR_OPERATION_ABORTED /* the operation was canceled */) {
		/*
		 * the WSA_OPERATION_ABORTED completion notification
		 * for a file descriptor that was closed
		 */
		stu_log_debug(err, "iocp: aborted event %p", ev);
		return STU_OK;
	}

	if (err) {
		stu_log_error(err, "GetQueuedCompletionStatus() returned operation error");
	}

	switch (key) {
	case STU_IOCP_ACCEPT:
		if (bytes) {
			ev->ready = 1;
		}
		break;

	case STU_IOCP_IO:
		ev->complete = 1;
		ev->ready = 1;
		break;

	case STU_IOCP_CONNECT:
		ev->ready = 1;
	}

	ev->available = bytes;

	stu_log_debug(2, "iocp event handler: %p", ev->handler);

	ev->handler(ev);

	return STU_OK;
}
