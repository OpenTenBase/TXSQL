#ifndef PX_BINARY_HEAP_INCLUDED
#define PX_BINARY_HEAP_INCLUDED

#include <algorithm>
#include <iostream>
#include "my_sys.h"
#include "mysys_err.h"
#include "priority_queue.h"
#include "sql/sort_param.h"
#include "sql/malloc_allocator.h"
#include "sql/sql_class.h"
#include "sql/sql_base.h"

// compare based on the sort_key
typedef bool (*binaryheap_comparator)(uint a, uint b, void *arg);

class binary_heap {
 public:
  binary_heap(uint element_size, void *arg, binaryheap_comparator cmp, THD *thd)
      : m_queue(NULL),
        m_capacity(element_size),
        m_size(0),
        m_compare(cmp),
        m_arg(arg),
        m_thd(thd) {}

  /**
    @retval: false of success, and true otherwise.
  */
  bool init_binary_heap() {
    if (!m_capacity) return true;
    m_queue = new (m_thd->mem_root) uint[m_capacity + 1];
    if (!m_queue || DBUG_EVALUATE_IF("px_binary_heap_error1", true, false)) {
      return true;
    }

    return false;
  }

  /* return the index ((i - 1) / 2) of the parent node of node i */
  inline uint parent(unsigned int i) {
    assert(i != 0);
    return (--i) >> 1;
  }

  /* return the index (2 * i + 1) of the left child of node i */
  inline uint left(unsigned int i) { return (i << 1) | 1; }

  /* return the index (2 * i + 2) of the right child of node */
  inline uint right(unsigned int i) { return (++i) << 1; }

  void reset() { m_size = 0; }
  uint size() { return m_size; }

  void add_unorderd(uint element) {
    if (m_size >= m_capacity) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "", "(PX::add_unorderd)");
      return;
    }
    m_queue[m_size++] = element;
  }

  void build() {
    if (m_size <= 1) return;
    for (int i = parent(m_size - 1); i >= 0; i--) sift_down(i);
  }

  void add(uint element) {
    if (m_size >= m_capacity) {
      my_error(ER_STD_BAD_ALLOC_ERROR, MYF(0), "out of binary heap space");
      return;
    }
    m_queue[m_size++] = element;
    sift_up(m_size - 1);
  }

  uint first() {
    assert(!empty());
    return m_queue[0];
  }

  uint remove_first() {
    assert(!empty());
    if (m_size == 1) {
      m_size--;
      return m_queue[0];
    }

    swap_node(0, m_size - 1);
    m_size--;
    sift_down(0);

    return m_queue[m_size];
  }

  void replace_first(uint element) {
    assert(!empty());
    m_queue[0] = element;
    if (m_size > 1) sift_down(0);
  }
  bool empty() { return m_size == 0; }

  void cleanup() {
    if (m_queue) destroy(m_queue);
  }

 private:
  void swap_node(uint a, uint b) {
    uint T;
    T = m_queue[a];
    m_queue[a] = m_queue[b];
    m_queue[b] = T;
  }

  void sift_down(uint node_off) {
    while (true) {
      uint left_off = left(node_off);
      uint right_off = right(node_off);
      uint swap_off = 0;

      if (left_off < m_size &&
          m_compare(m_queue[left_off], m_queue[node_off], m_arg))
        swap_off = left_off;

      if (right_off < m_size &&
          m_compare(m_queue[right_off], m_queue[node_off], m_arg)) {
        if (!swap_off ||
            m_compare(m_queue[right_off], m_queue[left_off], m_arg))
          swap_off = right_off;
      }

      if (!swap_off) break;

      swap_node(swap_off, node_off);
      node_off = swap_off;
    }
  }

  void sift_up(uint node_off) {
    bool cmp = false;
    uint parent_off;
    while (node_off != 0) {
      parent_off = parent(node_off);
      cmp = m_compare(m_queue[parent_off], m_queue[node_off], m_arg);
      if (cmp) break;

      swap_node(node_off, parent_off);
      node_off = parent_off;
    }
  }

 private:
  uint *m_queue;
  uint m_capacity;
  uint m_size;
  binaryheap_comparator m_compare;
  void *m_arg;
  THD *m_thd;
};

#endif