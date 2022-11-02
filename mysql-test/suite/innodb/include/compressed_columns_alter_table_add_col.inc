# include file for alter table tests
# four params to pass
# 1. $table_name
# 2. $new_column_name
# 3. $new_column_type
# 4. $alter_table_algorithm

# create original table
eval CREATE TABLE $table_name(
    id INT
) ENGINE=InnoDB;

# alter table with adding column
eval ALTER TABLE $table_name ADD $new_column_name $new_column_type COMPRESSED, ALGORITHM = $alter_table_algorithm;

# insert some records
eval INSERT INTO $table_name VALUES(1, REPEAT('bar', 300));
eval INSERT INTO $table_name VALUES(2, REPEAT('foo', 200));

# select results
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name;
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name = REPEAT('foo', 200);
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name not in (REPEAT('bar', 300));

# update records
eval UPDATE $table_name SET $new_column_name = REPEAT('abc', 100) WHERE id = 2;
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name;
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name = REPEAT('foo', 200);
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name not in (REPEAT('abc', 100));

# clean up
eval DROP TABLE $table_name;