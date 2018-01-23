/*
 * stu_rtmp_handshake.c
 *
 *  Created on: 2018年1月12日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

extern stu_str_t  STU_FLASH_POLICY_FILE_REQUEST;
extern stu_str_t  STU_FLASH_POLICY_FILE;

static const u_char  STU_RTMP_FP_KEY[] = {
	0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
	0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
	0x61, 0x73, 0x68, 0x20, 0x50, 0x6C, 0x61, 0x79,
	0x65, 0x72, 0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Player 001
	0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
	0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
	0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
	0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};

static const u_char  STU_RTMP_FMS_KEY[] = {
	0x47, 0x65, 0x6E, 0x75, 0x69, 0x6E, 0x65, 0x20,
	0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x46, 0x6C,
	0x61, 0x73, 0x68, 0x20, 0x4D, 0x65, 0x64, 0x69,
	0x61, 0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72,
	0x20, 0x30, 0x30, 0x31, // Genuine Adobe Flash Media Server 001
	0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8,
	0x2E, 0x00, 0xD0, 0xD1, 0x02, 0x9E, 0x7E, 0x57,
	0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
	0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};

static void         stu_rtmp_process_handshake(stu_event_t *ev);
static ssize_t      stu_rtmp_read_handshake(stu_rtmp_handshake_t *h);

static stu_int32_t  stu_rtmp_validate_client(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t *scheme);
static stu_int32_t  stu_rtmp_validate_scheme(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t scheme);
static u_char      *stu_rtmp_fill_random_buffer(u_char *dst, size_t n);
static stu_int32_t  stu_rtmp_find_digest(u_char *src, stu_uint8_t scheme);
static stu_int32_t  stu_rtmp_find_challenge(u_char *src, stu_uint8_t scheme);
static u_char *     stu_rtmp_make_digest(u_char *dst, stu_buf_t *src, const u_char *key, size_t len);


void
stu_rtmp_handshake_read_handler(stu_event_t *ev) {
	stu_connection_t *c;
	stu_int32_t       n, err;

	c = (stu_connection_t *) ev->data;

	//stu_mutex_lock(&c->lock);

	if (c->buffer.start == NULL) {
		c->buffer.start = (u_char *) stu_pcalloc(c->pool, STU_RTMP_HANDSHAKE_BUFFER_SIZE);
		c->buffer.pos = c->buffer.last = c->buffer.start;
		c->buffer.end = c->buffer.start + STU_RTMP_HANDSHAKE_BUFFER_SIZE;
		c->buffer.size = STU_RTMP_HANDSHAKE_BUFFER_SIZE;
	}
	c->buffer.pos = c->buffer.last = c->buffer.start;
	stu_memzero(c->buffer.start, c->buffer.size);

again:

	n = recv(c->fd, c->buffer.last, c->buffer.size, 0);
	if (n == -1) {
		err = stu_errno;
		if (err == EINTR) {
			stu_log_debug(3, "recv trying again: fd=%d, errno=%d.", c->fd, err);
			goto again;
		}

		if (err == EAGAIN) {
			stu_log_debug(3, "no data received: fd=%d, errno=%d.", c->fd, err);
			goto done;
		}

		stu_log_error(err, "Failed to recv data: fd=%d.", c->fd);
		goto failed;
	}

	if (n == 0) {
		stu_log_debug(4, "rtmp client has closed connection: fd=%d.", c->fd);
		goto failed;
	}

	c->buffer.last += n;
	stu_log_debug(4, "recv: fd=%d, bytes=%d.", c->fd, n);

	if (stu_strncmp(c->buffer.start, STU_FLASH_POLICY_FILE_REQUEST.data, STU_FLASH_POLICY_FILE_REQUEST.len) == 0) {
		n = send(c->fd, STU_FLASH_POLICY_FILE.data, STU_FLASH_POLICY_FILE.len, 0);
		if (n == -1) {
			stu_log_debug(4, "Failed to send policy file: fd=%d.", c->fd);
			goto failed;
		}

		stu_log_debug(4, "sent policy file: fd=%d, bytes=%d.", c->fd, n);

		goto done;
	}

	c->request = (void *) stu_rtmp_create_handshake(c);
	if (c->request == NULL) {
		stu_log_error(0, "Failed to create rtmp handshake.");
		goto failed;
	}

	//ev->handler = stu_rtmp_process_handshake;
	stu_rtmp_process_handshake(ev);

	goto done;

failed:

	stu_rtmp_close_connection(c);

done:

	stu_log_debug(4, "rtmp handshake done: state=%d.", ((stu_rtmp_handshake_t *) c->request)->state);

	//stu_mutex_unlock(&c->lock);
}

stu_rtmp_handshake_t *
stu_rtmp_create_handshake(stu_connection_t *c) {
	stu_rtmp_handshake_t *h;

	if (c->request == NULL) {
		h = stu_pcalloc(c->pool, sizeof(stu_rtmp_handshake_t));
		if (h == NULL) {
			stu_log_error(0, "Failed to create rtmp handshake.");
			return NULL;
		}
	} else {
		h = c->request;
	}

	h->connection = c;

	return h;
}

static void
stu_rtmp_process_handshake(stu_event_t *ev) {
	stu_connection_t      *c;
	stu_rtmp_handshake_t *h;
	stu_int32_t            rc;
	ssize_t                n;

	c = ev->data;
	h = c->request;

	stu_log_debug(4, "rtmp process handshake.");

	if (ev->timedout) {
		stu_log_error(STU_ETIMEDOUT, "Failed to process rtmp handshake.");

		c->timedout = TRUE;
		stu_rtmp_finalize_handshake(h, STU_ERROR);

		return;
	}

	rc = STU_DONE;

	for ( ;; ) {
		if (rc == STU_AGAIN) {
			n = stu_rtmp_read_handshake(h);
			if (n == STU_AGAIN || n == STU_ERROR) {
				stu_log_error(0, "rtmp failed to read handshake buffer.");
				stu_rtmp_finalize_handshake(h, STU_ERROR);
				return;
			}
		}

		rc = stu_rtmp_parse_handshake(h, &c->buffer);
		if (rc == STU_AGAIN) {
			/* a rtmp handshake is still not complete */
			continue;
		}

		stu_rtmp_finalize_handshake(h, rc);

		return;
	}
}

static ssize_t
stu_rtmp_read_handshake(stu_rtmp_handshake_t *h) {
	stu_connection_t *c;
	ssize_t           n;
	stu_int32_t       err;

	c = h->connection;

	n = c->buffer.last - c->buffer.pos;
	if (n > 0) {
		/* buffer remains */
		return n;
	}

again:

	n = recv(c->fd, c->buffer.last, c->buffer.end - c->buffer.last, 0);
	if (n == -1) {
		err = stu_errno;
		if (err == EINTR) {
			stu_log_debug(4, "recv trying again: fd=%d, errno=%d.", c->fd, err);
			goto again;
		}

		if (err == EAGAIN) {
			stu_log_debug(4, "no data received: fd=%d, errno=%d.", c->fd, err);
		}
	}

	if (n == 0) {
		c->close = TRUE;
		stu_log_error(0, "rtmp client prematurely closed connection.");
	}

	if (n == 0 || n == STU_ERROR) {
		c->error = TRUE;
		return STU_ERROR;
	}

	c->buffer.last += n;

	return n;
}

void
stu_rtmp_finalize_handshake(stu_rtmp_handshake_t *h, stu_int32_t rc) {
	stu_connection_t *c;
	u_char           *digest, *challenge;
	u_char            buf[3073];
	u_char            tmp[1504];
	stu_buf_t         b, t, d;
	stu_int32_t       n, off;
	stu_uint8_t       scheme;

	c = h->connection;

	b.start = b.pos = b.last = buf;
	b.end = b.start + 3073;
	b.size = 3073;

	t.pos = t.last = tmp;

	stu_memzero(b.start, b.size);

	stu_log_debug(4, "rtmp finalize handshake: %d", rc);

	if (rc == STU_OK) {
		if (stu_rtmp_validate_client(h->start, &digest, &challenge, &scheme) == STU_ERROR) {
			stu_log_error(0, "Failed to validate client.");
			goto failed;
		}

		// s0
		*b.last++ = STU_RTMP_VERSION_3;

		// s1
		b.pos = b.last;

		*(stu_uint32_t *) b.last = 0;
		b.last += 4;

		*(stu_uint32_t *) b.last = h->zero ? STU_RTMP_HANDSHAKE_VERSION : 0;
		b.last += 4;

		if (h->zero) {
			b.last = stu_rtmp_fill_random_buffer(b.last, 1496);
			off = stu_rtmp_find_digest(b.pos, scheme);

			t.last = stu_memcpy(t.pos, b.start, off);
			t.last = stu_memcpy(t.last, b.start + off + STU_RTMP_HANDSHAKE_DIGEST_SIZE, 1504 - off);

			b.last = stu_rtmp_make_digest(b.last, &t, STU_RTMP_FMS_KEY, 36);
			if (b.last == NULL) {
				stu_log_error(0, "Failed to make rtmp handshake digest.");
				goto failed;
			}
		} else {
			b.last = stu_rtmp_fill_random_buffer(b.last, STU_RTMP_HANDSHAKE_RANDOM_SIZE);
		}

		// s2
		b.pos = b.last;

		if (h->zero) {
			b.last = stu_rtmp_fill_random_buffer(b.last, 1504);

			d.pos = d.last = digest;
			d.last += STU_RTMP_HANDSHAKE_DIGEST_SIZE;

			t.last = stu_rtmp_make_digest(t.pos, &d, STU_RTMP_FMS_KEY, 68);
			b.last = stu_rtmp_make_digest(b.last, &b, (const u_char *) t.pos, t.last - t.pos);
		} else {
			b.last = stu_memcpy(b.last, h->start, STU_RTMP_HANDSHAKE_PACKET_SIZE);
		}

		// send s0 & s1 & s2
		n = send(c->fd, b.start, b.size, 0);
		if (n == -1) {
			stu_log_error(0, "Failed to send rtmp handshake packet c0 & c1.");
			goto failed;
		}

		return;
	}

	if (rc == STU_DONE) {
		// TODO: check c2

		c->buffer.start = c->buffer.end = NULL;
		c->buffer.pos = c->buffer.last = NULL;
		c->buffer.size = 0;

		stu_pool_reset(c->pool);
		stu_rtmp_free_handshake(h);

		c->read.handler = stu_rtmp_request_read_handler;

		return;
	}

failed:

	stu_rtmp_close_handshake(h);
}

static stu_int32_t
stu_rtmp_validate_client(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t *scheme) {
	if (stu_rtmp_validate_scheme(c1, digest, challenge, STU_RTMP_HANDSHAKE_SCHEME1) == STU_OK) {
		*scheme = STU_RTMP_HANDSHAKE_SCHEME1;
		return STU_OK;
	}

	if (stu_rtmp_validate_scheme(c1, digest, challenge, STU_RTMP_HANDSHAKE_SCHEME2) == STU_OK) {
		*scheme = STU_RTMP_HANDSHAKE_SCHEME2;
		return STU_OK;
	}

	return STU_ERROR;
}

static stu_int32_t
stu_rtmp_validate_scheme(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t scheme) {
	u_char       tmp[1504];
	u_char       dgt[STU_RTMP_HANDSHAKE_DIGEST_SIZE];
	stu_buf_t    t;
	stu_int32_t  off;

	t.pos = t.last = tmp;

	off = stu_rtmp_find_digest(c1, scheme);
	if (off == STU_ERROR) {
		return STU_ERROR;
	}

	*digest = c1 + off;

	t.last = stu_memcpy(t.pos, c1, off);
	t.last = stu_memcpy(t.last, c1 + off + STU_RTMP_HANDSHAKE_DIGEST_SIZE, 1504 - off);

	stu_rtmp_make_digest(dgt, &t, STU_RTMP_FP_KEY, 30);

	if (stu_strncmp(dgt, *digest, STU_RTMP_HANDSHAKE_DIGEST_SIZE) != 0) {
		return STU_ERROR;
	}

	off = stu_rtmp_find_challenge(c1, scheme);
	if (off == STU_ERROR) {
		return STU_ERROR;
	}

	*challenge = c1 + off;

	return STU_OK;
}

static u_char *
stu_rtmp_fill_random_buffer(u_char *dst, size_t n) {
	u_char *p;

	for (p = dst; n; n--) {
		*p++ = (u_char) rand();
	}

	return p;
}

static stu_int32_t
stu_rtmp_find_digest(u_char *src, stu_uint8_t scheme) {
	stu_int32_t  n, i, off;

	n = scheme ? 8 : 772;

	for (i = 0, off = 0; i < 4; i++) {
		off += src[n + i];
	}

	off = n + 4 + (off % 728);
	if (off + STU_RTMP_HANDSHAKE_DIGEST_SIZE > STU_RTMP_HANDSHAKE_PACKET_SIZE) {
		stu_log_error(0, "rtmp digest out of range: %d.", off);
		return STU_ERROR;
	}

	return off;
}

static stu_int32_t
stu_rtmp_find_challenge(u_char *src, stu_uint8_t scheme) {
	stu_int32_t  n, i, off;

	n = scheme ? 1534 : 768;


	for (i = 0, off = 0; i < 4; i++) {
		off += src[n + i];
	}

	off = 8 + (off % 632);
	if (scheme) {
		off += 764;
	}

	if (off + STU_RTMP_HANDSHAKE_CHALLENGE_SIZE > STU_RTMP_HANDSHAKE_PACKET_SIZE) {
		stu_log_error(0, "rtmp challenge out of range: %d.", off);
		return STU_ERROR;
	}

	return off;
}

static u_char *
stu_rtmp_make_digest(u_char *dst, stu_buf_t *src, const u_char *key, size_t len) {
	static stu_hmac_t *hmac;
	unsigned int       n;

	if (hmac == NULL) {
		hmac = stu_hmac_create();
	}

	stu_hmac_init(hmac, key, 36, EVP_sha256(), NULL);
	stu_hmac_update(hmac, src->pos, src->last - src->pos);
	stu_hmac_final(hmac, dst, &n);

	return dst + n;
}


void
stu_rtmp_free_handshake(stu_rtmp_handshake_t *h) {
	h->connection->request = NULL;
}

void
stu_rtmp_close_handshake(stu_rtmp_handshake_t *h) {
	stu_connection_t *c;

	c = h->connection;

	stu_rtmp_free_handshake(h);
	stu_rtmp_close_connection(c);
}
