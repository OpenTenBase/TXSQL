let $have_txsql = `SELECT VERSION() LIKE '%txsql%'`;
if (!$have_txsql)
{
  --skip Test not supported in cynos
}