# --include save_and_set_px.inc
create table t4 (
companynr tinyint(2) unsigned zerofill NOT NULL default '00',
companyname char(30) NOT NULL default '',
PRIMARY KEY (companynr),
UNIQUE KEY companyname(companyname)
) MAX_ROWS=50 PACK_KEYS=1 COMMENT='companynames';
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
analyze table t4;
Table	Op	Msg_type	Msg_text
test.t4	analyze	status	OK
explain format=tree select distinct 1 from t4 group by companynr;
EXPLAIN
-> Limit: 1 row(s)  (cost=1.45 rows=1)
    -> PX Receiver (slice: 0; workers: 1)
        -> PX Sender (slice: 1; workers: 4)
            -> Table scan on <temporary>
                -> Temporary table  (cost=1.45 rows=12)
                    -> Parallel index scan on t4 using companyname  (cost=1.45 rows=12)

select distinct 1 from t4 group by companynr;
1
1
drop table t4;
# --include set_px_to_saved.inc
