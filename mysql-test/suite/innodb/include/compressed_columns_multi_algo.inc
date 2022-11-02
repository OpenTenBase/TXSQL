# COMPRESSED => NON-COMPRESSED and NON-COMPRESSED => COMPRESSED
# four params to pass
# 1. $table_name
# 2. $column_name
# 3. $column_type
# 4. $algorithm_type

# create table
eval CREATE TABLE $table_name(
    id INT,
    $column_name $column_type algorithm = $algorithm_type
) ENGINE=InnoDB;

eval SHOW CREATE TABLE $table_name;

# insert some records
eval INSERT INTO $table_name VALUES(1, REPEAT('bar', 250));
eval INSERT INTO $table_name VALUES(2, REPEAT('foo', 260));
eval INSERT INTO $table_name VALUES(3, REPEAT('xyz', 300));

# select results
eval SELECT id, LEFT($column_name, 30), LENGTH($column_name) FROM $table_name;
eval SELECT id, LEFT($column_name, 30), LENGTH($column_name) FROM $table_name WHERE $column_name = REPEAT('foo', 260);
eval SELECT id, LEFT($column_name, 30), LENGTH($column_name) FROM $table_name WHERE $column_name not in (REPEAT('xyz', 300));

# update and select
eval UPDATE $table_name SET $column_name = REPEAT('opq', 100) WHERE id = 2;
eval SELECT id, LEFT($column_name, 30), LENGTH($column_name) FROM $table_name;
eval SELECT id, LEFT($column_name, 30), LENGTH($column_name) FROM $table_name WHERE $column_name = REPEAT('foo', 260);
eval SELECT id, LEFT($column_name, 30), LENGTH($column_name) FROM $table_name WHERE $column_name not in (REPEAT('opq', 100));

# clean up
eval DROP TABLE $table_name;