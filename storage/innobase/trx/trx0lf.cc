
#include "trx0sys.h"
#include "trx0trx.h"
#include "srv0srv.h"

static const uint32_t SNAPSHOT_SPIN_LOOP = 32;

void rw_trx_hash_t::init() {
  const ulint KEY_OFFSET = 0; // key field offset within rw_trx_hash_element_t
  const ulint KEY_LEN = sizeof(trx_id_t);
  const hash_get_key_function GET_KEY_FUNC = nullptr; // use KEY_OFFSET and KEY_LEN to extract key field
  CHARSET_INFO *charset = &my_charset_bin; // int* compare

  lf_hash_init2(&hash, sizeof(rw_trx_hash_element_t), LF_HASH_UNIQUE,
                KEY_OFFSET, KEY_LEN, GET_KEY_FUNC, charset, nullptr /* hash_func */,
                rw_trx_hash_elem_constructor,
                rw_trx_hash_elem_destructor,
                rw_trx_hash_elem_initializer);
  const int RW_TRX_HASH_SIZE = 256;
  hash.max_size = RW_TRX_HASH_SIZE;
}

void rw_trx_hash_t::destroy() {
  hash.alloc.destructor = rw_trx_hash_elem_shutdown_destructor;
  lf_hash_destroy(&hash);
}

LF_PINS* rw_trx_hash_t::get_pins(trx_t *trx) {
  if (!trx->rw_trx_hash_pins) {
    trx->rw_trx_hash_pins = lf_hash_get_pins(&hash);
    ut_a(trx->rw_trx_hash_pins);
  }
  return (trx->rw_trx_hash_pins);
}

void rw_trx_hash_t::put_pins(trx_t* trx) {
  if (trx->rw_trx_hash_pins) {
    lf_hash_put_pins(trx->rw_trx_hash_pins);
    trx->rw_trx_hash_pins = NULL;
  }
}

trx_t* rw_trx_hash_t::find(trx_t *caller_trx, trx_id_t trx_id, bool do_ref_count) {
  trx_t *trx = NULL;
  LF_PINS *pins = caller_trx ? get_pins(caller_trx) : lf_hash_get_pins(&hash);
  ut_a(pins);

  void *elem_addr = lf_hash_search(&hash, pins,
                                   reinterpret_cast<const void*>(&trx_id),
                                   sizeof(trx_id_t));
  rw_trx_hash_element_t *element= reinterpret_cast<rw_trx_hash_element_t*>(elem_addr);

  if (element) {
    mutex_enter(&element->mutex);
    trx = element->trx;
    if (trx) {
      ut_a(trx_id == trx->id);
      if (do_ref_count) {
        trx_mutex_enter(trx);
        const trx_state_t state = trx->state;
        if (state == TRX_STATE_COMMITTED_IN_MEMORY) {
          trx = NULL;
        } else {
          trx->n_ref++;
        }
        trx_mutex_exit(trx);
      }
    }
    mutex_exit(&element->mutex);

    // after lf_hash_search(actually my_lsearch),
    // pin[2] is used to pin object just found
    lf_hash_search_unpin(pins);
  }
  if (!caller_trx) {
    lf_hash_put_pins(pins);
  }
  return (trx);
}

void rw_trx_hash_t::insert(trx_t* trx) {
  int res = lf_hash_insert(&hash, get_pins(trx), reinterpret_cast<void*>(trx));
  ut_a(res == 0);
}

void rw_trx_hash_t::erase(trx_t* trx) {
  mutex_enter(&trx->rw_trx_hash_element->mutex);
  trx->rw_trx_hash_element->trx = 0;
  mutex_exit(&trx->rw_trx_hash_element->mutex);

  int res = lf_hash_delete(&hash, get_pins(trx),
                           reinterpret_cast<const void*>(&trx->id),
                           sizeof(trx_id_t));
  ut_a(res == 0);
}

int rw_trx_hash_t::iterate(trx_t *caller_trx,
                           my_hash_walk_action action,
                           void *argument) {
  LF_PINS *pins = caller_trx ? get_pins(caller_trx) : lf_hash_get_pins(&hash);
  ut_a(pins);

#ifdef UNIV_DEBUG
  debug_iterator_arg debug_arg = { action, argument };
  action = reinterpret_cast<my_hash_walk_action>(debug_iterator);
  argument = &debug_arg;
#endif

  int res = lf_hash_iterate(&hash, pins, action, argument);
  if (!caller_trx) {
    lf_hash_put_pins(pins);
  }
  return (res);
}

int rw_trx_hash_t::iterate(my_hash_walk_action action, void *argument) {
  return (iterate(current_trx(), action, argument));
}

bool rw_trx_hash_t::eliminate_duplicates(rw_trx_hash_element_t *element,
                                         eliminate_duplicates_arg *arg) {
  for (auto it = arg->ids.begin();
       it != arg->ids.end(); it++) {
    if (*it == element->id)
      return (false);
  }
  arg->ids.push_back(element->id);
  return (arg->action(element, arg->arg));
};

int rw_trx_hash_t::iterate_no_dups(trx_t *caller_trx,
                                   my_hash_walk_action action,
                                   void* argument) {
  eliminate_duplicates_arg arg(size() + 32, action, argument);
  return (iterate(caller_trx, reinterpret_cast<my_hash_walk_action>
                 (eliminate_duplicates), &arg));
}

int rw_trx_hash_t::iterate_no_dups(my_hash_walk_action action, void *argument) {
  return iterate_no_dups(current_trx(), action, argument);
}

bool get_min_trx_id_callback(rw_trx_hash_element_t *element,
                             trx_id_t *id) {
  if (element->id < *id) {
    mutex_enter(&element->mutex);
    /* We don't care about read-only transactions here. */
    if (element->trx && element->trx->rsegs.m_redo.rseg) {
      *id = element->id;
    }
    mutex_exit(&element->mutex);
  }
  return (false);
}

trx_id_t trx_sys_t::get_min_trx_id() {
  trx_id_t id = get_max_trx_id();
  THIS_HASH_ITERATE(nullptr, get_min_trx_id_callback, &id);
  return id;
}
/* Since we do not handle the gap between a trx increase max_trx_id
and insert into lf_hash, we may get a larger min_active_id here.
But these transactions can be treated as to be ACTIVE.
So be care when you use this function.  */
bool trx_sys_t::is_trx_id_possible_active(trx_id_t compare_to) {
  if (compare_to < m_min_active_id.load(std::memory_order_relaxed)) {
    /* Optimistic checking */
    return (false);
  }

  trx_id_t id = get_min_trx_id();

  trx_id_t old_val = m_min_active_id.load();
  while (old_val < id &&
         !m_min_active_id.compare_exchange_weak(old_val, id));

  return (compare_to >= id);
}

bool get_min_trx_no_callback(rw_trx_hash_element_t *element,
                             trx_id_t *no) {
  trx_id_t ele_no = element->no.load();
  if (ele_no != TRX_ID_MAX && ele_no < *no) {
    *no = ele_no;
  }
  return 0;
}

trx_id_t trx_sys_t::get_min_trx_no() {
  trx_id_t no;
  while ((no = get_max_trx_id()) != get_rw_trx_hash_version()) {
    ut_delay(1);
  }
  THIS_HASH_ITERATE(nullptr, get_min_trx_no_callback, &no);
  return (no);
}

void trx_sys_t::register_rw(trx_t *trx) {
  rw_lock_s_lock(lock, UT_LOCATION_HERE);
  trx->id = get_new_trx_id_no_refresh();
  rw_trx_hash.insert(trx);
  refresh_rw_trx_hash_version();
  rw_lock_s_unlock(lock);
}

void trx_sys_t::deregister_rw(trx_t *trx) {
  rw_trx_hash.erase(trx);
  trx_sys->hash_erase_happen();
}

bool trx_sys_t::is_registered(trx_t *caller_trx, trx_id_t id) {
  return (id > 0 && find(caller_trx, id, false));
}

trx_t* trx_sys_t::find(trx_t *caller_trx, trx_id_t id, bool do_ref_count) {
  return (rw_trx_hash.find(caller_trx, id, do_ref_count));
}

void trx_sys_t::assign_new_trx_no(trx_t *trx) {
  trx->no = get_new_trx_id_no_refresh();
  trx->rw_trx_hash_element->no = trx->no;
  refresh_rw_trx_hash_version();
}

bool trx_sys_t::copy_one_id(rw_trx_hash_element_t *element,
                            snapshot_ids_arg *arg) {
  // if an elem->id is large than ids_arg->m_id,
  // that means the elem/trx started after this reading transaction do snapshot_id,
  // which mean the elem/trx is not visible to this reading transaction.
  // According to visible policy, the elem/trx->id will be checked as
  // invisible by low_limit, the id will not need to be put into ids array.
  if (element->id < arg->m_id) {
    // if this trx is is put into m_ids, we need make sure to take its trx no
    // into account when calculating min_no.
    // trx no is TRX_MAX_ID if not set now.
    trx_id_t no = element->no;

    // m_ids will sort by caller of snapshot_ids()
    arg->m_ids->push_back(element->id);

    if (no < arg->m_no) {
      arg->m_no = no;
    }
  }
  return (false);
}

void trx_sys_t::snapshot_ids(trx_t *caller_trx,
                             trx_ids_t *ids,
                             trx_id_t *max_trx_id,
                             trx_id_t *min_trx_no,
                             int64_t *erase_version) {
  ut_ad(!mutex_own(&mutex));
  snapshot_ids_arg arg(ids);

  uint32_t max_count = SNAPSHOT_SPIN_LOOP;
  while ((arg.m_id = get_rw_trx_hash_version()) != get_max_trx_id()) {
    /* For background purge thread which has caller_trx = nullptr, we
    always let it spins. */
    if (caller_trx) {
      if (max_count == 0) {
        rw_lock_x_lock(lock, UT_LOCATION_HERE);
        arg.m_id = get_rw_trx_hash_version(); 
        rw_lock_x_unlock(lock);
        break;
      }
      max_count--;
    }
    ut_delay(1);
  }
  // after waiting for (hash_version != max_trx_id),
  // trx_id which less than max_trx_id must have been put into lf_hash
  // already (trx_id also may be removed since the trx commit done).
  // That mean all active trx less than max_trx_id is in lf_hash.

  arg.m_no = arg.m_id;
  ids->clear();
  ids->reserve(rw_trx_hash.size() + 32);

  *erase_version = get_hash_erase_version();
  std::atomic_thread_fence(std::memory_order_seq_cst);

  THIS_HASH_ITERATE(caller_trx, copy_one_id, &arg);
  *max_trx_id = arg.m_id;
  *min_trx_no = arg.m_no;
}

trx_id_t trx_sys_get_max_trx_id() {
  return (trx_sys->get_max_trx_id());
}

