/*
 ============================================================================
 Name        : kiwistrmd.c
 Author      : Tony Lau
 Version     : 1.x.xx
 Copyright   : kiwistrmd.com
 Description : High-performance Media Server for Html5
 ============================================================================
 */

#include "kiwistrmd.com/core/ksd_core.h"
#include "kiwistrmd.com/ksd_config.h"

extern const stu_str_t  __NAME;
extern const stu_str_t  __VERSION;

extern volatile ksd_cycle_t *ksd_cycle;


int main(void) {
	ksd_conf_t *conf;

	// init cycle
	if (ksd_cycle_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init cycle.");
		return EXIT_FAILURE;
	}

	conf = (ksd_conf_t *) &ksd_cycle->conf;

	// server info
	stu_log("GCC " __VERSION__);
	stu_log("%s/%s (" __TIME__ ", " __DATE__ ")", __NAME.data, __VERSION.data);

	// create pid
	if (ksd_cycle_create_pidfile(&conf->pid) == STU_ERROR) {
		stu_log_error(0, "Failed to create pid file: path=\"%s\".", conf->pid.name.data);
		return EXIT_FAILURE;
	}

	// master cycle
	ksd_process_master_cycle();

	// delete pid
	ksd_cycle_delete_pidfile(&conf->pid);

	return EXIT_SUCCESS;
}
