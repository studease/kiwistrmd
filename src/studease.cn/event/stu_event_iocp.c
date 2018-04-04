/*
 * stu_event_iocp.c
 *
 *  Created on: 2018Äê3ÔÂ30ÈÕ
 *      Author: Tony Lau
 */

#include "stu_event.h"

stu_fd_t  stu_iocp = NULL;


int32_t
stu_event_iocp_create() {
	stu_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (stu_iocp == NULL) {
		stu_log_error(stu_errno, "Failed to create iocp instance.");
		return STU_ERROR;
	}

	return STU_OK;
}


int32_t
stu_event_iocp_add(stu_event_t *ev, uint32_t event, uint32_t key) {
	stu_connection_t  *c;

	c = (stu_connection_t *) ev->data;

	c->read->active = 1;
	c->write->active = 1;

	stu_log_debug(2, "iocp add: fd=%d, key=%u, ov=%p", c->fd, key, &ev->ovlp);

	if (CreateIoCompletionPort((HANDLE) c->fd, ev->evfd, key, 0) == NULL) {
		stu_log_error(stu_errno, "CreateIoCompletionPort() failed");
		return STU_ERROR;
	}

	return STU_OK;
}

int32_t
stu_event_iocp_del(stu_event_t *ev, uint32_t event, uint32_t key) {
	return STU_OK;
}


int32_t
stu_event_iocp_process_events(HANDLE iocp, stu_msec_t timer, uint32_t flags) {
	stu_event_ovlp_t  *ovlp;
	stu_event_t       *ev;
	int                rc;
	uint32_t           key;
	u_long             bytes;
	stu_err_t          err;
	stu_msec_t         delta;

	if (timer == STU_TIMER_INFINITE) {
		timer = INFINITE;
	}

	stu_log_debug(2, "iocp timer: %M", timer);

	rc = GetQueuedCompletionStatus(iocp, &bytes, (PULONG_PTR) &key, (LPOVERLAPPED *) &ovlp, (u_long) timer);
	if (rc == 0) {
		err = stu_errno;
	} else {
		err = 0;
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
