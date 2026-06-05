#include "sql/table.h"
#include "sql/sql_base.h"
#include "txsql_cte.h"

namespace txsql {

bool CTE_node::Equals(const CTE_node *other) const {
	if (!other) {
		return false;
	}
	if (this->type != other->type) {
		return false;
	}
	return true;
}

CTE_view::CTE_view(const TABLE_LIST *view) : CTE_node(CTE_Type::VIEW), m_view(view) {}

txsql::hash_t CTE_view::Hash() const {
  assert(m_view);
  txsql::hash_t hash = txsql::Hash<uint32_t>((uint32_t)type);
  hash = txsql::CombineHash(hash, txsql::Hash<size_t>(m_view->select_stmt.length));
	hash = txsql::CombineHash(hash, txsql::Hash((const char *)m_view->select_stmt.str, m_view->select_stmt.length));
	return hash;
}

bool CTE_view::Equals(const CTE_node *other) const {
  if (!CTE_node::Equals(other)) {
    return false;
  }

  if (m_view->select_stmt.length != ((CTE_view *)other)->m_view->select_stmt.length) {
    return false;
  }

  if (memcmp(m_view->select_stmt.str,
      ((CTE_view *)other)->m_view->select_stmt.str, m_view->select_stmt.length)) {
    return false;
  }

  return true;
}

CTE_view_expr::CTE_view_expr(MEM_ROOT *mem_root) : count(1), tmp_tables(mem_root) {}

bool CTE_view_expr::is_cte_consumer(TABLE_LIST *table) const {
  assert(tmp_tables.size());
  auto first = tmp_tables[0];
  return (first == table) ? false : true;
}

void CTE_view_expr::remove_table(TABLE_LIST *tr) {
  (void)tmp_tables.erase_value(tr);
  tr->set_cte_expr(nullptr);
  --count;
}

TABLE *CTE_view_expr::clone_tmp_table(THD *thd, TABLE_LIST *tl) {
  assert(tl->txsql_cte() == this);
  TABLE *first = tmp_tables[0]->table;
  // Allocate clone on the memory root of the TABLE_SHARE.
  TABLE *t = static_cast<TABLE *>(first->s->mem_root.Alloc(sizeof(TABLE)));
  if (!t) return nullptr; /* purecov: inspected */
  if (open_table_from_share(thd, first->s, tl->alias,
                            /*
                              Pass db_stat == 0 to delay opening of table in SE,
                              as table is not instantiated in SE yet.
                            */
                            0,
                            /* We need record[1] for this TABLE instance. */
                            EXTRA_RECORD |
                                /*
                                  Use DELAYED_OPEN to have its own record[0]
                                  (necessary because db_stat is 0).
                                  Otherwise it would be shared with 'first'
                                  and thus a write to tmp table would modify
                                  the row just read by readers.
                                */
                                DELAYED_OPEN,
                            0, t, false, nullptr))
    return nullptr; /* purecov: inspected */
  assert(t->s == first->s && t != first && t->file != first->file);
  t->s->increment_ref_count();
  t->s->tmp_handler_count++;

  // In case this clone is used to fill the materialized table:
  bitmap_set_all(t->write_set);
  t->reginfo.lock_type = TL_WRITE;
  t->copy_blobs = true;
  tl->table = t;
  t->pos_in_table_list = tl;
  t->set_not_started();

  return t;
}

}