--echo # ----------------------------------------------------------------------
--echo # Test of aggregate function GROUP_CONCAT
--echo # ----------------------------------------------------------------------
create table t2(j json COMPRESSED);
insert into t2 values (json_array(ST_GeomFromText('LineString(0 0, 1 1, 2 2)')));
select group_concat(j order by j) from t2;
drop table t2;
