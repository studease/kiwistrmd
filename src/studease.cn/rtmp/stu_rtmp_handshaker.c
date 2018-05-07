/*
 * stu_rtmp_handshaker.c
 *
 *  Created on: 2018骞�1鏈�12鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_str_t  STU_RTMP_HANDSHAKE_COMPLETE = stu_string("complete");
stu_str_t  STU_RTMP_HANDSHAKE_ERROR    = stu_string("error");

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

static void         stu_rtmp_process_handshaker(stu_event_t *ev);
static ssize_t      stu_rtmp_read_handshaker(stu_rtmp_handshaker_t *h);

static stu_int32_t  stu_rtmp_validate_client(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t *scheme);
static stu_int32_t  stu_rtmp_validate_scheme(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t scheme);
static u_char      *stu_rtmp_fill_random_buffer(u_char *dst, size_t n);
static stu_int32_t  stu_rtmp_find_digest(u_char *src, stu_uint8_t scheme);
static stu_int32_t  stu_rtmp_find_challenge(u_char *src, stu_uint8_t scheme);
static u_char *     stu_rtmp_make_digest(u_char *dst, u_char *src, size_t len, const u_char *key, size_t size);


stu_int32_t
stu_rtmp_handshake(stu_connection_t *c, stu_addr_t *addr,
		stu_rtmp_handshaker_handler_pt complete, stu_rtmp_handshaker_handler_pt error) {
	stu_rtmp_handshaker_t *h;

	h = stu_rtmp_create_handshaker(c);
	if (h == NULL) {
		stu_log_error(0, "Failed to create rtmp handshaker: fd=%d.", c->fd);
		return STU_ERROR;
	}

	h->type = STU_RTMP_HANDSHAKER_TYPE_CLIENT;
	h->complete = complete;
	h->error = error;

	c->request = h;

	return stu_connect(c, addr);
}


void
stu_rtmp_handshaker_read_handler(stu_event_t *ev) {
	stu_connection_t      *c;
	stu_rtmp_handshaker_t *h;
	stu_int32_t            n;

	c = (stu_connection_t *) ev->data;

	//stu_mutex_lock(&c->lock);

	if (c->buffer.start == NULL) {
		c->buffer.start = (u_char *) stu_pcalloc(c->pool, STU_RTMP_HANDSHAKER_BUFFER_SIZE);
		c->buffer.pos = c->buffer.last = c->buffer.start;
		c->buffer.end = c->buffer.start + STU_RTMP_HANDSHAKER_BUFFER_SIZE;
		c->buffer.size = STU_RTMP_HANDSHAKER_BUFFER_SIZE;
	}

	n = c->recv(c, c->buffer.last, c->buffer.end - c->buffer.last);
	if (n == STU_AGAIN) {
		goto done;
	}

	if (n == STU_ERROR) {
		c->error = TRUE;
		goto failed;
	}

	if (n == 0) {
		stu_log_error(0, "rtmp remote peer prematurely closed connection.");
		c->close = TRUE;
		goto failed;
	}

	c->buffer.last += n;
	stu_log_debug(4, "recv: fd=%d, bytes=%d.", c->fd, n);

	if (stu_strncmp(c->buffer.start, STU_FLASH_POLICY_FILE_REQUEST.data, STU_FLASH_POLICY_FILE_REQUEST.len) == 0) {
		n = c->send(c, STU_FLASH_POLICY_FILE.data, STU_FLASH_POLICY_FILE.len);
		if (n == -1) {
			stu_log_error(stu_errno, "Failed to send policy file: fd=%d.", c->fd);
			goto failed;
		}

		stu_log_debug(4, "sent policy file: fd=%d, bytes=%d.", c->fd, n);

		goto done;
	}

	c->request = (void *) stu_rtmp_create_handshaker(c);
	if (c->request == NULL) {
		stu_log_error(0, "Failed to create rtmp handshaker: fd=%d.", c->fd);
		goto failed;
	}

	//ev->handler = stu_rtmp_process_handshaker;
	stu_rtmp_process_handshaker(ev);

	goto done;

failed:

	if (c->request) {
		h = c->request;
		if (h->type == STU_RTMP_HANDSHAKER_TYPE_CLIENT && h->error) {
			h->error(h);
		}
	}

	stu_connection_close(c);

done:

	//stu_mutex_unlock(&c->lock);

	return;
}

stu_rtmp_handshaker_t *
stu_rtmp_create_handshaker(stu_connection_t *c) {
	stu_rtmp_handshaker_t *h;

	if (c->request == NULL) {
		h = stu_pcalloc(c->pool, sizeof(stu_rtmp_handshaker_t));
		if (h == NULL) {
			stu_log_error(0, "Failed to create rtmp handshaker.");
			return NULL;
		}
	} else {
		h = c->request;
	}

	h->connection = c;

	return h;
}

static void
stu_rtmp_process_handshaker(stu_event_t *ev) {
	stu_connection_t      *c;
	stu_rtmp_handshaker_t *h;
	stu_int32_t            rc;
	ssize_t                n;

	c = ev->data;
	h = c->request;

	stu_log_debug(4, "rtmp process handshaker.");

	if (ev->timedout) {
		stu_log_error(STU_ETIMEDOUT, "Failed to process rtmp handshaker.");

		c->timedout = TRUE;
		stu_rtmp_finalize_handshaker(h, STU_ERROR);

		return;
	}

	rc = STU_DONE;

	for ( ;; ) {
		if (rc == STU_AGAIN) {
			n = stu_rtmp_read_handshaker(h);
			if (n == STU_AGAIN) {
				return;
			}

			if (n == STU_ERROR) {
				stu_log_error(0, "rtmp failed to read handshaker buffer.");
				stu_rtmp_finalize_handshaker(h, STU_ERROR);
				return;
			}
		}

		rc = stu_rtmp_parse_handshaker(h, &c->buffer);
		if (rc == STU_AGAIN) {
			stu_log_debug(4, "rtmp handshake parsing is still not complete.");
			continue;
		}

		stu_rtmp_finalize_handshaker(h, rc);

		return;
	}
}

static ssize_t
stu_rtmp_read_handshaker(stu_rtmp_handshaker_t *h) {
	stu_connection_t *c;
	ssize_t           n;

	c = h->connection;

	n = c->recv(c, c->buffer.last, c->buffer.end - c->buffer.last);
	if (n == STU_AGAIN) {
		return STU_AGAIN;
	}

	if (n == STU_ERROR) {
		c->error = TRUE;
		return STU_ERROR;
	}

	if (n == 0) {
		stu_log_error(0, "rtmp remote peer prematurely closed connection.");
		c->close = TRUE;
		return STU_ERROR;
	}

	c->buffer.last += n;
	stu_log_debug(4, "recv: fd=%d, bytes=%d.", c->fd, n);

	return n;
}

void
stu_rtmp_handshaker_write_handler(stu_event_t *ev) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[STU_RTMP_HANDSHAKER_C0C1_SIZE];
	stu_int32_t       n;

	c = (stu_connection_t *) ev->data;

	pos = tmp;
	stu_memzero(pos, STU_RTMP_HANDSHAKER_C0C1_SIZE);

	//stu_mutex_lock(&c->lock);

	stu_event_del(c->write, STU_WRITE_EVENT, 0);

	// C0
	*pos++ = STU_RTMP_VERSION_3;

	// C1 time
	*(stu_uint32_t *) pos = 0;
	pos += 4;

	// C1 zero
	*(stu_uint32_t *) pos = 0;
	pos += 4;

	// C1 random bytes
	stu_rtmp_fill_random_buffer(pos, STU_RTMP_HANDSHAKER_RANDOM_SIZE);

	// Send C0 & C1
	n = c->send(c, tmp, STU_RTMP_HANDSHAKER_C0C1_SIZE);
	if (n == -1) {
		stu_log_error(stu_errno, "Failed to send rtmp handshake packet C0 & C1.");
		stu_rtmp_close_handshaker(c->request);
	}

	//stu_mutex_unlock(&c->lock);
}

void
stu_rtmp_finalize_handshaker(stu_rtmp_handshaker_t *h, stu_int32_t rc) {
	stu_connection_t *c;
	u_char           *digest, *challenge;
	u_char            tmp[STU_RTMP_HANDSHAKER_BUFFER_SIZE];
	u_char            rnd[1504];
	stu_buf_t         buf, t;
	stu_int32_t       n, off;
	stu_uint8_t       scheme;

	c = h->connection;

	buf.start = buf.pos = buf.last = tmp;
	buf.end = buf.start + STU_RTMP_HANDSHAKER_BUFFER_SIZE;
	buf.size = STU_RTMP_HANDSHAKER_BUFFER_SIZE;

	t.start = t.pos = t.last = rnd;
	t.end = t.start + 1504;
	t.size = 1504;

	stu_log_debug(4, "rtmp finalize handshake: %d", rc);

	if (rc == STU_OK) {
		// S0
		*buf.last++ = STU_RTMP_VERSION_3;

		// S1
		buf.pos = buf.last;

		*(stu_uint32_t *) buf.last = 0;
		buf.last += 4;

		*(stu_uint32_t *) buf.last = h->zero ? STU_RTMP_HANDSHAKER_VERSION : 0;
		buf.last += 4;

		if (h->zero) {
			if (stu_rtmp_validate_client(h->start, &digest, &challenge, &scheme) == STU_ERROR) {
				stu_log_error(0, "Failed to validate client.");
				goto failed;
			}

			buf.last = stu_rtmp_fill_random_buffer(buf.last, 1496);
			off = stu_rtmp_find_digest(buf.pos, scheme);

			t.last = stu_memcpy(t.pos, buf.start, off);
			t.last = stu_memcpy(t.last, buf.start + off + STU_RTMP_HANDSHAKER_DIGEST_SIZE, 1472 - off);

			buf.last = stu_rtmp_make_digest(buf.last, t.pos, t.size, STU_RTMP_FMS_KEY, 36);
			if (buf.last == NULL) {
				stu_log_error(0, "Failed to make rtmp handshake digest.");
				goto failed;
			}
		} else {
			buf.last = stu_rtmp_fill_random_buffer(buf.last, STU_RTMP_HANDSHAKER_RANDOM_SIZE);
		}

		// S2
		buf.pos = buf.last;

		if (h->zero) {
			buf.last = stu_rtmp_fill_random_buffer(buf.last, 1504);

			t.last = stu_rtmp_make_digest(t.pos, digest, STU_RTMP_HANDSHAKER_DIGEST_SIZE, STU_RTMP_FMS_KEY, 68);
			buf.last = stu_rtmp_make_digest(buf.last, buf.pos, buf.last - buf.pos, (const u_char *) t.pos, t.last - t.pos);
		} else {
			buf.last = stu_memcpy(buf.last, h->start, STU_RTMP_HANDSHAKER_PACKET_SIZE);
		}

		// Send S0 & S1 & S2
		n = c->send(c, buf.start, buf.size);
		if (n == -1) {
			stu_log_error(stu_errno, "Failed to send rtmp handshake packet S0 & S1 & S2.");
			goto failed;
		}

		return;
	}

	if (rc == STU_DONE) {
		switch (h->type) {
		case STU_RTMP_HANDSHAKER_TYPE_SERVER:
			// TODO: Check C2
			stu_log_debug(4, "rtmp handshake done.");
			break;

		case STU_RTMP_HANDSHAKER_TYPE_CLIENT:
			// Send C2
			n = c->send(c, c->buffer.start + 1, STU_RTMP_HANDSHAKER_PACKET_SIZE);
			if (n == -1) {
				stu_log_error(stu_errno, "Failed to send rtmp handshake packet C2.");
				return;
			}
			break;

		default:
			stu_log_error(0, "Unknown rtmp handshaker type: %d", h->type);
		}

		c->buffer.start = c->buffer.end = NULL;
		c->buffer.pos = c->buffer.last = NULL;
		c->buffer.size = 0;

		stu_pool_reset(c->pool);
		stu_rtmp_free_handshaker(h);

		c->read->handler = stu_rtmp_request_read_handler;
		c->write->handler = NULL;

		if (h->type == STU_RTMP_HANDSHAKER_TYPE_CLIENT && h->complete) {
			h->complete(h);
		}

		return;
	}

failed:

	stu_rtmp_close_handshaker(h);
}

static stu_int32_t
stu_rtmp_validate_client(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t *scheme) {
	if (stu_rtmp_validate_scheme(c1, digest, challenge, STU_RTMP_HANDSHAKER_SCHEME1) == STU_OK) {
		*scheme = STU_RTMP_HANDSHAKER_SCHEME1;
		return STU_OK;
	}

	if (stu_rtmp_validate_scheme(c1, digest, challenge, STU_RTMP_HANDSHAKER_SCHEME2) == STU_OK) {
		*scheme = STU_RTMP_HANDSHAKER_SCHEME2;
		return STU_OK;
	}

	return STU_ERROR;
}

static stu_int32_t
stu_rtmp_validate_scheme(u_char *c1, u_char **digest, u_char **challenge, stu_uint8_t scheme) {
	u_char       tmp[1504];
	u_char       dgt[STU_RTMP_HANDSHAKER_DIGEST_SIZE];
	stu_buf_t    t;
	stu_int32_t  off;

	t.pos = t.last = tmp;

	off = stu_rtmp_find_digest(c1, scheme);
	if (off == STU_ERROR) {
		return STU_ERROR;
	}

	*digest = c1 + off;

	t.last = stu_memcpy(t.pos, c1, off);
	t.last = stu_memcpy(t.last, c1 + off + STU_RTMP_HANDSHAKER_DIGEST_SIZE, 1504 - off);

	stu_rtmp_make_digest(dgt, t.pos, t.last - t.pos, STU_RTMP_FP_KEY, 30);

	if (stu_strncmp(dgt, *digest, STU_RTMP_HANDSHAKER_DIGEST_SIZE) != 0) {
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
	if (off + STU_RTMP_HANDSHAKER_DIGEST_SIZE > STU_RTMP_HANDSHAKER_PACKET_SIZE) {
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

	if (off + STU_RTMP_HANDSHAKER_CHALLENGE_SIZE > STU_RTMP_HANDSHAKER_PACKET_SIZE) {
		stu_log_error(0, "rtmp challenge out of range: %d.", off);
		return STU_ERROR;
	}

	return off;
}

static u_char *
stu_rtmp_make_digest(u_char *dst, u_char *src, size_t len, const u_char *key, size_t size) {
	static stu_hmac_t *hmac;
	unsigned int       n;

	if (hmac == NULL) {
		hmac = stu_hmac_create();
	}

	stu_hmac_init(hmac, key, size, EVP_sha256(), NULL);
	stu_hmac_update(hmac, src, len);
	stu_hmac_final(hmac, dst, &n);

	return dst + n;
}


void
stu_rtmp_free_handshaker(stu_rtmp_handshaker_t *h) {
	h->connection->request = NULL;
}

void
stu_rtmp_close_handshaker(stu_rtmp_handshaker_t *h) {
	stu_connection_t *c;

	c = h->connection;

	stu_rtmp_free_handshaker(h);
	stu_connection_close(c);
}
