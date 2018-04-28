/*
 * stu_os.c
 *
 *  Created on: 2018Äê3ÔÂ30ÈÕ
 *      Author: Tony Lau
 */

#include "stu_os.h"

stu_uint32_t      stu_ncpu;
stu_uint32_t      stu_win32_version;
char              stu_unique[STU_INT32_LEN + 1];

extern stu_pid_t  stu_pid;

stu_os_io_t       stu_os_io = {
	stu_wsarecv,
	stu_udp_wsarecv,
	stu_wsasend,
	NULL,
	0
};


stu_int32_t
stu_os_init() {
	WSADATA    wsd;
	stu_err_t  err;

	/* init Winsock */
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0) {
		stu_log_error(stu_socket_errno, "WSAStartup() failed.");
		return STU_ERROR;
	}

	if (GetEnvironmentVariable("stu_unique", stu_unique, STU_INT32_LEN + 1) == 0) {
		err = stu_errno;
		if (err != ERROR_ENVVAR_NOT_FOUND) {
			stu_log_error(err, "GetEnvironmentVariable(\"stu_unique\") failed.");
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

	stu_log_debug(3, "WSARecv: fd=%d, rc=%d, %u of %z.", c->fd, rc, bytes, size);

	if (rc == -1) {
		rev->ready = 0;

		err = stu_socket_errno;
		if (err == WSAEWOULDBLOCK) {
			stu_log_error(err, "WSARecv() not ready.");
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
	u_long       bytes;
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
	bytes = 0;

	n = WSASend(c->fd, &wsabuf, 1, &bytes, 0, NULL, NULL);

	stu_log_debug(4, "WSASend: fd:%d, %d, %u of %z.", c->fd, n, bytes, size);

	if (n == 0) {
		if (bytes < size) {
			wev->ready = 0;
		}

		return bytes;
	}

	err = stu_socket_errno;

	if (err == WSAEWOULDBLOCK) {
		stu_log_error(err, "WSASend() not ready.");
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

	stu_log_debug(4, "WSARecv: fd:%d rc:%d %ul of %z.", c->fd, rc, bytes, size);

	if (rc == -1) {
		rev->ready = 0;
		err = stu_socket_errno;

		if (err == WSAEWOULDBLOCK) {
			stu_log_error(err, "WSARecv() not ready.");
			return STU_AGAIN;
		}

		rev->error = 1;

		return STU_ERROR;
	}

	return bytes;
}


ssize_t
stu_overlapped_wsarecv(stu_connection_t *c, u_char *buf, size_t size) {
	stu_event_t     *rev;
	WSABUF           wsabuf[1];
	LPWSAOVERLAPPED  ovlp;
	u_long           bytes, flags;
	stu_err_t        err;
	stu_int32_t      n;

	rev = c->read;

	if (!rev->ready) {
		stu_log_error(0, "second wsa post.");
		return STU_AGAIN;
	}

	stu_log_debug(3, "rev->complete: %d.", rev->complete);

	if (rev->complete) {
		rev->complete = 0;

		if (rev->ovlp.error) {
			stu_log_error(rev->ovlp.error, "WSARecv() failed.");
			return STU_ERROR;
		}

		stu_log_debug(3, "WSARecv ovlp: fd=%d, %u of %z.", c->fd, rev->available, size);

		return rev->available;
	}

	ovlp = (LPWSAOVERLAPPED) &rev->ovlp;
	stu_memzero(ovlp, sizeof(WSAOVERLAPPED));
	wsabuf[0].buf = (char *) buf;
	wsabuf[0].len = size;
	bytes = 0;
	flags = 0;

	n = WSARecv(c->fd, wsabuf, 1, &bytes, &flags, ovlp, NULL);

	rev->complete = 0;

	stu_log_debug(3, "WSARecv ovlp: fd=%d, n=%d, %u of %z.", c->fd, n, bytes, size);

	if (n == -1) {
		err = stu_socket_errno;
		if (err == WSA_IO_PENDING) {
			rev->active = 1;
			stu_log_error(err, "WSARecv() posted.");
			return STU_AGAIN;
		}

		stu_log_error(err, "WSARecv() failed.");

		rev->error = 1;
		return STU_ERROR;
	}

	/*
	 * if a socket was bound with I/O completion port
	 * then GetQueuedCompletionStatus() would anyway return its status
	 * despite that WSARecv() was already complete
	 */
	rev->active = 1;

	return STU_AGAIN;
}

ssize_t
stu_overlapped_wsasend(stu_connection_t *c, u_char *buf, size_t size) {
	stu_event_t     *wev;
	WSABUF           wsabuf;
	LPWSAOVERLAPPED  ovlp;
	u_long           bytes;
	stu_err_t        err;
	stu_int32_t      n;

	wev = c->write;

	if (!wev->ready) {
		return STU_AGAIN;
	}

	stu_log_debug(3, "wev->complete: %d.", wev->complete);

	if (!wev->complete) {
		/* post the overlapped WSASend() */

		/*
		 * WSABUFs must be 4-byte aligned otherwise
		 * WSASend() will return undocumented WSAEINVAL error.
		 */
		wsabuf.buf = (char *) buf;
		wsabuf.len = size;

		bytes = 0;

		ovlp = (LPWSAOVERLAPPED) &c->write->ovlp;
		stu_memzero(ovlp, sizeof(WSAOVERLAPPED));

		n = WSASend(c->fd, &wsabuf, 1, &bytes, 0, ovlp, NULL);

		stu_log_debug(3, "WSASend: fd:%d, n=%d, %u of %z.", c->fd, n, bytes, size);

		wev->complete = 0;

		if (n == 0) {
			/*
			 * if a socket was bound with I/O completion port then
			 * GetQueuedCompletionStatus() would anyway return its status
			 * despite that WSASend() was already complete
			 */
			wev->active = 1;
			return STU_AGAIN;
		}

		err = stu_socket_errno;
		if (err == WSA_IO_PENDING) {
			stu_log_error(err, "WSASend() posted.");
			wev->active = 1;
			return STU_AGAIN;
		}

		stu_log_error(err, "WSASend() failed.");

		wev->error = 1;
		return STU_ERROR;
	}

	/* the overlapped WSASend() complete */
	wev->complete = 0;
	wev->active = 0;

	if (wev->ovlp.error) {
		stu_log_error(wev->ovlp.error, "WSASend() failed.");
		return STU_ERROR;
	}

	bytes = wev->available;
	if (bytes < size) {
		wev->ready = 0;
	}

	stu_log_debug(3, "WSAGetOverlappedResult: fd=%d, %u of %z.", c->fd, bytes, size);

	return bytes;
}

ssize_t
stu_udp_overlapped_wsarecv(stu_connection_t *c, u_char *buf, size_t size) {
	stu_event_t      *rev;
	WSABUF            wsabuf[1];
	LPWSAOVERLAPPED   ovlp;
	u_long            bytes, flags;
	stu_err_t         err;
	stu_int32_t       n;

	rev = c->read;

	if (!rev->ready) {
		stu_log_error(0, "second wsa post.");
		return STU_AGAIN;
	}

	stu_log_debug(3, "rev->complete: %d.", rev->complete);

	if (rev->complete) {
		rev->complete = 0;

		if (rev->ovlp.error) {
			stu_log_error(rev->ovlp.error, "WSARecv() failed.");
			return STU_ERROR;
		}

		stu_log_debug(3, "WSARecv ovlp: fd=%d, %u of %z.", c->fd, rev->available, size);

		return rev->available;
	}

	ovlp = (LPWSAOVERLAPPED) &rev->ovlp;
	stu_memzero(ovlp, sizeof(WSAOVERLAPPED));
	wsabuf[0].buf = (char *) buf;
	wsabuf[0].len = size;
	flags = 0;
	bytes = 0;

	n = WSARecv(c->fd, wsabuf, 1, &bytes, &flags, ovlp, NULL);

	rev->complete = 0;

	stu_log_debug(3, "WSARecv ovlp: fd=%d, n=%d, %u of %z.", c->fd, n, bytes, size);

	if (n == -1) {
		err = stu_socket_errno;
		if (err == WSA_IO_PENDING) {
			rev->active = 1;
			stu_log_error(err, "WSARecv() posted.");
			return STU_AGAIN;
		}

		stu_log_error(err, "WSARecv() failed.");

		rev->error = 1;
		return STU_ERROR;
	}

	/*
	 * if a socket was bound with I/O completion port
	 * then GetQueuedCompletionStatus() would anyway return its status
	 * despite that WSARecv() was already complete
	 */
	rev->active = 1;

	return STU_AGAIN;
}
