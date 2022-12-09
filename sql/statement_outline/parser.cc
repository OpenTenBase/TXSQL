#include "sql/statement_outline/parser.h"
#include "sql/error_handler.h"
#include "sql/lex_token.h"  // TOK_XXX
#include "sql/mysqld.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"
#include "sql/sql_yacc.h"  // FORMAT_SYM

namespace statement_outline {
bool parse_query(THD *thd, const LEX_CSTRING query) {
  /*
    Clean up parse and item trees in case this function was called before for
    the same THD.
  */
  if (thd->lex->is_lex_started) {
    thd->end_statement();
    thd->cleanup_after_query();
  }

  lex_start(thd);

  if (alloc_query(thd, query.str, query.length))
    return true;  // Fatal error flag set

  Parser_state parser_state;
  if (parser_state.init(thd, query.str, query.length)) return 1;

  parser_state.m_input.m_has_digest = true;
  parser_state.m_input.m_compute_digest = true;
  thd->m_digest = &thd->m_digest_state;
  thd->m_digest->reset(thd->m_token_array, max_digest_length);
  parser_state.m_lip.exclude_hints_in_digest = true;

  parser_state.m_lip.stmt_prepare_mode = true;
  parser_state.m_lip.multi_statements = false;
  thd->lex->context_analysis_only |= CONTEXT_ANALYSIS_ONLY_PREPARE;

  // We call this in a standalone thread, no don't need to push an
  // error handler.
  if (parse_sql(thd, &parser_state, nullptr))
    return true;

  // Update SET_VAR hints otherwise Sys_var_hint::print does not work. This
  //  needn't restore because later we just read the hints.
  LEX *lex = thd->lex;
  if (lex->opt_hints_global && lex->opt_hints_global->sys_var_hint)
    lex->opt_hints_global->sys_var_hint->update_vars(thd);

  return false;
}

#define EXPLAIN_FORMAT_MINIMAL_LENGTH 6
#define SIZE_OF_A_TOKEN 2
/**
  Calculate the offset of the real query in the digest storage.

  This function calculates the number of bytes that should be skipped
  to digest bytes corresponding to "EXPLAIN" or "EXPLAIN FORMAT =
  xxx".

  @param[in]    token_array    token array of digest storage
  @param[in]    byte_count    token array byte count of digest storage

  @retval       skip_length      The number of bytes that need to be skipped
  to skip the EXPLAIN token
*/
static uint calculate_explain_skip_bytes(const unsigned char *token_array,
                                              size_t byte_count) {
  if (!token_array || byte_count <= SIZE_OF_A_TOKEN) return 0;

  const unsigned char *src = token_array;
  uint tok = src[0] | (src[1] << 8);

  if (tok != DESCRIBE) return 0;

  uint skip_length = 2;
  uint expected_length = EXPLAIN_FORMAT_MINIMAL_LENGTH;

  if (byte_count < SIZE_OF_A_TOKEN + expected_length + SIZE_OF_A_TOKEN)
    return skip_length;

  /*
    Make sure that there is enough size for "FORMAT = xxx" and at least
    one more token.
   */
  uint tok_format = src[2] | (src[3] << 8);
  uint tok_eq = src[4] | (src[5] << 8);
  uint tok_type = src[6] | (src[7] << 8);

  if (tok_format != FORMAT_SYM || tok_eq != EQ) return skip_length;
  /*
    FORMAT = { json | traditional | tree }
    Note that the format type may also be specified as quoted:

        EXPLAIN FORMAT='TrAdItIoNaL' SELECT 1

    The quoted type is reduced as TOK_GENERIC_VALUE in digest_storage.
    However, the input token stream is intact and still recognized
    by the parser.
   */

  if (tok_type == JSON_SYM || tok_type == TOK_GENERIC_VALUE) {
    skip_length += expected_length;
    return skip_length;
  }

  assert(tok_type == TOK_IDENT);
  expected_length += SIZE_OF_A_TOKEN;

  assert(byte_count >= SIZE_OF_A_TOKEN + expected_length + SIZE_OF_A_TOKEN);

  uint id_len = src[8] | (src[9] << 8);
  expected_length += id_len;

  assert(byte_count >= SIZE_OF_A_TOKEN + expected_length + SIZE_OF_A_TOKEN);

  skip_length += expected_length;

  return skip_length;
}

uchar *compute_digest_skip_explain(THD *thd, uchar *digest_buf, bool compute_all) {
  assert(thd->m_digest);
  auto *digest_storage = &thd->m_digest->m_digest_storage;
  size_t skip_bytes = calculate_explain_skip_bytes(
      digest_storage->m_token_array, digest_storage->m_byte_count);

  if (compute_all || skip_bytes != 0) {
    compute_digest_hash(digest_storage, digest_buf, skip_bytes);
    return digest_buf;
  }

  return digest_storage->m_hash;
}

/**
  This class implements the framework needed for the callback function that
  handles conditions that may arise during parsing via the HINT PARSER
*/
class Hint_parse_error_handler : public Internal_error_handler {
  THD *m_thd;
  std::vector<std::string> *m_messages;

 public:
  Hint_parse_error_handler(THD *thd, std::vector<std::string> *errmsgs)
      : m_thd(thd), m_messages(errmsgs) {
    thd->push_internal_handler(this);
  }

  bool handle_condition(THD *, uint sql_errno_u, const char *sqlstate,
                        Sql_condition::enum_severity_level *,
                        const char *msg) override {
    // Just save the error message, caller thread will raise it to users.
    if (m_messages) m_messages->emplace_back(msg);
    return false;
  }

  ~Hint_parse_error_handler() override { m_thd->pop_internal_handler(); }
};

PT_hint_list *parse_optimizer_hint(THD *thd, LEX_CSTRING &hint,
                                          std::vector<std::string> *errmsgs) {
  DBUG_TRACE;

  if (hint.length == 0) return nullptr;

  Parser_state *saved_parser_state = thd->m_parser_state;
  Parser_state ps;
  Lexer_yystype yylval;
  yylval.optimizer_hints = nullptr;

  if (ps.init(thd, hint.str, hint.length)) return nullptr;

  ps.m_lip.yylval = &yylval;
  ps.m_lip.m_digest = nullptr;
  ps.m_lip.multi_statements = false;
  thd->m_parser_state = &ps;

  Hint_parse_error_handler error_handler(thd, errmsgs);

  consume_optimizer_hints(&(ps.m_lip));

  /*
    Handled conditions are thrown away at this point - they are supposedly
    handled by handle_condition. If there was no handler supplied, the
    diagnostics area is not touched. It will contain any errors thrown by the
    parser.
  */
  if (errmsgs) {
    thd->get_stmt_da()->reset_diagnostics_area();
    thd->get_stmt_da()->reset_condition_info(thd);
  }
  thd->m_parser_state = saved_parser_state;

  return ps.m_lip.yylval->optimizer_hints;
}

bool apply_optimizer_hints(
    THD *thd, std::function<LEX_CSTRING(uint)> get_query_block_hint,
    std::vector<std::string> *errmsgs) {
  PT_hint_list *pt_list = nullptr;
  LEX *lex = thd->lex;

  bool has_failure = false;
  for (Query_block *select = lex->all_query_blocks_list; select != nullptr;
       select = select->next_select_in_list()) {
    LEX_CSTRING hint = get_query_block_hint(select->select_number);
    if (!hint.str) continue;

    Parse_context pc(thd, select);
    if (!(pt_list = parse_optimizer_hint(thd, hint, errmsgs)) ||
        pt_list->contextualize(&pc))
      has_failure = true;
  }
  return has_failure;
}

bool parser_visit_hints(THD *thd,
                        std::function<void(uint, const char *)> processor) {
  bool has_hint = false;
  LEX *lex = thd->lex;
  for (Query_block *select = lex->all_query_blocks_list; select != nullptr;
       select = select->next_select_in_list()) {
    String hints;
    // Use QT_NORMALIZED_FORMAT otherwide print_hints() output nothing because
    // some hints are not resolved.
    select->print_hints(thd, &hints, enum_query_type(QT_NORMALIZED_FORMAT));
    if (hints.length() == 0) continue;
    if (!has_hint) has_hint = true;
    processor(select->select_number, hints.ptr());
  }

  return has_hint;
}
}  // namespace statement_outline
