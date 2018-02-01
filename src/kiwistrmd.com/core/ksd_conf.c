/*
 * ksd_conf.c
 *
 *  Created on: 2018年1月29日
 *      Author: Tony Lau
 */

#include "ksd_core.h"

stu_str_t  KSD_CONF_DEFAULT_PATH = stu_string("conf/strmd.conf");

extern stu_uint8_t  STU_DEBUG;
extern stu_hash_t   stu_upstreams;
extern stu_str_t                  stu_http_root;
extern stu_str_t                  stu_rtmp_root;
extern stu_rtmp_method_bitmask_t  stu_rtmp_upstream_method_mask[];

static ksd_edition_mask_t  ksd_edition_mask[] = {
	{ stu_string("preview"), PREVIEW },
	{ stu_string("enterprise"), ENTERPRISE },
	{ stu_null_string, 0x00 }
};

static ksd_mode_mask_t  ksd_mode_mask[] = {
	{ stu_string("smooth"), STU_MQ_MODE_SMOOTH },
	{ stu_string("strict"), STU_MQ_MODE_STRICT },
	{ stu_null_string, 0x00 }
};

static stu_str_t  KSD_CONF_DEFAULT_PID     = stu_string("strmd.pid");
//static stu_str_t  KSD_CONF_DEFAULT_RECORDS = stu_string("%s/%s/%s/stream/%s"); // applications/live/_definst_/stream/xxx.xxx

static stu_str_t  KSD_CONF_LOG = stu_string("log");
static stu_str_t  KSD_CONF_PID = stu_string("pid");

static stu_str_t  KSD_CONF_EDITION = stu_string("edition");
static stu_str_t  KSD_CONF_MODE    = stu_string("mode");

static stu_str_t  KSD_CONF_MASTER_PROCESS   = stu_string("master_process");
static stu_str_t  KSD_CONF_WORKER_PROCESSES = stu_string("worker_processes");
static stu_str_t  KSD_CONF_WORKER_THREADS   = stu_string("worker_threads");
static stu_str_t  KSD_CONF_DEBUG            = stu_string("debug");

static stu_str_t  KSD_CONF_SERVER                    = stu_string("server");
static stu_str_t  KSD_CONF_SERVER_HTTP               = stu_string("http");
static stu_str_t  KSD_CONF_SERVER_LISTEN             = stu_string("listen");
static stu_str_t  KSD_CONF_SERVER_ROOT               = stu_string("root");
static stu_str_t  KSD_CONF_SERVER_CORS               = stu_string("cors");
static stu_str_t  KSD_CONF_SERVER_PUSH_STAT          = stu_string("push_stat");
static stu_str_t  KSD_CONF_SERVER_PUSH_STAT_INTERVAL = stu_string("push_stat_interval");

static stu_str_t  KSD_CONF_TARGET           = stu_string("target");
static stu_str_t  KSD_CONF_TARGET_PROTOCOL  = stu_string("protocol");
static stu_str_t  KSD_CONF_TARGET_METHOD    = stu_string("method");
static stu_str_t  KSD_CONF_TARGET_NAME      = stu_string("name");
static stu_str_t  KSD_CONF_TARGET_DST_ADDR  = stu_string("dst_addr");
static stu_str_t  KSD_CONF_TARGET_DST_PORT  = stu_string("dst_port");
static stu_str_t  KSD_CONF_TARGET_DST_APP   = stu_string("dst_app");
static stu_str_t  KSD_CONF_TARGET_DST_INST  = stu_string("dst_inst");
static stu_str_t  KSD_CONF_TARGET_DST_NAME  = stu_string("dst_name");
static stu_str_t  KSD_CONF_TARGET_ENABLE    = stu_string("enable");
static stu_str_t  KSD_CONF_TARGET_WEIGHT    = stu_string("weight");
static stu_str_t  KSD_CONF_TARGET_TIMEOUT   = stu_string("timeout");
static stu_str_t  KSD_CONF_TARGET_MAX_FAILS = stu_string("max_fails");

static stu_str_t  KSD_CONF_IDENT              = stu_string("ident");
static stu_str_t  KSD_CONF_STAT               = stu_string("stat");
static stu_str_t  KSD_CONF_UPSTREAM_PROTOCOL  = stu_string("protocol");
static stu_str_t  KSD_CONF_UPSTREAM_METHOD    = stu_string("method");
static stu_str_t  KSD_CONF_UPSTREAM_TARGET    = stu_string("target");
static stu_str_t  KSD_CONF_UPSTREAM_ADDRESS   = stu_string("address");
static stu_str_t  KSD_CONF_UPSTREAM_PORT      = stu_string("port");
static stu_str_t  KSD_CONF_UPSTREAM_WEIGHT    = stu_string("weight");
static stu_str_t  KSD_CONF_UPSTREAM_TIMEOUT   = stu_string("timeout");
static stu_str_t  KSD_CONF_UPSTREAM_MAX_FAILS = stu_string("max_fails");

static stu_int32_t  ksd_conf_get_default(ksd_conf_t *conf);
static stu_int32_t  ksd_conf_copy_targets(stu_hash_t *hash, stu_json_t *item);
static stu_int32_t  ksd_conf_copy_upstream_servers(stu_list_t *list, stu_str_t *name, stu_json_t *item);


stu_int32_t
ksd_conf_parse_file(ksd_conf_t *conf, u_char *name) {
	stu_json_t         *root, *item, *sub;
	stu_str_t          *v_string;
	stu_double_t       *v_double;
	ksd_edition_mask_t *e;
	ksd_mode_mask_t    *m;
	u_char              tmp[KSD_CONF_MAX_SIZE];
	stu_file_t          file;

	stu_memzero(&file, sizeof(stu_file_t));
	stu_memzero(tmp, KSD_CONF_MAX_SIZE);

	if (ksd_conf_get_default(conf) == STU_ERROR) {
		stu_log_error(0, "Failed to get default conf.");
		return STU_ERROR;
	}

	// read conf file
	file.fd = stu_file_open(name, STU_FILE_CREATE_OR_OPEN, STU_FILE_RDONLY, STU_FILE_DEFAULT_ACCESS);
	if (file.fd == STU_FILE_INVALID) {
		stu_log_error(stu_errno, "Failed to " stu_file_open_n " conf file \"%s\".", name);
		return STU_ERROR;
	}

	if (stu_file_read(&file, tmp, KSD_CONF_MAX_SIZE, 0) == STU_ERROR) {
		stu_log_error(stu_errno, "Failed to " stu_file_read_n " conf file \"%s\".", name);
		stu_file_close(file.fd);
		return STU_ERROR;
	}

	if (file.offset > KSD_CONF_MAX_SIZE) {
		stu_log_error(0, "conf file too large: %d.", file.offset);
		stu_file_close(file.fd);
		return STU_ERROR;
	}

	// parse conf file
	root = stu_json_parse((u_char *) tmp, file.offset);
	if (root == NULL || root->type != STU_JSON_TYPE_OBJECT) {
		stu_log_error(0, "Bad conf file format.");
		stu_file_close(file.fd);
		return STU_ERROR;
	}

	// log
	item = stu_json_get_object_item_by(root, &KSD_CONF_LOG);
	if (item && item->type == STU_JSON_TYPE_STRING) {
		/* use default */
	}

	// pid
	item = stu_json_get_object_item_by(root, &KSD_CONF_PID);
	if (item && item->type == STU_JSON_TYPE_STRING) {
		v_string = (stu_str_t *) item->value;

		conf->pid.name.data = stu_calloc(v_string->len + 1);
		if (conf->pid.name.data == NULL) {
			stu_log_error(0, "Failed to calloc pid name.");
			goto failed;
		}

		stu_strncpy(conf->pid.name.data, v_string->data, v_string->len);
		conf->pid.name.len = v_string->len;
	}

	// edition
	item = stu_json_get_object_item_by(root, &KSD_CONF_EDITION);
	if (item && item->type == STU_JSON_TYPE_STRING) {
		v_string = (stu_str_t *) item->value;

		for (e = ksd_edition_mask; e->name.len; e++) {
			if (stu_strncasecmp(v_string->data, e->name.data, e->name.len) == 0) {
				conf->edition = e->mask;
				break;
			}
		}
	}

	// mode
	item = stu_json_get_object_item_by(root, &KSD_CONF_MODE);
	if (item && item->type == STU_JSON_TYPE_STRING) {
		v_string = (stu_str_t *) item->value;

		for (m = ksd_mode_mask; m->name.len; m++) {
			if (stu_strncasecmp(v_string->data, m->name.data, m->name.len) == 0) {
				conf->mode = m->mask;
				break;
			}
		}
	}

	// master_process
	item = stu_json_get_object_item_by(root, &KSD_CONF_MASTER_PROCESS);
	if (item && item->type == STU_JSON_TYPE_BOOLEAN) {
		conf->master_process = item->value & TRUE;
	}

	// worker_processes
	item = stu_json_get_object_item_by(root, &KSD_CONF_WORKER_PROCESSES);
	if (item && item->type == STU_JSON_TYPE_NUMBER) {
		v_double = (stu_double_t *) item->value;
		conf->worker_processes = *v_double;
	}

	// worker_threads
	item = stu_json_get_object_item_by(root, &KSD_CONF_WORKER_THREADS);
	if (item && item->type == STU_JSON_TYPE_NUMBER) {
		v_double = (stu_double_t *) item->value;
		conf->worker_threads = *v_double;
	}

	// debug
	item = stu_json_get_object_item_by(root, &KSD_CONF_DEBUG);
	if (item && item->type == STU_JSON_TYPE_NUMBER) {
		v_double = (stu_double_t *) item->value;
		conf->debug = *v_double;

		STU_DEBUG = conf->debug;
	}

	// server
	item = stu_json_get_object_item_by(root, &KSD_CONF_SERVER);
	if (item && item->type == STU_JSON_TYPE_OBJECT) {
		// listen
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_LISTEN);
		if (sub && sub->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) sub->value;
			conf->port = (stu_uint64_t) *v_double;
		}

		// root
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_ROOT);
		if (sub && sub->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) sub->value;

			conf->root.data = stu_calloc(v_string->len + 1);
			if (conf->root.data == NULL) {
				stu_log_error(0, "Failed to calloc cors data.");
				goto failed;
			}

			stu_strncpy(conf->root.data, v_string->data, v_string->len);
			conf->root.len = v_string->len;
		}

		// cors
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_CORS);
		if (sub && sub->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) sub->value;

			conf->cors.data = stu_calloc(v_string->len + 1);
			if (conf->cors.data == NULL) {
				stu_log_error(0, "Failed to calloc cors data.");
				goto failed;
			}

			stu_strncpy(conf->cors.data, v_string->data, v_string->len);
			conf->cors.len = v_string->len;
		}

		// push_stat
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_PUSH_STAT);
		if (sub && sub->type == STU_JSON_TYPE_BOOLEAN) {
			conf->push_stat = sub->value & TRUE;
		}

		// push_stat_interval
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_PUSH_STAT_INTERVAL);
		if (sub && sub->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) sub->value;
			conf->push_stat_interval = *v_double * 1000;
		}
	}

	// http
	item = stu_json_get_object_item_by(root, &KSD_CONF_SERVER_HTTP);
	if (item && item->type == STU_JSON_TYPE_OBJECT) {
		// listen
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_LISTEN);
		if (sub && sub->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) sub->value;
			conf->http_port = (stu_uint64_t) *v_double;
		}

		// root
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_ROOT);
		if (sub && sub->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) sub->value;

			conf->http_root.data = stu_calloc(v_string->len + 1);
			if (conf->http_root.data == NULL) {
				stu_log_error(0, "Failed to calloc cors data.");
				goto failed;
			}

			stu_strncpy(conf->http_root.data, v_string->data, v_string->len);
			conf->http_root.len = v_string->len;
		}

		// cors
		sub = stu_json_get_object_item_by(item, &KSD_CONF_SERVER_CORS);
		if (sub && sub->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) sub->value;

			conf->http_cors.data = stu_calloc(v_string->len + 1);
			if (conf->http_cors.data == NULL) {
				stu_log_error(0, "Failed to calloc cors data.");
				goto failed;
			}

			stu_strncpy(conf->http_cors.data, v_string->data, v_string->len);
			conf->http_cors.len = v_string->len;
		}
	}

	// target
	item = stu_json_get_object_item_by(root, &KSD_CONF_TARGET);
	if (item && item->type == STU_JSON_TYPE_ARRAY) {
		if (ksd_conf_copy_targets(&conf->target, item) == STU_ERROR) {
			stu_log_error(0, "Failed to copy target hash.");
			goto failed;
		}
	}

	// ident
	item = stu_json_get_object_item_by(root, &KSD_CONF_IDENT);
	if (item && item->type == STU_JSON_TYPE_ARRAY) {
		if (ksd_conf_copy_upstream_servers(&conf->ident, &KSD_CONF_IDENT, item) == STU_ERROR) {
			stu_log_error(0, "Failed to copy upstream server list: name=\"%s\".", KSD_CONF_IDENT.data);
			goto failed;
		}
	}

	// stat
	item = stu_json_get_object_item_by(root, &KSD_CONF_STAT);
	if (item && item->type == STU_JSON_TYPE_ARRAY) {
		if (ksd_conf_copy_upstream_servers(&conf->stat, &KSD_CONF_STAT, item) == STU_ERROR) {
			stu_log_error(0, "Failed to copy upstream server list: name=\"%s\".", KSD_CONF_STAT.data);
			goto failed;
		}
	}

	stu_file_close(file.fd);
	stu_json_delete(root);

	return STU_OK;

failed:

	stu_file_close(file.fd);
	stu_json_delete(root);

	return STU_ERROR;
}

static stu_int32_t
ksd_conf_copy_targets(stu_hash_t *hash, stu_json_t *item) {
	stu_json_t                 *sub, *property;
	stu_str_t                  *v_string;
	stu_double_t               *v_double;
	stu_rtmp_upstream_server_t *server;
	stu_rtmp_method_bitmask_t  *method;

	for (sub = (stu_json_t *) item->value; sub; sub = sub->next) {
		if (sub->type != STU_JSON_TYPE_OBJECT) {
			continue;
		}

		server = stu_calloc(sizeof(stu_rtmp_upstream_server_t));
		if (server == NULL) {
			stu_log_error(0, "Failed to calloc rtmp upstream server.");
			return STU_ERROR;
		}

		// protocol
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_PROTOCOL);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->protocol.data = stu_calloc(v_string->len + 1);
			if (server->protocol.data == NULL) {
				stu_log_error(0, "Failed to calloc target protocol.");
				return STU_ERROR;
			}

			stu_strncpy(server->protocol.data, v_string->data, v_string->len);
			server->protocol.len = v_string->len;
		}

		// method
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_METHOD);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			for (method = stu_rtmp_upstream_method_mask; method->name.len; method++) {
				if (stu_strncasecmp(v_string->data, method->name.data, method->name.len) == 0) {
					server->method = method->mask;
					break;
				}
			}
		}

		// name
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_NAME);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->name.data = stu_calloc(v_string->len + 1);
			if (server->name.data == NULL) {
				stu_log_error(0, "Failed to calloc rtmp target name.");
				return STU_ERROR;
			}

			stu_strncpy(server->name.data, v_string->data, v_string->len);
			server->name.len = v_string->len;
		}

		// dst_addr
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_DST_ADDR);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->dst_addr.name.data = stu_calloc(v_string->len + 1);
			if (server->dst_addr.name.data == NULL) {
				stu_log_error(0, "Failed to calloc rtmp dst addr.");
				return STU_ERROR;
			}

			stu_strncpy(server->dst_addr.name.data, v_string->data, v_string->len);
			server->dst_addr.name.len = v_string->len;
		}

		// dst_port
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_DST_PORT);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->dst_port = *v_double;
		}

		// dst_app
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_DST_APP);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->dst_app.data = stu_calloc(v_string->len + 1);
			if (server->dst_addr.name.data == NULL) {
				stu_log_error(0, "Failed to calloc rtmp dst addr.");
				return STU_ERROR;
			}

			stu_strncpy(server->dst_app.data, v_string->data, v_string->len);
			server->dst_app.len = v_string->len;
		}

		// dst_inst
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_DST_INST);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->dst_inst.data = stu_calloc(v_string->len + 1);
			if (server->dst_inst.data == NULL) {
				stu_log_error(0, "Failed to calloc rtmp dst addr.");
				return STU_ERROR;
			}

			stu_strncpy(server->dst_inst.data, v_string->data, v_string->len);
			server->dst_inst.len = v_string->len;
		}

		// dst_name
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_DST_NAME);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->dst_name.data = stu_calloc(v_string->len + 1);
			if (server->dst_name.data == NULL) {
				stu_log_error(0, "Failed to calloc rtmp dst addr.");
				return STU_ERROR;
			}

			stu_strncpy(server->dst_name.data, v_string->data, v_string->len);
			server->dst_name.len = v_string->len;
		}

		// enable
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_ENABLE);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;
			server->enable = property->value & TRUE;
		}

		// weight
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_WEIGHT);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->weight = *v_double;
		}

		// timeout
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_TIMEOUT);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->timeout = *v_double;
		}

		// max_fails
		property = stu_json_get_object_item_by(sub, &KSD_CONF_TARGET_MAX_FAILS);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->max_fails = *v_double;
		}

		server->dst_addr.sockaddr.sin_family = AF_INET;
		server->dst_addr.sockaddr.sin_addr.s_addr = inet_addr((const char *) server->dst_addr.name.data);
		server->dst_addr.sockaddr.sin_port = htons(server->dst_port);
		bzero(&(server->dst_addr.sockaddr.sin_zero), 8);
		server->dst_addr.socklen = sizeof(struct sockaddr);

		stu_hash_insert_locked(hash, &server->name, server);
	}

	return STU_OK;
}

static stu_int32_t
ksd_conf_copy_upstream_servers(stu_list_t *list, stu_str_t *name, stu_json_t *item) {
	stu_json_t                *sub, *property;
	stu_str_t                 *v_string;
	stu_double_t              *v_double;
	stu_upstream_server_t     *server;
	stu_rtmp_method_bitmask_t *method;

	for (sub = (stu_json_t *) item->value; sub; sub = sub->next) {
		if (sub->type != STU_JSON_TYPE_OBJECT) {
			continue;
		}

		server = stu_calloc(sizeof(stu_upstream_server_t));
		if (server == NULL) {
			stu_log_error(0, "Failed to calloc upstream server.");
			return STU_ERROR;
		}

		server->name = *name;
		server->method = STU_HTTP_GET;

		// protocol
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_PROTOCOL);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->protocol.data = stu_calloc(v_string->len + 1);
			if (server->protocol.data == NULL) {
				stu_log_error(0, "Failed to calloc upstream server protocol.");
				return STU_ERROR;
			}

			stu_strncpy(server->protocol.data, v_string->data, v_string->len);
			server->protocol.len = v_string->len;
		}

		// method
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_METHOD);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			for (method = stu_rtmp_upstream_method_mask; method->name.len; method++) {
				if (stu_strncasecmp(v_string->data, method->name.data, method->name.len) == 0) {
					server->method = method->mask;
					break;
				}
			}
		}

		// target
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_TARGET);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;
			server->target.data = stu_calloc(v_string->len + 1);
			server->target.len = v_string->len;
			stu_strncpy(server->target.data, v_string->data, v_string->len);
		}

		// address
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_ADDRESS);
		if (property && property->type == STU_JSON_TYPE_STRING) {
			v_string = (stu_str_t *) property->value;

			server->addr.name.data = stu_calloc(v_string->len + 1);
			if (server->addr.name.data == NULL) {
				stu_log_error(0, "Failed to calloc upstream server address.");
				return STU_ERROR;
			}

			stu_strncpy(server->addr.name.data, v_string->data, v_string->len);
			server->addr.name.len = v_string->len;
		}

		// port
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_PORT);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->port = *v_double;
		}

		// weight
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_WEIGHT);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->weight = *v_double;
		}

		// timeout
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_TIMEOUT);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->timeout = *v_double;
		}

		// max_fails
		property = stu_json_get_object_item_by(sub, &KSD_CONF_UPSTREAM_MAX_FAILS);
		if (property && property->type == STU_JSON_TYPE_NUMBER) {
			v_double = (stu_double_t *) property->value;
			server->max_fails = *v_double;
		}

		server->addr.sockaddr.sin_family = AF_INET;
		server->addr.sockaddr.sin_addr.s_addr = inet_addr((const char *) server->addr.name.data);
		server->addr.sockaddr.sin_port = htons(server->port);
		bzero(&(server->addr.sockaddr.sin_zero), 8);
		server->addr.socklen = sizeof(struct sockaddr);

		stu_list_insert_tail(list, server);
	}

	return STU_OK;
}

static stu_int32_t
ksd_conf_get_default(ksd_conf_t *conf) {
	struct timeval  tv;
	stu_tm_t        tm;

	// log
	stu_gettimeofday(&tv);
	stu_localtime(tv.tv_sec, &tm);

	conf->log.name.data = stu_calloc(STU_FILE_PATH_MAX_LEN);
	if (conf->log.name.data == NULL) {
		stu_log_error(0, "Failed to calloc log file name.");
		return STU_ERROR;
	}

	stu_sprintf(conf->log.name.data, "logs/%4d-%02d-%02d %02d:%02d:%02d.log",
			tm.stu_tm_year, tm.stu_tm_mon, tm.stu_tm_mday,
			tm.stu_tm_hour, tm.stu_tm_min, tm.stu_tm_sec);
	conf->log.name.len = stu_strlen(conf->log.name.data);

	// pid
	conf->pid.name = KSD_CONF_DEFAULT_PID;

	// edition
	conf->edition = PREVIEW;

	// worker
	conf->master_process = TRUE;
	conf->worker_processes = 1;
	conf->worker_threads = 4;

	// server
	conf->port = 1935;
	conf->root = stu_rtmp_root;
	stu_str_null(&conf->cors);

	conf->push_stat = FALSE;
	conf->push_stat_interval = STU_RTMP_APPLICATION_PUSH_STAT_DEFAULT_INTERVAL * 1000;

	// http
	conf->http_port = 80;
	conf->http_root = stu_http_root;
	stu_str_null(&conf->http_cors);

	// target
	stu_hash_init(&conf->target, STU_RTMP_UPSTREAM_MAXIMUM, NULL, STU_HASH_FLAGS_LOWCASE|STU_HASH_FLAGS_REPLACE);

	// upstream
	stu_list_init(&conf->ident, NULL);
	stu_list_init(&conf->stat, NULL);

	if (stu_upstream_init_hash() == STU_ERROR) {
		return STU_ERROR;
	}

	if (stu_hash_insert(&stu_upstreams, &KSD_CONF_IDENT, &conf->ident) == STU_ERROR) {
		stu_log_error(0, "Failed to insert upstream list into hash, name=\"%d\".", KSD_CONF_IDENT.data);
		return STU_ERROR;
	}

	if (stu_hash_insert(&stu_upstreams, &KSD_CONF_STAT, &conf->stat) == STU_ERROR) {
		stu_log_error(0, "Failed to insert upstream list into hash, name=\"%d\".", KSD_CONF_STAT.data);
		return STU_ERROR;
	}

	return STU_OK;
}
