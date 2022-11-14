select murmurHashCodeAndMod(NULL,64)=murmurHashCodeAndMod('NULL',64) as eq, murmurHashCodeAndMod('NULL',64) as val;
eq	val
1	55
select murmurHashCodeAndMod('102030405060708090124',64)=murmurHashCodeAndMod(102030405060708090124,64) as eq, murmurHashCodeAndMod('102030405060708090124',64) as val;
eq	val
1	6
select
murmurHashCodeAndMod(102030405060708090124673452,64)=murmurHashCodeAndMod('102030405060708090124673452',64)
as eq, murmurHashCodeAndMod('102030405060708090124673452',64) as val;
eq	val
1	63
select
murmurHashCodeAndMod('10203040506070809012467',64)=murmurHashCodeAndMod(10203040506070809012467,64)
as eq, murmurHashCodeAndMod('10203040506070809012467',64) as val;
eq	val
1	1
select murmurHashCodeAndMod(1,64)= murmurHashCodeAndMod('1',64) as eq,
murmurHashCodeAndMod('1',64) as val;
eq	val
1	33
