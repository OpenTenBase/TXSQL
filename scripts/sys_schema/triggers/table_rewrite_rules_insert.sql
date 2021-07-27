DROP TRIGGER IF EXISTS query_rewrite.add_rule;
CREATE DEFINER='mysql.sys'@'localhost' TRIGGER query_rewrite.add_rule AFTER INSERT ON query_rewrite.table_rewrite_rules FOR EACH ROW SET global table_rewriter_add_rule=CONCAT_WS(',', NEW.db, NEW.table_name, NEW.table_name_new);
