if (`SELECT @@global.txsql_max_parallel_worker_threads > 0`)
{
  --skip Test requires: parallel execution disabled
}
