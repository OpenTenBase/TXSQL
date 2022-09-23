#
# set optimizer_switch='sort_merge_join=off' to skip sort_merge_join iterator
#
--disable_query_log
SET session optimizer_switch='sort_merge_join=off,hash_join=on';
--enable_query_log
