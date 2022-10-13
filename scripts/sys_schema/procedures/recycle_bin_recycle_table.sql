DROP PROCEDURE IF EXISTS recycle_bin_recycle_table;

DELIMITER $$

CREATE DEFINER='mysql.sys'@'localhost' PROCEDURE recycle_bin_recycle_table(
        IN recycle_table varchar(64)
    )
    SQL SECURITY INVOKER
    DETERMINISTIC
    MODIFIES SQL DATA
BEGIN
    DECLARE v_db_name VARCHAR(64);
    DECLARE v_table_name VARCHAR(64);
    SELECT ORIGIN_SCHEMA INTO v_db_name
           FROM mysql.recycle_bin_info  WHERE TABLE_NAME = recycle_table;
    SELECT ORIGIN_TABLE INTO v_table_name
           FROM mysql.recycle_bin_info  WHERE TABLE_NAME = recycle_table;
    set @contain_cdb_reycle = (select count(*) from information_schema.SCHEMATA where SCHEMA_NAME = '__cdb_recycle_bin__');
    set @recycle_bin_mode = (select @@recycle_bin_startup_mode);
    set @is_cdb = IF (@contain_cdb_reycle = 1 and @recycle_bin_mode <> "TXSQL", 1, 0);
    SET @recycle_bin_name = IF(@is_cdb = 0, "__txsql_recycle_bin__", "__cdb_recycle_bin__");
    SET @sys.tmp.recycle_bin_recycle_table.sql = CONCAT('RENAME TABLE `', @recycle_bin_name, '`.`',  recycle_table, '` TO `' , v_db_name, '`.`', v_table_name, '`');

    PREPARE stmt_recycle_table FROM @sys.tmp.recycle_bin_recycle_table.sql;
    EXECUTE stmt_recycle_table;
    DEALLOCATE PREPARE stmt_recycle_table;
END$$

DELIMITER ;
