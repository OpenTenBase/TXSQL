# 优化文档：消除冗余的常量 ORDER BY

## 背景

在 SQL 查询中，`ORDER BY` 子句用于对结果集进行排序。然而，在某些情况下，`ORDER BY` 列表中的某些（或全部）表达式可能在查询执行期间是常量的。这通常发生在以下情况：
1. 用户显式地对常量值进行排序（例如 `ORDER BY 1`）。
2. `WHERE` 子句中的等值条件将某个列约束为常量（例如 `SELECT * FROM t1 WHERE col1 = 5 ORDER BY col1`）。

在这些情况下，对常量值进行排序是没有任何意义的，并且会引入不必要的 CPU 和 I/O 开销（例如触发 `filesort`）。

## 优化方案

在查询优化器的 `JOIN::optimize` 阶段，特别是在常量传播（Constant Propagation）完成之后，我们可以检查 `ORDER BY` 列表。如果发现任何列表项是常量表达式（`const_item()` 返回 true），我们就可以安全地将其从排序列表中移除。

如果移除后 `ORDER BY` 列表为空，则整个排序操作被取消，完全避免了排序开销。

## 实现细节

### 修改文件
*   `sql/sql_optimizer.cc`

### 新增函数
引入了一个静态辅助函数 `remove_const_order_elements`：

```cpp
static void remove_const_order_elements(THD *thd, ORDER **order_ptr) {
  // 遍历 ORDER 链表
  // 如果 item->const_item() 为真，则从链表中移除该节点
  // 维护链表指针完整性
}
```

### 调用逻辑
在 `JOIN::optimize` 函数中，`optimize_cond`（负责 WHERE/HAVING 优化和常量传播）执行之后，调用上述函数：

```cpp
  /*
    Optimize ORDER BY:
    If any ORDER BY item is constant, remove it.
    This typically happens if the field is equated to a constant in WHERE.
  */
  if (order.order) {
    remove_const_order_elements(thd, &order.order);
    if (!order.order) {
      explain_flags.clear(ESC_ORDER_BY);
    }
  }
```

## 示例与效果

### 示例 1：完全消除排序

**查询：**
```sql
SELECT * FROM t1 WHERE id = 100 ORDER BY id;
```

**优化前：**
即使 `id` 已经被约束为 100，优化器可能仍然保留 `ORDER BY id`，导致执行路径中包含排序操作（除非索引已被用于访问）。

**优化后：**
`id` 被识别为常量，`ORDER BY` 列表被清空。查询不再执行排序。

### 示例 2：部分消除

**查询：**
```sql
SELECT * FROM t1 WHERE category = 'A' ORDER BY category, create_time;
```

**优化后：**
`category` 是常量，被移除。`ORDER BY` 简化为 `ORDER BY create_time`。这减少了排序键的长度，可能提升排序性能。

## 安全性

该优化依赖于 `Item::const_item()` 的正确性。由于它在 `optimize_cond` 之后执行，此时所有能够被推导为常量的字段（const tables, eq_ref, const expressions）都已被正确标记。移除这些常量排序项不会改变结果集的顺序语义。
