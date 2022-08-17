let $have_pq = `select @@txsql_max_parallel_worker_threads`;
if ($have_pq > 0) {
  --skip Can't run with pq
}
