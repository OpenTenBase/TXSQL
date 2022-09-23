#
# set optimizer_switch='sort_merge_join=on' to use sort_merge_join iterator
#
--disable_query_log
SET session optimizer_switch='sort_merge_join=on,hash_join=off';
--enable_query_log
