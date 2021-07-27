SET @has_table_rewriter_plugin=(select count(*) from information_schema.plugins where plugin_name='table_rewriter');
SET @str = "CREATE TABLE IF NOT EXISTS query_rewrite.table_rewrite_rules (
     db VARCHAR(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
     table_name VARCHAR(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
     table_name_new VARCHAR(5000) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
     PRIMARY KEY (db, table_name)
) DEFAULT CHARSET = utf8mb4 ENGINE = INNODB";
SET @cmd=IF(@has_table_rewriter_plugin,@str,'set @dummy = 0');
prepare stmt from @cmd;
execute stmt;
drop prepare stmt;
