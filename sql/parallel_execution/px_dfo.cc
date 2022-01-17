#include "px_dfo.h"
#include "sql/iterators/row_iterator.h"  // RowIterator
#include "sql/sql_lex.h"  // LEX

/**
  In this function, we split DFO and set property to it, for example, we set one
  has scan, set one to root, set the children of one.
*/
bool Dfo_mgr::do_split_iterator_tree(RowIterator *iterator, Dfo *&parent_dfo)
{
  Dfo *dfo = nullptr;
  if (nullptr == m_root_iterator)
    m_root_iterator = iterator;
  // TODO: join tab should be generated in each query block.
  JOIN *join = m_thd->lex->current_query_block()->join;
  if (iterator->type() == RowIterator::PHY_PX_RECEIVE) {
    if (create_dfo(join, m_root_iterator, dfo))
      return true;
    dfo->m_receiver = iterator;
  } else if (iterator->type() == RowIterator::PHY_PX_SEND) {
    if (create_dfo(join, iterator, dfo))
      return true;
    dfo->m_sender = iterator;
    m_dfos.push_back(dfo);
    dfo->set_parent_dfo(parent_dfo);
    if (nullptr != parent_dfo)
      parent_dfo->add_child_dfo(dfo);
  } else { /* do nothing.*/ }

  Dfo *current_dfo = (nullptr == dfo) ? parent_dfo : dfo;
  if (nullptr != current_dfo && (
       iterator->type() == RowIterator::PHY_TABLE_SCAN ||
       iterator->type() == RowIterator::PHY_CONST_TABLE ||
       iterator->type() == RowIterator::PHY_INDEX_SCAN ||
       iterator->type() == RowIterator::PHY_INDEX_RANGE_SCAN ||
       iterator->type() == RowIterator::PHY_REF ||
       iterator->type() == RowIterator::PHY_REF_OR_NULL))
    current_dfo->set_dfo_has_scan();

  Dfo *tmp_parent_dfo = nullptr;
  if (nullptr != dfo) parent_dfo = dfo;
  if (nullptr == m_root_dfo) m_root_dfo = dfo;
  if (nullptr != m_root_dfo) m_root_dfo->m_is_root_dfo = true;

  // Subsequent traversal to split iterator tree.
  for (unsigned i = 0; i < iterator->m_children.size(); ++i) {
    tmp_parent_dfo = parent_dfo;
    if (do_split_iterator_tree(iterator->m_children.at(i), tmp_parent_dfo))
      return true;
  }

  return false;
}

/* create dfo for the subplan which contains iterator tree. */
bool Dfo_mgr::create_dfo(JOIN *join, RowIterator *iterator, Dfo *&dfo)
{
  if (!(dfo = new (m_thd->mem_root) Dfo()))
    return true;

  dfo->m_root_iterator = iterator;
  dfo->m_dfo_id = m_dfo_id_counter++;
  dfo->m_has_scan = false; // Parallel read infos.
  dfo->m_is_root_dfo = false;
  dfo->m_join = join;
  return false;
}

/**
  Subsequent traversal of dfo tree to find out a pair of tasks to be executed
  soon, which contains a dfo and its parent dfo.
*/
bool Dfo_mgr::get_ready_dfos(std::vector<Dfo*> &dfos) const
{
  bool ret = false;
  for (unsigned i = 0; i < m_dfos.size(); ++i) {
    Dfo *current_dfo = m_dfos.at(i);
    if (current_dfo->is_dfo_finished()) {
      // The dfo has been finished and continue.
      continue;
    } else {
      if (!current_dfo->is_dfo_active()) {
        dfos.push_back(current_dfo);
        // Check nullptr for m_parent_dfo.
        dfos.push_back(current_dfo->m_parent_dfo);
        current_dfo->set_dfo_active();
      } else { /*do nothing.*/ }
      //TODO: handle root dfo.
      break;
    }
  }
  return ret;
}
