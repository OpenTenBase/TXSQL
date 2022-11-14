call mtr.add_suppression("change buffer is not empty, instant modify index column is not allowed");
set session debug = "+d,cdb_simulate_change_buffer_not_empty";
create table t1(id int not null primary key,c1 int) engine = innodb;
alter table t1 modify column id bigint not null auto_increment,algorithm=instant;
ERROR HY000: Index column can't be modified with ALGORITHM=INSTANT when change buffer is not empty
show create table t1;
Table	Create Table
t1	CREATE TABLE `t1` (
  `id` int NOT NULL,
  `c1` int DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
CREATE TABLE `t2` (
`id` int NOT NULL,
`name` varchar(35) DEFAULT NULL,
`part` binary(10) DEFAULT NULL,
PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
PARTITION BY LIST (`id`)
(PARTITION r0 VALUES IN (1,5,9,13,17,21) ENGINE = InnoDB,
PARTITION r1 VALUES IN (2,6,10,14,18,22) ENGINE = InnoDB,
PARTITION r2 VALUES IN (3,7,11,15,19,23) ENGINE = InnoDB,
PARTITION r3 VALUES IN (4,8,12,16,20,24) ENGINE = InnoDB);
alter table t2 modify column id bigint not null auto_increment,algorithm=instant;
ERROR HY000: Index column can't be modified with ALGORITHM=INSTANT when change buffer is not empty
show create table t2;
Table	Create Table
t2	CREATE TABLE `t2` (
  `id` int NOT NULL,
  `name` varchar(35) DEFAULT NULL,
  `part` binary(10) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
/*!50100 PARTITION BY LIST (`id`)
(PARTITION r0 VALUES IN (1,5,9,13,17,21) ENGINE = InnoDB,
 PARTITION r1 VALUES IN (2,6,10,14,18,22) ENGINE = InnoDB,
 PARTITION r2 VALUES IN (3,7,11,15,19,23) ENGINE = InnoDB,
 PARTITION r3 VALUES IN (4,8,12,16,20,24) ENGINE = InnoDB) */
set session debug = "-d,cdb_simulate_change_buffer_not_empty";
drop table t1,t2;
