#ifndef STATEMENT_OUTLINE_PARSER_INCLUDED
#define STATEMENT_OUTLINE_PARSER_INCLUDED
#include <functional>
#include <string>
#include <vector>
#include "my_inttypes.h"
#include "lex_string.h"

class PT_hint_list;
class THD;

namespace statement_outline {
/**
  Call parse_sql() to parse query so that we can extract optimizer hints as
  outline.

  @param thd the thread handler
  @param query The query includes hints.
*/
bool parse_query(THD *thd, const LEX_CSTRING query);

/**
   Parse the optimizer hint for one query block.

   Call this function to apply hint to parse tree when applying optimizer hint.

   @param[in]       thd         Thread context
   @param[in]       hint        Optimizer hint
   @param[out]      errmsgs     Error messages if fail to parse.

   @reval           hint list   Optimizer hint list
*/
PT_hint_list *parse_optimizer_hint(THD *thd, LEX_CSTRING &hint,
                                   std::vector<std::string> *errmsgs);

/**
  Apply optimizer hints to current THD(@param thd)'s parser tree.

  The function get the hint for every query block by calling callback @param
  get_query_block_hint and return error messages to @param errmsg.
*/
bool apply_optimizer_hints(
    THD *thd, std::function<LEX_CSTRING(uint)> get_query_block_hint,
    std::vector<std::string> *errmsg);

/**
  Walk every query block call function @param processor for each optimizer hint

  It feeds the callback function @param processor 2 parameters: query block
  number and hint string.

 @retval Return true if there are hints.
*/
bool parser_visit_hints(
    THD *thd,
    std::function<void(uint, const char *)> processor);

/**
  Computer digest for current query if it is required but skip EXPLAIN [FORMAT =
  XXX] tokens.

  @param digest_buf The buffer to save result digest
  @param compute_all If it sets false, Just compute digest for EXPLAIN command.

  @retval return buffer contains current statement digest. It may be different
  with digest_buf.

  Note, MySQL think EXPLAIN and query without explain are different queries.  We
  want to users check outline by EXPLAIN command so we make one query and its
  EXPLAIN has same digest.
*/
uchar *compute_digest_skip_explain(THD *thd, uchar *digest_buf, bool compute_all);
}  // namespace statement_outline
#endif /* STATEMENT_OUTLINE_PARSER_INCLUDED */
