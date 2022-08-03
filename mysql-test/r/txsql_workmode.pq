CALL mtr.add_suppression('[txsql]:');
id
1
2
3
4
5
Variable_name	Value
--- READWRITE ---
Variable_name	Value
cdb_working_mode	READWRITE
----- switch to tencent_conn -----
Variable_name	Value
cdb_working_mode_default	READWRITE
cdb_working_mode_enabled	OFF
cdb_workmode_life_interval	10
----- Stage 1. switch to tencent_conn to switch to OFFLINE -----
USER
tencentroot
root
event_scheduler
USER
tencentroot
event_scheduler
--- OFFLINE ---
Variable_name	Value
cdb_working_mode	OFFLINE
----- switch to default -----
ERROR HY000: MySQL server has gone away
ERROR HY000: MySQL server has gone away
--- new connection test ---
ERROR HY000: The server is currently in offline mode
----- Stage 2. switch to tencent_conn to switch to READONLY -----
----- switch to default -----
--- READONLY ---
Variable_name	Value
cdb_working_mode	READONLY
id
1
2
3
4
5
ERROR HY000: The MySQL server is running with the root option so it cannot execute this statement
----- Stage 3. switch to tencent_conn to switch to READWRITE -----
----- switch to default -----
id
1
2
3
4
5
--- OFFLINE ---
Variable_name	Value
cdb_working_mode	OFFLINE
USER
tencentroot
event_scheduler
----- switch to default -----
ERROR HY000: MySQL server has gone away
ERROR HY000: MySQL server has gone away
----- switch to default -----
USER
root
event_scheduler
id
1
2
2
3
4
5
----- Stage 4. switch to tencent_conn to switch to NORMAL -----
USER
tencentroot
root
event_scheduler
----- switch to default -----
id
1
2
2
2
3
4
5
----- Stage 5. switch to tencent_conn to restore configuration -----
