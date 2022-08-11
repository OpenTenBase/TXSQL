###############################################
# test of gather worker error (Diagnostics_area)
###############################################
CREATE TABLE t (a INT) ENGINE=InnoDB;
INSERT INTO t VALUES (1), (2);
SELECT DISTINCT SQL_BIG_RESULT ST_GeomFromGeoJSON('1', 1, ST_SRID(a)) FROM t;
ERROR 22023: Invalid GIS data provided to function st_srid.
DROP TABLE t;
CREATE TABLE t(
a YEAR NOT NULL,
b DECIMAL(29,5) UNSIGNED ZEROFILL NOT NULL
) ENGINE=INNODB;
Warnings:
Warning	1681	The ZEROFILL attribute is deprecated and will be removed in a future release. Use the LPAD function to zero-pad numbers, or store the formatted numbers in a CHAR column.
Warning	1681	UNSIGNED for decimal and floating point data types is deprecated and support for it will be removed in a future release.
INSERT INTO t VALUES (2000,1),(2000,1),(2000,9999);
SELECT * FROM t ORDER BY (a-b);
ERROR 22003: DECIMAL UNSIGNED value is out of range in '(`test`.`t`.`a` - `test`.`t`.`b`)'
DROP TABLE t;
