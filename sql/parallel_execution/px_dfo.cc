#include "px_dfo.h"
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/parallel_execution/px_interface.h" // px_max_parallel_threads
#include "sql/parallel_execution/px_optimizer.h"  // get_parallel_degree_hint
#include "sql/parallel_execution/px_receiver.h"  // PX_receiver
#include "sql/parallel_execution/px_sender.h"  // PX_sender
#include "sql/sql_lex.h"  // LEX
#include "sql/sql_class.h"  // THD

extern bool px_partition(uint dop, void *&scan_ctx, TABLE *table, PX_SCAN_TYPE type,
                         uint keyno, TABLE_REF *ref, bool reverse_scan, uint &partitions);

bool is_scan_iterator(RowIterator *iterator) {
  return (iterator->type() == RowIterator::PHY_TABLE_SCAN ||
       iterator->type() == RowIterator::PHY_CONST_TABLE ||
       iterator->type() == RowIterator::PHY_INDEX_SCAN ||
       iterator->type() == RowIterator::PHY_INDEX_RANGE_SCAN ||
       iterator->type() == RowIterator::PHY_REF ||
       iterator->type() == RowIterator::PHY_REF_OR_NULL ||
       iterator->type() == RowIterator::PHY_EQ_REF);
}

bool is_partitionable_scan_iterator(RowIterator *iterator) {
  return (iterator->type() == RowIterator::PHY_INDEX_RANGE_SCAN ||
        iterator->type() == RowIterator::PHY_TABLE_SCAN ||
        iterator->type() == RowIterator::PHY_INDEX_SCAN ||
        iterator->type() == RowIterator::PHY_REF);
}

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
    m_dfos_hash.insert(std::pair<int64_t, Dfo*>(dfo->dfo_id(), dfo));
    dfo->set_parent_dfo(parent_dfo);
    if (nullptr != parent_dfo) {
      parent_dfo->add_child_dfo(dfo);  // add dfo as child as parent_dfo.
      if (!parent_dfo->is_root_dfo()) parent_dfo->set_dfo_type(INTERNAL_DFO);
      // set the left most child of parent.
      if (1 == parent_dfo->m_child_dfos.size()) dfo->set_left_most();
    }
    if (create_exchange_info(dfo->dfo_id(), parent_iterator, dfo->dfo_id(), parent_dfo->dfo_id()))
      return true;
  } else { /* do nothing.*/ }

  Dfo *current_dfo = (nullptr == dfo) ? parent_dfo : dfo;
  if (nullptr != current_dfo && is_scan_iterator(iterator) &&
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
  if (!(dfo = new (m_thd->mem_root) Dfo())) {
    my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
    return true;
  }
  dfo->set_root_iterator(iterator);
  dfo->set_dfo_id(m_dfo_id_counter++);
  return false;
}

/* create exchange info for the dfo pair. */
bool Dfo_mgr::create_exchange_info(int64_t dfo_id, RowIterator *iterator,
    int64_t producer_dfo_id, int64_t consumer_dfo_id) {
  assert(producer_dfo_id == dfo_id && consumer_dfo_id >= 0);
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
          false, /*need_materialize=*/
          rehash_for_px); /*reshuffle_func_t=*/
    if (!exchange_info) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", __FUNCTION__);
      return true;
    }
    exchange_info->set_exchange_id(exchange_id);
    exchange_info->set_producer_dfo_id(producer_dfo_id);
    exchange_info->set_consumer_dfo_id(consumer_dfo_id);
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
bool Dfo_mgr::analyze_resource_allocation(int64_t *cores)
{
  std::vector<Dfo*> ready_dfos;
  PX_exchange_context *context = m_thd->px_exchange_context;
  PX_exchange_info *exchange_info = nullptr;
  Dfo *last = nullptr; // last parent of scheduling pair.
  int64_t total = 0;
  const size_t default_dop = m_thd->variables.px_parallel_degree;
  // Zero dop by definition prevents parallel execution.
  assert(default_dop > 0);

  while(true) {
    if (get_ready_dfos(ready_dfos)) {
      break;
    } else if (ready_dfos.size() == 0) {
      // No dfos to schedule any more.
      break;
    } else if (ready_dfos.size() != 2) {
      assert(0);
    } else { // TODO: more complex allocation, background.
      Dfo *child = ready_dfos[0];
      Dfo *parent = ready_dfos[1];

      // FIXME set dop of inner nodes to reasonable
      child->set_dfo_dop(1);
      parent->set_dfo_dop(1);

      if (!(exchange_info = context->get(child->dfo_id()))) {
        assert(0);
        return true;
      }

      if (DBUG_EVALUATE_IF("simulate_right_deep_plan", true, false) ||
          (last && last != child && last != parent)) {
        PX_PRINT_ERROR("detected right-deep tree");
        m_thd->need_fallback = true;
        return true; // Not consider right-deep situation.
      }

      if (!m_thd->m_is_worker) {
        exchange_info->set_num_receivers(parent->dop());

        TableRowIterator *scan;
        if (child->is_leaf_dfo() && (scan = analyze_parallel_table(child))) {
          /*
            Refine leaf dop to min(real_dop,default_dop).

            Note that the dfo graph is private to each parallel thread, while
            exchange is shared. Although dynamic partitioning provides real dop,
            it is done only on the coordinator, the workers have to inherit real
            dop through exchange.
           */
          uint partitions = 0;
          PX_table_descriptor *desc  = scan->get_table_descriptor();
          if (!desc) {
            return true;
          }

          size_t given_dop = get_parallel_degree_hint(m_thd);
          // If given_dop was 0, this place could not be entered.
          assert(given_dop > 0);
          if (given_dop == UINT_MAX32) {
            given_dop = default_dop;
          }

          int err = px_partition(given_dop, child->m_px_scan_ctx, desc->table(),
                                 desc->type(), desc->keyno(), desc->ref(), desc->reverse_scan(),
                                 partitions);    
          if (err) {
            PX_PRINT_ERROR(
                "partitioning table %s (%llu rows) with default dop %lu "
                "got error %d",
                desc->table()->alias, desc->table()->file->stats.records, given_dop, err);
            return true;
          }

          assert(child->m_px_scan_ctx);
          PX_PRINT_INFO(
              "partitioning table %s (%llu rows) with dop %lu "
              "got %u partitions",
              desc->table()->alias, desc->table()->file->stats.records, given_dop, partitions);
          size_t real_dop = partitions;
          /*
            No dynamic partition suggests EOF for the iterator. There
            still should be one thread to process the empty source.
          */
          real_dop = real_dop < 1 ? 1 : real_dop;
          real_dop = real_dop > given_dop ? given_dop : real_dop;
          child->set_dfo_dop(real_dop);
          exchange_info->set_num_senders(real_dop);
          //qep_tab->set_parallel_workers(real_dop);
        } else {
          exchange_info->set_num_senders(child->dop());
        }
      } else {
        TableRowIterator *scan;
        if (child->is_leaf_dfo() && (scan = analyze_parallel_table(child))) {
          /*
            Although parllel scan context will be passed over to workers
            while a task is set up, the scan iterator itself still need to
            switch to parallel scan.
           */
          scan->set_parallel_scan();
        }
        assert(exchange_info->num_senders() > 0);
        child->set_dfo_dop(exchange_info->num_senders());
      }

      // Get the the maximum threads per each pair to reserve workers.
      if (total < child->dop() + parent->dop())
        total = child->dop() + parent->dop();

      // Set dfo state, required by iteration.
      child->set_dfo_finished(true);
      last = parent; // Child will be finished.
    }
  }

  // Reset dfo state.
  for (unsigned i = 0; i < m_normalized_dfo_tree.size(); ++i)
    m_normalized_dfo_tree.at(i)->set_dfo_finished(false);

  assert(total > 0);
  if (cores) *cores = total;

  return false;
}

/**
  Try to find parallel table in given DFO and prepare its dynamic partitions.

  The DFO's dop is thus refined by the real number of partitions.

  Currently we just pick the leftmost table that can be scanned parallelly.
  More elegantly, we'll choose which table to read in parallel based on the
  code model.

  @return the parallel table iterator or nullptr
*/
TableRowIterator *analyze_parallel_table(Dfo *dfo)
{
  // The root iterator may be PX_sender or the root of the entire tree.
  RowIterator *root, *iterator;
  root = iterator = dfo->root_iterator();
  assert(iterator);

  do {
    if (is_partitionable_scan_iterator(iterator)) {
      return down_cast<TableRowIterator *>(iterator);
    } else {
      if (0 == iterator->m_children.size())
        iterator = nullptr;
      else if (iterator != root &&
               (iterator->type() == RowIterator::PHY_PX_SEND ||
                iterator->type() == RowIterator::PHY_PX_RECEIVE))
        iterator = nullptr;
      else
        iterator = iterator->m_children[0];
    }
  } while (iterator);

  return nullptr;
}
