if (`SELECT @@global.px_max_parallel_threads > 0`)
{
  --skip Test requires: parallel execution disabled
}
