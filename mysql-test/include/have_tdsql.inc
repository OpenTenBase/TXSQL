let $have_tdsql = `select @@have_tdsql`;
if ($have_tdsql == 0) {
  --skip Need run with tdsql
}
