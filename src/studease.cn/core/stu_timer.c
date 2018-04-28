/*
 * stu_timer.c
 *
 *  Created on: 2017骞�11鏈�16鏃�
 *      Author: Tony Lau
 */

#include "../stu_config.h"
#include "stu_core.h"

static stu_timer_t  stu_timer;


/*
 * the event timer rbtree may contain the duplicate keys, however,
 * it should not be a problem, because we use the rbtree to find
 * a minimum timer value only
 */
stu_int32_t
stu_timer_init(void) {
	stu_mutex_init(&stu_timer.lock, NULL);
	stu_rbtree_init(&stu_timer.tree, &stu_timer.sentinel, stu_rbtree_insert_timer_value, NULL);

	return STU_OK;
}

stu_msec_t
stu_timer_find(void) {
	stu_rbtree_node_t *node, *root, *sentinel;
	stu_msec_int_t     timer;

	stu_mutex_lock(&stu_timer.lock);

	if (stu_timer.tree.root == &stu_timer.sentinel) {
		timer = STU_TIMER_INFINITE;
		goto done;
	}

	root = stu_timer.tree.root;
	sentinel = stu_timer.tree.sentinel;
	node = stu_rbtree_min(root, sentinel);

	timer = (stu_msec_int_t) (node->key - stu_current_msec);
	timer = timer > 0 ? timer : 0;

done:

	stu_mutex_unlock(&stu_timer.lock);

	return (stu_msec_t) timer;
}

void
stu_timer_expire(void) {
	stu_rbtree_node_t *node, *root, *sentinel;
	stu_event_t       *ev;

	stu_mutex_lock(&stu_timer.lock);

	sentinel = stu_timer.tree.sentinel;

	for ( ;; ) {
		root = stu_timer.tree.root;
		if (root == sentinel) {
			goto done;
		}

		node = stu_rbtree_min(root, sentinel);

		/* node->key > stu_current_time */
		if ((stu_msec_int_t) (node->key - stu_current_msec) > 0) {
			goto done;
		}

		ev = (stu_event_t *) ((char *) node - offsetof(stu_event_t, timer));
		stu_log_debug(2, "timer delete: fd=%d, key=%lu.", stu_timer_ident(ev->data), ev->timer.key);

		stu_rbtree_delete(&stu_timer.tree, &ev->timer);

#if (STU_DEBUG)
		ev->timer.left = NULL;
		ev->timer.right = NULL;
		ev->timer.parent = NULL;
#endif

		ev->timedout = 1;
		ev->timer_set = 0;

		ev->handler(ev);
	}

done:

	stu_mutex_unlock(&stu_timer.lock);
}

void
stu_timer_cancel(void) {
	stu_rbtree_node_t  *node, *root, *sentinel;
	stu_event_t        *ev;

	stu_mutex_lock(&stu_timer.lock);

	sentinel = stu_timer.tree.sentinel;

	for ( ;; ) {
		root = stu_timer.tree.root;
		if (root == sentinel) {
			goto done;
		}

		node = stu_rbtree_min(root, sentinel);

		ev = (stu_event_t *) ((char *) node - offsetof(stu_event_t, timer));
		if (!ev->cancelable) {
			goto done;
		}

		stu_log_debug(2, "timer cancel: fd=%d, key=%lu.", stu_timer_ident(ev->data), ev->timer.key);

		stu_rbtree_delete(&stu_timer.tree, &ev->timer);

#if (STU_DEBUG)
		ev->timer.left = NULL;
		ev->timer.right = NULL;
		ev->timer.parent = NULL;
#endif

		ev->timer_set = 0;

		ev->handler(ev);
	}

done:

	stu_mutex_unlock(&stu_timer.lock);
}


void
stu_timer_add(stu_event_t *ev, stu_msec_t timer) {
	stu_mutex_lock(&stu_timer.lock);
	stu_timer_add_locked(ev, timer);
	stu_mutex_unlock(&stu_timer.lock);
}

void
stu_timer_add_locked(stu_event_t *ev, stu_msec_t timer) {
	stu_msec_t      key;
	stu_msec_int_t  diff;

	key = stu_current_msec + timer;

	if (ev->timer_set) {
		/*
		 * Use a previous timer value if difference between it and a new
		 * value is less than STU_TIMER_LAZY_DELAY milliseconds: this allows
		 * to minimize the rbtree operations for fast connections.
		 */

		diff = (stu_msec_int_t) (key - ev->timer.key);
		if (stu_abs(diff) < STU_TIMER_LAZY_DELAY) {
			stu_log_debug(2, "timer: %d, old: %lu, new: %lu.", stu_timer_ident(ev->data), ev->timer.key, key);
			return;
		}

		stu_timer_del_locked(ev);
	}

	ev->timer.key = key;
	stu_log_debug(2, "timer add: fd=%d, delay=%lu, key=%lu.", stu_timer_ident(ev->data), timer, ev->timer.key);

	stu_rbtree_insert(&stu_timer.tree, &ev->timer);

	ev->timer_set = 1;
}

void
stu_timer_del(stu_event_t *ev) {
	stu_mutex_lock(&stu_timer.lock);
	stu_timer_del_locked(ev);
	stu_mutex_unlock(&stu_timer.lock);
}

void
stu_timer_del_locked(stu_event_t *ev) {
	stu_rbtree_delete(&stu_timer.tree, &ev->timer);
	stu_log_debug(2, "timer delete: fd=%d, key=%lu.", stu_timer_ident(ev->data), ev->timer.key);

#if (STU_DEBUG)
	ev->timer.left = NULL;
	ev->timer.right = NULL;
	ev->timer.parent = NULL;
#endif

	ev->timer_set = 0;
}
