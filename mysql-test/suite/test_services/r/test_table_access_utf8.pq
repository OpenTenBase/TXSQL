SET NAMES "utf8mb4";
INSTALL COMPONENT 'file://component_test_table_access';
SELECT HEX('∀p∊ℙ');
HEX('∀p∊ℙ')
E2888070E2888AE28499
SELECT HEX('∃s∊ℚ');
HEX('∃s∊ℚ')
E2888373E2888AE2849A
SELECT HEX('∃s∊𝕊');
HEX('∃s∊?')
E2888373E2888AF09D958A
SELECT HEX('s(p)≎⊤');
HEX('s(p)≎⊤')
73287029E2898EE28AA4
CREATE SCHEMA `∀p∊ℙ`;
show databases;
Database
information_schema
mtr
mysql
performance_schema
sys
test
∀p∊ℙ
CREATE TABLE `∀p∊ℙ`.`∃s∊ℚ`(`s(p)≎⊤` VARCHAR(255) CHARACTER SET `utf8mb4`);
CREATE TABLE `∀p∊ℙ`.`∃s∊𝕊`(`s(p)≎⊤` VARCHAR(255) CHARACTER SET `utf8mb4`);
Warnings:
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
use `∀p∊ℙ`;
show tables;
Tables_in_∀p∊ℙ
∃s∊?
∃s∊ℚ
show create table `∃s∊ℚ`;
Table	Create Table
∃s∊ℚ	CREATE TABLE `∃s∊ℚ` (
  `s(p)≎⊤` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
show create table `∃s∊𝕊`;
Table	Create Table
∃s∊?	CREATE TABLE `∃s∊?` (
  `s(p)≎⊤` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci
Warnings:
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
SELECT * FROM `∀p∊ℙ`.`∃s∊ℚ`;
s(p)≎⊤
SELECT * FROM `∀p∊ℙ`.`∃s∊𝕊`;
s(p)≎⊤
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
Warnings:
SELECT test_table_access_driver("MATH-INSERT-UTF8MB3");
test_table_access_driver("MATH-INSERT-UTF8MB3")
OK
SELECT * FROM `∀p∊ℙ`.`∃s∊ℚ`;
s(p)≎⊤
∀p∊ℙ ∃s∊ℚ s(p)≎⊤
TRUNCATE TABLE `∀p∊ℙ`.`∃s∊ℚ`;
SELECT * FROM `∀p∊ℙ`.`∃s∊ℚ`;
s(p)≎⊤
SELECT test_table_access_driver("MATH-INSERT-UTF8MB4");
test_table_access_driver("MATH-INSERT-UTF8MB4")
OK
SELECT * FROM `∀p∊ℙ`.`∃s∊𝕊`;
s(p)≎⊤
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
Warnings:
∀p∊ℙ ∃s∊𝕊 s(p)≎⊤
TRUNCATE TABLE `∀p∊ℙ`.`∃s∊𝕊`;
Warnings:
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
SELECT * FROM `∀p∊ℙ`.`∃s∊𝕊`;
s(p)≎⊤
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
Warnings:
##################
# SERVER RESTART #
##################
# restart
SELECT * FROM `∀p∊ℙ`.`∃s∊ℚ`;
s(p)≎⊤
∀p∊ℙ ∃s∊ℚ s(p)≎⊤
SELECT * FROM `∀p∊ℙ`.`∃s∊𝕊`;
s(p)≎⊤
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
Warnings:
∀p∊ℙ ∃s∊𝕊 s(p)≎⊤
DROP TABLE `∀p∊ℙ`.`∃s∊ℚ`;
DROP TABLE `∀p∊ℙ`.`∃s∊𝕊`;
Warnings:
Warning	1300	Cannot convert string '\xE2\x88\x83s\xE2\x88...' from utf8mb4 to utf8mb3
DROP SCHEMA `∀p∊ℙ`;
UNINSTALL COMPONENT 'file://component_test_table_access';
