let $have_pq = `select @@txsql_max_parallel_worker_threads`;
if ($have_pq > 0) {
  --disable_query_log
  --eval set @@txsql_parallel_degree=$saved_parallel_degree;
  --enable_query_log
}