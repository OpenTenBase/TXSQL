
--let $value = query_get_value(show variables like 'innodb_backquery_enable', Value, 1)
if ($value == No such row)
{
  --skip Test need backquery
}