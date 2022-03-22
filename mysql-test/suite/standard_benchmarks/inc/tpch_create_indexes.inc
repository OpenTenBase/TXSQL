## indexes
create index c_custkey_idx on customer(c_custkey);
create index c_nationkey_idx on customer(c_nationkey);
create index c_acctbal_idx on customer(c_acctbal);

create index l_orderkey_idx on lineitem(l_orderkey);
create index l_partkey_idx on lineitem(l_partkey);
create index l_suppkey_idx on lineitem(l_suppkey);
create index l_extendedprice_idx on lineitem(l_extendedprice);
create index l_quantity_idx on lineitem(l_quantity);
create index l_discount_idx on lineitem(l_discount);
create index l_tax_idx on lineitem(l_tax);

create index o_orderkey_idx on orders(o_orderkey);
create index o_custkey_idx on orders(o_custkey);
create index o_totalprice_idx on orders(o_totalprice);
create index o_orderpriority_idx on orders(o_orderpriority);
create index o_shippriority_idx on orders(o_shippriority);

create index p_partkey_idx on part(p_partkey);
create index p_size_idx on part(p_size);
create index p_retailprice_idx on part(p_retailprice);

create index ps_partkey_idx on partsupp(ps_partkey);
create index ps_suppkey_idx on partsupp(ps_suppkey);
create index ps_availqty_idx on partsupp(ps_availqty);

create index s_suppkey_idx on supplier(s_suppkey);
create index s_nationkey_idx on supplier(s_nationkey);
create index s_acctbal_idx on supplier(s_acctbal);


create index i_l_shipdate on lineitem (l_shipdate);
create index i_l_suppkey_partkey on lineitem (l_partkey, l_suppkey);
create index i_l_receiptdate on lineitem (l_receiptdate);
create index i_l_orderkey_quantity on lineitem (l_orderkey, l_quantity);
create index i_l_commitdate on lineitem (l_commitdate);
