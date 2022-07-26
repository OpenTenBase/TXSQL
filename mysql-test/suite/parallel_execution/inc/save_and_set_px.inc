--disable_query_log
SET @saved_txsql_max_parallel_worker_threads = @@txsql_max_parallel_worker_threads;
SET @saved_txsql_parallel_table_record_threshold = @@txsql_parallel_table_record_threshold;
SET @saved_txsql_parallel_cost_threshold = @@txsql_parallel_cost_threshold;
SET @saved_optimizer_context_max_mem_size = @@txsql_optimizer_context_max_mem_size;
# set global so as to be inherited by new connections
SET global txsql_max_parallel_worker_threads = 8;
SET global txsql_parallel_table_record_threshold = 0;
SET global txsql_parallel_cost_threshold = 0;
SET global debug = '+d,disable_debug_sync_check';
# set session to make it effective in the current connection
SET txsql_parallel_table_record_threshold = 0;
SET txsql_parallel_cost_threshold = 0;
SET debug = '+d,disable_debug_sync_check';
--enable_query_log
--echo # --include save_and_set_px.inc
