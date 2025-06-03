let $have_pq = `select @@txsql_max_parallel_worker_threads`;
if ($have_pq > 0) {
  --disable_query_log
  --let $saved_parallel_degree= `SELECT @@txsql_parallel_degree`
  set @@txsql_parallel_degree=1;
  --enable_query_log
}