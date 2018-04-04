/*
 * stu_os.c
 *
 *  Created on: 2018Äê3ÔÂ30ÈÕ
 *      Author: Tony Lau
 */

#include "stu_os.h"

stu_os_io_t  stu_os_io = {
	stu_wsarecv,
	stu_udp_wsarecv,
	stu_wsasend,
	NULL,
	0
};

stu_uint32_t      stu_ncpu;
stu_uint32_t      stu_win32_version;
char              stu_unique[STU_INT32_LEN + 1];

extern stu_pid_t  stu_pid;


stu_int32_t
stu_os_init() {
	stu_err_t  err;

	if (GetEnvironmentVariable("stu_unique", stu_unique, STU_INT32_LEN + 1) == 0) {
		err = stu_errno;
		if (err != ERROR_ENVVAR_NOT_FOUND) {
			stu_log_error(err, "GetEnvironmentVariable(\"stu_unique\") failed");
			return STU_ERROR;
		}

		stu_sprintf((u_char *) stu_unique, "%d", stu_pid);
	}

	return STU_OK;
}


ssize_t
stu_wsarecv(stu_connection_t *c, u_char *buf, size_t size) {
	stu_event_t *rev;
	WSABUF       wsabuf[1];
	u_long       bytes, flags;
	stu_err_t    err;
	stu_int32_t  rc;

	rev = c->read;

	wsabuf[0].buf = (char *) buf;
	wsabuf[0].len = size;
	flags = 0;
	bytes = 0;

	rc = WSARecv(c->fd, wsabuf, 1, &bytes, &flags, NULL, NULL);
	stu_log_debug(4, "WSARecv: fd=%d, rc=%d, %ul of %z", c->fd, rc, bytes, size);

	if (rc == -1) {
		rev->ready = 0;

		err = stu_socket_errno;
		if (err == WSAEWOULDBLOCK) {
			stu_log_error(err, "WSARecv() not ready");
			return STU_AGAIN;
		}

		return STU_ERROR;
	}

	if (bytes < size) {
		rev->ready = 0;
	}

	if (bytes == 0) {
		rev->eof = 1;
	}

	return bytes;
}

ssize_t
stu_wsasend(stu_connection_t *c, u_char *buf, size_t size) {
	stu_event_t *wev;
	WSABUF       wsabuf;
	u_long       sent;
	stu_err_t    err;
	stu_int32_t  n;

	wev = c->write;

	if (!wev->ready) {
		return STU_AGAIN;
	}

	/*
	 * WSABUF must be 4-byte aligned otherwise
	 * WSASend() will return undocumented WSAEINVAL error.
	 */
	wsabuf.buf = (char *) buf;
	wsabuf.len = size;
	sent = 0;

	n = WSASend(c->fd, &wsabuf, 1, &sent, 0, NULL, NULL);
	stu_log_debug(4, "WSASend: fd:%d, %d, %ul of %uz", c->fd, n, sent, size);

	if (n == 0) {
		if (sent < size) {
			wev->ready = 0;
		}

		return sent;
	}

	err = stu_socket_errno;

	if (err == WSAEWOULDBLOCK) {
		stu_log_error(err, "WSASend() not ready");
		wev->ready = 0;
		return STU_AGAIN;
	}

	wev->error = 1;

	return STU_ERROR;
}


ssize_t
stu_udp_wsarecv(stu_connection_t *c, u_char *buf, size_t size) {
	stu_event_t *rev;
	WSABUF       wsabuf[1];
	u_long       bytes, flags;
	stu_err_t    err;
	stu_int32_t  rc;

	rev = c->read;

	wsabuf[0].buf = (char *) buf;
	wsabuf[0].len = size;
	flags = 0;
	bytes = 0;

	rc = WSARecv(c->fd, wsabuf, 1, &bytes, &flags, NULL, NULL);
	stu_log_debug(4, "WSARecv: fd:%d rc:%d %ul of %z", c->fd, rc, bytes, size);

	if (rc == -1) {
		rev->ready = 0;
		err = stu_socket_errno;

		if (err == WSAEWOULDBLOCK) {
			stu_log_error(err, "WSARecv() not ready");
			return STU_AGAIN;
		}

		rev->error = 1;

		return STU_ERROR;
	}

	return bytes;
}
