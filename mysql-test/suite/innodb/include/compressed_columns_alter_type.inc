# COMPRESSED => NON-COMPRESSED and NON-COMPRESSED => COMPRESSED
# four params to pass
# 1. $table_name
# 2. $column_name
# 3. $original_column_type
# 4. $new_column_type

# create original table
eval CREATE TABLE $table_name(
    id INT,
    $column_name $original_column_type
) ENGINE=InnoDB;

# insert some records
eval INSERT INTO $table_name VALUES(1, REPEAT('bar', 250));
eval INSERT INTO $table_name VALUES(2, REPEAT('foo', 260));

# alter table
eval ALTER TABLE $table_name MODIFY $column_name $new_column_type;

# insert some records
eval INSERT INTO $table_name VALUES(3, REPEAT('abc', 200));
eval INSERT INTO $table_name VALUES(4, REPEAT('xyz', 300));

# select results
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name;
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name = REPEAT('foo', 260);
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name not in (REPEAT('xyz', 300));

# update and select
eval UPDATE $table_name SET $new_column_name = REPEAT('opq', 100) WHERE id = 2;
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name;
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name = REPEAT('foo', 260);
eval SELECT id, LEFT($new_column_name, 30), LENGTH($new_column_name) FROM $table_name WHERE $new_column_name not in (REPEAT('opq', 100));

# clean up
eval DROP TABLE $table_name;