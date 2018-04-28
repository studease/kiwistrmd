/*
 * stu_connection.c
 *
 *  Created on: 2017骞�11鏈�16鏃�
 *      Author: Tony Lau
 */

#include "../stu_config.h"
#include "stu_core.h"

static void  stu_connection_free(stu_connection_t *c);


stu_int32_t
stu_connect(stu_socket_t s, stu_addr_t *addr) {
	stu_int32_t  rc;
	stu_err_t    err;

	rc = connect(s, (struct sockaddr *) &addr->sockaddr, addr->socklen);
	if (rc == -1) {
		err = stu_errno;
		if (err != STU_EINPROGRESS
#if (STU_WIN32)
				/* Winsock returns WSAEWOULDBLOCK (STU_EAGAIN) */
				&& err != STU_EAGAIN
#endif
				) {
			if (err == STU_ECONNREFUSED
#if (STU_LINUX)
					/*
					 * Linux returns EAGAIN instead of ECONNREFUSED
					 * for unix sockets if listen queue is full
					 */
					|| err == STU_EAGAIN
#endif
					|| err == STU_ECONNRESET
					|| err == STU_ENETDOWN
					|| err == STU_ENETUNREACH
					|| err == STU_EHOSTDOWN
					|| err == STU_EHOSTUNREACH) {
				// ERR
			} else {
				// CRIT
			}

			stu_log_error(err, "connect() to %s failed: fd=%d.", addr->name.data, s);

			return STU_ERROR;
		}
	}

	return STU_OK;
}

stu_connection_t *
stu_connection_get(stu_socket_t s) {
	stu_connection_t *c;

	c = stu_calloc(sizeof(stu_connection_t) + 2 * sizeof(stu_event_t));
	if (c == NULL) {
		return NULL;
	}

	c->pool = stu_pool_create(STU_CONNECTION_POOL_DEFAULT_SIZE);
	if (c->pool == NULL) {
		stu_free(c);
		return NULL;
	}

	c->read = (stu_event_t *) ((u_char *) c + sizeof(stu_connection_t));
	c->write = (stu_event_t *) ((u_char *) c->read + sizeof(stu_event_t));

	c->fd = s;
	c->read->data = (void *) c;
	c->write->data = (void *) c;
	c->idle = TRUE;

	stu_log_debug(3, "got connection: c=%p, fd=%d.", c, c->fd);

	return c;
}

void
stu_connection_close(stu_connection_t *c) {
	stu_socket_t  fd;

	if (c->fd == (stu_socket_t) STU_SOCKET_INVALID) {
		stu_log_error(0, "connection already closed.");
		return;
	}
/*
	if (c->read->timer_set) {
		stu_del_timer(c->read);
	}

	if (c->write->timer_set) {
		stu_del_timer(c->write);
	}
*/
	stu_event_del(c->read, STU_READ_EVENT, STU_CLOSE_EVENT);
	stu_event_del(c->write, STU_WRITE_EVENT, STU_CLOSE_EVENT);

	fd = c->fd;

	c->fd = (stu_socket_t) -1;
	c->close = TRUE;
	stu_socket_close(fd);

	stu_log_debug(3, "freed connection: c=%p, fd=%d.", c, fd);
	stu_connection_free(c);
}


static void
stu_connection_free(stu_connection_t *c) {
	stu_upstream_cleanup(c);
	c->destroyed = TRUE;

	stu_pool_destroy(c->pool);
	stu_free((void *) c);
}
