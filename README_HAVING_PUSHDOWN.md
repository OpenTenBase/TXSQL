# SQL 改写优化：HAVING 条件下推到 WHERE

## 1. 优化背景

在 SQL 查询中，`HAVING` 子句通常用于过滤分组聚合后的结果，而 `WHERE` 子句用于过滤分组前的原始数据。然而，在实际应用中，用户（或自动生成的 SQL）可能会将一些不依赖于聚合结果的过滤条件写在 `HAVING` 中。

例如：
```sql
SELECT col1, SUM(col2)
FROM t1
GROUP BY col1
HAVING col1 > 100 AND SUM(col2) > 1000;
```

其中 `col1 > 100` 是一个非聚合条件。如果将其保留在 `HAVING` 中，数据库引擎必须先对所有数据进行分组聚合，然后再过滤 `col1 > 100` 的组。

如果将其重写为：
```sql
SELECT col1, SUM(col2)
FROM t1
WHERE col1 > 100
GROUP BY col1
HAVING SUM(col2) > 1000;
```
数据库引擎可以在扫描表时就利用 `col1 > 100` 进行过滤（甚至利用索引），从而显著减少参与聚合的数据量，提升查询性能。

## 2. 优化方案

在 `JOIN::optimize` 阶段的早期（在 `make_join_plan` 之前），实现一个自动改写逻辑：

1.  **识别**：遍历 `HAVING` 子句中的所有 AND 连接的条件。
2.  **判定**：检查每个条件是否满足下推标准：
    *   不包含聚合函数（`SUM`, `COUNT`, `AVG` 等）。
    *   不包含窗口函数。
    *   不包含 `ROLLUP` 相关表达式。
    *   不包含 `Item_ref`（即不引用 Select List 中的别名，因为 `WHERE` 执行时 Select List 可能尚未计算）。
3.  **下推**：将满足标准的条件从 `HAVING` 列表中移除，并添加到 `WHERE` 列表中。
4.  **重构**：重新构建 `HAVING` 和 `WHERE` 的条件树。

## 3. 代码实现

修改文件：`sql/sql_optimizer.cc`

### 新增函数

*   `contains_ref_item(Item *item)`: 递归检查条件表达式树中是否包含 `Item_ref`。这是为了安全性，防止将依赖于投影列别名的条件下推到 `WHERE`，因为 `WHERE` 子句执行时投影列可能尚未计算。
*   `push_having_to_where(THD *thd, JOIN *join)`: 执行核心的下推逻辑。

### 调用时机

在 `JOIN::optimize` 函数中，位于查询块的 `m_select_limit` 处理之后，且在 `optimize_cond`（针对 `WHERE` 的优化）之前。这样可以确保下推后的条件能够被后续的优化器逻辑（如常量传播、索引选择等）充分利用。

## 4. 预期效果

*   **减少聚合开销**：对于 TPC-DS 等 Benchmark 中的复杂查询，如果存在此类可下推的条件，将大幅减少 Hash Join 或 Group By 的输入数据量。
*   **索引利用**：原本在 `HAVING` 中的条件无法用于索引范围扫描，下推到 `WHERE` 后，优化器可以选择更优的索引访问路径。
*   **语义等价性**：由于只下推了不依赖聚合结果的条件，且排除了别名引用，保证了改写前后的语义严格一致。

