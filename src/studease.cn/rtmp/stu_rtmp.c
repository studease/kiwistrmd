/*
 * stu_rtmp.c
 *
 *  Created on: 2018骞�1鏈�12鏃�
 *      Author: Tony Lau
 */

#include "../stu_config.h"
#include "../core/stu_core.h"
#include "stu_rtmp.h"

stu_str_t            stu_rtmp_root = stu_string("applications");

stu_rtmp_amf_t      *stu_rtmp_properties;
stu_rtmp_amf_t      *stu_rtmp_version;

extern stu_thread_t  stu_threads[STU_THREAD_MAXIMUM];
extern stu_int32_t   stu_thread_n;

static stu_socket_t  stu_rtmpfd;
static stu_int32_t   stu_rtmp_thread_n = -1;

static void  stu_rtmp_handler(stu_event_t *ev);


stu_int32_t
stu_rtmp_init() {
	stu_rtmp_amf_t *item;
	stu_str_t       key, val;

	// FMS properties
	stu_rtmp_properties = stu_rtmp_amf_create_object(NULL);
	stu_rtmp_properties->ended = TRUE;

	stu_str_set(&key, "fmsVer");
	stu_str_set(&val, "FMS/5,0,3,3029");
	item = stu_rtmp_amf_create_string(&key, val.data, val.len);
	stu_rtmp_amf_add_item_to_object(stu_rtmp_properties, item);

	stu_str_set(&key, "capabilities");
	item = stu_rtmp_amf_create_number(&key, 255);
	stu_rtmp_amf_add_item_to_object(stu_rtmp_properties, item);

	stu_str_set(&key, "mode");
	item = stu_rtmp_amf_create_number(&key, 1);
	stu_rtmp_amf_add_item_to_object(stu_rtmp_properties, item);

	// FMS version
	stu_rtmp_version = stu_rtmp_amf_create_ecma_array(NULL);

	stu_str_set(&key, "version");
	stu_str_set(&val, "FMS/5,0,3,3029");
	item = stu_rtmp_amf_create_string(&key, val.data, val.len);
	stu_rtmp_amf_add_item_to_object(stu_rtmp_version, item);

	// message
	if (stu_rtmp_message_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp message hash.");
		return STU_ERROR;
	}

	// filter
	if (stu_rtmp_filter_init_hash() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp filter hash.");
		return STU_ERROR;
	}

	// phase
	if (stu_rtmp_phase_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp phase.");
		return STU_ERROR;
	}

	if (stu_rtmp_phase_flv_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp phase flv.");
		return STU_ERROR;
	}

	// application
	if (stu_rtmp_application_init_hash() == STU_ERROR) {
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_listen(stu_fd_t evfd, uint16_t port) {
	stu_connection_t   *c;
	int                 optval;
	socklen_t           optlen;
	struct sockaddr_in  sa;

	optlen = sizeof(optval);

	stu_rtmpfd = stu_socket(AF_INET, SOCK_STREAM, 0);
	if (stu_rtmpfd == -1) {
		stu_log_error(stu_errno, "Failed to create rtmp server fd.");
		return STU_ERROR;
	}


	optval = 1;
	if (setsockopt(stu_rtmpfd, SOL_SOCKET, SO_REUSEADDR, (void *) &optval, optlen) == -1) {
		stu_log_error(stu_errno, "setsockopt(SO_REUSEADDR) failed while setting rtmp server fd.");
		return STU_ERROR;
	}

#if (STU_LINUX)

	if (setsockopt(stu_rtmpfd, SOL_SOCKET, SO_REUSEPORT, (void *) &optval, optlen) == -1) {
		stu_log_error(stu_errno, "setsockopt(SO_REUSEPORT) failed while setting rtmp server fd.");
		return STU_ERROR;
	}

# endif
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

	c->read->evfd = evfd;
	c->read->handler = stu_rtmp_handler;

	if (stu_event_add(c->read, STU_READ_EVENT, 0) == STU_ERROR) {
		stu_log_error(0, "Failed to add rtmp server event.");
		return STU_ERROR;
	}

	stu_memzero(&sa.sin_zero, 8);
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

	fd = accept(stu_rtmpfd, (struct sockaddr*) &sa, &socklen);
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

	c->read->evfd = stu_threads[stu_rtmp_thread_n].evfd;
	c->write->evfd = stu_threads[stu_rtmp_thread_n].evfd;
	c->recv = stu_os_io.recv;
	c->send = stu_os_io.send;

	c->read->handler = stu_rtmp_handshaker_read_handler;

	if (stu_event_add(c->read, STU_READ_EVENT, STU_CLEAR_EVENT) == STU_ERROR) {
		stu_log_error(0, "Failed to add rtmp handshaker read event.");
		return;
	}
}
