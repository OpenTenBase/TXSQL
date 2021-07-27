DROP TRIGGER IF EXISTS query_rewrite.del_rule;
CREATE DEFINER='mysql.sys'@'localhost' TRIGGER query_rewrite.del_rule AFTER DELETE ON query_rewrite.table_rewrite_rules FOR EACH ROW SET global table_rewriter_del_rule=CONCAT_WS(',', OLD.db, OLD.table_name);
