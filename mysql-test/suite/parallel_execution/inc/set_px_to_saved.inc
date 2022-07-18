--disable_query_log
SET global px_max_parallel_threads = @saved_px_max_parallel_threads;
SET global px_parallel_table_record_threshold = @saved_px_parallel_table_record_threshold;
SET global px_parallel_cost_threshold = @saved_px_parallel_cost_threshold;
SET global txsql_optimizer_context_max_mem_size = @saved_optimizer_context_max_mem_size;
SET global debug = '-d,disable_debug_sync_check';
--enable_query_log
--echo # --include set_px_to_saved.inc
