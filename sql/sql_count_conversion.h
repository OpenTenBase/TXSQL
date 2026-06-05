#ifndef SQL_COUNT_CONVERSION_INCLUDED
#define SQL_COUNT_CONVERSION_INCLUDED

class Item;
class Item_sum_count;
class THD;
template <class T>
class mem_root_deque;

extern bool can_convert_to_zero(Item *item);
extern bool count_col_conversion(THD *thd, const mem_root_deque<Item *> &fields);
extern bool count_col_conversion(THD *thd, Item_sum_count *item_count);

#endif /* SQL_SELECT_INCLUDED */