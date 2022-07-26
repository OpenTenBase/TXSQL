--disable_query_log
SET global txsql_max_parallel_worker_threads = @saved_txsql_max_parallel_worker_threads;
SET global txsql_parallel_table_record_threshold = @saved_txsql_parallel_table_record_threshold;
SET global txsql_parallel_cost_threshold = @saved_txsql_parallel_cost_threshold;
SET global txsql_optimizer_context_max_mem_size = @saved_optimizer_context_max_mem_size;
SET global debug = '-d,disable_debug_sync_check';
--enable_query_log
--echo # --include set_px_to_saved.inc
