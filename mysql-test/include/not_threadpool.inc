--disable_warnings
if (`SELECT count(*) FROM performance_schema.global_variables WHERE
      VARIABLE_NAME = 'THREAD_HANDLING' AND
      VARIABLE_VALUE in('loaded-dynamically','pool-of-threads')`){
  skip Test requires: 'not_threadpool';
}
--enable_warnings
