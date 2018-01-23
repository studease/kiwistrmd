/*
 * stu_rtmp.c
 *
 *  Created on: 2018年1月12日
 *      Author: Tony Lau
 */

#include "../stu_config.h"
#include "../core/stu_core.h"
#include "stu_rtmp.h"

stu_str_t            stu_rtmp_root = stu_string("applications");

extern stu_thread_t  stu_threads[STU_THREAD_MAXIMUM];
extern stu_int32_t   stu_thread_n;

static stu_int32_t   stu_rtmp_thread_n = -1;

static stu_socket_t  stu_rtmpfd;

static void  stu_rtmp_handler(stu_event_t *ev);


stu_int32_t
stu_rtmp_init() {
	if (stu_rtmp_phase_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp phase.");
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_listen(stu_fd_t epfd, uint16_t port) {
	stu_connection_t   *c;
	int                 optval;
	socklen_t           optlen;
	struct sockaddr_in  sa;

	optlen = sizeof(optval);

	stu_rtmpfd = socket(AF_INET, SOCK_STREAM, 0);
	if (stu_rtmpfd == -1) {
		stu_log_error(stu_errno, "Failed to create rtmp server fd.");
		return STU_ERROR;
	}

	optval = 1;
	if (setsockopt(stu_rtmpfd, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, optlen) == -1) {
		stu_log_error(stu_errno, "setsockopt(SO_REUSEADDR) failed while setting rtmp server fd.");
		return STU_ERROR;
	}

	if (setsockopt(stu_rtmpfd, SOL_SOCKET, SO_REUSEPORT, (void *) &optval, optlen) == -1) {
		stu_log_error(stu_errno, "setsockopt(SO_REUSEPORT) failed while setting rtmp server fd.");
		return STU_ERROR;
	}

/*
	if (getsockopt(stu_rtmpfd, SOL_SOCKET, SO_SNDBUF, (void *) &optval, &optlen) == -1) {
		stu_log_error(stu_errno, "getsockopt(SO_SNDBUF) failed while setting rtmp server fd.");
		return STU_ERROR;
	}
*/
	optval = 32768;
	if (setsockopt(stu_rtmpfd, SOL_SOCKET, SO_SNDBUF, (void *) &optval, optlen) == -1) {
		stu_log_error(stu_errno, "setsockopt(SO_SNDBUF) failed while setting rtmp server fd.");
		return STU_ERROR;
	}
/*
	if (getsockopt(stu_rtmpfd, SOL_SOCKET, SO_SNDBUF, (void *) &optval, &optlen) == -1) {
		stu_log_error(stu_errno, "getsockopt(SO_SNDBUF) failed while setting rtmp server fd.");
		return STU_ERROR;
	}
*/
	if (stu_nonblocking(stu_rtmpfd) == -1) {
		stu_log_error(stu_errno, "fcntl(O_NONBLOCK) failed while setting rtmp server fd.");
		return STU_ERROR;
	}

	c = stu_connection_get(stu_rtmpfd);
	if (c == NULL) {
		stu_log_error(0, "Failed to get rtmp server connection.");
		return STU_ERROR;
	}

	c->read.epfd = epfd;
	c->read.handler = stu_rtmp_handler;

	if (stu_event_add(&c->read, STU_READ_EVENT, 0) == STU_ERROR) {
		stu_log_error(0, "Failed to add rtmp server event.");
		return STU_ERROR;
	}

	bzero(&(sa.sin_zero), 8);
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htons(INADDR_ANY);
	sa.sin_port = htons(port);

	stu_log("Binding sockaddr(%hu)...", port);
	if (bind(stu_rtmpfd, (struct sockaddr*)&sa, sizeof(sa))) {
		stu_log_error(stu_errno, "Failed to bind rtmp server fd.");
		return STU_ERROR;
	}

	stu_log("Listening on port %d.", port);
	if (listen(stu_rtmpfd, port)) {
		stu_log_error(stu_errno, "Failed to listen on port %d.\n", port);
		return STU_ERROR;
	}

	return STU_OK;
}


static void
stu_rtmp_handler(stu_event_t *ev) {
	stu_connection_t   *c;
	stu_socket_t        fd;
	struct sockaddr_in  sa;
	socklen_t           socklen;
	stu_int32_t         err;

	socklen = sizeof(sa);

again:

	fd = accept(stu_rtmpfd, (struct sockaddr*)&sa, &socklen);
	if (fd == -1) {
		err = stu_errno;
		if (err == EAGAIN) {
			stu_log_debug(3, "already accepted by other threads: errno=%d.", err);
			return;
		}

		if (err == EINTR) {
			stu_log_debug(3, "accept trying again: errno=%d.", err);
			goto again;
		}

		stu_log_error(err, "Failed to accept!");
		return;
	}

	if (stu_nonblocking(fd) == -1) {
		stu_log_error(stu_errno, "fcntl(O_NONBLOCK) failed while setting rtmp client fd.");
		return;
	}

	c = stu_connection_get(fd);
	if (c == NULL) {
		stu_log_error(0, "Failed to get rtmp client connection.");
		return;
	}

	if (++stu_rtmp_thread_n >= stu_thread_n) {
		stu_rtmp_thread_n = 0;
	}

	c->read.epfd = stu_threads[stu_rtmp_thread_n].epfd;
	c->write.epfd = stu_threads[stu_rtmp_thread_n].epfd;

	c->read.handler = stu_rtmp_handshake_read_handler;

	if (stu_event_add(&c->read, STU_READ_EVENT, STU_CLEAR_EVENT) == STU_ERROR) {
		stu_log_error(0, "Failed to add rtmp handshaker read event.");
		return;
	}
}
