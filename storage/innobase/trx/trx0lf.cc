#include "trx0sys.h"
#include "trx0trx.h"
#include "srv0srv.h"

uint srv_snapshot_spin_loop = 32;

void rw_trx_hash_t::init() {
  lf_hash_init2(&hash, sizeof(rw_trx_hash_element_t), LF_HASH_UNIQUE, 0,
               sizeof(trx_id_t), 0, &my_charset_bin, NULL,
               rw_trx_hash_constructor,
               rw_trx_hash_destructor,
               reinterpret_cast<lf_hash_init_func*>(&rw_trx_hash_initializer));
  hash.max_size = srv_rw_trx_hash_size;
}

void rw_trx_hash_t::destroy() {
  hash.alloc.destructor = rw_trx_hash_shutdown_destructor;
  lf_hash_destroy(&hash);
}

LF_PINS* rw_trx_hash_t::get_pins(trx_t *trx) {
  if (!trx->rw_trx_hash_pins) {
    trx->rw_trx_hash_pins= lf_hash_get_pins(&hash);
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
  rw_trx_hash_element_t *element= reinterpret_cast<rw_trx_hash_element_t*>
    (lf_hash_search(&hash, pins, reinterpret_cast<const void*>(&trx_id),
                    sizeof(trx_id_t)));

  if (element) {
    mutex_enter(&element->mutex);
    lf_hash_search_unpin(pins);

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
  ut_ad(!find(NULL, trx->id, false));
}

int rw_trx_hash_t::iterate(trx_t *caller_trx, my_hash_walk_action action, void *argument) {
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
  return (arg->action(element, arg->argument));
};

int rw_trx_hash_t::iterate_no_dups(trx_t *caller_trx,
                                   my_hash_walk_action action, void* argument) {
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

bool trx_sys_t::is_trx_id_possible_active(trx_id_t compare_to) {

  if (compare_to < m_min_active_id.load(std::memory_order_relaxed)) {
    /* Optimistic checking */
    return (false);
  }

  trx_id_t id = get_max_trx_id();
  rw_trx_hash.iterate(reinterpret_cast<my_hash_walk_action>
                      (get_min_trx_id_callback), &id);

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
  trx_id_t no = get_max_trx_id();
  rw_trx_hash.iterate(reinterpret_cast<my_hash_walk_action>
                      (get_min_trx_no_callback), &no);

  return (no);
}

void trx_sys_t::register_rw(trx_t *trx) {
  rw_lock_s_lock(lock);
  trx->id = get_new_trx_id_no_refresh();
  rw_trx_hash.insert(trx);
  refresh_rw_trx_hash_version();
  rw_lock_s_unlock(lock);
  mvcc->set_view_flag(false);
}

void trx_sys_t::deregister_rw(trx_t *trx) {
  rw_trx_hash.erase(trx);
  mvcc->set_view_flag(false);
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
  mvcc->set_view_flag(false);
}

bool trx_sys_t::copy_one_id(rw_trx_hash_element_t *element,
                                   snapshot_ids_arg *arg) {
  if (element->id < arg->m_id) {
    trx_id_t no = element->no;
    arg->m_ids->push_back(element->id);
    if (no < arg->m_no) {
      arg->m_no = no;
    }
  }
  return (false);
}

bool trx_sys_t::snapshot_ids(trx_t *caller_trx, trx_ids_t *ids, trx_id_t *max_trx_id,
                             trx_id_t *min_trx_no, bool try_clone_global) {
  ut_ad(!mutex_own(&mutex));
  snapshot_ids_arg arg(ids);
  uint32_t max_count = srv_snapshot_spin_loop;
  while ((arg.m_id = get_rw_trx_hash_version()) != get_max_trx_id()) {
    /* For background purge thread which has caller_trx = nullptr, we
    always let it spins. */
    if (caller_trx) {
      if (max_count == 0) {
        rw_lock_x_lock(lock);
        arg.m_id = get_rw_trx_hash_version(); 
        rw_lock_x_unlock(lock);
        break;
      }

      max_count--;

      if (try_clone_global && mvcc->is_clone_valid()) {
        return false;
      }
    }
    
    ut_delay(1);
  }

  if (try_clone_global && mvcc->is_clone_valid()) {
    return false;
  }

  arg.m_no = arg.m_id;

  ids->clear();
  ids->reserve(rw_trx_hash.size() + 32);
  rw_trx_hash.iterate(caller_trx,
                      reinterpret_cast<my_hash_walk_action>(copy_one_id),
                      &arg);

  *max_trx_id= arg.m_id;
  *min_trx_no= arg.m_no;

  return true;
}

void trx_sys_t::flush_max_trx_id() {
  mtr_t mtr;
  trx_sysf_t *sys_header;

  if (!srv_read_only_mode) {
    mtr_start(&mtr);

    sys_header = trx_sysf_get(&mtr);
    /* The page is x locked, so only one thread can reach here */
    mlog_write_ull(sys_header + TRX_SYS_TRX_ID_STORE, trx_sys->get_max_trx_id(),
                   &mtr);
    mtr_commit(&mtr);
  }
}

trx_id_t trx_sys_get_max_trx_id() {
  return (trx_sys->get_max_trx_id());
}
