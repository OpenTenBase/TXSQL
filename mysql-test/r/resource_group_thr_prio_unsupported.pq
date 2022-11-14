# Resource group tests with thread priority disabled.
# Create Resource Group Tests
# 1. Invalid Resource Group Name.
CREATE RESOURCE GROUP 'invalid_utf8¿ÿres' TYPE=USER VCPU=1-3, 1;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near ''invalid_utf8¿ÿres' TYPE=USER VCPU=1-3, 1' at line 1
# 2. Invalid Resource Group Type
CREATE RESOURCE GROUP test TYPE=INVALID_TYPE VCPU=3-1, 0;
ERROR 42000: You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'INVALID_TYPE VCPU=3-1, 0' at line 1
# 3. Invalid VCPU ID and VCPU Range
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=MAX_CPU_NUM;
ERROR HY000: Invalid cpu id MAX_CPU_NUM
# 3.1 Invalid VCPU Range.
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=3-1;
ERROR HY000: Invalid VCPU range 3-1
# 4. Invalid Value of thread priority.
# 5. Invalid Value of thread priority for user resource group type.
# 5.1 Thread priority ignored warning.
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3 THREAD_PRIORITY=-5;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
SHOW WARNINGS;
Level	Code	Message
Warning	3659	Attribute thread_priority is ignored (using default value).
DROP RESOURCE GROUP user_resource;
# 6. Create a valid user resource group.
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3;
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME="user_resource";
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
user_resource	USER	1	1-3	0
# 7. Resource group already exists error.
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3 THREAD_PRIORITY=5;
ERROR HY000: Resource Group 'user_resource' exists
# 7.1 Case sensitivity of RESOURCE GROUP.
CREATE RESOURCE GROUP USER_resource TYPE=USER;
ERROR HY000: Resource Group 'USER_resource' exists
DROP RESOURCE GROUP user_resource;
# 8. Privilege Error - user with default privileges can't create resource group.
CREATE USER u1;
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3 THREAD_PRIORITY=5;
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
# 9. Privilege Error  - user with  RESOURCE_GROUP_USER privilege can't create resource` group.
GRANT RESOURCE_GROUP_USER ON *.* TO u1;
DROP USER u1;
# 10. Length of resource group name is greater than maximum allowed limit of identifier names.
CREATE RESOURCE GROUP user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaafffffffff TYPE=USER;
ERROR 42000: Identifier name 'user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' is too long
# ALTER resource group tests
# 1. Alter on non-existent resource group.
ALTER RESOURCE GROUP user_resource VCPU=3;
ERROR HY000: Resource Group 'user_resource' does not exist.
# 2. Alter with a invalid CPU ID
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3 THREAD_PRIORITY=5;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
ALTER RESOURCE GROUP user_resource VCPU=MAX_CPU_NUM;
ERROR HY000: Invalid cpu id MAX_CPU_NUM
# 3. Alter with invalid CPU range.
ALTER RESOURCE GROUP user_resource VCPU=3-1;
ERROR HY000: Invalid VCPU range 3-1
# 4. Alter with invalid thread_priority
# 5. Alter with invalid thread priority for a user resource group.
ALTER RESOURCE GROUP user_resource THREAD_PRIORITY=-5;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
# 6. DISABLE and ENABLE the resource group.
ALTER RESOURCE GROUP user_resource DISABLE;
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME="user_resource";
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
user_resource	USER	0	1-3	0
ALTER RESOURCE GROUP user_resource ENABLE;
# 7. Test FORCE option of ALTER
SET RESOURCE GROUP user_resource;
SELECT COUNT(*) = 1 FROM performance_schema.threads WHERE RESOURCE_GROUP="user_resource";
COUNT(*) = 1
1
ALTER RESOURCE GROUP user_resource FORCE;
ERROR HY000: Option FORCE invalid as DISABLE option is not specified.
ALTER RESOURCE GROUP user_resource DISABLE;
ALTER RESOURCE GROUP user_resource DISABLE FORCE;
SELECT COUNT(*) = 0 FROM performance_schema.threads WHERE RESOURCE_GROUP="user_resource";
COUNT(*) = 0
1
# 8. Privilege Error - user with default privileges can't alter resource group.
CREATE USER u1;
ALTER RESOURCE GROUP user_resource VCPU=0 THREAD_PRIORITY=5;
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
# 9. Privilege Error  - user with  RESOURCE_GROUP_USER privilege can't alter resource group.
GRANT RESOURCE_GROUP_USER ON *.* TO u1;
DROP USER u1;
# 10. Length of resource group name is greater than maximum allowed limit of identifier names.
ALTER RESOURCE GROUP user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaafffffffff DISABLE;
ERROR 42000: Identifier name 'user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' is too long
# 11. Default resource groups can't be altered.
ALTER RESOURCE GROUP SYS_default VCPU=0;
ERROR HY000: Alter operation is disallowed on default resource groups.
ALTER RESOURCE GROUP USR_default VCPU=1;
ERROR HY000: Alter operation is disallowed on default resource groups.
# 12. Alter VCPU IDS and/or thread priority
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME='user_resource';
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
user_resource	USER	0	1-3	0
ALTER RESOURCE GROUP user_resource VCPU=0,2;
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME='user_resource';
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
user_resource	USER	0	0,2	0
# DROP resource group tests.
# 1. DROP on a non-existent resource group.
DROP RESOURCE GROUP user_resource1;
ERROR HY000: Resource Group 'user_resource1' does not exist.
# 2. DROP an existing resource group.
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME="user_resource";
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
user_resource	USER	0	0,2	0
DROP RESOURCE GROUP user_resource;
# 3. Test FORCE option of resource group.
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3 THREAD_PRIORITY=7;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
SET RESOURCE GROUP user_resource;
DROP RESOURCE GROUP user_resource;
ERROR HY000: Resource group user_resource is busy.
SELECT COUNT(*) = 1 FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME="user_resource";
COUNT(*) = 1
1
DROP RESOURCE GROUP user_resource FORCE;
SELECT COUNT(*) = 0 FROM INFORMATION_SCHEMA.RESOURCE_GROUPS WHERE RESOURCE_GROUP_NAME="user_resource";
COUNT(*) = 0
1
# 4. Privilege Error - user with RESOURCE_GROUP_USER privilege can't drop resource group.
CREATE USER u1;
GRANT RESOURCE_GROUP_USER ON *.* TO u1;
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3;
DROP RESOURCE GROUP user_resource;
DROP USER u1;
# 5. Length of resource group name is greater than maximum allowed limit of identifier names.
DROP RESOURCE GROUP user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaafffffffff;
ERROR 42000: Identifier name 'user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' is too long
# 6. Default resource groups can't be dropped.
DROP RESOURCE GROUP USR_default;
ERROR HY000: Drop operation  operation is disallowed on default resource groups.
DROP RESOURCE GROUP SYS_default;
ERROR HY000: Drop operation  operation is disallowed on default resource groups.
# SET resource group tests.
# 1. Assigning a non-existing resource group.
SET RESOURCE GROUP user_resource;
ERROR HY000: Resource Group 'user_resource' does not exist.
# 2. SET current session to a resource group.
CREATE RESOURCE GROUP user_resource TYPE=USER VCPU=1-3 THREAD_PRIORITY=7;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
SET RESOURCE GROUP user_resource;
SELECT COUNT(*) = 1 FROM performance_schema.threads WHERE RESOURCE_GROUP="user_resource";
COUNT(*) = 1
1
# 3. Assign a system thread to a user resource group.
SET RESOURCE GROUP user_resource FOR pfs_system_thread_id;;
ERROR HY000: Unable to bind resource group user_resource with thread id (pfs_system_thread_id).(Resource group type and thread type doesn't match.).
# 4. Assign a system thread to system resource group.
CREATE RESOURCE GROUP system_resource TYPE=SYSTEM VCPU=0-2 THREAD_PRIORITY=-1;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
SET RESOURCE GROUP system_resource FOR pfs_system_thread_id;;
SELECT THREAD_ID, NAME, RESOURCE_GROUP FROM performance_schema.threads WHERE THREAD_ID=pfs_system_thread_id;;
THREAD_ID	NAME	RESOURCE_GROUP
pfs_system_thread_id	thread/innodb/io_write_thread	system_resource
# 5. Assign a group of threads to a resource group.
SET RESOURCE GROUP system_resource FOR pfs_system_thread_id1, pfs_system_thread_id2;;
SELECT COUNT(*) = 2 FROM performance_schema.threads WHERE THREAD_ID IN (pfs_system_thread_id1, pfs_system_thread_id2);;
COUNT(*) = 2
1
# 6. Privilege Tests
CREATE USER u1;
SET RESOURCE GROUP user_resource;
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
GRANT SUPER ON *.* TO u1;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
FLUSH PRIVILEGES;
SET RESOURCE GROUP user_resource;
SET RESOURCE GROUP system_resource;
ERROR HY000: Unable to bind resource group system_resource with thread id ID.(System resource group can't be applied to user thread.).
DROP USER u1;
# 7. SET RESOURCE GROUP on invalid thread id will give warning.
SET RESOURCE GROUP user_resource FOR 77777;
ERROR HY000: Invalid thread id (77777).
# 8. Disable resource group and assign to the current session - This should give error.
ALTER RESOURCE GROUP user_resource DISABLE;
SET RESOURCE GROUP user_resource;
ERROR HY000: Resource group user_resource is disabled.
DROP RESOURCE GROUP user_resource FORCE;
# 9. Length of resource group name is greater than maximum allowed limit of identifier names.
SET RESOURCE GROUP user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaafffffffff;
ERROR 42000: Identifier name 'user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' is too long
# 10. Do not allow a system resource group to be assigned to user thread.
SET RESOURCE GROUP SYS_default;
ERROR HY000: Unable to bind resource group SYS_default with thread id ID.(System resource group can't be applied to user thread.).
# Query Hint Tests
# 1. Hint on non-existent Resource Group.
SELECT /*+ RESOURCE_GROUP(rg) */ 1;
1
1
Warning	3651	Resource Group 'rg' does not exist.
Warnings:
# 2. Using system resource group in hint for a non-privilege user should give warning
CREATE USER u1;
GRANT SUPER on *.* to u1;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
SELECT /*+ RESOURCE_GROUP(system_resource) */ 1;
1
1
Warning	3661	Unable to bind resource group system_resource with thread id ID.(System resource group can't be bound with a session thread).
Warnings:
DROP USER u1;
# 3. Length of resource group name is greater than maximum allowed limit of identifier names.
#    length shall be truncated in this case and warning output to user.
SELECT /*+ RESOURCE GROUP (user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaffffffff#f) */ 1;
1
1
Warning	1064	Optimizer hint syntax error near 'RESOURCE GROUP (user_resource_1234567890aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa' at line 1
Warnings:
# Information Schema and Privilege related tests.
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS;
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
SYS_default	SYSTEM	1	NUM_VCPUS	0
USR_default	USER	1	NUM_VCPUS	0
system_resource	SYSTEM	1	NUM_VCPUS	0
CREATE USER u1;
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS;
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
GRANT SUPER ON *.* TO u1;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
FLUSH PRIVILEGES;
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS;
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
SYS_default	SYSTEM	1	NUM_VCPUS	0
USR_default	USER	1	NUM_VCPUS	0
system_resource	SYSTEM	1	NUM_VCPUS	0
GRANT SUPER ON *.* TO u1;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
FLUSH PRIVILEGES;
SELECT * FROM INFORMATION_SCHEMA.RESOURCE_GROUPS;
RESOURCE_GROUP_NAME	RESOURCE_GROUP_TYPE	RESOURCE_GROUP_ENABLED	VCPU_IDS	THREAD_PRIORITY
SYS_default	SYSTEM	1	NUM_VCPUS	0
USR_default	USER	1	NUM_VCPUS	0
system_resource	SYSTEM	1	NUM_VCPUS	0
DROP USER u1;
DROP RESOURCE GROUP system_resource FORCE;
# BUG 27638623 - DEBUG ASSERT WHEN EXECUTING CREATE RESOURCE GROUP
#                IN STORED FUNCTION
CREATE FUNCTION func() RETURNS INT
BEGIN
CREATE RESOURCE GROUP Batch TYPE = USER VCPU = 2-3;
RETURN 0;
END|
ERROR HY000: Explicit or implicit commit is not allowed in stored function or trigger.
CREATE RESOURCE GROUP Batch TYPE = USER VCPU = 2-3|
CREATE FUNCTION func() RETURNS INT
BEGIN
SET RESOURCE GROUP Batch;
RETURN 0;
END|
SELECT func();
func()
0
SELECT COUNT(*) = 1 FROM performance_schema.threads WHERE RESOURCE_GROUP="Batch";
COUNT(*) = 1
1
DROP RESOURCE GROUP Batch FORCE;
DROP FUNCTION func;
CREATE RESOURCE GROUP test TYPE = USER VCPU=2-3;
SET @rg_name='test';
SET @cmd = CONCAT("SET RESOURCE GROUP ", @rg_name);
SELECT @cmd;
@cmd
SET RESOURCE GROUP test
PREPARE stmt FROM @cmd;
EXECUTE stmt;
SELECT COUNT(*) = 1 FROM performance_schema.threads WHERE RESOURCE_GROUP="test";
COUNT(*) = 1
1
DROP RESOURCE GROUP test FORCE;
# 1. Create three users with different privileges.
CREATE USER u1;
CREATE USER u2;
CREATE USER u3;
GRANT SUPER ON *.* TO u1;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
GRANT SUPER ON *.* TO u2;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
FLUSH PRIVILEGES;
# 2. Create a user resource group and with new IO/memory item specified.
CREATE RESOURCE GROUP res_grp1 TYPE=USER VCPU=1,3-4 THREAD_PRIORITY=1 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 THREAD_IOPS_MAX_PERCENT=50 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
# 3. Create a system resource group. Can not specify IO/Memory items of system resource group.
CREATE RESOURCE GROUP res_grp2 TYPE=SYSTEM VCPU=1,3-4 THREAD_PRIORITY=-1 IOPS_LIMIT=1000 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
CREATE RESOURCE GROUP res_grp2 TYPE=SYSTEM VCPU=1,3-4 THREAD_PRIORITY=-1 MEMORY_LIMIT=30000000 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
CREATE RESOURCE GROUP res_grp2 TYPE=SYSTEM VCPU=1,3-4 THREAD_PRIORITY=-1 THREAD_IOPS_MAX_PERCENT=50 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
CREATE RESOURCE GROUP res_grp2 TYPE=SYSTEM VCPU=1,3-4 THREAD_PRIORITY=-1 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
CREATE RESOURCE GROUP res_grp2 TYPE=SYSTEM VCPU=1,3-4 THREAD_PRIORITY=-1 ENABLE;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
# 4. Modify the new IO and memory items of resource group and check modify resource group privileges for different users.
## 4.1 Cannot modify system resource group even if got SUPER or SUPER privilege.
ALTER RESOURCE GROUP res_grp2 THREAD_PRIORITY=-1 IOPS_LIMIT=2000 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
ALTER RESOURCE GROUP res_grp2 THREAD_PRIORITY=-1 MEMORY_LIMIT=4000000 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
ALTER RESOURCE GROUP res_grp2 THREAD_PRIORITY=-1 THREAD_IOPS_MAX_PERCENT=40 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
ALTER RESOURCE GROUP res_grp2 THREAD_PRIORITY=-1 THREAD_MEMORY_MAX_PERCENT=60 ENABLE;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
## 4.2 Only SUPER privileges can modify user resource group.
ALTER RESOURCE GROUP res_grp1 VCPU=1,3-4 THREAD_PRIORITY=1 IOPS_LIMIT=2000 MEMORY_LIMIT=4000000 THREAD_IOPS_MAX_PERCENT=40 THREAD_MEMORY_MAX_PERCENT=60 ENABLE;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
ALTER RESOURCE GROUP res_grp1 VCPU=2,3-4 IOPS_LIMIT=3000 MEMORY_LIMIT=4000000 ENABLE;
ALTER RESOURCE GROUP res_grp1 THREAD_IOPS_MAX_PERCENT=41 THREAD_MEMORY_MAX_PERCENT=81 ENABLE;
# 5. Set user connection to user resource group.
## 5.1 Failed due to resource group overrun.
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=500 ENABLE;
SET RESOURCE GROUP res_grp1;
ERROR HY000: New resource group res_grp1 exceed iops or memory limit.
## 5.2 Succeed in setting after altering resource group.
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=5000000 ENABLE;
SET RESOURCE GROUP res_grp1;
SET RESOURCE GROUP res_grp1;
## 5.3 Failed due to insufficient permissions.
SET RESOURCE GROUP res_grp1;
ERROR 42000: Access denied; you need (at least one of) the SUPER privilege(s) for this operation
## 5.4 Set user to resource group.
SET RESOURCE GROUP res_grp1 for USER u1;
## 5.5 Remove the user from resource group.
SET RESOURCE GROUP USR_default for USER u1;
# 6. Test the behavior when the resource_groups_extra table is modified.
call mtr.add_suppression("Can not find the field in the resource_groups_extra table");
call mtr.add_suppression("Error in open and lock the resource_groups_extra table");
call mtr.add_suppression("Null pointer error found when closing the resource_groups_extra table");
## 6.1 Test the behavior after deleting a column
ALTER TABLE mysql.resource_groups_extra DROP COLUMN iops_limit;
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=1000000 ENABLE;
ERROR HY000: Failed to write or update resource_group_extra table.
ALTER TABLE mysql.resource_groups_extra ADD COLUMN `iops_limit` BIGINT UNSIGNED NOT NULL;
ALTER TABLE mysql.resource_groups_extra DROP COLUMN memory_limit;
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=1000000 ENABLE;
ERROR HY000: Failed to write or update resource_group_extra table.
ALTER TABLE mysql.resource_groups_extra ADD COLUMN `memory_limit` BIGINT UNSIGNED NOT NULL;
ALTER TABLE mysql.resource_groups_extra DROP COLUMN thread_iops_max_percent;
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=1000000 ENABLE;
ERROR HY000: Failed to write or update resource_group_extra table.
ALTER TABLE mysql.resource_groups_extra ADD COLUMN `thread_iops_max_percent` INT UNSIGNED NOT NULL;
ALTER TABLE mysql.resource_groups_extra DROP COLUMN thread_memory_max_percent;
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=1000000 ENABLE;
ERROR HY000: Failed to write or update resource_group_extra table.
ALTER TABLE mysql.resource_groups_extra ADD COLUMN `thread_memory_max_percent` INT UNSIGNED NOT NULL;
## 6.2 Create and alter resource group after dropping table.
DROP TABLE mysql.resource_groups_extra;
CREATE RESOURCE GROUP res_grp3 TYPE=USER VCPU=1,3-4 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 THREAD_IOPS_MAX_PERCENT=50 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
ERROR 42S02: Table 'mysql.resource_groups_extra' doesn't exist
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=1000000 ENABLE;
ERROR 42S02: Table 'mysql.resource_groups_extra' doesn't exist
## 6.3 Create a new table with extra column and test its behavior.
USE mysql;
CREATE TABLE IF NOT EXISTS resource_groups_extra (
Resource_group_name varchar(64) NOT NULL,
Resource_group_type enum('SYSTEM','USER') COLLATE utf8_bin NOT NULL,
Resource_group_enabled tinyint(1) NOT NULL,
Cpu_id_mask varchar(1024) COLLATE utf8_bin NOT NULL,
Thread_priority int NOT NULL,
Iops_limit bigint unsigned NOT NULL,
Memory_limit bigint unsigned NOT NULL,
Thread_iops_max_percent int unsigned NOT NULL,
Thread_memory_max_percent int unsigned NOT NULL,
Sql_timeout int unsigned NOT NULL,
Timeout_action varchar(64) NOT NULL,
PRIMARY KEY(Resource_group_name)
) ENGINE=InnoDB STATS_PERSISTENT=0 CHARACTER SET utf8 COLLATE utf8_bin comment='Extra information of resource group' ROW_FORMAT=DYNAMIC TABLESPACE=mysql;
Warnings:
Warning	3778	'utf8mb3_bin' is a collation of the deprecated character set UTF8MB3. Please consider using UTF8MB4 with an appropriate collation instead.
Warning	1681	Integer display width is deprecated and will be removed in a future release.
Warning	3778	'utf8mb3_bin' is a collation of the deprecated character set UTF8MB3. Please consider using UTF8MB4 with an appropriate collation instead.
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
Warning	3778	'utf8mb3_bin' is a collation of the deprecated character set UTF8MB3. Please consider using UTF8MB4 with an appropriate collation instead.
SELECT * FROM mysql.resource_groups_extra;
Resource_group_name	Resource_group_type	Resource_group_enabled	Cpu_id_mask	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Sql_timeout	Timeout_action
## 6.4 Add record to new mysql.resource_groups_extra table when alter res_grp;
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=1000000 ENABLE;
ALTER RESOURCE GROUP res_grp2 VCPU=1;
SELECT * FROM mysql.resource_groups_extra;
Resource_group_name	Resource_group_type	Resource_group_enabled	Cpu_id_mask	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Sql_timeout	Timeout_action
res_grp1	SYSTEM	0		0	3000	1000000	41	81	0	NONE
res_grp2	SYSTEM	0		0	0	0	0	0	0	NONE
## 6.5 Create new resource group after mysql.resource_groups_extra is modified.
CREATE RESOURCE GROUP res_grp3 TYPE=USER VCPU=1,3-4 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 THREAD_IOPS_MAX_PERCENT=50 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
SELECT * FROM mysql.resource_groups_extra;
Resource_group_name	Resource_group_type	Resource_group_enabled	Cpu_id_mask	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Sql_timeout	Timeout_action
res_grp1	SYSTEM	0		0	3000	1000000	41	81	0	NONE
res_grp2	SYSTEM	0		0	0	0	0	0	0	NONE
res_grp3	SYSTEM	0		0	1000	30000000	50	50	0	NONE
## 6.6 Drop necessary column of mysql.resource_groups_table.
ALTER TABLE mysql.resource_groups_extra DROP COLUMN resource_group_type;
ALTER TABLE mysql.resource_groups_extra DROP COLUMN resource_group_enabled;
ALTER TABLE mysql.resource_groups_extra DROP COLUMN cpu_id_mask;
ALTER TABLE mysql.resource_groups_extra DROP COLUMN thread_priority;
DROP RESOURCE GROUP res_grp3;
# 7. Check resource group status by 'show resource group status'.
SHOW RESOURCE GROUP STATUS;
Resource_group_name	Type	Enabled	Vcpu_ids	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Current_iops	Current_memory	Sql_Timeout	Timeout_action
res_grp2	System	1	1	#	#	#	#	#	#	#	0	NONE
res_grp1	User	1	2,3:4	#	#	#	#	#	#	#	0	NONE
SYS_default	System	1	NULL	#	#	#	#	#	#	#	0	
USR_default	User	1	NULL	#	#	#	#	#	#	#	0	
# 8. Remove thread from resource group.
SET RESOURCE GROUP USR_default;
SET RESOURCE GROUP USR_default;
SHOW RESOURCE GROUP STATUS;
Resource_group_name	Type	Enabled	Vcpu_ids	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Current_iops	Current_memory	Sql_Timeout	Timeout_action
res_grp2	System	1	1	0	0	0	0	0	0	0	0	NONE
res_grp1	User	1	2,3:4	0	3000	1000000	41	81	0	0	0	NONE
SYS_default	System	1	NULL	0	0	0	0	0	0	0	0	
USR_default	User	1	NULL	0	0	0	0	0	0	0	0	
## 8.2 After clearing the thread of the resource group, the resource group can be set at will.
ALTER RESOURCE GROUP res_grp1 IOPS_LIMIT=2 MEMORY_LIMIT=5  ENABLE;
SHOW RESOURCE GROUP STATUS;
Resource_group_name	Type	Enabled	Vcpu_ids	Thread_priority	Iops_limit	Memory_limit	Thread_iops_max_percent	Thread_memory_max_percent	Current_iops	Current_memory	Sql_Timeout	Timeout_action
res_grp2	System	1	1	0	0	0	0	0	0	0	0	NONE
res_grp1	User	1	2,3:4	0	2	5	41	81	0	0	0	NONE
SYS_default	System	1	NULL	0	0	0	0	0	0	0	0	
USR_default	User	1	NULL	0	0	0	0	0	0	0	0	
# 9. Move resource group by hints.
call mtr.add_suppression("failed to move to resource group res_grp1 from USR_default");
## 9.1 Reset resource group.
ALTER RESOURCE GROUP res_grp1 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 ENABLE;
SELECT /*+ RESOURCE_GROUP(res_grp1) */ 1;
1
1
## 9.2 Failed due to resource group overrun.
ALTER RESOURCE GROUP res_grp1 IOPS_LIMIT=1 MEMORY_LIMIT=5  ENABLE;
SELECT /*+ RESOURCE_GROUP(res_grp1) */ 1;
1
1
DATE_TIME [Warning] [MY-013942] [Server] CDB: failed to move to resource group res_grp1 from USR_default
# 10. Check the behavior of the thread after the resource group exceeds limit.
call mtr.add_suppression("milliseconds due to iops control of resource group");
## 10.1 iops test
ALTER RESOURCE GROUP res_grp1 MEMORY_LIMIT=50000000;
SET RESOURCE GROUP res_grp1;
USE INFORMATION_SCHEMA;
## 10.2 The memory limit test is handed over to manual testing
# 11. Behavior under the thread pool.
## 11.1 Can not modify thread_handling when new resource group was created
SET GLOBAL THREAD_HANDLING= 2;
ERROR HY000: Because the resource group is used, the thread_handling modification failed, please clear the self-created resource group.
SET RESOURCE GROUP USR_default;
SET RESOURCE GROUP USR_default;
DROP RESOURCE GROUP res_grp1;
DROP RESOURCE GROUP res_grp2;
SET GLOBAL THREAD_HANDLING= 2;
## 11.2 Can not use resource group when thread_pool is enabled
SET RESOURCE GROUP USR_DEFAULT;
ERROR HY000: Feature Resource Groups is unsupported (Thread pool plugin enabled).
CREATE RESOURCE GROUP res_grp1 TYPE=USER VCPU=1,3-4 THREAD_PRIORITY=1 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 THREAD_IOPS_MAX_PERCENT=50 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
ERROR HY000: Feature Resource Groups is unsupported (Thread pool plugin enabled).
## 11.3 Resource group is enabled again when thread_pool is diabled.
SET GLOBAL THREAD_HANDLING= 0;
CREATE RESOURCE GROUP res_grp1 TYPE=USER VCPU=1,3-4 THREAD_PRIORITY=1 IOPS_LIMIT=1000 MEMORY_LIMIT=30000000 THREAD_IOPS_MAX_PERCENT=50 THREAD_MEMORY_MAX_PERCENT=50 ENABLE;
Warnings:
Warning	3659	Attribute thread_priority is ignored (using default value).
SET RESOURCE GROUP res_grp1;
SET RESOURCE GROUP USR_default;
DROP RESOURCE GROUP res_grp1;
## 12 Create resource group tests for io and memory limit.
# 12.1. Invalid memory limit percent
CREATE RESOURCE GROUP user_resource TYPE=USER THREAD_MEMORY_MAX_PERCENT=200;
ERROR HY000: Invalid thread_memory_max_percent value 200 for User resource group user_resource. Allowed range is [1, 100].
# 12.2. Invalid iops limit percent
CREATE RESOURCE GROUP user_resource TYPE=USER THREAD_IOPS_MAX_PERCENT=200;
ERROR HY000: Invalid thread_iops_max_percent value 200 for User resource group user_resource. Allowed range is [1, 100].
# 12.3. limit for system resource group.
CREATE RESOURCE GROUP sys_resource TYPE=SYSTEM THREAD_IOPS_MAX_PERCENT=200;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
# 12.4. Correct usage.
CREATE RESOURCE GROUP user_resource TYPE=USER MEMORY_LIMIT=99999999;
## 13 Alter resource group tests for io and memory limit.
# 13.1. Invalid memory limit
ALTER RESOURCE GROUP user_resource MEMORY_LIMIT=0;
ERROR HY000: Invalid memory_limit value 0 for User resource group user_resource. Allowed range is [1, 18446744073709551615].
# 13.2. Invalid iops limit
ALTER RESOURCE GROUP user_resource IOPS_LIMIT=0;
ERROR HY000: Invalid iops_limit value 0 for User resource group user_resource. Allowed range is [1, 18446744073709551615].
# 13.3. Invalid memory limit percent
ALTER RESOURCE GROUP user_resource THREAD_MEMORY_MAX_PERCENT=200;
ERROR HY000: Invalid thread_memory_max_percent value 200 for User resource group user_resource. Allowed range is [1, 100].
# 13.4. Invalid iops limit percent
ALTER RESOURCE GROUP user_resource THREAD_IOPS_MAX_PERCENT=200;
ERROR HY000: Invalid thread_iops_max_percent value 200 for User resource group user_resource. Allowed range is [1, 100].
# 13.5. limit for system resource group.
CREATE RESOURCE GROUP sys_resource TYPE=SYSTEM;
ALTER RESOURCE GROUP sys_resource THREAD_IOPS_MAX_PERCENT=200;
ERROR HY000: Alter IOPS/MEMORY items of system resource group is disallowed.
DROP RESOURCE GROUP sys_resource;
set resource group USR_default;
# 14 test for system resource group applied to user account.
CREATE RESOURCE GROUP sys_resource TYPE=SYSTEM;
SET RESOURCE GROUP sys_resource FOR USER root;
ERROR HY000: System resource group can't be applied to user account.
DROP RESOURCE GROUP sys_resource;
# 15 Cleanup
REVOKE SUPER ON *.* FROM u1;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
DROP USER u1;
REVOKE SUPER ON *.* FROM u2;
Warnings:
Warning	1287	The SUPER privilege identifier is deprecated
DROP USER u2;
DROP USER u3;
