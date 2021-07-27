delimiter $$ 
create procedure create_table_rewriter_function()
        begin
        declare have_table_rewriter_plugin int;
        select count(*) into have_table_rewriter_plugin from information_schema.plugins where plugin_name='table_rewriter';
        if have_table_rewriter_plugin = 1 then
		create function load_table_rewriter_rules returns string soname 'table_rewriter.so';
        end if;
end $$ 
delimiter ;
drop function if exists load_table_rewriter_rules;
call create_table_rewriter_function();
drop procedure create_table_rewriter_function;
