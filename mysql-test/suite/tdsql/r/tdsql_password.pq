set names utf8;
Warnings:
Warning	3719	'utf8' is currently an alias for the character set UTF8MB3, but will be an alias for UTF8MB4 in a future release. Please consider using UTF8MB4 in order to be unambiguous.
select tdsql_password('');
tdsql_password('')

select tdsql_password(NULL);
tdsql_password(NULL)

select tdsql_password('a');
tdsql_password('a')
*667F407DE7C6AD07358FA38DAED7828A72014B4E
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb');
tdsql_password('bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb')
*EB5FD5A206A74E76EB00D99ECCE7B8D2114B7942
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('中文||');
tdsql_password('中文||')
*6C259F8E1049A794301D97957B88A651A04DC1F6
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('%%%&@#*&^!)(*#&&@||');
tdsql_password('%%%&@#*&^!)(*#&&@||')
*0DEA9719334B35A2D5BD31F7AC6D57FA5FAD8C75
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
set names latin1;
select tdsql_password('');
tdsql_password('')

select tdsql_password(NULL);
tdsql_password(NULL)

select tdsql_password('a');
tdsql_password('a')
*667F407DE7C6AD07358FA38DAED7828A72014B4E
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb');
tdsql_password('bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb')
*EB5FD5A206A74E76EB00D99ECCE7B8D2114B7942
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('中文||');
tdsql_password('中文||')
*6C259F8E1049A794301D97957B88A651A04DC1F6
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('%%%&@#*&^!)(*#&&@||');
tdsql_password('%%%&@#*&^!)(*#&&@||')
*0DEA9719334B35A2D5BD31F7AC6D57FA5FAD8C75
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
set names gbk;
select tdsql_password('');
tdsql_password('')

select tdsql_password(NULL);
tdsql_password(NULL)

select tdsql_password('a');
tdsql_password('a')
*667F407DE7C6AD07358FA38DAED7828A72014B4E
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb');
tdsql_password('bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb')
*EB5FD5A206A74E76EB00D99ECCE7B8D2114B7942
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('中文||');
tdsql_password('�?��||')
*6C259F8E1049A794301D97957B88A651A04DC1F6
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
select tdsql_password('%%%&@#*&^!)(*#&&@||');
tdsql_password('%%%&@#*&^!)(*#&&@||')
*0DEA9719334B35A2D5BD31F7AC6D57FA5FAD8C75
Warning	1681	'PASSWORD' is deprecated and will be removed in a future release.
Warnings:
