/*
 * ksd_request.c
 *
 *  Created on: 2018年1月30日
 *      Author: Tony Lau
 */

#include "ksd_core.h"

static stu_int32_t  ksd_filter_handler(stu_websocket_request_t *r);
static stu_int32_t  ksd_phase_upgrade_handler(stu_http_request_t *r);

static stu_int32_t  ksd_upgrade_preview_handler(stu_http_request_t *r);
static stu_int32_t  ksd_upgrade_enterprise_handler(stu_http_request_t *r);

static stu_int32_t  ksd_request_phase_handler(stu_websocket_request_t *r);
static void         ksd_request_analyze_protocol(stu_websocket_request_t *r);

static void         ksd_push_stat_handler(stu_event_t *ev);
static stu_int32_t  ksd_push_stat_generate_request(stu_connection_t *pc);
static stu_int32_t  ksd_push_stat_analyze_response(stu_connection_t *pc);
static void         ksd_push_stat_finalize_handler(stu_connection_t *c, stu_int32_t rc);

extern volatile ksd_cycle_t   *ksd_cycle;
extern stu_fd_t                ksd_epfd;
//extern stu_hash_t              stu_rtmp_applications;
extern stu_list_t              stu_http_phases;
extern stu_list_t              stu_websocket_phases;

static stu_connection_t       *ksd_timer_push_stat;
static stu_str_t               KSD_UPSTREAM_STAT = stu_string("stat");

static stu_websocket_filter_t  ksd_filter = {
	stu_string("/"), ksd_filter_handler
};

static stu_http_phase_t        ksd_phase_upgrader = {
	ksd_phase_upgrade_handler
};

static stu_websocket_phase_t   ksd_phase_responder = {
	ksd_request_phase_handler
};

stu_int32_t
ksd_request_init() {
	stu_list_elt_t *e;

	if (stu_websocket_filter_add(&ksd_filter.pattern, ksd_filter.handler) == STU_ERROR) {
		stu_log_error(0, "Failed to add ksd filter.");
		return STU_ERROR;
	}

	e = stu_list_insert_tail(&stu_http_phases, &ksd_phase_upgrader);
	if (e == NULL) {
		stu_log_error(0, "Failed to insert ksd upgrader phase.");
		return STU_ERROR;
	}

	e = stu_list_insert_tail(&stu_websocket_phases, &ksd_phase_responder);
	if (e == NULL) {
		stu_log_error(0, "Failed to insert ksd responder phase.");
		return STU_ERROR;
	}

	return STU_OK;
}


static stu_int32_t
ksd_filter_handler(stu_websocket_request_t *r) {
	r->status = STU_DECLINED;
	return STU_OK;
}

static stu_int32_t
ksd_phase_upgrade_handler(stu_http_request_t *r) {
	stu_int32_t  rc;

	rc = STU_ERROR;

	/* upgrade */
	switch (ksd_cycle->conf.edition) {
	case PREVIEW:
		rc = ksd_upgrade_preview_handler(r);
		break;

	case ENTERPRISE:
		rc = ksd_upgrade_enterprise_handler(r);
		break;

	default:
		stu_log_error(0, "unknown edition: %d.", ksd_cycle->conf.edition);
		break;
	}

	return rc;
}


static stu_int32_t
ksd_upgrade_preview_handler(stu_http_request_t *r) {
	return STU_OK;
}

static stu_int32_t
ksd_upgrade_enterprise_handler(stu_http_request_t *r) {
	return STU_OK;
}


static stu_int32_t
ksd_request_phase_handler(stu_websocket_request_t *r) {
	stu_connection_t *c;

	c = r->connection;

	switch (r->frames_in.opcode) {
	case STU_WEBSOCKET_OPCODE_TEXT:
	case STU_WEBSOCKET_OPCODE_BINARY:
		ksd_request_analyze_protocol(r);
		break;

	case STU_WEBSOCKET_OPCODE_CLOSE:
		stu_log_debug(5, "close frame.");
		ksd_close_connection(c);
		break;

	case STU_WEBSOCKET_OPCODE_PING:
		stu_log_debug(3, "ping frame.");
		break;

	case STU_WEBSOCKET_OPCODE_PONG:
		stu_log_debug(3, "pong frame.");
		break;

	default:
		break;
	}

	return STU_OK;
}

static void
ksd_request_analyze_protocol(stu_websocket_request_t *r) {

}


void
ksd_request_read_handler(stu_event_t *ev) {

}


void
ksd_free_request(stu_websocket_request_t *r) {
	stu_websocket_free_request(r);
}

void
ksd_close_request(stu_websocket_request_t *r) {
	stu_connection_t *c;

	c = r->connection;

	ksd_free_request(r);
	ksd_close_connection(c);
}

void
ksd_close_connection(stu_connection_t *c) {

}


stu_int32_t
ksd_add_push_stat_timer(stu_msec_t timer) {
	stu_connection_t *c;

	c = ksd_timer_push_stat;

	if (c == NULL) {
		c = stu_connection_get((stu_socket_t) STU_SOCKET_INVALID);
		if (c == NULL) {
			stu_log_error(0, "Failed to get connection for pushing stat.");
			return STU_ERROR;
		}

		c->read.epfd = ksd_epfd;
		c->write.epfd = ksd_epfd;

		c->write.handler = ksd_push_stat_handler;

		ksd_timer_push_stat = c;
	}

	stu_timer_add_locked(&c->write, timer);

	return STU_OK;
}

static void
ksd_push_stat_handler(stu_event_t *ev) {
	stu_connection_t *c;

	c = ksd_timer_push_stat;

	if (stu_upstream_create(c, KSD_UPSTREAM_STAT.data, KSD_UPSTREAM_STAT.len) == STU_ERROR) {
		stu_log_error(0, "Failed to create http upstream \"%s\".", KSD_UPSTREAM_STAT.data);
		return;
	}

	c->upstream->read_event_handler = stu_http_upstream_read_handler;
	c->upstream->write_event_handler = stu_http_upstream_write_handler;

	c->upstream->create_request_pt = stu_http_upstream_create_request;
	c->upstream->reinit_request_pt = stu_http_upstream_reinit_request;
	c->upstream->generate_request_pt = ksd_push_stat_generate_request;
	c->upstream->process_response_pt = stu_http_upstream_process_response;
	c->upstream->analyze_response_pt = ksd_push_stat_analyze_response;
	c->upstream->finalize_handler_pt = ksd_push_stat_finalize_handler;
	c->upstream->cleanup_pt = stu_http_upstream_cleanup;

	if (stu_upstream_connect(c->upstream->peer) != STU_OK) {
		stu_log_error(0, "Failed to connect http upstream \"%s\".", KSD_UPSTREAM_STAT.data);
	}
}

static stu_int32_t
ksd_push_stat_generate_request(stu_connection_t *pc) {
	return STU_OK;
}

static stu_int32_t
ksd_push_stat_analyze_response(stu_connection_t *pc) {
	stu_http_request_t *pr;
	stu_upstream_t     *u;

	pr = (stu_http_request_t *) pc->request;
	u = pc->upstream;

	if (pr->headers_out.status != STU_HTTP_OK) {
		stu_log_error(0, "Bad push stat response: status=%d.", pr->headers_out.status);
		return STU_ERROR;
	}

	stu_log_debug(4, "ksd push stat done.");

	u->finalize_handler_pt(u->connection, pr->headers_out.status);

	return STU_OK;
}

static void
ksd_push_stat_finalize_handler(stu_connection_t *c, stu_int32_t rc) {
	c->upstream->cleanup_pt(c);

	stu_timer_add_locked(&c->write, ksd_cycle->conf.push_stat_interval);
}
