DROP PROCEDURE IF EXISTS recycle_bin_purge_table;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE recycle_bin_purge_table(
        IN recycle_table varchar(64)
    )
    SQL SECURITY INVOKER
    DETERMINISTIC
    MODIFIES SQL DATA
BEGIN
    set @contain_cdb_reycle = (select count(*) from information_schema.SCHEMATA where SCHEMA_NAME = '__cdb_recycle_bin__');
    set @recycle_bin_mode = (select @@recycle_bin_startup_mode);
    set @is_cdb = IF (@contain_cdb_reycle = 1 and @recycle_bin_mode <> "TXSQL", 1, 0);
    SET @recycle_bin_name = IF(@is_cdb = 0, "__txsql_recycle_bin__", "__cdb_recycle_bin__");
    SET @sys.tmp.recycle_bin_purge_table.sql = CONCAT('DROP TABLE `', @recycle_bin_name, '`.`', recycle_table, '`');
    PREPARE stmt_drop_table FROM @sys.tmp.recycle_bin_purge_table.sql;
    EXECUTE stmt_drop_table;
    DEALLOCATE PREPARE stmt_drop_table;
END $$

DELIMITER ;
