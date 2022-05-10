#include "px_dfo.h"
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/parallel_execution/px_receiver.h"  // PX_receiver
#include "sql/parallel_execution/px_sender.h"  // PX_sender
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_class.h"  // THD
#include "sql/log.h"

extern bool px_partition(uint dop, void *&scan_ctx, TABLE *table, PX_SCAN_TYPE type,
                         uint keyno, TABLE_REF *ref, bool reverse_scan, uint &partitions);

/**
  In this function, we split DFO and set property to it, for example, we set
  one has scan, set one to root, set the children of one.
*/
bool Dfo_mgr::do_split(RowIterator *parent_iterator,
  RowIterator *iterator, Dfo *&parent_dfo)
{
  Dfo *dfo = nullptr;
  if (nullptr == m_root_iterator)
    m_root_iterator = iterator;
  if (iterator->type() == RowIterator::PHY_PX_RECEIVE) {
    if (!m_root_dfo) {
      if (create_dfo(m_root_iterator, dfo))
        return true;
      m_dfos_hash.insert(std::pair<int64_t, Dfo*>(-1, dfo));
    }
  } else if (iterator->type() == RowIterator::PHY_PX_SEND) {
    if (create_dfo(iterator, dfo))
      return true;
    if (create_exchange_info(dfo->dfo_id(), parent_iterator))
      return true;
    m_dfos_hash.insert(std::pair<int64_t, Dfo*>(dfo->dfo_id(), dfo));
    dfo->set_parent_dfo(parent_dfo);
    if (nullptr != parent_dfo) {
      parent_dfo->add_child_dfo(dfo);  // add dfo as child as parent_dfo.
      if (!parent_dfo->is_root_dfo()) parent_dfo->set_dfo_type(INTERNAL_DFO);
      // set the left most child of parent.
      if (1 == parent_dfo->m_child_dfos.size()) dfo->set_left_most();
    }
  } else { /* do nothing.*/ }

  Dfo *current_dfo = (nullptr == dfo) ? parent_dfo : dfo;
  if (nullptr != current_dfo && (
       iterator->type() == RowIterator::PHY_TABLE_SCAN ||
       iterator->type() == RowIterator::PHY_CONST_TABLE ||
       iterator->type() == RowIterator::PHY_INDEX_SCAN ||
       iterator->type() == RowIterator::PHY_INDEX_RANGE_SCAN ||
       iterator->type() == RowIterator::PHY_REF ||
       iterator->type() == RowIterator::PHY_REF_OR_NULL) &&
       !current_dfo->is_internal_dfo())
    current_dfo->set_dfo_type(LEAF_DFO);

  Dfo *tmp_parent_dfo = nullptr;
  if (nullptr != dfo) parent_dfo = dfo;
  if (nullptr == m_root_dfo) m_root_dfo = dfo;
  if (nullptr != m_root_dfo) m_root_dfo->set_dfo_type(ROOT_DFO);

  for (unsigned i = 0; i < iterator->m_children.size(); ++i) {
    tmp_parent_dfo = parent_dfo;
    // Pre-order traversal to split iterator tree.
    if (do_split(iterator, iterator->m_children.at(i), tmp_parent_dfo))
      return true;
    // Post-order traversal to push into normalized dfo tree.
    if (iterator->type() == RowIterator::PHY_PX_SEND)
      m_normalized_dfo_tree.push_back(dfo);
  }

  return false;
}

/* create dfo for the subplan which contains iterator tree. */
bool Dfo_mgr::create_dfo(RowIterator *iterator, Dfo *&dfo)
{
  if (!(dfo = new (m_thd->mem_root) Dfo()))
    return true;
  dfo->set_root_iterator(iterator);
  dfo->set_dfo_id(m_dfo_id_counter++);
  return false;
}

/* create exchange info for the dfo pair. */
bool Dfo_mgr::create_exchange_info(int64_t dfo_id, RowIterator *iterator)
{
  PX_exchange_info *exchange_info = nullptr;
  int64_t exchange_id = dfo_id; // Exchange id is same as dfo id.
  // We set the exchange info between PX_receiver and PX_sender.
  PX_receiver *receiver = static_cast<PX_receiver*>(iterator);
  assert(receiver->m_children.size() == 1);
  PX_sender *sender = static_cast<PX_sender *>(receiver->m_children.at(0));
  // Create exchange info if coordinator get exchange info if worker.
  if (m_thd->m_is_worker) {
    // we set it here and return true when not found.
    exchange_info = m_thd->px_exchange_context->get(exchange_id);
    if (nullptr == exchange_info) return true;
  } else {
    exchange_info = new (m_thd->mem_root) PX_exchange_info(
          m_thd, /*THD=*/
          PX_GATHER_EXCHANGE, /*exchange_type=*/
          PX_MQ_CHANNEL, /*channel_type=*/
          0, /*senders=*/
          0, /*receivers=*/
          PX_COMPACT_ROW, /*exchange_format=*/
          false); /*need_materialize=*/
    if (nullptr == exchange_info) return true;
    if (!m_thd->m_is_worker)
      sql_print_information("Dfo_mgr::%s:%d %d-th exchange info created.",
        __FUNCTION__, __LINE__, exchange_id);
    exchange_info->set_exchange_id(exchange_id);
    m_thd->px_exchange_context->insert(exchange_info);
    if (1 == dfo_id) exchange_info->set_top_exchange();
  }
  sender->set_exchange_info(exchange_info);
  receiver->set_exchange_info(exchange_info);
  return false;
}

/**
  Post-order traversal of dfo tree to find out a pair of tasks to be executed
  soon, which contains a dfo and its parent dfo.
*/
bool Dfo_mgr::get_ready_dfos(std::vector<Dfo*> &dfos) const
{
  bool ret = false;
  dfos.clear();
  for (unsigned i = 0; i < m_normalized_dfo_tree.size(); ++i) {
    Dfo *current_dfo = m_normalized_dfo_tree.at(i);
    if (current_dfo->is_dfo_finished()) {
      // The dfo has been finished and continue.
      continue;
    } else {
      // Check nullptr for m_parent_dfo.
      dfos.push_back(current_dfo);
      dfos.push_back(current_dfo->parent_dfo());
      //TODO: handle root dfo, real multiple scheduling.
      break;
    }
  }
  return ret;
}

/**
  In this function, We did the following things:
  1. A simple resource allocation strategy to set every stages' dop.
  2. Init the exchange info.
*/
void Dfo_mgr::analyze_resource_allocation()
{
  std::vector<Dfo*> ready_dfos;
  PX_exchange_context *context = m_thd->px_exchange_context;
  while(true) {
    if (get_ready_dfos(ready_dfos)) {
      break;
    } else if (ready_dfos.size() == 0) {
      // No dfos to schedule any more.
      break;
    } else if (ready_dfos.size() != 2) {
      assert(0);
    } else { // TODO: more complex resource allocation.
      size_t child_dop = 0;
      Dfo *child = ready_dfos[0];
      Dfo *parent = ready_dfos[1];
      parent->set_dfo_dop(1); // Set parent dfo dop to 1 currently.
      if (child->is_leaf_dfo()) { // Set child dfo dop to (max-1)
        child_dop = m_thd->variables.cdb_parallel_degree - 1;
        // Currently we choose the left most child leaf iterator.
        if (partition_scan(child, child_dop)) return;
      } else { // Set child dfo dop to 1 currently.
        child->set_dfo_dop(1);
      }
      if (!m_thd->m_is_worker)
        sql_print_information("Dfo_mgr::%s:%d Resource allocation child[%d] "
          "parent[%d]", __FUNCTION__, __LINE__, child->dop(), parent->dop());
      PX_exchange_info *exchange = context->get(child->dfo_id());
      // Set dop of <child,parent> dfo pair and init exchange info.
      exchange->set_dop(child->dop(), parent->dop());
      if (!m_thd->m_is_worker && exchange->init()) return;
      child->set_dfo_finished(true);
    }
  }
  set_total_cores(m_thd->variables.cdb_parallel_degree); // TODO
  // Reset finish flag for dfos in original normalized dfo tree.
  for (unsigned i = 0; i < m_normalized_dfo_tree.size(); ++i)
    m_normalized_dfo_tree.at(i)->set_dfo_finished(false);
}

/**
  Currently we just pick the leftmost table that can be scanned parallelly.
  More elegantly, we'll choose which table to read in parallel based on the
  code model.

  @return true when error, false when success.
*/
bool Dfo_mgr::partition_scan(Dfo *dfo, size_t &dop)
{
  // Currently support several paralllel scan by PX_reader:
  // 1. PHY_TABLE_SCAN;
  // 2. PHY_INDEX_SCAN;
  // 3. PHY_INDEX_RANGE_SCAN;
  // 4. PHY_REF
  // Currently, choose the left most child of the iterator tree.
  RowIterator *iterator = dfo->root_iterator()->m_children[0];
  assert(nullptr != iterator); // child iterator of PX_sender.
  while(nullptr != iterator) {
    if (iterator->type() == RowIterator::PHY_INDEX_RANGE_SCAN ||
        iterator->type() == RowIterator::PHY_TABLE_SCAN ||
        iterator->type() == RowIterator::PHY_INDEX_SCAN ||
        iterator->type() == RowIterator::PHY_REF) {
      uint partitions = 0;
      TableRowIterator *scan = static_cast<TableRowIterator *>(iterator);
      PX_table_descriptor *descriptor = scan->get_table_descriptor();
      int err = px_partition(dop, dfo->m_px_scan_ctx, descriptor->table(),
                             descriptor->type(), descriptor->keyno(),
                             descriptor->ref(), descriptor->reverse_scan(), partitions);
      destroy(descriptor);
      if (!err) {
        assert(dfo->m_px_scan_ctx);
        scan->set_parallel_scan();
        partitions = (partitions <= 0 ? 1 : partitions);
        dfo->set_dfo_dop(dop < partitions ? dop : partitions);
        break;
      } else {
        // assert(!dfo->m_px_scan_ctx);
        if (m_thd->killed) return true;
        sql_print_warning("Dfo_mgr::%s:%d px_partition error"
          "to fallback to serial execution.", __FUNCTION__, __LINE__);
        m_thd->need_fallback = true;
        return true; 
      }
    } else {
      if (0 == iterator->m_children.size())
        iterator = nullptr;
      else if (iterator->type() == RowIterator::PHY_PX_SEND)
        iterator = nullptr;
      else
        iterator = iterator->m_children[0];
    }
  }
  return false;
}

void Dfo_mgr::set_synchronization_info_for_dfo_tree(RowIterator *iterator)
{
  // Post-order traversal to set sychronzation of the dfo tree.
  for (unsigned i = 0; i < iterator->m_children.size(); ++i) {
    set_synchronization_info_for_dfo_tree(iterator->m_children.at(i));
    if (iterator->type() == RowIterator::PHY_PX_RECEIVE) {
      PX_receiver *receiver = static_cast<PX_receiver*>(iterator);
      PX_sender *sender = static_cast<PX_sender*>(iterator->m_children[0]);
      uint exchange_id = sender->get_pei()->exchange_id();
      Dfo *dfo = m_dfos_hash[exchange_id]; // exchange id is same as dfo id.
      // Synchronization topology for the dfo tree, we set sender and receiver:
      // For sender iterator the synchronization rule is:
      // 1. As leaf node, make him KEY role.
      // 2. As middle node, as first child, make him LOCK role.
      // 3. As middle node, as right child, make him KEY role.
      // For receiver iterator the sychrnization rule is:
      // 1. As parent of leaf node, make him LOCK role.
      // 2. As middle node, as parent of first child, make him KEY role.
      // 3. As middle node, as parent of right child, make him LOCK role.
      // Currently we do following setting.
      if (dfo->is_leaf_dfo() && dfo->parent_dfo()->is_root_dfo()) {
        // Currently two dfo scheduling, we keep it special.
        sender->m_role = RowIterator::SynchronizeRoleType::SYN_LOCK;
        receiver->m_role = RowIterator::SynchronizeRoleType::SYN_KEY;
      } else if (dfo->is_leaf_dfo()) {
        sender->m_role = RowIterator::SynchronizeRoleType::SYN_KEY;
        receiver->m_role = RowIterator::SynchronizeRoleType::SYN_LOCK;
      } else if (dfo->is_internal_dfo() && dfo->left_most()) {
        sender->m_role = RowIterator::SynchronizeRoleType::SYN_LOCK;
        receiver->m_role = RowIterator::SynchronizeRoleType::SYN_KEY;
      } else {
        sender->m_role = RowIterator::SynchronizeRoleType::SYN_KEY;
        receiver->m_role = RowIterator::SynchronizeRoleType::SYN_LOCK;
      }
    }
  }
}