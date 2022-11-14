#test for tencentroot status
#Show some tencent status variables with tencentroot
show status like 'Cdb_proxy_counts';
Variable_name	Value
Cdb_proxy_counts	0
#Show some tencent status variables with root, nothing should happen
show status like 'Cdb_proxy_counts';
Variable_name	Value
