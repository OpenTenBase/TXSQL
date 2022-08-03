# Testing of table sample that uses DEBUG functionality.
#
# Just for coverage.
#
set debug="+d,set_filter_effect_COND_FILTER_STALE";
select * from t1 tablesample (1.1) repeatable(1) left join t2 on t1.a=t2.a;
ERROR 42000: This version of MySQL doesn't yet support 'table sample with join'
set debug="-d,set_filter_effect_COND_FILTER_STALE";
set debug="+d,filesort_force_sort_row_ids";
set debug="+d,table_sample_force_rescan";
select * from t2 tablesample (1.1) repeatable(1) order by t2.b;
a	b
939	61
929	71
911	89
904	96
891	109
882	118
858	142
855	145
847	153
791	209
768	232
745	255
581	419
340	660
317	683
278	722
265	735
203	797
102	898
set debug="-d,table_sample_force_rescan";
set debug="-d,filesort_force_sort_row_ids";
