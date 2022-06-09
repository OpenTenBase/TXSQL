/*****************************************************************************

Copyright (c) 2005, 2021, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file row/row0merge.cc
 New index creation routines using a merge sort

 Created 12/4/2005 Jan Lindstrom
 Completed by Sunny Bains and Marko Makela
 *******************************************************/

#include <fcntl.h>
#include <math.h>
#include <sys/types.h>

#include <sql_class.h>
#include "btr0bulk.h"
#include "dict0crea.h"
#include "dict0dd.h"
#include "fsp0sysspace.h"
#include "ha_prototypes.h"
#include "handler0alter.h"
#include "lob0lob.h"
#include "lock0lock.h"
#include "my_psi_config.h"
#include "pars0pars.h"
#include "row0ext.h"
#include "row0import.h"
#include "row0ins.h"
#include "row0log.h"
#include "row0merge.h"
#include "row0sel.h"
#include "trx0purge.h"
#include "ut0new.h"
#include "ut0sort.h"
#include "ut0stage.h"
#include "log0chkp.h"
#include "os0thread-create.h"
#include "row0pread.h"
#include "handler/ha_innodb.h"

#include "my_dbug.h"
#include "sql/table.h"

/* Ignore posix_fadvise() on those platforms where it does not exist */
#if defined _WIN32
#define posix_fadvise(fd, offset, len, advice) /* nothing */
#endif                                         /* _WIN32 */

/** Insert sorted data tuples to the index.
@param[in]	trx		current transaction
@param[in]	index		index to be inserted
@param[in]	old_table	old table
@param[in]	fd		file descriptor
@param[in,out]	block		file buffer
@param[in]	row_buf		row_buf the sorted data tuples,
or NULL if fd, block will be used instead
@param[in,out]	btr_bulk	btr bulk instance
@param[in,out]	stage		performance schema accounting object, used by
ALTER TABLE. If not NULL stage->begin_phase_insert() will be called initially
and then stage->inc() will be called for each record that is processed.
@return DB_SUCCESS or error number */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t row_merge_insert_index_tuples(
    trx_t *trx, dict_index_t *index, const dict_table_t *old_table, int fd,
    row_merge_block_t *block, const row_merge_buf_t *row_buf, BtrBulk *btr_bulk,
    Alter_stage *stage = nullptr);

/** Encode an index record. */
static void row_merge_buf_encode(
    byte **b,                  /*!< in/out: pointer to
                               current end of output buffer */
    const dict_index_t *index, /*!< in: index */
    const mtuple_t *entry,     /*!< in: index fields
                               of the record to encode */
    ulint n_fields)            /*!< in: number of fields
                               in the entry */
{
  ulint size;
  ulint extra_size;

  size = rec_get_serialize_size(index, entry->fields, n_fields, nullptr,
                                &extra_size, MAX_ROW_VERSION);
  ut_ad(size >= extra_size);

  /* Encode extra_size + 1 */
  if (extra_size + 1 < 0x80) {
    *(*b)++ = (byte)(extra_size + 1);
  } else {
    ut_ad((extra_size + 1) < 0x8000);
    *(*b)++ = (byte)(0x80 | ((extra_size + 1) >> 8));
    *(*b)++ = (byte)(extra_size + 1);
  }

  rec_serialize_dtuple(*b + extra_size, index, entry->fields, n_fields,
                             nullptr);

  *b += size;
}

/** Allocate a sort buffer.
 @return own: sort buffer */
static MY_ATTRIBUTE((malloc)) row_merge_buf_t *row_merge_buf_create_low(
    mem_heap_t *heap,    /*!< in: heap where allocated */
    dict_index_t *index, /*!< in: secondary index */
    ulint max_tuples,    /*!< in: maximum number of
                         data tuples */
    ulint buf_size)      /*!< in: size of the buffer,
                         in bytes */
{
  row_merge_buf_t *buf;

  ut_ad(max_tuples > 0);

  ut_ad(max_tuples <= srv_sort_buf_size);

  buf = static_cast<row_merge_buf_t *>(mem_heap_zalloc(heap, buf_size));
  buf->heap = heap;
  buf->index = index;
  buf->max_tuples = max_tuples;
  buf->tuples = static_cast<mtuple_t *>(
      ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, 2 * max_tuples * sizeof *buf->tuples));
  buf->tmp_tuples = buf->tuples + max_tuples;

  return (buf);
}

/** Allocate a sort buffer.
 @return own: sort buffer */
row_merge_buf_t *row_merge_buf_create(
    dict_index_t *index) /*!< in: secondary index */
{
  row_merge_buf_t *buf;
  ulint max_tuples;
  ulint buf_size;
  mem_heap_t *heap;

  max_tuples = static_cast<ulint>(srv_sort_buf_size) /
               std::max(static_cast<ulint>(1), index->get_min_size());

  buf_size = (sizeof *buf);

  heap = mem_heap_create(buf_size, UT_LOCATION_HERE);

  buf = row_merge_buf_create_low(heap, index, max_tuples, buf_size);

  return (buf);
}

/** Empty a sort buffer.
 @return sort buffer */
row_merge_buf_t *row_merge_buf_empty(
    row_merge_buf_t *buf) /*!< in,own: sort buffer */
{
  ulint buf_size = sizeof *buf;
  ulint max_tuples = buf->max_tuples;
  mem_heap_t *heap = buf->heap;
  dict_index_t *index = buf->index;
  mtuple_t *tuples = buf->tuples;

  mem_heap_empty(heap);

  buf = static_cast<row_merge_buf_t *>(mem_heap_zalloc(heap, buf_size));
  buf->heap = heap;
  buf->index = index;
  buf->max_tuples = max_tuples;
  buf->tuples = tuples;
  buf->tmp_tuples = buf->tuples + max_tuples;

  return (buf);
}

/** Deallocate a sort buffer. */
void row_merge_buf_free(
    row_merge_buf_t *buf) /*!< in,own: sort buffer to be freed */
{
  ut::free(buf->tuples);
  mem_heap_free(buf->heap);
}

#ifdef UNIV_DEBUG
#define row_merge_buf_redundant_convert(trx, index, row_field, field, len, \
                                        page_size, is_sdi, heap)           \
  row_merge_buf_redundant_convert_func(trx, index, row_field, field, len,  \
                                       page_size, is_sdi, heap)
#else /* UNIV_DEBUG */
#define row_merge_buf_redundant_convert(trx, index, row_field, field, len, \
                                        page_size, is_sdi, heap)           \
  row_merge_buf_redundant_convert_func(trx, index, row_field, field, len,  \
                                       page_size, is_sdi, heap)
#endif /* UNIV_DEBUG */

/** Convert the field data from compact to redundant format.
@param[in]	trx		current transaction
@param[in]	clust_index	clustered index being built
@param[in]	row_field	field to copy from
@param[out]	field		field to copy to
@param[in]	len		length of the field data
@param[in]	page_size	compressed BLOB page size,
                                zero for uncompressed BLOBs */
#ifdef UNIV_DEBUG
/**
@param[in]	is_sdi		true for SDI indexes */
#endif /* UNIV_DEBUG */
/**
@param[in,out]	heap		memory heap where to allocate data when
                                converting to ROW_FORMAT=REDUNDANT, or NULL
                                when not to invoke
                                row_merge_buf_redundant_convert(). */
static void row_merge_buf_redundant_convert_func(
    trx_t *trx, const dict_index_t *clust_index, const dfield_t *row_field,
    dfield_t *field, ulint len, const page_size_t &page_size,
    bool is_sdi,
    mem_heap_t *heap) {
  ut_ad(DATA_MBMINLEN(field->type.mbminmaxlen) == 1);
  ut_ad(DATA_MBMAXLEN(field->type.mbminmaxlen) > 1);

  byte *buf = (byte *)mem_heap_alloc(heap, len);
  ulint field_len = row_field->len;
  ut_ad(field_len <= len);

  if (row_field->ext) {
    const byte *field_data = static_cast<byte *>(dfield_get_data(row_field));
    ulint ext_len;

    ut_a(field_len >= BTR_EXTERN_FIELD_REF_SIZE);
    ut_a(memcmp(field_data + field_len - BTR_EXTERN_FIELD_REF_SIZE,
                field_ref_zero, BTR_EXTERN_FIELD_REF_SIZE));

    byte *data = lob::btr_copy_externally_stored_field(
        nullptr, clust_index, &ext_len, nullptr, field_data, page_size,
        field_len, is_sdi, heap);

    ut_ad(ext_len < len);

    memcpy(buf, data, ext_len);
    field_len = ext_len;
  } else {
    memcpy(buf, row_field->data, field_len);
  }

  memset(buf + field_len, 0x20, len - field_len);

  dfield_set_data(field, buf, len);
}

#ifdef UNIV_DEBUG
thread_local bool process_pk_row_debug = false;
thread_local bool inject_trx_interrupted = false;
#endif

/** Insert a data tuple into a sort buffer.
@param[in,out]	buf		sort buffer
@param[in]	old_table	original table
@param[in]	new_table	new table
@param[in,out]	psort_info	parallel sort info
@param[in]	row		table row
@param[in]	ext		cache of externally stored
                                column prefixes, or NULL
@param[in,out]	conv_heap	memory heap where to allocate data when
                                converting to ROW_FORMAT=REDUNDANT, or NULL
                                when not to invoke
                                row_merge_buf_redundant_convert()
@param[in,out]	err		set if error occurs
@param[in,out]	v_heap		heap memory to process data for virtual column
@param[in,out]	my_table	mysql table object
@param[in]	trx		transaction object
@param[in,out]	multi_val_added	non-zero indicates this number of multi-value
                                data has been put to the buffer, and it should
                                just continue from this point, otherwise,
                                this is a new row to be added to buffer.
                                For the output, non-zero means the new number
                                of multi-value data which have been handled,
                                while zero means this is a normal row or all
                                data of the multi-value data in this row have
                                been parsed
@return number of rows added, 0 if out of space, or UNIV_NO_INDEX_VALUE
if this is a multi-value index and current row has nothing valid to be
indexed */
static ulint row_merge_buf_add(row_merge_buf_t *buf,
                               const dict_table_t *old_table,
                               const dict_table_t *new_table,
                               const dtuple_t *row,
                               const row_ext_t *ext,
                               mem_heap_t *conv_heap, dberr_t *err,
                               mem_heap_t **v_heap, TABLE *my_table,
                               trx_t *trx, ulint *multi_val_added) {
  ulint i;
  const dict_index_t *index;
  mtuple_t *entry;
  dfield_t *field;
  const dict_field_t *ifield;
  ulint n_fields;
  ulint data_size;
  ulint extra_size;
  ulint n_row_added = 0;
  ulint n_row_to_add = 0;
  multi_value_data *multi_v = nullptr;
  DBUG_TRACE;

  if (buf->n_tuples >= buf->max_tuples) {
    return 0;
  }

  DBUG_EXECUTE_IF("ib_row_merge_buf_readd_failure", if (process_pk_row_debug) { *err = DB_ERROR; return 0; });
  DBUG_EXECUTE_IF("ib_row_merge_buf_add_two", if (buf->n_tuples >= 2) return 0;);
  DBUG_EXECUTE_IF("ib_row_merge_buf_add_failure", *err = DB_ERROR; return 2;);

  UNIV_PREFETCH_R(row->fields);

  assert(!(buf->index->type & DICT_FTS));
  index = buf->index;

  /* create spatial index should not come here */
  ut_ad(!dict_index_is_spatial(index));

add_next:
  n_fields = dict_index_get_n_fields(index);

  if (buf->n_tuples >= buf->max_tuples) {
    return n_row_added;
  }

  DBUG_EXECUTE_IF("row_merge_add_multi_value",
                  if (n_row_added == 5) return n_row_added;);

  entry = &buf->tuples[buf->n_tuples];
  field = entry->fields = static_cast<dfield_t *>(
      mem_heap_alloc(buf->heap, n_fields * sizeof *entry->fields));

  data_size = 0;
  extra_size = UT_BITS_IN_BYTES(index->n_nullable);

  ifield = index->get_field(0);

  for (i = 0; i < n_fields; i++, field++, ifield++) {
    ulint len;
    const dict_col_t *col;
    const dict_v_col_t *v_col = nullptr;
    ulint col_no;
    ulint fixed_len;
    dfield_t *row_field;

    col = ifield->col;
    if (col->is_virtual()) {
      v_col = reinterpret_cast<const dict_v_col_t *>(col);
    }

    col_no = dict_col_get_no(col);

    /* Use callback to get the virtual column value */
    if (col->is_virtual()) {
      const dict_index_t *clust_index = new_table->first_index();
      if (col->is_multi_value()) {
        ut_ad(index->is_multi_value());
        row_field = dtuple_get_nth_v_field(row, v_col->v_pos);

        if (n_row_to_add == 0) {
          row_field = innobase_get_computed_value(
              row, v_col, clust_index, v_heap, buf->heap, ifield,
              trx->mysql_thd, my_table, old_table, nullptr, nullptr, nullptr);

          if (row_field == nullptr) {
            *err = DB_COMPUTE_VALUE_FAILED;
            return 0;
          }

          if (dfield_is_null(row_field)) {
            n_row_to_add = 1;
          } else if (row_field->len != UNIV_NO_INDEX_VALUE) {
            multi_v = static_cast<multi_value_data *>(row_field->data);

            if (*multi_val_added == 0) {
              n_row_to_add = multi_v->num_v;
              row_field->data = const_cast<void *>(multi_v->datap[0]);
              row_field->len = multi_v->data_len[0];
            } else {
              n_row_to_add = multi_v->num_v - *multi_val_added;
              row_field->data =
                  const_cast<void *>(multi_v->datap[*multi_val_added]);
              row_field->len = multi_v->data_len[*multi_val_added];
            }
          } else {
            /* Nothing to be indexed */
            return UNIV_NO_INDEX_VALUE;
          }

          ut_ad(n_row_to_add > 0);
        } else {
          row_field->data =
              const_cast<void *>(multi_v->datap[*multi_val_added]);
          row_field->len = multi_v->data_len[*multi_val_added];
        }
      } else {
        row_field = innobase_get_computed_value(
            row, v_col, clust_index, v_heap, nullptr, ifield, trx->mysql_thd,
            my_table, old_table, nullptr, nullptr, nullptr);

        if (row_field == nullptr) {
          *err = DB_COMPUTE_VALUE_FAILED;
          return 0;
        }
      }
      dfield_copy(field, row_field);
    } else {
      row_field = dtuple_get_nth_field(row, col_no);
      dfield_copy(field, row_field);
    }

    if (field->len != UNIV_SQL_NULL && col->mtype == DATA_MYSQL &&
        col->len != field->len) {
      if (conv_heap != nullptr) {
        row_merge_buf_redundant_convert(
            trx, old_table->first_index(), row_field, field, col->len,
            dict_table_page_size(old_table), dict_table_is_sdi(old_table->id),
            conv_heap);
      } else {
        /* Field length mismatch should not
        happen when rebuilding redundant row
        format table. */
        ut_ad(dict_table_is_comp(index->table));
      }
    }

    len = dfield_get_len(field);

    if (dfield_is_null(field)) {
      ut_ad(!(col->prtype & DATA_NOT_NULL));
      continue;
    } else if (!ext) {
    } else if (index->is_clustered()) {
      /* Flag externally stored fields. */
      const byte *buf = row_ext_lookup(ext, col_no, &len);
      if (UNIV_LIKELY_NULL(buf)) {
        ut_a(buf != field_ref_zero);
        if (i < dict_index_get_n_unique(index)) {
          dfield_set_data(field, buf, len);
        } else {
          dfield_set_ext(field);
          len = dfield_get_len(field);
        }
      }
    } else if (!col->is_virtual()) {
      /* Only non-virtual column are stored externally */
      const byte *buf = row_ext_lookup(ext, col_no, &len);
      if (UNIV_LIKELY_NULL(buf)) {
        ut_a(buf != field_ref_zero);
        dfield_set_data(field, buf, len);
      }
    }

    /* If a column prefix index, take only the prefix */

    if (ifield->prefix_len) {
      len = dtype_get_at_most_n_mbchars(
          col->prtype, col->mbminmaxlen, ifield->prefix_len, len,
          static_cast<char *>(dfield_get_data(field)));
      dfield_set_len(field, len);
    }

    ut_ad(len <= col->len || DATA_LARGE_MTYPE(col->mtype) ||
          (col->mtype == DATA_POINT && len == DATA_MBR_LEN));

    fixed_len = ifield->fixed_len;
    if (fixed_len && !dict_table_is_comp(index->table) &&
        DATA_MBMINLEN(col->mbminmaxlen) != DATA_MBMAXLEN(col->mbminmaxlen)) {
      /* CHAR in ROW_FORMAT=REDUNDANT is always
      fixed-length, but in the temporary file it is
      variable-length for variable-length character
      sets. */
      fixed_len = 0;
    }

    if (fixed_len) {
#ifdef UNIV_DEBUG
      ulint mbminlen = DATA_MBMINLEN(col->mbminmaxlen);
      ulint mbmaxlen = DATA_MBMAXLEN(col->mbminmaxlen);

      /* len should be between size calcualted base on
      mbmaxlen and mbminlen */
      ut_ad(len <= fixed_len);
      ut_ad(!mbmaxlen || len >= mbminlen * (fixed_len / mbmaxlen));

      ut_ad(!dfield_is_ext(field));
#endif /* UNIV_DEBUG */
    } else if (dfield_is_ext(field)) {
      extra_size += 2;
    } else if (len < 128 || (!DATA_BIG_COL(col))) {
      extra_size++;
    } else {
      /* For variable-length columns, we look up the
      maximum length from the column itself.  If this
      is a prefix index column shorter than 256 bytes,
      this will waste one byte. */
      extra_size += 2;
    }
    data_size += len;
  }

#ifdef UNIV_DEBUG
  {
    ulint size;
    ulint extra;

    size = rec_get_serialize_size(index, entry->fields, n_fields, nullptr,
                                  &extra, MAX_ROW_VERSION);

    ut_ad(data_size + extra_size == size);
    ut_ad(extra_size == extra);
  }
#endif /* UNIV_DEBUG */

  /* Add to the total size of the record in row_merge_block_t
  the encoded length of extra_size and the extra bytes (extra_size).
  See row_merge_buf_write() for the variable-length encoding
  of extra_size. */
  data_size += (extra_size + 1) + ((extra_size + 1) >= 0x80);

  /* Record size can exceed page size while converting to
  redundant row format. But there is assert
  ut_ad(size < UNIV_PAGE_SIZE) in rec_offs_data_size().
  It may hit the assert before attempting to insert the row. */
  if (conv_heap != nullptr && data_size > UNIV_PAGE_SIZE) {
    *err = DB_TOO_BIG_RECORD;
  }

  ut_ad(data_size < srv_sort_buf_size);

  /* Reserve one byte for the end marker of row_merge_block_t. */
  if (buf->total_size + data_size >= srv_sort_buf_size - 1) {
    return (index->is_multi_value() ? n_row_added : 0);
  }

  buf->total_size += data_size;
  buf->n_tuples++;
  n_row_added++;

  field = entry->fields;

  /* Copy the data fields. */

  do {
    dfield_dup(field++, buf->heap);
  } while (--n_fields);

  if (conv_heap != nullptr) {
    mem_heap_empty(conv_heap);
  }

  if (n_row_added < n_row_to_add) {
    ut_ad(index->is_multi_value());
    *multi_val_added += 1;

    DBUG_EXECUTE_IF("row_merge_add_multi_value",
                    if (*multi_val_added == 7) return n_row_added;);

    goto add_next;
  }

  if (index->is_multi_value()) {
    *multi_val_added = 0;
  }

  return n_row_added;
}

/** Report a duplicate key.
@param[in,out] dup For reporting duplicates
@param[in] entry Duplicate index entry */
void row_merge_dup_report(row_merge_dup_t *dup, const dfield_t *entry) {
  if (!dup->n_dup++) {
    /* Only report the first duplicate record,
    but count all duplicate records. */
    innobase_fields_to_mysql(dup->table, dup->index, entry);
  }
}

/** Compare two tuples.
 @param[in]	index	index tree
 @param[in]	n_uniq	number of unique fields
 @param[in]	n_field	number of fields
 @param[in]	a	first tuple to be compared
 @param[in]	b	second tuple to be compared
 @param[in,out]	dup	for reporting duplicates, NULL if non-unique index
 @return positive, 0, negative if a is greater, equal, less, than b,
 respectively */
static MY_ATTRIBUTE((warn_unused_result)) int row_merge_tuple_cmp(
    const dict_index_t *index, ulint n_uniq, ulint n_field, const mtuple_t &a,
    const mtuple_t &b, row_merge_dup_t *dup) {
  int cmp;
  const dfield_t *af = a.fields;
  const dfield_t *bf = b.fields;
  ulint n = n_uniq;
  const dict_field_t *f = index->fields;
  ut_ad(n > 0);
  ut_ad(n_uniq <= n_field);

  /* Compare the fields of the tuples until a difference is
  found or we run out of fields to compare.  If !cmp at the
  end, the tuples are equal. */
  do {
    cmp = cmp_dfield_dfield(af++, bf++, (f++)->is_ascending);
  } while (!cmp && --n);

  if (cmp) {
    return (cmp);
  }

  if (dup) {
    /* Report a duplicate value error if the tuples are
    logically equal.  NULL columns are logically inequal,
    although they are equal in the sorting order.  Find
    out if any of the fields are NULL. */
    for (const dfield_t *df = a.fields; df != af; df++) {
      if (dfield_is_null(df)) {
        goto no_report;
      }
    }

    row_merge_dup_report(dup, a.fields);
  }

no_report:
  /* The n_uniq fields were equal, but we compare all fields so
  that we will get the same (internal) order as in the B-tree. */
  for (n = n_field - n_uniq + 1; --n;) {
    cmp = cmp_dfield_dfield(af++, bf++, (f++)->is_ascending);
    if (cmp) {
      return (cmp);
    }
  }

  /* This should never be reached, except in a secondary index
  when creating a secondary index and a PRIMARY KEY, and there
  is a duplicate in the PRIMARY KEY that has not been detected
  yet. Internally, an index must never contain duplicates. */
  return (cmp);
}

/** Wrapper for row_merge_tuple_sort() to inject some more context to
UT_SORT_FUNCTION_BODY().
@param tuples array of tuples that being sorted
@param aux work area, same size as tuples[]
@param low lower bound of the sorting area, inclusive
@param high upper bound of the sorting area, inclusive */
#define row_merge_tuple_sort_ctx(tuples, aux, low, high) \
  row_merge_tuple_sort(index, n_uniq, n_field, dup, tuples, aux, low, high)
/** Wrapper for row_merge_tuple_cmp() to inject some more context to
UT_SORT_FUNCTION_BODY().
@param a first tuple to be compared
@param b second tuple to be compared
@return positive, 0, negative, if a is greater, equal, less, than b,
respectively */
#define row_merge_tuple_cmp_ctx(a, b) \
  row_merge_tuple_cmp(index, n_uniq, n_field, a, b, dup)

/** Merge sort the tuple buffer in main memory. */
static void row_merge_tuple_sort(
    const dict_index_t *index, /*!< in: index tree */
    ulint n_uniq,              /*!< in: number of unique fields */
    ulint n_field,             /*!< in: number of fields */
    row_merge_dup_t *dup,      /*!< in/out: reporter of duplicates
                               (NULL if non-unique index) */
    mtuple_t *tuples,          /*!< in/out: tuples */
    mtuple_t *aux,             /*!< in/out: work area */
    ulint low,                 /*!< in: lower bound of the
                               sorting area, inclusive */
    ulint high)                /*!< in: upper bound of the
                               sorting area, exclusive */
{
  ut_ad(n_field > 0);
  ut_ad(n_uniq <= n_field);

  UT_SORT_FUNCTION_BODY(row_merge_tuple_sort_ctx, tuples, aux, low, high,
                        row_merge_tuple_cmp_ctx);
}

/** Sort a buffer.
@param[in,out] buf Sort buffer
@param[in,out] dup Reporter of duplicates (null if non-unique index) */
void row_merge_buf_sort(row_merge_buf_t *buf, row_merge_dup_t *dup) {
  ut_ad(!dict_index_is_spatial(buf->index));

  row_merge_tuple_sort(buf->index, dict_index_get_n_unique(buf->index),
                       dict_index_get_n_fields(buf->index), dup, buf->tuples,
                       buf->tmp_tuples, 0, buf->n_tuples);
}

/** Write a buffer to a block. */
void row_merge_buf_write(
    const row_merge_buf_t *buf, /*!< in: sorted buffer */
    const merge_file_t *of MY_ATTRIBUTE((unused)),
    /*!< in: output file */
    row_merge_block_t *block,
    std::mutex *of_mutex MY_ATTRIBUTE((unused))) /*!< out: buffer for writing to file */
{
  const dict_index_t *index = buf->index;
  ulint n_fields = dict_index_get_n_fields(index);
  byte *b = &block[0];

  DBUG_TRACE;

  for (ulint i = 0; i < buf->n_tuples; i++) {
    const mtuple_t *entry = &buf->tuples[i];

    row_merge_buf_encode(&b, index, entry, n_fields);
    ut_ad(b < &block[srv_sort_buf_size]);

#ifdef UNIV_DEBUG
    if (of_mutex != nullptr) {
      // here we touch of->offset
      of_mutex->lock();
    }
    DBUG_PRINT("ib_merge_sort",
               ("%p,fd=%d,%lu %lu: %s", reinterpret_cast<const void *>(b),
                of->fd, ulong(of->offset), ulong(i),
                rec_printer(entry->fields, n_fields).str().c_str()));
    if (of_mutex != nullptr) {
      of_mutex->unlock();
    }
#endif
  }

  /* Write an "end-of-chunk" marker. */
  ut_a(b < &block[srv_sort_buf_size]);
  ut_a(b == &block[0] + buf->total_size);
  *b++ = 0;
#ifdef UNIV_DEBUG_VALGRIND
  /* The rest of the block is uninitialized.  Initialize it
  to avoid bogus warnings. */
  memset(b, 0xff, &block[srv_sort_buf_size] - b);
#endif /* UNIV_DEBUG_VALGRIND */

#ifdef UNIV_DEBUG
  if (of_mutex != nullptr) {
    of_mutex->lock();
  }
  DBUG_PRINT("ib_merge_sort",
             ("write %p,%d,%lu EOF", reinterpret_cast<const void *>(b), of->fd,
              ulong(of->offset)));
  if (of_mutex != nullptr) {
    of_mutex->unlock();
  }
#endif
}

/** Create a memory heap and allocate space for row_merge_rec_offsets()
 and mrec_buf_t[3].
 @return memory heap */
static mem_heap_t *row_merge_heap_create(
    const dict_index_t *index, /*!< in: record descriptor */
    mrec_buf_t **buf,          /*!< out: 3 buffers */
    ulint **offsets1,          /*!< out: offsets */
    ulint **offsets2)          /*!< out: offsets */
{
  ulint i = 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index);
  mem_heap_t *heap =
      mem_heap_create(2 * i * sizeof **offsets1 + 3 * sizeof **buf, UT_LOCATION_HERE);

  *buf = static_cast<mrec_buf_t *>(mem_heap_alloc(heap, 3 * sizeof **buf));
  *offsets1 = static_cast<ulint *>(mem_heap_alloc(heap, i * sizeof **offsets1));
  *offsets2 = static_cast<ulint *>(mem_heap_alloc(heap, i * sizeof **offsets2));

  (*offsets1)[0] = (*offsets2)[0] = i;
  (*offsets1)[1] = (*offsets2)[1] = dict_index_get_n_fields(index);

  return (heap);
}

/** Create a memory heap and allocate space for row_merge_rec_offsets()
 and mrec_buf_t[K + 1].
 @return memory heap */
static mem_heap_t *row_merge_heap_create_with_k(
    const dict_index_t *index, /*!< in: record descriptor */
    mrec_buf_t **buf,          /*!< out: K buffers */
    std::vector<ulint *> & offsets,
    const ulint K)
{
  ulint i = 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index);
  mem_heap_t *heap =
      mem_heap_create(K * i * sizeof(ulint *) + (K + 1) * sizeof **buf);

  *buf = static_cast<mrec_buf_t *>(mem_heap_alloc(heap, (K + 1) * sizeof **buf));

  for (ulint k = 0; k < K; k++) {
    ulint * offs = static_cast<ulint *>(mem_heap_alloc(heap, i * sizeof(ulint *)));
    offsets[k] = offs;
    offsets[k][0] = i;
    offsets[k][1] = dict_index_get_n_fields(index);
  }

  return (heap);
}

/** Read a merge block from the file system.
 @return true if request was successful, false if fail */
bool row_merge_read(int fd,                 /*!< in: file descriptor */
                     ulint offset,           /*!< in: offset where to read
                                             in number of row_merge_block_t
                                             elements */
                     row_merge_block_t *buf) /*!< out: data */
{
  os_offset_t ofs = ((os_offset_t)offset) * srv_sort_buf_size;
  dberr_t err;

  DBUG_TRACE;
  DBUG_PRINT("ib_merge_sort", ("fd=%d ofs=" UINT64PF, fd, ofs));
  DBUG_EXECUTE_IF("row_merge_read_failure", return false;);

  IORequest request;

  /* Merge sort pages are never compressed. */
  request.disable_compression();

  err = os_file_read_no_error_handling_int_fd(request, nullptr, fd, buf, ofs,
                                              srv_sort_buf_size, nullptr);

#ifdef POSIX_FADV_DONTNEED
  /* Each block is read exactly once.  Free up the file cache. */
  posix_fadvise(fd, ofs, srv_sort_buf_size, POSIX_FADV_DONTNEED);
#endif /* POSIX_FADV_DONTNEED */

  if (err != DB_SUCCESS) {
    ib::error(ER_IB_MSG_965) << "Failed to read merge block at " << ofs;
  }

  return err == DB_SUCCESS;
}

/** Write a merge block to the file system.
 @return true if request was successful, false if fail */
bool row_merge_write(int fd,          /*!< in: file descriptor */
                      ulint offset,    /*!< in: offset where to write,
                                       in number of row_merge_block_t elements */
                      const void *buf) /*!< in: data */
{
  size_t buf_len = srv_sort_buf_size;
  os_offset_t ofs = buf_len * (os_offset_t)offset;
  dberr_t err;

  DBUG_TRACE;
  DBUG_PRINT("ib_merge_sort", ("fd=%d ofs=" UINT64PF, fd, ofs));
  DBUG_EXECUTE_IF("row_merge_write_failure", return false;);

  IORequest request(IORequest::WRITE);

  request.disable_compression();

  err = os_file_write_int_fd(request, "(merge)", fd, buf, ofs, buf_len);

#ifdef POSIX_FADV_DONTNEED
  /* The block will be needed on the next merge pass,
  but it can be evicted from the file cache meanwhile. */
  posix_fadvise(fd, ofs, buf_len, POSIX_FADV_DONTNEED);
#endif /* POSIX_FADV_DONTNEED */

  return err == DB_SUCCESS;
}

/** Read a merge record.
 @return pointer to next record, or NULL on I/O error or end of list */
const byte *row_merge_read_rec(
    row_merge_block_t *block,  /*!< in/out: file buffer */
    mrec_buf_t *buf,           /*!< in/out: secondary buffer */
    const byte *b,             /*!< in: pointer to record */
    const dict_index_t *index, /*!< in: index of the record */
    int fd,                    /*!< in: file descriptor */
    ulint *foffs,              /*!< in/out: file offset */
    const mrec_t **mrec,       /*!< out: pointer to merge record,
                               or NULL on end of list
                               (non-NULL on I/O error) */
    ulint *offsets)            /*!< out: offsets of mrec */
{
  ulint extra_size;
  ulint data_size;
  ulint avail_size;

  bool is_variable = false;

  ut_ad(block);
  ut_ad(buf);
  ut_ad(b >= &block[0]);
  ut_ad(b < &block[srv_sort_buf_size]);
  ut_ad(index);
  ut_ad(foffs);
  ut_ad(mrec);
  ut_ad(offsets);

  ut_ad(*offsets == 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index));

  DBUG_TRACE;

  extra_size = *b++;

  if (UNIV_UNLIKELY(!extra_size)) {
    /* End of list */
    *mrec = nullptr;
    DBUG_PRINT("ib_merge_sort",
               ("read %p,%p,%d,%lu EOF\n", reinterpret_cast<const void *>(b),
                reinterpret_cast<const void *>(block), fd, ulong(*foffs)));
    return nullptr;
  }

  if (extra_size >= 0x80) {
    /* Read another byte of extra_size. */

    if (UNIV_UNLIKELY(b >= &block[srv_sort_buf_size])) {
      if (!row_merge_read(fd, ++(*foffs), block)) {
      err_exit:
        /* Signal I/O error. */
        *mrec = b;
        return nullptr;
      }

      /* Wrap around to the beginning of the buffer. */
      b = &block[0];
    }

    extra_size = (extra_size & 0x7f) << 8;
    extra_size |= *b++;
  }

  /* Normalize extra_size.  Above, value 0 signals "end of list". */
  extra_size--;

  /* Read the extra bytes. */

  if (UNIV_UNLIKELY(b + extra_size >= &block[srv_sort_buf_size])) {
    /* The record spans two blocks.  Copy the entire record
    to the auxiliary buffer and handle this as a special
    case. */

    avail_size = &block[srv_sort_buf_size] - b;
    ut_ad(avail_size < sizeof *buf);
    memcpy(*buf, b, avail_size);

    if (!row_merge_read(fd, ++(*foffs), block)) {
      goto err_exit;
    }

    /* Wrap around to the beginning of the buffer. */
    b = &block[0];

    /* Copy the record. */
    memcpy(*buf + avail_size, b, extra_size - avail_size);
    b += extra_size - avail_size;

    *mrec = *buf + extra_size;

    if (index->cached_offs_pddl.is_cached.load()) {
			const_cast<dict_index_t *> (index)->cached_offs_pddl.get_cached_offsets(
				offsets, 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index));
		} else {
			is_variable = rec_deserialize_init_offsets(*mrec, index, offsets);

			if (!is_variable && index->cached_offs_pddl.is_init.load()) {
				const_cast<dict_index_t *> (index)->cached_offs_pddl.set_offsets(
					offsets, 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index));
			}
		}

    data_size = rec_offs_data_size(offsets);

    /* These overflows should be impossible given that
    records are much smaller than either buffer, and
    the record starts near the beginning of each buffer. */
    ut_a(extra_size + data_size < sizeof *buf);
    ut_a(b + data_size < &block[srv_sort_buf_size]);

    /* Copy the data bytes. */
    memcpy(*buf + extra_size, b, data_size);
    b += data_size;

    goto func_exit;
  }

  *mrec = b + extra_size;

  if (index->cached_offs_pddl.is_cached.load()) {
		const_cast<dict_index_t *> (index)->cached_offs_pddl.get_cached_offsets(
			offsets, 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index));
	} else {
		is_variable = rec_deserialize_init_offsets(*mrec, index, offsets);
		// if we don't initilize index->cached_offs_pddl, do not
		// cache offsets.
		if (!is_variable && index->cached_offs_pddl.is_init.load()) {
			const_cast<dict_index_t *> (index)->cached_offs_pddl.set_offsets(
				offsets, 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index));
		}
	}

  data_size = rec_offs_data_size(offsets);
  ut_ad(extra_size + data_size < sizeof *buf);

  b += extra_size + data_size;

  if (UNIV_LIKELY(b < &block[srv_sort_buf_size])) {
    /* The record fits entirely in the block.
    This is the normal case. */
    goto func_exit;
  }

  /* The record spans two blocks.  Copy it to buf. */

  b -= extra_size + data_size;
  avail_size = &block[srv_sort_buf_size] - b;
  memcpy(*buf, b, avail_size);
  *mrec = *buf + extra_size;

  /* We cannot invoke rec_offs_make_valid() here, because there
  are no REC_N_NEW_EXTRA_BYTES between extra_size and data_size.
  Similarly, rec_offs_validate() would fail, because it invokes
  rec_get_status(). */
  ut_d(offsets[2] = (ulint)*mrec);
  ut_d(offsets[3] = (ulint)index);

  if (!row_merge_read(fd, ++(*foffs), block)) {
    goto err_exit;
  }

  /* Wrap around to the beginning of the buffer. */
  b = &block[0];

  /* Copy the rest of the record. */
  memcpy(*buf + avail_size, b, extra_size + data_size - avail_size);
  b += extra_size + data_size - avail_size;

func_exit:
  DBUG_PRINT("ib_merge_sort",
             ("%p,%p,fd=%d,%lu: %s", reinterpret_cast<const void *>(b),
              reinterpret_cast<const void *>(block), fd, ulong(*foffs),
              rec_printer(*mrec, 0, offsets).str().c_str()));
  return b;
}

/** Write a merge record. */
static void row_merge_write_rec_low(
    byte *b, /*!< out: buffer */
    ulint e, /*!< in: encoded extra_size */
#ifdef UNIV_DEBUG
    ulint size,           /*!< in: total size to write */
    int fd,               /*!< in: file descriptor */
    ulint foffs,          /*!< in: file offset */
#endif                    /* UNIV_DEBUG */
    const mrec_t *mrec,   /*!< in: record to write */
    const ulint *offsets) /*!< in: offsets of mrec */
#ifndef UNIV_DEBUG
#define row_merge_write_rec_low(b, e, size, fd, foffs, mrec, offsets) \
  row_merge_write_rec_low(b, e, mrec, offsets)
#endif /* !UNIV_DEBUG */
{
  DBUG_TRACE;

#ifdef UNIV_DEBUG
  const byte *const end = b + size;
#endif /* UNIV_DEBUG */
  assert(e == rec_offs_extra_size(offsets) + 1);
  DBUG_PRINT("ib_merge_sort",
             ("%p,fd=%d,%lu: %s", reinterpret_cast<const void *>(b), fd,
              ulong(foffs), rec_printer(mrec, 0, offsets).str().c_str()));

  if (e < 0x80) {
    *b++ = (byte)e;
  } else {
    *b++ = (byte)(0x80 | (e >> 8));
    *b++ = (byte)e;
  }

  memcpy(b, mrec - rec_offs_extra_size(offsets), rec_offs_size(offsets));
  assert(b + rec_offs_size(offsets) == end);
}

/** Write an end-of-list marker.
 @return pointer to end of block, or NULL on error */
static byte *row_merge_write_eof(
    row_merge_block_t *block, /*!< in/out: file buffer */
    byte *b,                  /*!< in: pointer to end of block */
    int fd,                   /*!< in: file descriptor */
    ulint *foffs);            /*!< in/out: file offset */

/** Write a merge record.
 @return pointer to end of block, or NULL on error */
static byte *row_merge_write_rec(
    row_merge_block_t *block, /*!< in/out: file buffer */
    mrec_buf_t *buf,          /*!< in/out: secondary buffer */
    byte *b,                  /*!< in: pointer to end of block */
    int fd,                   /*!< in: file descriptor */
    ulint *foffs,             /*!< in/out: file offset */
    const mrec_t *mrec,       /*!< in: record to write */
    const ulint *offsets)     /*!< in: offsets of mrec */
{
  ulint extra_size;
  ulint size;
  ulint avail_size;

  ut_ad(block);
  ut_ad(buf);
  ut_ad(b >= &block[0]);
  ut_ad(b < &block[srv_sort_buf_size]);
  ut_ad(mrec);
  ut_ad(foffs);
  ut_ad(mrec < &block[0] || mrec > &block[srv_sort_buf_size]);
  ut_ad(mrec < buf[0] || mrec > buf[1]);

  /* Normalize extra_size.  Value 0 signals "end of list". */
  extra_size = rec_offs_extra_size(offsets) + 1;

  size = extra_size + (extra_size >= 0x80) + rec_offs_data_size(offsets);

  if (UNIV_UNLIKELY(b + size >= &block[srv_sort_buf_size])) {
    /* The record spans two blocks.
    Copy it to the temporary buffer first. */
    avail_size = &block[srv_sort_buf_size] - b;

    row_merge_write_rec_low(buf[0], extra_size, size, fd, *foffs, mrec,
                            offsets);

    /* Copy the head of the temporary buffer, write
    the completed block, and copy the tail of the
    record to the head of the new block. */
    memcpy(b, buf[0], avail_size);

    if (!row_merge_write(fd, (*foffs)++, block)) {
      return (nullptr);
    }

    UNIV_MEM_INVALID(&block[0], srv_sort_buf_size);

    /* Copy the rest. */
    b = &block[0];
    memcpy(b, buf[0] + avail_size, size - avail_size);
    b += size - avail_size;
  } else {
    row_merge_write_rec_low(b, extra_size, size, fd, *foffs, mrec, offsets);
    b += size;
  }

  return (b);
}

/** Write an end-of-list marker.
 @return pointer to end of block, or NULL on error */
static byte *row_merge_write_eof(
    row_merge_block_t *block, /*!< in/out: file buffer */
    byte *b,                  /*!< in: pointer to end of block */
    int fd,                   /*!< in: file descriptor */
    ulint *foffs)             /*!< in/out: file offset */
{
  ut_ad(block);
  ut_ad(b >= &block[0]);
  ut_ad(b < &block[srv_sort_buf_size]);
  ut_ad(foffs);

  DBUG_TRACE;
  DBUG_PRINT("ib_merge_sort",
             ("%p,%p,fd=%d,%lu", reinterpret_cast<const void *>(b),
              reinterpret_cast<const void *>(block), fd, ulong(*foffs)));

  *b++ = 0;
  UNIV_MEM_ASSERT_RW(&block[0], b - &block[0]);
  UNIV_MEM_ASSERT_W(&block[0], srv_sort_buf_size);
#ifdef UNIV_DEBUG_VALGRIND
  /* The rest of the block is uninitialized.  Initialize it
  to avoid bogus warnings. */
  memset(b, 0xff, &block[srv_sort_buf_size] - b);
#endif /* UNIV_DEBUG_VALGRIND */

  if (!row_merge_write(fd, (*foffs)++, block)) {
    return nullptr;
  }

  UNIV_MEM_INVALID(&block[0], srv_sort_buf_size);
  return &block[0];
}

/** Create a temporary file if it has not been created already.
@param[in,out]	tmpfd	temporary file handle
@param[in]	path	location for creating temporary file
@return file descriptor, or -1 on failure */
static MY_ATTRIBUTE((warn_unused_result)) int row_merge_tmpfile_if_needed(
    int *tmpfd, const char *path) {
  if (*tmpfd < 0) {
    *tmpfd = row_merge_file_create_low(path);
    if (*tmpfd >= 0) {
      MONITOR_ATOMIC_INC(MONITOR_ALTER_TABLE_SORT_FILES);
    }
  }

  return (*tmpfd);
}

/** Create a temporary file for merge sort if it was not created already.
@param[in,out]	file	merge file structure
@param[in]	tmpfd	temporary file handle
@param[in]	nrec	number of records in the file
@param[in]	path	location for creating temporary file
@return file descriptor, or -1 on failure */
static MY_ATTRIBUTE((warn_unused_result)) int row_merge_file_create_if_needed(
    merge_file_t *file, int *tmpfd, ulint nrec, const char *path) {
  DBUG_EXECUTE_IF("row_merge_file_create_if_needed_INJECT_ERR", return -1;);
  ut_ad(file->fd < 0 || *tmpfd >= 0);
  if (file->fd < 0 && row_merge_file_create(file, path) >= 0) {
    MONITOR_ATOMIC_INC(MONITOR_ALTER_TABLE_SORT_FILES);
    if (row_merge_tmpfile_if_needed(tmpfd, path) < 0) {
      return (-1);
    }

    file->n_rec = nrec;
  }

  ut_ad(file->fd < 0 || *tmpfd >= 0);
  return (file->fd);
}

/** Copy the merge data tuple from another merge data tuple.
@param[in]	mtuple		source merge data tuple
@param[in,out]	prev_mtuple	destination merge data tuple
@param[in]	n_unique	number of unique fields exist in the mtuple
@param[in,out]	heap		memory heap where last_mtuple allocated */
static void row_mtuple_create(const mtuple_t *mtuple, mtuple_t *prev_mtuple,
                              ulint n_unique, mem_heap_t *heap) {
  memcpy(prev_mtuple->fields, mtuple->fields,
         n_unique * sizeof *mtuple->fields);

  dfield_t *field = prev_mtuple->fields;

  for (ulint i = 0; i < n_unique; i++) {
    dfield_dup(field++, heap);
  }
}

/** Compare two merge data tuples.
@param[in]	prev_mtuple	merge data tuple
@param[in]	current_mtuple	merge data tuple
@param[in,out]	dup		reporter of duplicates
@retval positive, 0, negative if current_mtuple is greater, equal, less, than
last_mtuple. */
static int row_mtuple_cmp(const mtuple_t *prev_mtuple,
                          const mtuple_t *current_mtuple,
                          row_merge_dup_t *dup) {
  ut_ad(dup->index->is_clustered());
  const ulint n_unique = dict_index_get_n_unique(dup->index);

  return (row_merge_tuple_cmp(dup->index, n_unique, n_unique, *current_mtuple,
                              *prev_mtuple, dup));
}

struct alignas(ut::INNODB_CACHE_LINE_SIZE) ThreadSampler {
  mem_heap_t *heap;
  std::vector<mtuple_t> sample_mtuple_vector;
  ulint visit_count;
  ulint max_sample_cnt;

  void init(ulint max_sample_cnt_param) {
    heap = mem_heap_create(1000, UT_LOCATION_HERE);
    visit_count = 0;
    max_sample_cnt = max_sample_cnt_param;
  }
  void destroy() {
    sample_mtuple_vector.clear();
    sample_mtuple_vector.~vector();
    mem_heap_free(heap);
  }
};

struct QuantilerComparator {

  void init(dict_index_t *index_param, ulint n_uniq_param, ulint n_field_param) {
    index = index_param;
    n_uniq = n_uniq_param;
    n_field = n_field_param;
  }

  // return true if a < b
  bool operator()(const mtuple_t &a, const mtuple_t &b) {
    return row_merge_tuple_cmp(index, n_uniq, n_field, a, b, nullptr) < 0;
  }

  dict_index_t *index;
  ulint n_uniq;
  ulint n_field;
};

struct Quantiler {
  struct Point {
    const mrec_t *mrec;
    ulint *offsets;

    Point() : mrec(nullptr), offsets(nullptr) {}
    Point(const mrec_t *m, ulint *o) : mrec(m), offsets(o) {}
  };

  struct PointComparator {
    void init(dict_index_t *index_param, struct TABLE *table_param) {
      index = index_param;
      table = table_param;
    }

    bool operator()(const Quantiler::Point &q1, const Quantiler::Point &q2) const {
      int cmp = cmp_rec_rec_simple(q1.mrec, q2.mrec, q1.offsets, q2.offsets, index, table);
      return cmp < 0;
    }

    dict_index_t *index;
    struct TABLE *table;
  };

  void init(int scan_parallel_param, int sort_parallel_param,
            dict_index_t *index_param, ulint sample_step_param,
            struct TABLE *table_param, ulint max_sample_cnt_param);
  void destroy();
  void sample_visit_rec(int thread_id, const mtuple_t *mtuple);
  void build_quantiles();
  int partition_to(const mrec_t *target, ulint *offsets) const;

  void mtuple_dup_from(mtuple_t &to, const mtuple_t *from, mem_heap_t *heap);
  mrec_t *mtuple_convert_to_mrec(const mtuple_t *mtuple, byte **b);

  int scan_parallel;
  int sort_parallel;
  ulint sample_step;
  dict_index_t *index;

  ThreadSampler *samplers;
  void *samplers_addr;

  mem_heap_t *convert_heap;
  std::vector<Point> quantiles;
  PointComparator point_comparator;
};

struct Quantilers {
  Quantiler *quantiler_arr;
  int count;

  void init(int index_count, int scan_parallel_param, int sort_parallel_param,
            dict_index_t **index_param, ulint sample_step_param,
            struct TABLE *table_param, ulint max_sample_cnt_param) {
    count = index_count;
    quantiler_arr = new Quantiler[count];
    for (int i = 0; i < count; i++) {
      quantiler_arr[i].init(scan_parallel_param, sort_parallel_param,
                         index_param[i], sample_step_param, table_param,
                         max_sample_cnt_param);
    }
  }

  void destroy() {
    for (int i = 0; i < count; i++) {
      quantiler_arr[i].destroy();
    }
    delete []quantiler_arr;
  }

  void sample_visit_rec(int index_id, int thread_id, const mtuple_t *mtuple) {
    assert(index_id < count);
    quantiler_arr[index_id].sample_visit_rec(thread_id, mtuple);
  }

  void build_quantiles(size_t parallel_sort_threads) {
    for (int i = 0; i < count; i++) {
      quantiler_arr[i].sort_parallel = parallel_sort_threads;
      quantiler_arr[i].build_quantiles();
    }
  }

};

void Quantiler::init(int scan_parallel_param, int sort_parallel_param,
                       dict_index_t *index_param, ulint sample_step_param,
                       struct TABLE *table_param, ulint max_sample_cnt_param) {
  this->scan_parallel = scan_parallel_param;
  this->sort_parallel = sort_parallel_param;
  this->index = index_param;
  this->sample_step = sample_step_param;

  samplers_addr = ut::malloc(scan_parallel * sizeof(ThreadSampler) +
                                  ut::INNODB_CACHE_LINE_SIZE);
  samplers = static_cast<ThreadSampler*>(::ut_align(samplers_addr,
                                         ut::INNODB_CACHE_LINE_SIZE));
  for (int i = 0; i < scan_parallel; i++) {
    ThreadSampler *cur = samplers + i;
    new (cur) ThreadSampler();
    cur->init(max_sample_cnt_param);
  }

  convert_heap = mem_heap_create(1000, UT_LOCATION_HERE);

  point_comparator.init(index_param, table_param);

}

void Quantiler::destroy() {
  for (int i = 0; i < scan_parallel; i++) {
    samplers[i].destroy();
  }
  ut::free(samplers_addr);
  quantiles.clear();
  mem_heap_free(convert_heap);
}

void Quantiler::mtuple_dup_from(mtuple_t &to, const mtuple_t *from, mem_heap_t *heap) {
  ulint n_fields = dict_index_get_n_fields(index);
  to.fields = static_cast<dfield_t*>(mem_heap_alloc(heap, n_fields * sizeof(dfield_t)));

  for (ulint f = 0; f < n_fields; f++) {
    dfield_copy(&(to.fields[f]), from->fields[f].clone(heap));
  }
}

void Quantiler::sample_visit_rec(int thread_id, const mtuple_t *mtuple) {
  ut_ad(thread_id >= 0 && thread_id < scan_parallel);

  ThreadSampler &s = samplers[thread_id];
  if (s.visit_count >= sample_step && s.max_sample_cnt > 0) {
    mtuple_t new_sample_mtuple;
    mtuple_dup_from(new_sample_mtuple, mtuple, s.heap);
    s.sample_mtuple_vector.push_back(new_sample_mtuple);
    s.visit_count = 0;
    s.max_sample_cnt--;
  }
  s.visit_count++;
}

mrec_t *Quantiler::mtuple_convert_to_mrec(const mtuple_t *mtuple, byte **b) {
  mrec_t *ret = nullptr;

  ulint size = 0;
  ulint extra_size = 0;
  size = rec_get_serialize_size(index, mtuple->fields, dict_index_get_n_fields(index),
                                     nullptr, &extra_size, MAX_ROW_VERSION);
  ut_ad(size >= extra_size);

  /* Encode extra_size + 1 */
  if (extra_size + 1 < 0x80) {
    *(*b)++ = (byte)(extra_size + 1);
  } else {
    ut_ad((extra_size + 1) < 0x8000);
    *(*b)++ = (byte)(0x80 | ((extra_size + 1) >> 8));
    *(*b)++ = (byte)(extra_size + 1);
  }

  ret = *b + extra_size;
  rec_serialize_dtuple(*b + extra_size, index, mtuple->fields,
                             dict_index_get_n_fields(index), nullptr);
  *b += size;
  return ret;
}

void Quantiler::build_quantiles() {
  std::vector<mtuple_t> total_sample_vector;
  for (int t = 0; t < scan_parallel; t++) {
    std::vector<mtuple_t> &cur_vec = samplers[t].sample_mtuple_vector;
    std::copy(cur_vec.begin(), cur_vec.end(), std::back_inserter(total_sample_vector));
  }

  ut_a(total_sample_vector.size() >= (size_t)sort_parallel);
  point_comparator.index = index;

  QuantilerComparator qc;
  qc.init(index, dict_index_get_n_unique(index), dict_index_get_n_unique(index));
  std::sort(total_sample_vector.begin(), total_sample_vector.end(), qc);
  DBUG_EXECUTE_IF("different_subtree_level_inject",
                    total_sample_vector.resize(sort_parallel * 10););

  const int seg_size = total_sample_vector.size() / sort_parallel;
  const int remainder = total_sample_vector.size() % sort_parallel;
  const ulint offsets_i = 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index);
  quantiles.clear();

  for (int t = 0; t < sort_parallel - 1; t++) {
    int q_idx = seg_size * (t + 1) + (t < remainder ? 1 : 0) - 1;
    ut_ad(q_idx >= 0);
    mtuple_t q_mtuple = total_sample_vector[q_idx];
    ulint size = 0;
    ulint extra_size = 0;
    size = rec_get_serialize_size(index, q_mtuple.fields,
            dict_index_get_n_fields(index), nullptr, &extra_size, MAX_ROW_VERSION);

    size += (extra_size >= 0x80) + 2;

    byte *block = (byte*)mem_heap_alloc(convert_heap, size);

    mrec_t *q_mrec = mtuple_convert_to_mrec(&q_mtuple, &block);

    ulint *q_offsets = nullptr;
    q_offsets = static_cast<ulint*>(mem_heap_alloc(convert_heap,
                                    offsets_i * sizeof(ulint)));
    q_offsets[0] = offsets_i;
    q_offsets[1] = dict_index_get_n_fields(index);
    rec_deserialize_init_offsets(q_mrec, index, q_offsets);

    Quantiler::Point q{q_mrec, q_offsets};
    quantiles.push_back(q);
  }
#ifdef UNIV_DEBUG_PARALLEL_DDL
  ib::info() << "[TXSQL PARALLEL DDL] Building quantiles, samples["
             << total_sample_vector.size() << "] "
             << "Quantiles[" << quantiles.size() << "].";
#endif /* UNIV_DEBUG_PARALLEL_DDL */
}

int Quantiler::partition_to(const mrec_t *target_mrec, ulint *offsets) const {
  Quantiler::Point target;
  target.mrec = target_mrec;
  target.offsets = offsets;

  auto find_iter = std::lower_bound(quantiles.begin(), quantiles.end(),
                                    target, point_comparator);
  int partition_id = find_iter - quantiles.begin();

  ut_ad(partition_id >= 0 && partition_id < sort_parallel);
  return partition_id;
}

/** Reads clustered index of the table and create temporary files
containing the index entries for the indexes to be built.
@param[in]	trx		transaction
@param[in,out]	table		MySQL table object, for reporting erroneous
records
@param[in]	old_table	table where rows are read from
@param[in]	new_table	table where indexes are created; identical to
old_table unless creating a PRIMARY KEY
@param[in]	online		true if creating indexes online
@param[in]	index		indexes to be created
@param[in]	files		temporary files
@param[in]	key_numbers	MySQL key numbers to create
@param[in]	n_index		number of indexes to create
@param[in]	add_cols	default values of added columns, or NULL
@param[in]	add_v		newly added virtual columns along with indexes
@param[in]	col_map		mapping of old column numbers to new ones, or
NULL if old_table == new_table
@param[in]	add_autoinc	number of added AUTO_INCREMENT columns, or
ULINT_UNDEFINED if none is added
@param[in,out]	sequence	autoinc sequence
@param[in,out]	block		file buffer
@param[in]	skip_pk_sort	whether the new PRIMARY KEY will follow
existing order
@param[in,out]	tmpfd		temporary file handle
@param[in,out]	stage		performance schema accounting object, used by
ALTER TABLE. stage->n_pk_recs_inc() will be called for each record read and
stage->inc() will be called for each page read.
@param[in]	eval_table	mysql table used to evaluate virtual column
                                value, see innobase_get_computed_value().
@return DB_SUCCESS or error */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    row_merge_read_clustered_index(
        trx_t *trx, struct TABLE *table, const dict_table_t *old_table,
        dict_table_t *new_table, bool online, dict_index_t **index,
        merge_file_t *files, const ulint *key_numbers, ulint n_index,
        const dtuple_t *add_cols, const dict_add_v_col_t *add_v,
        const ulint *col_map, ulint add_autoinc, ddl::Sequence &sequence,
        row_merge_block_t *block, bool skip_pk_sort, int *tmpfd,
        Alter_stage *stage, struct TABLE *eval_table, Quantilers *quantilers) {
  dict_index_t *clust_index;         /* Clustered index */
  mem_heap_t *row_heap;              /* Heap memory to create
                                     clustered index tuples */
  row_merge_buf_t **merge_buf;       /* Temporary list for records*/
  mem_heap_t *v_heap = nullptr;      /* Heap memory to process large
                                  data for virtual column */
  btr_pcur_t pcur;                   /* Cursor on the clustered
                                     index */
  mtr_t mtr;                         /* Mini-transaction */
  dberr_t err = DB_SUCCESS;          /* Return code */
  ulint n_nonnull = 0;               /* number of columns
                                     changed to NOT NULL */
  ulint *nonnull = nullptr;          /* NOT NULL columns */
  BtrBulk *clust_btr_bulk = nullptr;
  bool clust_temp_file = false;
  mem_heap_t *mtuple_heap = nullptr;
  mtuple_t prev_mtuple;
  mem_heap_t *conv_heap = nullptr;
  Flush_observer *observer = trx->flush_observer;
  DBUG_TRACE;

  ut_ad((old_table == new_table) == !col_map);
  ut_ad(!add_cols || col_map);

  trx->op_info = "reading clustered index";

  /* Create and initialize memory for record buffers */

  merge_buf = static_cast<row_merge_buf_t **>(
      ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, n_index * sizeof *merge_buf));

  row_merge_dup_t clust_dup = {index[0], table, col_map, 0};
  dfield_t *prev_fields;
  const ulint n_uniq = dict_index_get_n_unique(index[0]);

  ut_ad(trx->mysql_thd != nullptr);

  const char *path = thd_innodb_tmpdir(trx->mysql_thd);

  ut_ad(!skip_pk_sort || index[0]->is_clustered());
  /* There is no previous tuple yet. */
  prev_mtuple.fields = nullptr;

  for (ulint i = 0; i < n_index; i++) {
    assert(!(index[i]->type & DICT_FTS));
    merge_buf[i] = row_merge_buf_create(index[i]);
  }

  mtr_start(&mtr);

  /* Find the clustered index and create a persistent cursor
  based on that. */

  clust_index = const_cast<dict_table_t *>(old_table)->first_index();

  pcur.open_at_side(true, clust_index, BTR_SEARCH_LEAF, false, 0, &mtr);

  if (old_table != new_table) {
    /* The table is being rebuilt.  Identify the columns
    that were flagged NOT NULL in the new table, so that
    we can quickly check that the records in the old table
    do not violate the added NOT NULL constraints. */

    nonnull = static_cast<ulint *>(
        ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY,
          new_table->get_n_cols() * sizeof *nonnull));

    for (ulint i = 0; i < old_table->get_n_cols(); i++) {
      if (old_table->get_col(i)->prtype & DATA_NOT_NULL) {
        continue;
      }

      const ulint j = col_map[i];

      if (j == ULINT_UNDEFINED) {
        /* The column was dropped. */
        continue;
      }

      if (new_table->get_col(j)->prtype & DATA_NOT_NULL) {
        nonnull[n_nonnull++] = j;
      }
    }

    if (!n_nonnull) {
      ut::free(nonnull);
      nonnull = nullptr;
    }
  }

  row_heap = mem_heap_create(sizeof(mrec_buf_t), UT_LOCATION_HERE);

  if (dict_table_is_comp(old_table) && !dict_table_is_comp(new_table)) {
    conv_heap = mem_heap_create(sizeof(mrec_buf_t), UT_LOCATION_HERE);
  }

  if (skip_pk_sort) {
    prev_fields =
        static_cast<dfield_t *>(ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY,
          n_uniq * sizeof *prev_fields));
    mtuple_heap = mem_heap_create(sizeof(mrec_buf_t), UT_LOCATION_HERE);
  } else {
    prev_fields = nullptr;
  }

  /* Scan the clustered index. */
  for (;;) {
    const rec_t *rec;
    ulint *offsets;
    const dtuple_t *row;
    row_ext_t *ext = nullptr;
    page_cur_t *cur = pcur.get_page_cur();

    mem_heap_empty(row_heap);

    page_cur_move_to_next(cur);

    stage->n_pk_recs_inc();

    if (page_cur_is_after_last(cur)) {
      stage->inc(1);

      if (UNIV_UNLIKELY(trx_is_interrupted(trx))) {
        err = DB_INTERRUPTED;
        trx->error_key_num = 0;
        goto func_exit;
      }

      if (online && old_table != new_table) {
        err = row_log_table_get_error(clust_index);
        if (err != DB_SUCCESS) {
          trx->error_key_num = 0;
          goto func_exit;
        }
      }

#ifndef UNIV_DEBUG
#define dbug_run_purge false
#else  /* UNIV_DEBUG */
      bool dbug_run_purge = false;
#endif /* UNIV_DEBUG */
      DBUG_EXECUTE_IF("ib_purge_on_create_index_page_switch",
                      dbug_run_purge = true;);

      if (dbug_run_purge ||
          rw_lock_get_waiters(dict_index_get_lock(clust_index))) {
        /* There are waiters on the clustered
        index tree lock, likely the purge
        thread. Store and restore the cursor
        position, and yield so that scanning a
        large table will not starve other
        threads. */

        /* Store the cursor position on the last user
        record on the page. */
        pcur.move_to_prev_on_page();
        /* Leaf pages must never be empty, unless
        this is the only page in the index tree. */
        ut_ad(pcur.is_on_user_rec() ||
              pcur.get_block()->page.id.page_no() ==
                  clust_index->page);

        pcur.store_position(&mtr);
        mtr_commit(&mtr);

        if (dbug_run_purge) {
          /* This is for testing
          purposes only (see
          DBUG_EXECUTE_IF above).  We
          signal the purge thread and
          hope that the purge batch will
          complete before we execute
          btr_pcur_restore_position(). */
          trx_purge_run();
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        /* Give the waiters a chance to proceed. */
        std::this_thread::yield();

        mtr_start(&mtr);
        /* Restore position on the record, or its
        predecessor if the record was purged
        meanwhile. */
        pcur.restore_position(BTR_SEARCH_LEAF, &mtr, UT_LOCATION_HERE);
        /* Move to the successor of the
        original record. */
        if (!pcur.move_to_next_user_rec(&mtr)) {
        end_of_index:
          row = nullptr;
          mtr_commit(&mtr);
          mem_heap_free(row_heap);
          ut::free(nonnull);
          goto write_buffers;
        }
      } else {
        page_no_t next_page_no;
        buf_block_t *block;

        next_page_no = btr_page_get_next(page_cur_get_page(cur), &mtr);

        if (next_page_no == FIL_NULL) {
          goto end_of_index;
        }

        block = page_cur_get_block(cur);
        block =
            btr_block_get(page_id_t(block->page.id.space(), next_page_no),
                          block->page.size, BTR_SEARCH_LEAF, UT_LOCATION_HERE,
                          clust_index, &mtr);

        btr_leaf_page_release(page_cur_get_block(cur), BTR_SEARCH_LEAF, &mtr);
        page_cur_set_before_first(block, cur);
        page_cur_move_to_next(cur);

        ut_ad(!page_cur_is_after_last(cur));
      }
    }

    rec = page_cur_get_rec(cur);

    offsets =
        rec_get_offsets(rec, clust_index, nullptr, ULINT_UNDEFINED,
                        UT_LOCATION_HERE, &row_heap);

    if (online) {
      /* Perform a REPEATABLE READ.

      When rebuilding the table online,
      row_log_table_apply() must not see a newer
      state of the table when applying the log.
      This is mainly to prevent false duplicate key
      errors, because the log will identify records
      by the PRIMARY KEY, and also to prevent unsafe
      BLOB access.

      When creating a secondary index online, this
      table scan must not see records that have only
      been inserted to the clustered index, but have
      not been written to the online_log of
      index[]. If we performed READ UNCOMMITTED, it
      could happen that the ADD INDEX reaches
      ONLINE_INDEX_COMPLETE state between the time
      the DML thread has updated the clustered index
      but has not yet accessed secondary index. */
      ut_ad(MVCC::is_view_active(trx->read_view));

      if (!trx->read_view->changes_visible(
              row_get_rec_trx_id(rec, clust_index, offsets), old_table->name)) {
        rec_t *old_vers;

        row_vers_build_for_consistent_read(rec, &mtr, clust_index, &offsets,
                                           trx->read_view, &row_heap, row_heap,
                                           &old_vers, nullptr, nullptr);

        rec = old_vers;

        if (!rec) {
          continue;
        }
      }

      if (rec_get_deleted_flag(rec, dict_table_is_comp(old_table))) {
        /* This record was deleted in the latest
        committed version, or it was deleted and
        then reinserted-by-update before purge
        kicked in. Skip it. */
        continue;
      }

      ut_ad(!rec_offs_any_null_extern(clust_index, rec, offsets));
    } else if (rec_get_deleted_flag(rec, dict_table_is_comp(old_table))) {
      /* Skip delete-marked records.

      Skipping delete-marked records will make the
      created indexes unuseable for transactions
      whose read views were created before the index
      creation completed, but preserving the history
      would make it tricky to detect duplicate
      keys. */
      continue;
    }

    /* When !online, we are holding a lock on old_table, preventing
    any inserts that could have written a record 'stub' before
    writing out off-page columns. */
    ut_ad(!rec_offs_any_null_extern(clust_index, rec, offsets));

    /* Build a row based on the clustered index. */

    row = row_build_w_add_vcol(ROW_COPY_POINTERS, clust_index, rec, offsets,
                               new_table, add_cols, add_v, col_map, &ext,
                               row_heap);
    ut_ad(row);

    for (ulint i = 0; i < n_nonnull; i++) {
      const dfield_t *field = &row->fields[nonnull[i]];

      ut_ad(dfield_get_type(field)->prtype & DATA_NOT_NULL);

      if (dfield_is_null(field)) {
        err = DB_INVALID_NULL;
        trx->error_key_num = 0;
        goto func_exit;
      }
    }

    if (add_autoinc != ULINT_UNDEFINED) {
      ut_ad(add_autoinc < new_table->get_n_user_cols());

      const dfield_t *dfield;

      dfield = dtuple_get_nth_field(row, add_autoinc);
      if (dfield_is_null(dfield)) {
        goto write_buffers;
      }

      const dtype_t *dtype = dfield_get_type(dfield);
      byte *b = static_cast<byte *>(dfield_get_data(dfield));

      if (sequence.eof()) {
        err = DB_ERROR;
        trx->error_key_num = 0;

        ib_senderrf(trx->mysql_thd, IB_LOG_LEVEL_ERROR, ER_AUTOINC_READ_FAILED);

        goto func_exit;
      }

      ulonglong value = sequence++;

      switch (dtype_get_mtype(dtype)) {
        case DATA_INT: {
          bool usign;
          ulint len = dfield_get_len(dfield);

          usign = dtype_get_prtype(dtype) & DATA_UNSIGNED;
          mach_write_ulonglong(b, value, len, usign);

          break;
        }

        case DATA_FLOAT:
          mach_float_write(b, static_cast<float>(value));
          break;

        case DATA_DOUBLE:
          mach_double_write(b, static_cast<double>(value));
          break;

        default:
          ut_ad(0);
      }
    }

  write_buffers:
    /* Build all entries for all the indexes to be created
    in a single scan of the clustered index. */

    bool skip_sort = skip_pk_sort && merge_buf[0]->index->is_clustered();

    for (ulint i = 0; i < n_index; i++, skip_sort = false) {
      row_merge_buf_t *buf = merge_buf[i];
      merge_file_t *file = &files[i];
      ulint rows_added = 0;
      ulint multi_val_added = 0;

      if (UNIV_LIKELY(row &&
                      (rows_added = row_merge_buf_add(
                           buf, old_table, new_table,
                           row, ext, conv_heap, &err, &v_heap,
                           eval_table, trx, &multi_val_added)) &&
                      multi_val_added == 0)) {
        if (rows_added == UNIV_NO_INDEX_VALUE) {
          /* Nothing to be indexed from current row, skip this index */
          ut_ad(buf->index->is_multi_value());
          continue;
        }

        /* If we are creating FTS index,
        a single row can generate more
        records for tokenized word */
        file->n_rec += rows_added;

        if (err != DB_SUCCESS) {
          ut_ad(err == DB_TOO_BIG_RECORD || err == DB_COMPUTE_VALUE_FAILED);
          break;
        }

        /* add success, continue next index entry for the current pk record */
        if (quantilers) {
          quantilers->sample_visit_rec((int)i, 0, &buf->tuples[buf->n_tuples - 1]);
        }

        assert(!(buf->index->type & DICT_FTS));

        if (skip_sort) {
          ut_ad(buf->n_tuples > 0);
          const mtuple_t *curr = &buf->tuples[buf->n_tuples - 1];

          ut_ad(i == 0);
          ut_ad(merge_buf[0]->index->is_clustered());
          /* Detect duplicates by comparing the
          current record with previous record.
          When temp file is not used, records
          should be in sorted order. */
          if (prev_mtuple.fields != nullptr &&
              (row_mtuple_cmp(&prev_mtuple, curr, &clust_dup) == 0)) {
            err = DB_DUPLICATE_KEY;
            trx->error_key_num = key_numbers[0];
            goto func_exit;
          }

          prev_mtuple.fields = curr->fields;
        }

        continue;
      }

      if (multi_val_added != 0) {
        /* This signalizes that a partial row was added to the buffer due to
        reaching its size limit. We need to increment the file size by this
        amount */
        file->n_rec += multi_val_added;
      }

      if (err == DB_COMPUTE_VALUE_FAILED) {
        trx->error_key_num = i;
        goto func_exit;
      }

      /* The buffer must be sufficiently large
      to hold at least one record. It may only
      be empty when we reach the end of the
      clustered index. row_merge_buf_add()
      must not have been called in this loop. */
      ut_ad(buf->n_tuples || row == nullptr);

      /* We have enough data tuples to form a block.
      Sort them and write to disk if temp file is used
      or insert into index if temp file is not used. */
      ut_ad(old_table == new_table ? !buf->index->is_clustered()
                                   : (i == 0) == buf->index->is_clustered());

      /* We have enough data tuples to form a block.
      Sort them (if !skip_sort) and write to disk. */
    write_buffer_and_retry:
      if (buf->n_tuples) {
        if (skip_sort) {
          /* Temporary File is not used.
          so insert sorted block to the index */
          if (row != nullptr) {
            bool mtr_committed = false;

            /* We are not at the end of
            the scan yet. We must
            mtr_commit() in order to be
            able to call log_free_check()
            in row_merge_insert_index_tuples().
            Due to mtr_commit(), the
            current row will be invalid, and
            we must reread it on the next
            loop iteration. */
            if (!mtr_committed) {
              pcur.move_to_prev_on_page();
              pcur.store_position(&mtr);
              mtr_commit(&mtr);
            }
          }

          mem_heap_empty(mtuple_heap);
          prev_mtuple.fields = prev_fields;

          row_mtuple_create(&buf->tuples[buf->n_tuples - 1], &prev_mtuple,
                            n_uniq, mtuple_heap);

          if (clust_btr_bulk == nullptr) {
            clust_btr_bulk = ut::new_withkey<BtrBulk>(
                UT_NEW_THIS_FILE_PSI_KEY, index[i], trx->id, observer);
            err = clust_btr_bulk->init();
            if (err != DB_SUCCESS) {
              ut::delete_(clust_btr_bulk);
              clust_btr_bulk = nullptr;
              break;
            }
          } else {
            clust_btr_bulk->latch();
          }

          err = row_merge_insert_index_tuples(trx, index[i], old_table, -1,
                                              nullptr, buf, clust_btr_bulk);

          if (row == nullptr) {
            err = clust_btr_bulk->finish(err);
            ut::delete_(clust_btr_bulk);
            clust_btr_bulk = nullptr;
          } else {
            /* Release latches for possible
            log_free_chck in spatial index
            build. */
            clust_btr_bulk->release();
          }

          if (err != DB_SUCCESS) {
            break;
          }

          if (row != nullptr) {
            /* Restore the cursor on the
            previous clustered index record,
            and empty the buffer. The next
            iteration of the outer loop will
            advance the cursor and read the
            next record (the one which we
            had to ignore due to the buffer
            overflow). */
            mtr_start(&mtr);
            pcur.restore_position(BTR_SEARCH_LEAF, &mtr, UT_LOCATION_HERE);
            buf = row_merge_buf_empty(buf);
            /* Restart the outer loop on the
            record. We did not insert it
            into any index yet. */
            ut_ad(i == 0);
            break;
          }
        } else if (dict_index_is_unique(buf->index)) {
          row_merge_dup_t dup = {buf->index, table, col_map, 0};

          row_merge_buf_sort(buf, &dup);

          if (dup.n_dup) {
            err = DB_DUPLICATE_KEY;
            trx->error_key_num = key_numbers[i];
            break;
          }
        } else {
          row_merge_buf_sort(buf, nullptr);
        }
      } else if (online && new_table == old_table) {
        /* Note the newest transaction that
        modified this index when the scan was
        completed. We prevent older readers
        from accessing this index, to ensure
        read consistency. */

        trx_id_t max_trx_id;

        ut_a(row == nullptr);
        rw_lock_x_lock(dict_index_get_lock(buf->index), UT_LOCATION_HERE);
        ut_a(dict_index_get_online_status(buf->index) == ONLINE_INDEX_CREATION);

        max_trx_id = row_log_get_max_trx(buf->index);

        if (max_trx_id > buf->index->trx_id) {
          buf->index->trx_id = max_trx_id;
        }

        rw_lock_x_unlock(dict_index_get_lock(buf->index));
      }

      /* Secondary index and clustered index which is
      not in sorted order can use the temporary file.
      Fulltext index should not use the temporary file. */
      if (!skip_sort) {
        /* In case we can have all rows in sort buffer,
        we can insert directly into the index without
        temporary file if clustered index does not uses
        temporary file. */
        assert(!(buf->index->type & DICT_FTS));
        if (row == nullptr && file->fd == -1 && !clust_temp_file) {
          DBUG_EXECUTE_IF("row_merge_write_failure",
                          err = DB_TEMP_FILE_WRITE_FAIL;
                          trx->error_key_num = i; goto all_done;);

          DBUG_EXECUTE_IF("row_merge_tmpfile_fail", err = DB_OUT_OF_MEMORY;
                          trx->error_key_num = i; goto all_done;);

          BtrBulk btr_bulk(index[i], trx->id, observer);
          err = btr_bulk.init();
          if (err == DB_SUCCESS) {
            err = row_merge_insert_index_tuples(trx, index[i], old_table, -1,
                                                nullptr, buf, &btr_bulk);

            err = btr_bulk.finish(err);
          }

          DBUG_EXECUTE_IF("row_merge_insert_big_row", err = DB_TOO_BIG_RECORD;);

          if (err != DB_SUCCESS) {
            break;
          }
        } else {
          if (row_merge_file_create_if_needed(file, tmpfd, buf->n_tuples,
                                              path) < 0) {
            err = DB_OUT_OF_MEMORY;
            trx->error_key_num = i;
            goto func_exit;
          }

          /* Ensure that duplicates in the
          clustered index will be detected before
          inserting secondary index records. */
          if (buf->index->is_clustered()) {
            clust_temp_file = true;
          }

          ut_ad(file->n_rec > 0);

          row_merge_buf_write(buf, file, block);

          if (!row_merge_write(file->fd, file->offset++, block)) {
            err = DB_TEMP_FILE_WRITE_FAIL;
            trx->error_key_num = i;
            break;
          }

          UNIV_MEM_INVALID(&block[0], srv_sort_buf_size);
        }
      }
      merge_buf[i] = row_merge_buf_empty(buf);

      if (UNIV_LIKELY(row != nullptr)) {
        /* Try writing the record again, now
        that the buffer has been written out
        and emptied. */

        if (UNIV_UNLIKELY(!(rows_added = row_merge_buf_add(
                                buf, old_table, new_table,
                                row, ext, conv_heap, &err,
                                &v_heap, table, trx, &multi_val_added)))) {
          /* An empty buffer should have enough
          room for at least one record. */
          ut_error;
        }

        if (err != DB_SUCCESS) {
          break;
        }

        /* add success, continue next index entry for the current pk record */
        if (quantilers) {
          quantilers->sample_visit_rec((int)i, 0, &buf->tuples[buf->n_tuples - 1]);
        }

        ut_ad(rows_added != UNIV_NO_INDEX_VALUE);

        file->n_rec += rows_added;

        /* It's possible that there're some multi-value data left.
        So write current buffer to disk and try with an empty buffer again. */
        if (multi_val_added != 0) {
          goto write_buffer_and_retry;
        }
      }
    }

    if (row == nullptr) {
      goto all_done;
    }

    if (err != DB_SUCCESS) {
      goto func_exit;
    }

    if (v_heap) {
      mem_heap_empty(v_heap);
    }
  }

func_exit:
  if (mtr.is_active()) {
    mtr_commit(&mtr);
  }
  mem_heap_free(row_heap);
  ut::free(nonnull);

all_done:
  if (clust_btr_bulk != nullptr) {
    ut_ad(err != DB_SUCCESS);
    clust_btr_bulk->latch();
    err = clust_btr_bulk->finish(err);
    ut::delete_(clust_btr_bulk);
  }

  if (prev_fields != nullptr) {
    ut::free(prev_fields);
    mem_heap_free(mtuple_heap);
  }

  if (v_heap) {
    mem_heap_free(v_heap);
  }

  if (conv_heap != nullptr) {
    mem_heap_free(conv_heap);
  }

  for (ulint i = 0; i < n_index; i++) {
    row_merge_buf_free(merge_buf[i]);
  }

  ut::free(merge_buf);

  pcur.close();

  trx->op_info = "";

  return err;
}

ulint get_sample_step(dict_table_t * old_table, dict_index_t **indexes, ulint n_indexes,
                      THD *thd, size_t &max_sample_cnt) {
  size_t n_rows = old_table->stat_n_rows;
  size_t buffer_size = thd_txsql_ddl_buffer_size(thd);

  size_t index_rows_size = 0;
  for (ulint i = 0; i < n_indexes; i++) {
    index_rows_size += indexes[0]->get_max_size();
  }
  max_sample_cnt = buffer_size / std::max((ulint)1, index_rows_size);
  max_sample_cnt = std::min(max_sample_cnt, n_rows);
  ulint step = n_rows / (max_sample_cnt + 1) + 1;
  ut_a(n_rows / step <= max_sample_cnt);

  return step;
}

struct alignas(ut::INNODB_CACHE_LINE_SIZE) index_reader_info_t {
  mem_heap_t *row_heap;
  /* heap memory to process large data for virtual column
  will allocated if necessary */
  mem_heap_t *v_heap;
  row_merge_block_t *block;
  /* each element for one index */
  row_merge_buf_t **merge_buf_arr;

  /* each element for one index.
  create_merge_file will reset file->n_rec, so we have to
  add file->n_rec later instead scanning every one record. */
  ulint *n_rec_to_added_arr;

  index_reader_info_t()
    : row_heap(nullptr),
      v_heap(nullptr),
      block(nullptr),
      merge_buf_arr(nullptr),
      n_rec_to_added_arr(nullptr) {}
};

struct alignas(ut::INNODB_CACHE_LINE_SIZE) tmp_file_info_t {
  merge_file_t *file;
  std::atomic<bool> file_opened;
  mutable std::mutex file_mutex;

  tmp_file_info_t()
    : file(nullptr),
      file_opened(false),
      file_mutex() {}
};

static dberr_t write_merge_buf_to_file(
    ulint idx, index_reader_info_t &index_reader_info,
    tmp_file_info_t &tmp_file_info, int *tmpfd,
    trx_t *trx, ulint &n_rec_to_added, const char* path) {

  DBUG_EXECUTE_IF("write_merge_buf_to_file_failure", return DB_TEMP_FILE_WRITE_FAIL;);

  dberr_t err = DB_SUCCESS;
  row_merge_buf_t *buf = index_reader_info.merge_buf_arr[idx];
  row_merge_block_t *block = index_reader_info.block;
  merge_file_t *file = tmp_file_info.file;
  std::atomic<bool> &file_opened = tmp_file_info.file_opened;
  std::mutex &file_mutex = tmp_file_info.file_mutex;

  /* Step 1, open tmp file
  there may be many threads trying to open */
  if (!file_opened.load()) {
    file_mutex.lock();
    if (!file_opened.load()) {
      if (row_merge_file_create_if_needed(file, tmpfd,
            /*initial n_rec*/0, path) < 0) {
        err = DB_OUT_OF_MEMORY;
        trx->error_key_num = idx;
        file_mutex.unlock();
        return err;
      }
      file_opened.store(true);
    }
    file_mutex.unlock();
  }
  ut_ad(file_opened.load());

  /* Step 2, write buf to block */
  row_merge_buf_write(buf, file, block, &file_mutex);

  /* Step 3, write block to tmp file */
  file_mutex.lock();
  file->n_rec += n_rec_to_added;
  ulint my_offset = file->offset++;
  file_mutex.unlock();

  if (!row_merge_write(file->fd, my_offset, block)) {
    err = DB_TEMP_FILE_WRITE_FAIL; 
    trx->error_key_num = idx;
    return err;
  }
  UNIV_MEM_INVALID(&block[0], srv_sort_buf_size); 

  return err;
}

static dberr_t sort_merge_buf(
    ulint idx, row_merge_buf_t *buf, TABLE *table, const ulint *col_map,
    trx_t *trx, const ulint *key_numbers, bool online,
    const dict_table_t *old_table, const dict_table_t *new_table) {
  DBUG_EXECUTE_IF("sort_merge_buf_failure", return DB_TEMP_FILE_WRITE_FAIL;);
  dberr_t err = DB_SUCCESS;
  ut_a(buf->n_tuples);

  if (dict_index_is_unique(buf->index)) {
    row_merge_dup_t dup = {buf->index, table, col_map, 0};
    row_merge_buf_sort(buf, &dup);
    if (dup.n_dup) {
      err = DB_DUPLICATE_KEY;
      trx->error_key_num = key_numbers[idx];
    }
  } else {
    /* not unique index, no duplication check */
    row_merge_buf_sort(buf, nullptr);
  }
  return err;
}

static dberr_t process_pk_row(
    ulint n_index, index_reader_info_t &index_reader_info,
    size_t thread_id, tmp_file_info_t *tmp_files, struct TABLE *table,
    const dict_table_t *old_table, const dict_table_t *new_table,
    const ulint *col_map, row_ext_t *ext, const dtuple_t *row,
    struct TABLE *eval_table, const ulint *key_numbers, trx_t *trx,
    std::mutex &stage_mutex, bool online, int* tmpfd,
    bool first_rec_in_page, const char* path, Alter_stage *stage,
    std::atomic<dberr_t> &global_err, Quantilers *quantilers, size_t &n_rows) {

  DBUG_EXECUTE_IF("process_pk_row_SET_GE",
                  int r = rand();
                  std::this_thread::sleep_for(std::chrono::milliseconds(
                    /*us=*/ (r % 10)* 1000 * 1000)););

  auto g_err = global_err.load();
  if (g_err != DB_SUCCESS) {
    return g_err;
  }

  ulint multi_val_added = 0;
  dberr_t err = DB_SUCCESS;

  for (ulint i = 0; i < n_index; i++) {
    row_merge_buf_t *buf = index_reader_info.merge_buf_arr[i];
    mem_heap_t *v_heap = index_reader_info.v_heap;
    ulint &n_rec_to_added = index_reader_info.n_rec_to_added_arr[i];
    ulint rows_added = 0;

    rows_added = row_merge_buf_add(buf, old_table,
        new_table, row, ext, /*conv_heap*/nullptr,
        &err, &v_heap, eval_table, trx, &multi_val_added);

    /* we do not process multi-value index creation when using parallel readers */
    ut_a(multi_val_added == 0);
    ut_ad(rows_added != UNIV_NO_INDEX_VALUE);

    n_rows += rows_added;

    if (rows_added > 0) {
      /* we increase file->n_rec later, so use thread local n_rec_to_added. */
      n_rec_to_added += rows_added;
      if (err != DB_SUCCESS) {
        trx->error_key_num = i;
        break;
      }
      /* add success, continue next index entry for the current pk record */
      if (quantilers) {
        quantilers->sample_visit_rec((int)i, (int)thread_id,
                                     &buf->tuples[buf->n_tuples - 1]);
      }
      continue;
    }

    /* add fail, buf full, sort and write to file */
    /* When there are more than one parallel_sort_threads, we do not need to
    sort merge buf, since there has a partition stage. When there is only one
    sort thread, merge sort needs each run in order. */
    if (!quantilers) {
      err = sort_merge_buf(i, buf, table, col_map, trx, key_numbers, online,
          old_table, new_table);
      if (err != DB_SUCCESS) {
        break;
      }
    }

    err = write_merge_buf_to_file(i, index_reader_info, tmp_files[i],
        tmpfd, trx, n_rec_to_added, path);
    if (err != DB_SUCCESS) {
      break;
    }

#ifdef UNIV_DEBUG
    process_pk_row_debug = true;
#endif

    /* Note that, the current row is not processed yet since buf is full.
    empty the buf and re-add it again */
    index_reader_info.merge_buf_arr[i] = row_merge_buf_empty(buf);
    rows_added = row_merge_buf_add(buf, old_table,
        new_table, row, ext, /*conv_heap*/nullptr,
        &err, &v_heap, eval_table, trx, &multi_val_added);
    if (err != DB_SUCCESS) {
      break;
    }
    if (quantilers) {
      quantilers->sample_visit_rec((int)i, (int)thread_id, &buf->tuples[buf->n_tuples - 1]);
    }
    n_rec_to_added = rows_added;
  } // end for index loop

  /* update stage statistic */
  if (first_rec_in_page) {
    stage->n_pk_recs_inc(n_rows);
    stage->inc(1);
    n_rows = 0;
  }

  /* periodically state check */
  if (first_rec_in_page) {
#ifdef UNIV_DEBUG
    inject_trx_interrupted = true;
#endif
    if (trx_is_interrupted(trx)) {
      err = DB_INTERRUPTED;
    }
  }

  dberr_t ge = global_err.load();
  if (ge == DB_SUCCESS && err != DB_SUCCESS) {
    global_err.compare_exchange_strong(ge, err);
  }
  return err;
}

struct parallel_scan_context_t
{
  bool online;
  struct TABLE *table;
  struct TABLE *eval_table;
  const dict_table_t *old_table;
  dict_table_t *new_table;
  const ulint *key_numbers;
  const dtuple_t *add_cols;
  const dict_add_v_col_t *add_v;
  const ulint *col_map;
  bool skip_pk_sort;
};

/** Read clustered index of the table parallelly and create temporary files
containing the index entries for the indexes to be built.
@param[in] trx        transaction
@param[in,out] table  MySQL table object, for reporting erroneous records
@param[in] old_table  table where rows are read from
@param[in] new_table  table where indexes are created; identical to old_table unless
                      creating a PRIMARY KEY.
@param[in] online     true if creating  indexes online
@param[in] index      indexes to be created
@param[in] files      temporary files
@param[in] key_numbers    MySQL key numbers to create
@param[in] n_index    number of indexes to create
@param[in] add_cols   default values of added columns or NULL
@param[in] add_v      newly added virtual columns along with indexes
@param[in] col_map    mapping of old column numbers to new ones, or NULL if
                      old_table==new_table
@param[in,out] block  file buffer
@param[in] skip_pk_sort  whether the new PRIMARY KEY will follow existing order
@param[in,out] tmpfd  temporary file handle
@param[in,out] stage  performance schema accounting object, used by ALTER TABLE.
                      stage->n_pk_recs_inc() will be called for each record read and
                      stage->inc() will be called for each page read.
@param[in] eval_table     mysql table used to evaluate virtual column value,
                          see innobase_get_computed_value().
@param[in] parallel_threads   number of threads to scan clustered index parallelly
@return DB_SUCCESS or error
*/

struct alignas(ut::INNODB_CACHE_LINE_SIZE) RowCounter {
  size_t count;

  RowCounter() : count(0) {}
};

static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    row_merge_parallel_read_clustered_index(
        parallel_scan_context_t &ps_ctx, trx_t *trx, dict_index_t **index,
        ulint n_index, merge_file_t *files, int *tmpfd,
        Alter_stage *stage, size_t parallel_threads,
        Quantilers *quantilers) {

  struct TABLE *table = ps_ctx.table;
  const dict_table_t *old_table = ps_ctx.old_table;
  dict_table_t *new_table = ps_ctx.new_table;
  bool online = ps_ctx.online;
  const ulint *key_numbers = ps_ctx.key_numbers;
  const dtuple_t *add_cols = ps_ctx.add_cols;
  const dict_add_v_col_t *add_v = ps_ctx.add_v;
  const ulint *col_map = ps_ctx.col_map;
  struct TABLE *eval_table = ps_ctx.eval_table;

  dict_index_t *clust_index; // clustered index
  mtr_t mtr;
  dberr_t err = DB_SUCCESS;
  ulint n_nonnull = 0;
  ulint *nonnull = nullptr; /* NOT NULL columns */

  ut_ad((old_table == new_table) == !col_map);
  ut_ad(!add_cols || col_map);

  trx->op_info = "parallel reading clustered index";

  ut_ad(trx->mysql_thd != nullptr);
  const char *path = thd_innodb_tmpdir(trx->mysql_thd);
  ut_ad(!ps_ctx.skip_pk_sort || index[0]->is_clustered());

  /* Find the clustered index and create a persistent cursor
     based on that */
  clust_index = const_cast<dict_table_t*>(old_table)->first_index();

  if (old_table != new_table) {
    /* The table is being rebuilt.  Identify the columns 
    that were flagged NOT NULL in the new table, so that
    we can quickly check that the records in the old table
    do not violate the added NOT NULL constraints. */   
    nonnull = static_cast<ulint *>(ut::malloc(new_table->get_n_cols() * sizeof *nonnull));
    for (ulint i = 0; i < old_table->get_n_cols(); i++) {
      if (old_table->get_col(i)->prtype & DATA_NOT_NULL) {
        continue;
      }
      const ulint j = col_map[i];
      if (j == ULINT_UNDEFINED) {
         /* The column was dropped. */
         continue;
      }
      if (new_table->get_col(j)->prtype & DATA_NOT_NULL) {
        nonnull[n_nonnull++] = j;
      }
    } // end for
    if (!n_nonnull) {
      ut::free(nonnull);
      nonnull = nullptr;
    }
  }

  ut::allocator<row_merge_block_t> alloc(mem_key_row_merge_sort);

  void *n_rows_addr = ut::malloc(sizeof(RowCounter) * parallel_threads +
                      ut::INNODB_CACHE_LINE_SIZE);
  RowCounter *n_rows = static_cast<RowCounter*>(::ut_align(n_rows_addr,
                                                ut::INNODB_CACHE_LINE_SIZE));
  for (ulint i = 0; i < parallel_threads; i++) {
    new (n_rows + i) RowCounter();
  }

  void *index_bufs_addr = ut::malloc(sizeof(index_reader_info_t) * parallel_threads +
                                          ut::INNODB_CACHE_LINE_SIZE);
  index_reader_info_t *index_bufs =
      static_cast<index_reader_info_t*>(::ut_align(index_bufs_addr,
                                        ut::INNODB_CACHE_LINE_SIZE));
  for (ulint t = 0; t < parallel_threads; t++) {
    index_reader_info_t *cur = index_bufs + t;
    new (cur) index_reader_info_t();
    cur->row_heap = mem_heap_create(sizeof(mrec_buf_t), UT_LOCATION_HERE);
    cur->block = alloc.allocate(3 * srv_sort_buf_size);

    cur->merge_buf_arr = (row_merge_buf_t**)
      ut::malloc(n_index * sizeof(row_merge_buf_t*));
    for (ulint i = 0; i < n_index; i++) {
      cur->merge_buf_arr[i] = row_merge_buf_create(index[i]);
    }

    cur->n_rec_to_added_arr = new ulint[n_index];
    for (ulint i = 0; i < n_index; i++) {
      cur->n_rec_to_added_arr[i] = 0;
    }
  }

  void *tmp_files_addr = ut::malloc(sizeof(tmp_file_info_t) * n_index +
                                         ut::INNODB_CACHE_LINE_SIZE);
  tmp_file_info_t *tmp_files =
      static_cast<tmp_file_info_t*>(::ut_align(tmp_files_addr,
                                    ut::INNODB_CACHE_LINE_SIZE));
  for (ulint i = 0; i < n_index; i++) {
    tmp_file_info_t *cur = tmp_files + i;
    new (cur) tmp_file_info_t();
    cur->file = &(files[i]);
  }

  std::mutex stage_mutex;
  std::atomic<dberr_t> global_err{DB_SUCCESS};

  Parallel_reader::Scan_range full_scan;
  Parallel_reader::Config config(full_scan, clust_index);
  Parallel_reader reader(/*max_threads=*/parallel_threads);

  auto process_row = [&](const Parallel_reader::Ctx *ctx) {
    const rec_t *rec = ctx->m_rec;
    const size_t thread_id = ctx->thread_id();
    const bool first_rec_in_page = ctx->m_first_rec;
    ut_a(thread_id < parallel_threads);

    mem_heap_t *row_heap = index_bufs[thread_id].row_heap;
    row_ext_t *ext = nullptr;

    mem_heap_empty(row_heap);

    ulint *offsets = rec_get_offsets(rec, clust_index, nullptr,
        ULINT_UNDEFINED, UT_LOCATION_HERE, &row_heap);
    /* Build a row based on the clustered index */
    const dtuple_t *row = row_build_w_add_vcol(ROW_COPY_POINTERS,
        clust_index, rec, offsets, new_table, add_cols, add_v,
        col_map, &ext, row_heap);

    ut_ad(row);
    for (ulint i = 0; i < n_nonnull; i++) {
      const dfield_t *field = &row->fields[nonnull[i]];
      ut_ad(dfield_get_type(field)->prtype & DATA_NOT_NULL);
      if (dfield_is_null(field)) {
        err = DB_INVALID_NULL;
        trx->error_key_num = 0;
        global_err.store(err);
        return err;
      }
    }

    return process_pk_row(n_index, index_bufs[thread_id], thread_id,
        tmp_files, table, old_table, new_table, col_map, ext, row,
        eval_table, key_numbers, trx, stage_mutex, online, tmpfd,
        first_rec_in_page, path, stage, global_err, quantilers, n_rows[thread_id].count);
  };

  trx_t *scan_trx = (online ? trx : nullptr);
  err = reader.add_scan(scan_trx, config, process_row);

  if (err == DB_SUCCESS) {
    err = reader.run(parallel_threads);
  }

  if (err == DB_SUCCESS) {
    auto reader_thread_end_traverse = [&](const size_t thread_id) {
      ut_a(thread_id < parallel_threads);
      dberr_t error = DB_SUCCESS;

      auto g_err = global_err.load();
      if (g_err != DB_SUCCESS) {
        return g_err;
      }

      for (ulint i = 0; i < n_index; i++) {
        tmp_file_info_t &tmp_file_info  = tmp_files[i];
        ulint &n_rec_to_added = index_bufs[thread_id].n_rec_to_added_arr[i];
        row_merge_buf_t *buf = index_bufs[thread_id].merge_buf_arr[i];
        if (buf->n_tuples == 0) {
          continue;
        }
        if (!quantilers) {
          error = sort_merge_buf(i, buf, table, col_map, trx, key_numbers,
              online, old_table, new_table);
          if (error != DB_SUCCESS) {
            break;
          }
        }
        error = write_merge_buf_to_file(i, index_bufs[thread_id], tmp_file_info,
            tmpfd, trx, n_rec_to_added, path);
        if (error != DB_SUCCESS) {
          break;
        }
      }

      for (ulint i = 0; i < n_index && error == DB_SUCCESS; i++) {
        row_merge_buf_t *buf = index_bufs[thread_id].merge_buf_arr[i];
        if (online && new_table == old_table) {
          /* Note the newest transaction that
             modified this index when the scan was
             completed. We prevent older readers
             from accessing this index, to ensure
             read consistency. */
          rw_lock_x_lock(dict_index_get_lock(buf->index), UT_LOCATION_HERE);
          ut_a(dict_index_get_online_status(buf->index) == ONLINE_INDEX_CREATION);
          trx_id_t max_trx_id = row_log_get_max_trx(buf->index);
          if (max_trx_id > buf->index->trx_id) {
            buf->index->trx_id = max_trx_id;
          }
          rw_lock_x_unlock(dict_index_get_lock(buf->index));
        }
      }

      dberr_t ge = global_err.load();
      DBUG_EXECUTE_IF("set_global_err", err = DB_ERROR;);
      if (ge == DB_SUCCESS && err != DB_SUCCESS) {
        global_err.compare_exchange_strong(ge, err);
      }
      return error;
    };
    for (ulint t = 0; t < parallel_threads; t++) {
      reader_thread_end_traverse(t);
    }
  }

  /* cleanup resource */
  for (ulint t = 0; t < parallel_threads; t++) {
    mem_heap_free(index_bufs[t].row_heap);
    if (index_bufs[t].v_heap) {
      mem_heap_free(index_bufs[t].v_heap);
    }
    if (index_bufs[t].block) {
      alloc.deallocate(index_bufs[t].block);
    }
    for (ulint i = 0; i < n_index; i++) {
      row_merge_buf_free(index_bufs[t].merge_buf_arr[i]);
    }
    ut::free(index_bufs[t].merge_buf_arr);
    if (index_bufs[t].n_rec_to_added_arr) {
      delete[] index_bufs[t].n_rec_to_added_arr;
    }
  }
  ut::free(index_bufs_addr);
  ut::free(tmp_files_addr);
  ut::free(n_rows_addr);
  ut::free(nonnull);

  trx->op_info = "";
  return err;
}



/** Write a record via buffer 2 and read the next record to buffer N.
@param N number of the buffer (0 or 1)
@param INDEX record descriptor
@param AT_END statement to execute at end of input */
#define ROW_MERGE_WRITE_GET_NEXT_LOW(N, INDEX, AT_END)                       \
  do {                                                                       \
    b2 = row_merge_write_rec(&block[2 * srv_sort_buf_size], &buf[2], b2,     \
                             of->fd, &of->offset, mrec##N, offsets##N);      \
    if (UNIV_UNLIKELY(!b2 || ++of->n_rec > file->n_rec)) {                   \
      goto corrupt;                                                          \
    }                                                                        \
    b##N =                                                                   \
        row_merge_read_rec(&block[N * srv_sort_buf_size], &buf[N], b##N,     \
                              INDEX, file->fd, foffs##N, &mrec##N, offsets##N); \
    if (UNIV_UNLIKELY(!b##N)) {                                              \
      if (mrec##N) {                                                         \
        goto corrupt;                                                        \
      }                                                                      \
        AT_END; \
      } \
  } while (0)

/** Write a record via buffer K and read the next record to buffer N.
@param N number of the buffer (0 to K - 1)
@param INDEX record descriptor */
#define ROW_MERGE_WRITE_GET_NEXT_WITH_K_LOW(N, INDEX)          \
  do {                                                                       \
    b[K] = row_merge_write_rec(&block[K * srv_sort_buf_size], &buf[K], b[K],     \
                             of->fd, &of->offset, sr.mrec, sr.offsets);      \
    if (UNIV_UNLIKELY(!b[K] || ++of->n_rec > file->n_rec)) {                   \
      goto corrupt;                                                          \
    }                                                                        \
    b[N] = const_cast<byte *>(                                               \
      row_merge_read_rec(&block[N * srv_sort_buf_size], &buf[N], b[N],       \
        INDEX, file->fd, &foffs[N], &mrec[N], offsets[N]));                  \
    if (UNIV_UNLIKELY(!b[N])) {                                              \
      if (mrec[N]) {                                                         \
        goto corrupt;                                                        \
      }                                                                      \
    } \
    if (mrec[N]) {                                                             \
      srn.mrec = mrec[N];                                                       \
      srn.offsets = offsets[N];                                                 \
      srn.from = N;                                                             \
      m_pq.push(srn);                                                           \
    } \
  } while (0)

#ifdef HAVE_PSI_STAGE_INTERFACE
#define ROW_MERGE_WRITE_GET_NEXT(N, INDEX, AT_END)  \
  do {                                              \
    ROW_MERGE_WRITE_GET_NEXT_LOW(N, INDEX, AT_END); \
  } while (0)
#else /* HAVE_PSI_STAGE_INTERFACE */
#define ROW_MERGE_WRITE_GET_NEXT(N, INDEX, AT_END) \
  ROW_MERGE_WRITE_GET_NEXT_LOW(N, INDEX, AT_END)
#endif /* HAVE_PSI_STAGE_INTERFACE */
#define ROW_MERGE_WRITE_GET_NEXT_WITH_K(N, INDEX) \
  ROW_MERGE_WRITE_GET_NEXT_WITH_K_LOW(N, INDEX)

/** Merge two blocks of records on disk and write a bigger block.
@param[in]	dup	descriptor of index being created
@param[in]	file	file containing index entries
@param[in,out]	block	3 buffers
@param[in,out]	foffs0	offset of first source list in the file
@param[in,out]	foffs1	offset of second source list in the file
@param[in,out]	of	output file
@param[in,out]	stage	performance schema accounting object, used by
ALTER TABLE. If not NULL stage->inc() will be called for each record
processed.
@return DB_SUCCESS or error code */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    row_merge_blocks(const row_merge_dup_t *dup, const merge_file_t *file,
                     row_merge_block_t *block, ulint *foffs0, ulint *foffs1,
                     merge_file_t *of, Alter_stage *stage) {
  mem_heap_t *heap; /*!< memory heap for offsets0, offsets1 */

  mrec_buf_t *buf;     /*!< buffer for handling
                       split mrec in block[] */
  const byte *b0;      /*!< pointer to block[0] */
  const byte *b1;      /*!< pointer to block[srv_sort_buf_size] */
  byte *b2;            /*!< pointer to block[2 * srv_sort_buf_size] */
  const mrec_t *mrec0; /*!< merge rec, points to block[0] or buf[0] */
  const mrec_t *mrec1; /*!< merge rec, points to
                       block[srv_sort_buf_size] or buf[1] */
  ulint *offsets0;     /* offsets of mrec0 */
  ulint *offsets1;     /* offsets of mrec1 */

  DBUG_TRACE;
  DBUG_PRINT("ib_merge_sort",
             ("fd=%d,%lu+%lu to fd=%d,%lu", file->fd, ulong(*foffs0),
              ulong(*foffs1), of->fd, ulong(of->offset)));

  heap = row_merge_heap_create(dup->index, &buf, &offsets0, &offsets1);

  /* Write a record and read the next record.  Split the output
  file in two halves, which can be merged on the following pass. */

  if (!row_merge_read(file->fd, *foffs0, &block[0]) ||
      !row_merge_read(file->fd, *foffs1, &block[srv_sort_buf_size])) {
  corrupt:
    mem_heap_free(heap);
    return DB_CORRUPTION;
  }

  b0 = &block[0];
  b1 = &block[srv_sort_buf_size];
  b2 = &block[2 * srv_sort_buf_size];

  b0 = row_merge_read_rec(&block[0], &buf[0], b0, dup->index, file->fd, foffs0,
                          &mrec0, offsets0);
  b1 = row_merge_read_rec(&block[srv_sort_buf_size], &buf[srv_sort_buf_size],
                          b1, dup->index, file->fd, foffs1, &mrec1, offsets1);
  if (UNIV_UNLIKELY(!b0 && mrec0) || UNIV_UNLIKELY(!b1 && mrec1)) {
    goto corrupt;
  }
  ulint cur_tuple_processed = 0;
  static const ulint INC_PROGRESS_STEP = 100;
  while (mrec0 && mrec1) {
    int cmp = cmp_rec_rec_simple(mrec0, mrec1, offsets0, offsets1, dup->index,
                                 dup->table);
    if (cmp < 0) {
      ROW_MERGE_WRITE_GET_NEXT(0, dup->index, goto merged);
    } else if (cmp) {
      ROW_MERGE_WRITE_GET_NEXT(1, dup->index, goto merged);
    } else {
      mem_heap_free(heap);
      return DB_DUPLICATE_KEY;
    }
    if (++cur_tuple_processed >= INC_PROGRESS_STEP) {
      stage->inc(cur_tuple_processed);
      cur_tuple_processed = 0;
    }
  }

merged:
  if (mrec0) {
    /* append all mrec0 to output */
    for (;;) {
      ROW_MERGE_WRITE_GET_NEXT(0, dup->index, goto done0);
    }
  }
done0:
  if (mrec1) {
    /* append all mrec1 to output */
    for (;;) {
      ROW_MERGE_WRITE_GET_NEXT(1, dup->index, goto done1);
    }
  }
done1:

  mem_heap_free(heap);
  b2 = row_merge_write_eof(&block[2 * srv_sort_buf_size], b2, of->fd,
                           &of->offset);
  return b2 ? DB_SUCCESS : DB_CORRUPTION;
}

struct SortRecord {
  const mrec_t *mrec;
  const ulint *offsets;
  ulint from;
};

struct SortRecordComparator {
  void init(dict_index_t *index_param, TABLE *table_param) {
    index = index_param;
    table = table_param;
  }
  bool operator()(const SortRecord &r1, const SortRecord &r2) {
    int cmp = cmp_rec_rec_simple(r1.mrec, r2.mrec, r1.offsets, r2.offsets, index, table);
    return cmp > 0;
  }

  dict_index_t *index;
  TABLE *table;
};

/** Merge K blocks of records on disk and write a bigger block.
@param[in]	dup	descriptor of index being created
@param[in]	file	file containing index entries
@param[in,out]	block	K+1 buffers
@param[in,out]	foffs	offsets of source lists in the file
@param[in,out]	of	output file
@param[in,out]	stage	performance schema accounting object, used by
ALTER TABLE. If not NULL stage->inc() will be called for each record
processed.
@param[in] K-ways merge sort
@return DB_SUCCESS or error code */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t
    row_merge_blocks_with_k(const row_merge_dup_t *dup, const merge_file_t *file,
                     row_merge_block_t *block, ulint *foffs, merge_file_t *of,
                     Alter_stage *stage, const ulint K) {
  mem_heap_t *heap; /*!< memory heap for offsets*/

  mrec_buf_t *buf;     /*!< buffer for handling
                       split mrec in block[] */
  std::vector<byte*> b;
  b.resize(K + 1);
  std::vector<const mrec_t*> mrec;
  mrec.resize(K);
  std::vector<ulint *> offsets;
  offsets.resize(K);

  SortRecordComparator sr_comparator;
  sr_comparator.init(dup->index, dup->table);
  /** Priority queue for ordering the rows. */
  std::priority_queue<SortRecord, std::vector<SortRecord, ut_allocator<SortRecord>>,
                      SortRecordComparator> m_pq(sr_comparator);

  SortRecord sr, srn;
  ulint cur_tuple_processed = 0;
  static const ulint INC_PROGRESS_STEP = 100;

  heap = row_merge_heap_create_with_k(dup->index, &buf, offsets, K);

  /* Write a record and read the next record.  Split the output
  file in two halves, which can be merged on the following pass. */

  for (ulint i = 0; i < K; i++) {
    if (foffs[i] != ULINT_MAX) {
      if (!row_merge_read(file->fd, foffs[i], &block[srv_sort_buf_size * i])) {
        goto corrupt;
      }
    }
  }
  for (ulint i = 0; i <= K; i++) {
    b[i] = &block[i * srv_sort_buf_size];
  }

  for (ulint i = 0; i < K; i++) {
    if (foffs[i] != ULINT_MAX) {
      b[i] = const_cast<byte *>(row_merge_read_rec(&block[i * srv_sort_buf_size],
                                                   &buf[i], b[i], dup->index, file->fd,
                                                   &foffs[i], &mrec[i], offsets[i]));
      if (UNIV_UNLIKELY(!b[i] && mrec[i])) {
        goto corrupt;
      }
      if (mrec[i]) {
        sr.mrec = mrec[i];
        sr.offsets = offsets[i];
        sr.from = i;
        m_pq.push(sr);
      }
    }
  }

  while (!m_pq.empty()) {
    sr = m_pq.top();
    m_pq.pop();
    // ib::info() << "rec[" << sr.from << "]=" <<
    // << rec_printer(mrec[sr.from], 0, offsets[sr.from]).str();
    ROW_MERGE_WRITE_GET_NEXT_WITH_K(sr.from, dup->index);

    if (++cur_tuple_processed >= INC_PROGRESS_STEP) {
      stage->inc(cur_tuple_processed);
      cur_tuple_processed = 0;
    }
  }

  mem_heap_free(heap);
  b[K] = row_merge_write_eof(&block[K * srv_sort_buf_size], b[K], of->fd,
                           &of->offset);
  return b[K] ? DB_SUCCESS : DB_CORRUPTION;

corrupt:
  mem_heap_free(heap);
  return DB_CORRUPTION;
}

/** Copy a block of index entries.
@param[in]	index	index being created
@param[in]	file	input file
@param[in,out]	block	3 buffers
@param[in,out]	foffs0	input file offset
@param[in,out]	of	output file
@param[in,out]	stage	performance schema accounting object, used by
ALTER TABLE. If not NULL stage->inc() will be called for each record
processed.
@return true on success, false on failure */
static MY_ATTRIBUTE((warn_unused_result)) bool
    row_merge_blocks_copy(const dict_index_t *index, const merge_file_t *file,
                          row_merge_block_t *block, ulint *foffs0,
                          merge_file_t *of, Alter_stage *stage) {
  mem_heap_t *heap; /*!< memory heap for offsets0, offsets1 */

  mrec_buf_t *buf;     /*!< buffer for handling
                       split mrec in block[] */
  const byte *b0;      /*!< pointer to block[0] */
  byte *b2;            /*!< pointer to block[2 * srv_sort_buf_size] */
  const mrec_t *mrec0; /*!< merge rec, points to block[0] */
  ulint *offsets0;     /* offsets of mrec0 */
  ulint *offsets1;     /* dummy offsets */

  DBUG_TRACE;
  DBUG_PRINT("ib_merge_sort", ("fd=%d," ULINTPF " to fd=%d," ULINTPF, file->fd,
                               *foffs0, of->fd, of->offset));

  heap = row_merge_heap_create(index, &buf, &offsets0, &offsets1);

  /* Write a record and read the next record.  Split the output
  file in two halves, which can be merged on the following pass. */

  if (!row_merge_read(file->fd, *foffs0, &block[0])) {
  corrupt:
    mem_heap_free(heap);
    return false;
  }

  b0 = &block[0];

  b2 = &block[2 * srv_sort_buf_size];

  b0 = row_merge_read_rec(&block[0], &buf[0], b0, index, file->fd, foffs0,
                          &mrec0, offsets0);
  if (UNIV_UNLIKELY(!b0 && mrec0)) {
    goto corrupt;
  }

  if (mrec0) {
    /* append all mrec0 to output */
    for (;;) {
      ROW_MERGE_WRITE_GET_NEXT(0, index, goto done0);
    }
  }
done0:

  /* The file offset points to the beginning of the last page
  that has been read.  Update it to point to the next block. */
  (*foffs0)++;

  mem_heap_free(heap);
  return row_merge_write_eof(&block[2 * srv_sort_buf_size], b2, of->fd,
                             &of->offset) != nullptr;
}

/** Merge disk files.
@param[in]	trx		transaction
@param[in]	dup		descriptor of index being created
@param[in,out]	file		file containing index entries
@param[in,out]	block		3 buffers
@param[in,out]	tmpfd		temporary file handle
@param[in,out]	num_run		Number of runs that remain to be merged
@param[in,out]	run_offset	Array that contains the first offset number
for each merge run
@param[in,out]	stage		performance schema accounting object, used by
ALTER TABLE. If not NULL stage->inc() will be called for each record
processed.
@return DB_SUCCESS or error code */
static dberr_t row_merge(trx_t *trx, const row_merge_dup_t *dup,
                         merge_file_t *file, row_merge_block_t *block,
                         int *tmpfd, ulint *num_run, ulint *run_offset,
                         Alter_stage *stage) {
  ulint foffs0;    /*!< first input offset */
  ulint foffs1;    /*!< second input offset */
  dberr_t error;   /*!< error code */
  merge_file_t of; /*!< output file */
  const ulint ihalf = run_offset[*num_run / 2];
  /*!< half the input file */
  ulint n_run = 0;
  /*!< num of runs generated from this merge */

  UNIV_MEM_ASSERT_W(&block[0], 3 * srv_sort_buf_size);

  ut_ad(ihalf < file->offset);

  of.fd = *tmpfd;
  of.offset = 0;
  of.n_rec = 0;

#ifdef POSIX_FADV_SEQUENTIAL
  /* The input file will be read sequentially, starting from the
  beginning and the middle.  In Linux, the POSIX_FADV_SEQUENTIAL
  affects the entire file.  Each block will be read exactly once. */
  posix_fadvise(file->fd, 0, 0, POSIX_FADV_SEQUENTIAL | POSIX_FADV_NOREUSE);
#endif /* POSIX_FADV_SEQUENTIAL */

  /* Merge blocks to the output file. */
  foffs0 = 0;
  foffs1 = ihalf;

  UNIV_MEM_INVALID(run_offset, *num_run * sizeof *run_offset);

  for (; foffs0 < ihalf && foffs1 < file->offset; foffs0++, foffs1++) {
    if (trx_is_interrupted(trx)) {
      return (DB_INTERRUPTED);
    }

    /* Remember the offset number for this run */
    run_offset[n_run++] = of.offset;

    error = row_merge_blocks(dup, file, block, &foffs0, &foffs1, &of, stage);

    if (error != DB_SUCCESS) {
      return (error);
    }
  }

  /* Copy the last blocks, if there are any. */

  while (foffs0 < ihalf) {
    if (UNIV_UNLIKELY(trx_is_interrupted(trx))) {
      return (DB_INTERRUPTED);
    }

    /* Remember the offset number for this run */
    run_offset[n_run++] = of.offset;

    if (!row_merge_blocks_copy(dup->index, file, block, &foffs0, &of, stage)) {
      return (DB_CORRUPTION);
    }
  }

  ut_ad(foffs0 == ihalf);

  while (foffs1 < file->offset) {
    if (trx_is_interrupted(trx)) {
      return (DB_INTERRUPTED);
    }

    /* Remember the offset number for this run */
    run_offset[n_run++] = of.offset;

    if (!row_merge_blocks_copy(dup->index, file, block, &foffs1, &of, stage)) {
      return (DB_CORRUPTION);
    }
  }

  ut_ad(foffs1 == file->offset);

  if (UNIV_UNLIKELY(of.n_rec != file->n_rec)) {
    return (DB_CORRUPTION);
  }

  ut_ad(n_run <= *num_run);

  *num_run = n_run;

  /* Each run can contain one or more offsets. As merge goes on,
  the number of runs (to merge) will reduce until we have one
  single run. So the number of runs will always be smaller than
  the number of offsets in file */
  ut_ad((*num_run) <= file->offset);

  /* The number of offsets in output file is always equal or
  smaller than input file */
  ut_ad(of.offset <= file->offset);

  /* Swap file descriptors for the next pass. */
  *tmpfd = file->fd;
  *file = of;

  UNIV_MEM_INVALID(&block[0], 3 * srv_sort_buf_size);

  return (DB_SUCCESS);
}

ulint *row_merge_sort_init_offset(merge_file_t *file) {
	/* "run_offset" records each run's first offset number */
	ulint *run_offset = (ulint *)ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY,
          file->offset * sizeof(ulint));
  for (ulint i = 0; i < file->offset; ++i) {
    run_offset[i] = i;
  }
	return run_offset;
}

/** Merge disk files with k-ways merge sort.
@param[in]	trx		transaction
@param[in]	dup		descriptor of index being created
@param[in,out]	file		file containing index entries
@param[in,out]	block		K+1 buffers
@param[in,out]	tmpfd		temporary file handle
@param[in,out]	num_run		Number of runs that remain to be merged
@param[in,out]	run_offset	Array that contains the first offset number
for each merge run
@param[in,out]	stage		performance schema accounting object, used by
ALTER TABLE. If not NULL stage->inc() will be called for each record
processed.
@param[in] K-ways merge sort
@return DB_SUCCESS or error code */
static dberr_t row_merge_with_k(trx_t *trx, const row_merge_dup_t *dup,
                         merge_file_t *file, row_merge_block_t *block,
                         int *tmpfd, ulint *num_run, ulint *run_offset,
                         ut_stage_alter_t *stage, const ulint K) {
  ulint foffs[128];

  dberr_t error;   /*!< error code */
  merge_file_t of; /*!< output file */
  ulint ihalf[128];
  ihalf[0] = 0;
  for (ulint i = 1; i < K; i++) {
    ihalf[i] = run_offset[*num_run / K * i];
  }

  /*!< half the input file */
  ulint n_run = 0;
  /*!< num of runs generated from this merge */

  UNIV_MEM_ASSERT_W(&block[0], (K + 1) * srv_sort_buf_size);

  ut_ad(ihalf[K - 1] < file->offset);

  ihalf[K] = file->offset;

  of.fd = *tmpfd;
  of.offset = 0;
  of.n_rec = 0;

#ifdef POSIX_FADV_SEQUENTIAL
  /* The input file will be read sequentially, starting from the
  beginning and the middle.  In Linux, the POSIX_FADV_SEQUENTIAL
  affects the entire file.  Each block will be read exactly once. */
  posix_fadvise(file->fd, 0, 0, POSIX_FADV_SEQUENTIAL | POSIX_FADV_NOREUSE);
#endif /* POSIX_FADV_SEQUENTIAL */

  /* Merge blocks to the output file. */
  foffs[0] = 0;
  for (ulint i = 1; i < K; i++) {
    foffs[i] = ihalf[i];
  }

  UNIV_MEM_INVALID(run_offset, *num_run * sizeof *run_offset);
  ulint finish = 0;
  while (true) {
    if (trx_is_interrupted(trx)) {
      return (DB_INTERRUPTED);
    }

    finish = 0;
    for (ulint i = 0; i < K; i++) {
      if (foffs[i] == ihalf[i + 1]) {
        finish++;
      }
    }

    if (finish >= K - 1) {
      break;
    }

    /* Remember the offset number for this run */
    run_offset[n_run++] = of.offset;

    error = row_merge_blocks_with_k(dup, file, block, foffs, &of, stage, K);

    if (error != DB_SUCCESS) {
      return (error);
    }
    for (ulint i = 0; i < K; i++) {
      if (foffs[i] < ihalf[i + 1]) {
        foffs[i]++;
      }
    }
  }

  for (ulint i = 0; i < K; i++) {
    while (foffs[i] < ihalf[i + 1]) {
      if (UNIV_UNLIKELY(trx_is_interrupted(trx))) {
        return (DB_INTERRUPTED);
      }
      run_offset[n_run++] = of.offset;

      if (!row_merge_blocks_copy(dup->index, file, block, &foffs[i], &of, stage)) {
        ib::error() << "row_merge_blocks_copy failed, foffs=" << foffs[i]
                    << " ihalf=" << ihalf[i + 1];
        return (DB_CORRUPTION);
      }
    }
  }


  for (ulint i = 0; i < K; i++) {
    ut_ad(foffs[i] == ihalf[i + 1]);
  }

  ut_ad(n_run <= *num_run);

  *num_run = n_run;

  /* Each run can contain one or more offsets. As merge goes on,
  the number of runs (to merge) will reduce until we have one
  single run. So the number of runs will always be smaller than
  the number of offsets in file */
  ut_ad((*num_run) <= file->offset);

  /* The number of offsets in output file is always equal or
  smaller than input file */
  ut_ad(of.offset <= file->offset);

  /* Swap file descriptors for the next pass. */
  *tmpfd = file->fd;
  *file = of;

  UNIV_MEM_INVALID(&block[0], (K + 1) * srv_sort_buf_size);

  return (DB_SUCCESS);
}


/** Merge disk files with k-ways merge sort.
@param[in]	trx	transaction
@param[in]	dup	descriptor of index being created
@param[in,out]	file	file containing index entries
@param[in,out]	block	K+1 buffers
@param[in,out]	tmpfd	temporary file handle
@param[in,out]	stage	performance schema accounting object, used by
ALTER TABLE. If not NULL, stage->begin_phase_sort() will be called initially
and then stage->inc() will be called for each record processed.
@return DB_SUCCESS or error code */
dberr_t row_merge_sort_with_k(trx_t *trx, const row_merge_dup_t *dup,
                       merge_file_t *file, row_merge_block_t *block, int *tmpfd,
                       Alter_stage *stage /* = NULL */) {
  ulint K = file->offset;
  if (file->offset > (ulint)cdb_parallel_ddl_merge_sort_k_value) {
    K = (ulint)cdb_parallel_ddl_merge_sort_k_value;
  }
  ulint half[128];
  for (ulint i = 1; i < K; i++) {
    half[i] = file->offset / K * i;
  }
  ulint num_runs;
  ulint *run_offset;
  dberr_t error = DB_SUCCESS;
  DBUG_TRACE;

  /* Record the number of merge runs we need to perform */
  num_runs = file->offset;

  /* If num_runs are less than 1, nothing to merge */
  if (num_runs <= 1) {
    return error;
  }

  /* "run_offset" records each run's first offset number */
  run_offset = row_merge_sort_init_offset(file);


  /* This tells row_merge() where to start for the first round
  of merge. */
  for (ulint i = 1; i < K; i++) {
    run_offset[half[i]] = half[i];
  }

  /* The file should always contain at least one byte (the end
  of file marker).  Thus, it must be at least one block. */
  ut_ad(file->offset > 0);

  /* Single-threaded merge */

  /* Merge the runs until we have one big run */
  do {
    K = std::min(K, num_runs);

    error =
      row_merge_with_k(trx, dup, file, block,
        tmpfd, &num_runs, run_offset, stage, K);

    if (error != DB_SUCCESS) {
      break;
    }

    UNIV_MEM_ASSERT_RW(run_offset, num_runs * sizeof *run_offset);

  } while (num_runs > 1);

  ut_free(run_offset);

  return error;
}

/** Merge disk files.
@param[in]	trx	transaction
@param[in]	dup	descriptor of index being created
@param[in,out]	file	file containing index entries
@param[in,out]	block	3 buffers
@param[in,out]	tmpfd	temporary file handle
@param[in,out]	stage	performance schema accounting object, used by
ALTER TABLE. If not NULL, stage->begin_phase_sort() will be called initially
and then stage->inc() will be called for each record processed.
@return DB_SUCCESS or error code */
dberr_t row_merge_sort(trx_t *trx, const row_merge_dup_t *dup,
                       merge_file_t *file, row_merge_block_t *block, int *tmpfd,
                       Alter_stage *stage /* = NULL */) {
  const ulint half = file->offset / 2;
  ulint num_runs;
  ulint *run_offset;
  dberr_t error = DB_SUCCESS;
  DBUG_TRACE;

  /* Record the number of merge runs we need to perform */
  num_runs = file->offset;

  /* If num_runs are less than 1, nothing to merge */
  if (num_runs <= 1) {
    return error;
  }

  /* "run_offset" records each run's first offset number */
  run_offset = row_merge_sort_init_offset(file);


  /* This tells row_merge() where to start for the first round
  of merge. */
  run_offset[half] = half;

  /* The file should always contain at least one byte (the end
  of file marker).  Thus, it must be at least one block. */
  ut_ad(file->offset > 0);

  /* Merge the runs until we have one big run */
  do {
    error =
      row_merge(trx, dup, file, block, tmpfd, &num_runs, run_offset, stage);

    if (error != DB_SUCCESS) {
      break;
    }

    UNIV_MEM_ASSERT_RW(run_offset, num_runs * sizeof *run_offset);
  } while (num_runs > 1);

  ut::free(run_offset);

  return error;
}

#ifdef UNIV_DEBUG
#define row_merge_copy_blobs(trx, index, mrec, offsets, page_size, tuple, \
                             is_sdi, heap)                                \
  row_merge_copy_blobs_func(trx, index, mrec, offsets, page_size, tuple,  \
                            is_sdi, heap)
#else /* UNIV_DEBUG */
#define row_merge_copy_blobs(trx, index, mrec, offsets, page_size, tuple, \
                             is_sdi, heap)                                \
  row_merge_copy_blobs_func(trx, index, mrec, offsets, page_size, tuple, \
                             is_sdi, heap)
#endif /* UNIV_DEBUG */

/** Copy externally stored columns to the data tuple.
@param[in]	trx		current transaction
@param[in]	index		index dictionary object.
@param[in]	mrec		record containing BLOB pointers,
                                or NULL to use tuple instead
@param[in]	offsets		offsets of mrec
@param[in]	page_size	compressed page size in bytes, or 0
@param[in,out]	tuple		data tuple */
#ifdef UNIV_DEBUG
/**
@param[in]	is_sdi		true for SDI Indexes */
#endif /* UNIV_DEBUG */
/**
@param[in,out]	heap		memory heap */
static void row_merge_copy_blobs_func(trx_t *trx, const dict_index_t *index,
                                      const mrec_t *mrec, const ulint *offsets,
                                      const page_size_t &page_size,
                                      dtuple_t *tuple,
                                      bool is_sdi,
                                      mem_heap_t *heap) {
  ut_ad(mrec == nullptr || rec_offs_any_extern(offsets));

  for (ulint i = 0; i < dtuple_get_n_fields(tuple); i++) {
    ulint len;
    const void *data;
    dfield_t *field = dtuple_get_nth_field(tuple, i);
    ulint field_len;
    const byte *field_data;

    if (!dfield_is_ext(field)) {
      continue;
    }

    ut_ad(!dfield_is_null(field));

    /* During the creation of a PRIMARY KEY, the table is
    X-locked, and we skip copying records that have been
    marked for deletion. Therefore, externally stored
    columns cannot possibly be freed between the time the
    BLOB pointers are read (row_merge_read_clustered_index())
    and dereferenced (below). */
    if (mrec == nullptr) {
      field_data = static_cast<byte *>(dfield_get_data(field));
      field_len = dfield_get_len(field);

      ut_a(field_len >= BTR_EXTERN_FIELD_REF_SIZE);

      ut_a(memcmp(field_data + field_len - BTR_EXTERN_FIELD_REF_SIZE,
                  field_ref_zero, BTR_EXTERN_FIELD_REF_SIZE));

      data = lob::btr_copy_externally_stored_field(
          nullptr, index, &len, nullptr, field_data, page_size, field_len,
          is_sdi, heap);
    } else {
      data = lob::btr_rec_copy_externally_stored_field(
          nullptr, index, mrec, offsets, page_size, i, &len, nullptr, is_sdi,
          heap);
    }

    /* Because we have locked the table, any records
    written by incomplete transactions must have been
    rolled back already. There must not be any incomplete
    BLOB columns. */
    ut_a(data);

    dfield_set_data(field, data, len);
  }
}

/** Convert a merge record to a typed data tuple. Note that externally
stored fields are not copied to heap.
@param[in,out]	index	index on the table
@param[in]	mtuple	merge record
@param[in]	dtuple	data tuple of records */
static void row_merge_mtuple_to_dtuple(dict_index_t *index, dtuple_t *dtuple,
                                       const mtuple_t *mtuple) {
  ut_ad(!dict_index_is_ibuf(index));

  memcpy(dtuple->fields, mtuple->fields,
         dtuple->n_fields * sizeof *mtuple->fields);
}

/** Insert sorted data tuples to the index.
@param[in]	trx		current transaction
@param[in]	index		index to be inserted
@param[in]	old_table	old table
@param[in]	fd		file descriptor
@param[in,out]	block		file buffer
@param[in]	row_buf		row_buf the sorted data tuples,
or NULL if fd, block will be used instead
@param[in,out]	btr_bulk	btr bulk instance
@param[in,out]	stage		performance schema accounting object, used by
ALTER TABLE. If not NULL stage->begin_phase_insert() will be called initially
and then stage->inc() will be called for each record that is processed.
@return DB_SUCCESS or error number */
static MY_ATTRIBUTE((warn_unused_result)) dberr_t row_merge_insert_index_tuples(
    trx_t *trx, dict_index_t *index, const dict_table_t *old_table, int fd,
    row_merge_block_t *block, const row_merge_buf_t *row_buf, BtrBulk *btr_bulk,
    Alter_stage *stage/* = NULL */) {
  const byte *b;
  mem_heap_t *heap;
  mem_heap_t *tuple_heap;
  dberr_t error = DB_SUCCESS;
  ulint foffs = 0;
  ulint *offsets;
  mrec_buf_t *buf;
  ulint n_rows = 0;
  ulint num_recs = 0;
  dtuple_t *dtuple;
  DBUG_TRACE;

  ut_ad(!srv_read_only_mode);
  ut_ad(!(index->type & DICT_FTS));
  ut_ad(!dict_index_is_spatial(index));
  ut_ad(trx->id);

  if (stage != nullptr) {
    stage->begin_phase_insert();
  }

  tuple_heap = mem_heap_create(1000, UT_LOCATION_HERE);

  {
    ulint i = 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index);
    heap = mem_heap_create(sizeof *buf + i * sizeof *offsets, UT_LOCATION_HERE);
    offsets = static_cast<ulint *>(mem_heap_alloc(heap, i * sizeof *offsets));
    offsets[0] = i;
    offsets[1] = dict_index_get_n_fields(index);
  }

  if (row_buf != nullptr) {
    ut_ad(fd == -1);
    ut_ad(block == nullptr);
    DBUG_EXECUTE_IF("row_merge_read_failure", error = DB_CORRUPTION;
                    goto err_exit;);
    buf = nullptr;
    b = nullptr;
    dtuple = dtuple_create(heap, dict_index_get_n_fields(index));
    dtuple_set_n_fields_cmp(dtuple, dict_index_get_n_unique_in_tree(index));
  } else {
    b = block;
    dtuple = nullptr;

    if (!row_merge_read(fd, foffs, block)) {
      error = DB_CORRUPTION;
      goto err_exit;
    } else {
      buf = static_cast<mrec_buf_t *>(mem_heap_alloc(heap, sizeof *buf));
    }
  }

  for (;;) {
    const mrec_t *mrec;
    mtr_t mtr;

    if (stage != nullptr) {
      stage->inc(1);
    }

    if (row_buf != nullptr) {
      if (n_rows >= row_buf->n_tuples) {
        break;
      }

      /* Convert merge tuple record from
      row buffer to data tuple record */
      row_merge_mtuple_to_dtuple(index, dtuple, &row_buf->tuples[n_rows]);

      n_rows++;
      /* BLOB pointers must be copied from dtuple */
      mrec = nullptr;
    } else {
      b = row_merge_read_rec(block, buf, b, index, fd, &foffs, &mrec, offsets);
      if (UNIV_UNLIKELY(!b)) {
        /* End of list, or I/O error */
        if (mrec) {
          error = DB_CORRUPTION;
        }
        break;
      }

      dtuple = row_rec_to_index_entry_low(mrec, index, offsets, tuple_heap);
    }

    const dict_index_t *old_index = old_table->first_index();

    if (index->is_clustered() && dict_index_is_online_ddl(old_index)) {
      error = row_log_table_get_error(old_index);
      if (error != DB_SUCCESS) {
        break;
      }
    }

    /* If there are externally stored columns. */
    if (dtuple->has_ext()) {
      ut_ad(index->is_clustered());
      /* Off-page columns can be fetched safely
      when concurrent modifications to the table
      are disabled. (Purge can process delete-marked
      records, but row_merge_read_clustered_index()
      would have skipped them.)

      When concurrent modifications are enabled,
      row_merge_read_clustered_index() will
      only see rows from transactions that were
      committed before the ALTER TABLE started
      (REPEATABLE READ).

      Any modifications after the
      row_merge_read_clustered_index() scan
      will go through row_log_table_apply().
      Any modifications to off-page columns
      will be tracked by
      row_log_table_blob_alloc() and
      row_log_table_blob_free(). */
      row_merge_copy_blobs(trx, old_index, mrec, offsets,
                           dict_table_page_size(old_table), dtuple,
                           dict_index_is_sdi(index), tuple_heap);
    }

    ut_ad(dtuple_validate(dtuple));

    error = btr_bulk->insert(dtuple);
    num_recs++;

    if (error != DB_SUCCESS) {
      goto err_exit;
    }

    mem_heap_empty(tuple_heap);
  }

err_exit:
  mem_heap_free(tuple_heap);
  mem_heap_free(heap);

  return error;
}

/** Create temporary merge files in the given paramater path, and if
UNIV_PFS_IO defined, register the file descriptor with Performance Schema.
@param[in]	path	location for creating temporary merge files.
@return File descriptor */
int row_merge_file_create_low(const char *path) {
  int fd;
  if (path == nullptr) {
    path = innobase_mysql_tmpdir();
  }
#ifdef UNIV_PFS_IO
  /* This temp file open does not go through normal
  file APIs, add instrumentation to register with
  performance schema */
  Datafile df;
  df.make_filepath(path, "Innodb Merge Temp File", NO_EXT);

  struct PSI_file_locker *locker = nullptr;
  PSI_file_locker_state state;

  locker = PSI_FILE_CALL(get_thread_file_name_locker)(
      &state, innodb_temp_file_key.m_value, PSI_FILE_OPEN, df.filepath(),
      &locker);

  if (locker != nullptr) {
    PSI_FILE_CALL(start_file_open_wait)(locker, __FILE__, __LINE__);
  }
#endif /* UNIV_PFS_IO */
  fd = innobase_mysql_tmpfile(path);
#ifdef UNIV_PFS_IO
  if (locker != nullptr) {
    PSI_FILE_CALL(end_file_open_wait_and_bind_to_descriptor)(locker, fd);
  }
#endif /* UNIV_PFS_IO */

  if (fd < 0) {
    ib::error(ER_IB_MSG_967) << "Cannot create temporary merge file";
    return (-1);
  }
  return (fd);
}

/** Create a merge file in the given location.
@param[out]	merge_file	merge file structure
@param[in]	path		location for creating temporary file
@return file descriptor, or -1 on failure */
int row_merge_file_create(merge_file_t *merge_file, const char *path) {
  merge_file->fd = row_merge_file_create_low(path);
  merge_file->offset = 0;
  merge_file->n_rec = 0;

  if (merge_file->fd >= 0) {
    if (srv_disable_sort_file_cache) {
      os_file_set_nocache(merge_file->fd, "row0merge.cc", "sort");
    }
  }
  return (merge_file->fd);
}

/** Destroy a merge file. And de-register the file from Performance Schema
 if UNIV_PFS_IO is defined. */
void row_merge_file_destroy_low(int fd) /*!< in: merge file descriptor */
{
#ifdef UNIV_PFS_IO
  struct PSI_file_locker *locker = nullptr;
  PSI_file_locker_state state;
  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, fd,
                                                            PSI_FILE_CLOSE);
  if (locker != nullptr) {
    PSI_FILE_CALL(start_file_wait)(locker, 0, __FILE__, __LINE__);
  }
#endif
  if (fd >= 0) {
    close(fd);
  }
#ifdef UNIV_PFS_IO
  if (locker != nullptr) {
    PSI_FILE_CALL(end_file_wait)(locker, 0);
  }
#endif
}
/** Destroy a merge file. */
void row_merge_file_destroy(
    merge_file_t *merge_file) /*!< in/out: merge file structure */
{
  ut_ad(!srv_read_only_mode);

  if (merge_file->fd != -1) {
    row_merge_file_destroy_low(merge_file->fd);
    merge_file->fd = -1;
  }
}

/** Write an MLOG_INDEX_LOAD record to indicate in the redo-log
that redo-logging of individual index pages was disabled, and
the flushing of such pages to the data files was completed.
@param[in]	index	an index tree on which redo logging was disabled */
static void row_merge_write_redo(const dict_index_t *index) {
  mtr_t mtr;
  byte *log_ptr = nullptr;

  ut_ad(!index->table->is_temporary());
  mtr.start();
  if (mlog_open(&mtr, 11 + 8, log_ptr)) {
    log_ptr = mlog_write_initial_log_record_low(MLOG_INDEX_LOAD, index->space,
                                                index->page, log_ptr, &mtr);
    mach_write_to_8(log_ptr, index->id);
    mlog_close(&mtr, log_ptr + 8);
  }
  mtr.commit();
}

/* return 1 for not parallel */
static void check_if_can_use_parallel_ddl(trx_t *trx, dict_index_t **indexes, ulint n_indexes,
    dict_table_t *old_table, dict_table_t *new_table, struct TABLE *eval_table, ulint add_autoinc,
    bool skip_pk_sort, size_t & parallel_scan_threads, size_t & parallel_sort_threads) {

  parallel_scan_threads = parallel_sort_threads = 1;

  size_t thd_pll_read_thread_num = thd_parallel_read_threads(trx->mysql_thd);
  parallel_sort_threads = thd_txsql_ddl_threads(trx->mysql_thd);
  /* this will reserve the threads, and should release explicitly */
  parallel_scan_threads = Parallel_reader::available_threads(thd_pll_read_thread_num, false);
  if (parallel_scan_threads <= 1 || skip_pk_sort) {
    parallel_scan_threads = 1;
  }
  return;
}

/** if there is no so many run, adjust parallel_sort_threads */
static void check_if_have_enough_run(merge_file_t *merge_files, int n_indexes,
                                     dict_index_t **indexes, size_t &parallel_sort_threads,
                                     bool skip_pk_sort) {
  for (int i = 0; i < n_indexes; i++) {
    if (indexes[i]->is_clustered() && skip_pk_sort) {
      continue;
    }
    if (merge_files[i].offset < parallel_sort_threads) {
      parallel_sort_threads = merge_files[i].offset;
      continue;
    }
  }
}

struct BufferRecord {
  const byte *data_start;
  const byte *data_end;
  const byte *mrec;
  const ulint *offsets;
};

struct BufferRecordComparator {
  void init(dict_index_t *index_param, TABLE *table_param) {
    index = index_param;
    table = table_param;
  }
  bool operator()(const BufferRecord &r1, const BufferRecord &r2) {
    return cmp_rec_rec_simple(r1.mrec, r2.mrec, r1.offsets, r2.offsets, index, table) < 0;
  }

  dict_index_t *index;
  TABLE *table;
};

struct merge_file_wrapper_t {
  void init(const char *path) {
    file = new merge_file_t();
    //each merge file corresponds to a tmp file used in single-threaded merge sort
    tmpfd = -1;

    int fd = row_merge_file_create_if_needed(file, &tmpfd, /*initial_n_rec*/0, path);
    offset.store(0);
    ut_a(fd >= 0);
  }

  ~merge_file_wrapper_t() {
    delete file;
    file = nullptr;
  }

  merge_file_t *file;
  int tmpfd;
  std::atomic<ulint> offset;
};

struct LocalPartitionBuffer {
  LocalPartitionBuffer(){}

  ~LocalPartitionBuffer() {
    mem_heap_free(heap);
  }

  void init(merge_file_wrapper_t *output_files_param,
            dict_index_t *index_param,
            struct TABLE *table_param);
  void destroy();
  void sort_records_and_fill_to_buf();
  void append(const byte *b, const byte *b_after,
              const mrec_t *rec, ulint *offsets);
  void tail_padding();
  void flush_buf();
  void flush_tail();

  merge_file_wrapper_t *output_file;

  std::vector<BufferRecord> records;

  mem_heap_t *heap;
  byte *data;
  ulint len;
  ulint n_rec;
  byte *aux_data;  //Auxiliary space to use for sort.
  byte *offsets_data;
  ulint offsets_data_len;

  BufferRecordComparator br_cmp;
  ulint offsets_i;
};

void LocalPartitionBuffer::init(merge_file_wrapper_t *output_file_param,
                                dict_index_t *index_param,
                                struct TABLE *table_param) {
  output_file = output_file_param;
  br_cmp.init(index_param, table_param);

  offsets_i = 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index_param);
  const ulint max_rec_count_in_buf =
        srv_sort_buf_size / std::max(static_cast<ulint>(1), index_param->get_min_size());
  heap = mem_heap_create(1000, UT_LOCATION_HERE);

  data = (byte *) mem_heap_alloc(heap, srv_sort_buf_size);
  aux_data = (byte *) mem_heap_alloc(heap, srv_sort_buf_size);
  len = 0;

  offsets_data = (byte *) mem_heap_alloc(heap,
                 sizeof(ulint) * offsets_i * max_rec_count_in_buf);
  offsets_data_len = 0;

  n_rec = 0;
}

void LocalPartitionBuffer::tail_padding() {
  ut_a(len < srv_sort_buf_size);
  byte *b = data + len;
  *b++ = 0; // end flag of run
  memset(b, 0xff, data + srv_sort_buf_size - b);
}

void LocalPartitionBuffer::flush_buf() {
  tail_padding();
  ulint write_offset = output_file->offset.fetch_add(1);
  bool write_succ = 
      row_merge_write(output_file->file->fd, write_offset, data);
  ut_ad(write_succ);
  len = 0;
  offsets_data_len = 0;
}

void LocalPartitionBuffer::sort_records_and_fill_to_buf() {
  std::sort(records.begin(), records.end(), br_cmp);

  int buf_len = 0;
  for (auto &r : records) {
    memcpy(aux_data + buf_len, r.data_start, r.data_end - r.data_start);
    buf_len += r.data_end - r.data_start;
  }
  std::swap(aux_data, data);
}

void LocalPartitionBuffer::append(const byte *b, const byte *b_after,
                                  const mrec_t *mrec, ulint *offsets) {
  ulint size = b_after - b;
  if (len + size >= srv_sort_buf_size) {
    sort_records_and_fill_to_buf();
    flush_buf();
    records.clear();
  }

  BufferRecord br;
  br.data_start = data + len;
  br.data_end = data + len + (b_after - b);
  br.mrec = data + len + (mrec - b);
  br.offsets = (ulint*)(offsets_data + offsets_data_len);

  memcpy(data + len, b, b_after - b);
  memcpy(offsets_data + offsets_data_len, offsets, offsets_i * sizeof(ulint));

  records.push_back(br);

  len += b_after - b;
  offsets_data_len += offsets_i * sizeof(ulint);
  n_rec++;
}

void LocalPartitionBuffer::flush_tail() {
  sort_records_and_fill_to_buf();
  flush_buf();
  records.clear();
}

std::atomic<bool> interrupt_(false);

void parallel_partition_file(
                  ulint start_run, ulint end_run,
                  merge_file_t *input_file,
                  merge_file_wrapper_t *output_files,
                  const Quantiler *quantiler,
                  dict_index_t *index, struct TABLE *table,
                  ulint sort_parallel) {
  ulint *offsets = nullptr;
  mrec_buf_t *buf = nullptr;
  const ulint offsets_i =
      1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(index);

  ut::allocator<row_merge_block_t> alloc(mem_key_row_merge_sort);
  row_merge_block_t * block = alloc.allocate(srv_sort_buf_size);

  mem_heap_t * heap = mem_heap_create(offsets_i * sizeof(*offsets) + sizeof(*buf), UT_LOCATION_HERE);
  buf = static_cast<mrec_buf_t*>(mem_heap_alloc(heap, sizeof(*buf)));

  const byte *b = nullptr;
  const byte *b_after = nullptr;
  ulint cur_run = start_run;
  const mrec_t *mrec = nullptr;

  offsets = static_cast<ulint*>(mem_heap_alloc(heap, offsets_i * sizeof(*offsets)));
  offsets[0] = offsets_i;
  offsets[1] = dict_index_get_n_fields(index);

  dberr_t worker_err = DB_SUCCESS;

  // prepare sort_parallel local partition buffers
  LocalPartitionBuffer *local_partition_buffers 
                                      = new LocalPartitionBuffer[sort_parallel];
  for (ulint b = 0; b < sort_parallel; b++) {
    local_partition_buffers[b].init(&output_files[b], index, table);
  }

  if (!row_merge_read(input_file->fd, cur_run, block)) {
    worker_err = DB_CORRUPTION;
    interrupt_.store(true);
    return;
  }
  b = block;

  while (cur_run < end_run && !interrupt_.load()) {
    b_after = row_merge_read_rec(block, buf, b, index, input_file->fd,
                                 &cur_run, &mrec, offsets);
    if (UNIV_UNLIKELY(!b_after)) {
      if (mrec) {
        worker_err = DB_CORRUPTION;
        break;
      } else if (++cur_run >= end_run) {
        continue;
      } else if (!row_merge_read(input_file->fd, cur_run, block)) {
        worker_err = DB_CORRUPTION;
        break;
      } else {
        b = block;
        continue;
      }
    }
    int partition_id = quantiler->partition_to(mrec, offsets);
    auto &local_pb = local_partition_buffers[partition_id];
    local_pb.append(b, b_after, mrec, offsets);
    b = b_after;
  }
  if (worker_err != DB_SUCCESS) {
    interrupt_.store(true);
  }

  for (ulint p = 0; p < sort_parallel; p++) {
    LocalPartitionBuffer &lpb = 
            local_partition_buffers[p];
    lpb.flush_tail();
    output_files[p].file->n_rec += lpb.n_rec;
  }

  // free allocated memory.
  alloc.deallocate(block);
  mem_heap_free(heap);

  delete[] local_partition_buffers;
  local_partition_buffers = nullptr;
}

void parallel_merge_sort(ulint id, merge_file_wrapper_t *file_wrapper,
                  trx_t *trx, row_merge_dup_t *dup,
                  Alter_stage *stage, dberr_t& err) {
  err = DB_SUCCESS;
  ut::allocator<row_merge_block_t> alloc(mem_key_row_merge_sort);
  row_merge_block_t *block =
    alloc.allocate((txsql_parallel_ddl_merge_sort_k_value + 1) *
                          srv_sort_buf_size);
  if (txsql_parallel_ddl_merge_sort_k_value == 2) {
    err = row_merge_sort(trx, dup, file_wrapper->file, block,
        &file_wrapper->tmpfd, stage);
  }
  else {
    err = row_merge_sort_with_k(trx, dup, file_wrapper->file, block,
        &file_wrapper->tmpfd, stage);
  }

  alloc.deallocate(block);
}

void row_partition_file_destroy(merge_file_wrapper_t * partition_files,
                                ulint n_files) {
  for (ulint i = 0; i < n_files; i++) {
    close(partition_files[i].file->fd);
    partition_files[i].file->fd = -1;
  }
  delete[] partition_files;
  partition_files = nullptr;
}

dberr_t partition_and_sort(trx_t *trx, row_merge_dup_t *dup,
                           merge_file_t *file, row_merge_block_t *block,
                           Alter_stage *stage, Quantiler *quantiler,
                           dict_index_t *index, struct TABLE *table,
                           merge_file_wrapper_t *output_files, bool skip_pk_sort) {
  ut_ad(quantiler);
  dberr_t err = DB_SUCCESS;
#ifdef UNIV_DEBUG_PARALLEL_DDL
  auto start_time = std::chrono::steady_clock::now();
#endif /* UNIV_DEBUG_PARALLEL_DDL */
  index->cached_offs_pddl.init();
  // prepare parallel threads, these threads are for partitioning and then sorting
  const ulint num_runs = file->offset;
  const ulint sort_parallel = quantiler->sort_parallel;

  // prepare sort_parallel files to store partitioned data
  const char *path = thd_innodb_tmpdir(trx->mysql_thd);
  for (ulint f = 0; f < sort_parallel; f++) {
    output_files[f].init(path);
  }

  std::vector<std::thread> partition_workers;
  ulint run = 0;
  for (ulint t = 0; t < sort_parallel; t++) {
    ulint start_run = run;
    ulint end_run = run + (num_runs / sort_parallel) \
                    + (((num_runs % sort_parallel) > ((ulint)t)) ? 1 : 0);
    partition_workers.push_back(std::thread(parallel_partition_file,
                                start_run, end_run, file, output_files,
                                quantiler, index, table, sort_parallel));
    run = end_run;
  }

  DBUG_EXECUTE_IF(
        "parallel_partition_files_corruption",
        DBUG_SET("-d,parallel_partition_files_corruption");
        interrupt_.store(true););

  for (auto &partition_worker : partition_workers) {
    partition_worker.join();
  }

  if (!interrupt_.load()) {
    for (ulint f = 0; f < sort_parallel; f++) {
      output_files[f].file->offset = output_files[f].offset;
#ifdef UNIV_DEBUG_PARALLEL_DDL
      ib::info() << "[TXSQL PARALLEL DDL] Partition(" << f << ") has ("
                << output_files[f].file->n_rec << ") records.";
#endif
    }
  } else {
    err = DB_CORRUPTION;
  }
  if (err != DB_SUCCESS) return err;
  DBUG_EXECUTE_IF(
        "crash_after_partition_before_merge_sort",
        sql_print_information("Crashing "
                              "crash_after_partition_before_merge_sort.");
        DBUG_SET("-d,crash_after_partition_before_merge_sort");
        DBUG_SUICIDE(););

#ifdef UNIV_DEBUG_PARALLEL_DDL
  auto middle_time = std::chrono::steady_clock::now();
  ib::info() << "[TXSQL PARALLEL DDL] Partition finished, costs("
              << std::chrono::duration_cast<std::chrono::microseconds>(
                  middle_time - start_time).count()
              << ") ms";
#endif /* UNIV_DEBUG_PARALLEL_DDL */

  /* Now we perform merge sort for output_files in parallel.
  Each sort worker is responsible for one output_file.
  */

  std::vector<std::thread> sort_workers(sort_parallel);
  std::vector<dberr_t> sort_results(sort_parallel);
  ulong total_cnt = 0;
  for (uint s = 0; s < sort_parallel; s++) {
    sort_workers[s] = std::thread(parallel_merge_sort, s, &output_files[s],
                                  trx, dup, stage, std::ref(sort_results[s]));
    total_cnt += output_files[s].file->n_rec;
  }
  // check that after partitioning, the total num of rec not changed.
  ut_a(total_cnt == file->n_rec);

  for (auto &sort_worker : sort_workers) {
    sort_worker.join();
  }
  DBUG_EXECUTE_IF(
        "parallel_merge_sort_fail",
        DBUG_SET("-d,parallel_merge_sort_fail");
        sort_results[0] = DB_CORRUPTION;);
  // check the results of all sort workers.
  for (auto &res : sort_results) {
    if (res != DB_SUCCESS) {
      err = res;
      break;
    }
  }
  total_cnt = 0;
  for (uint s = 0; s < sort_parallel; s++) {
    total_cnt += output_files[s].file->n_rec;
  }
  ut_a(total_cnt == file->n_rec);
  DBUG_EXECUTE_IF(
        "crash_after_merge_sort_before_build_btree",
        sql_print_information("Crashing "
                              "crash_after_merge_sort_before_build_btree.");
        DBUG_SET("-d,crash_after_merge_sort_before_build_btree");
        DBUG_SUICIDE(););
#ifdef UNIV_DEBUG_PARALLEL_DDL
  auto end_time = std::chrono::steady_clock::now();
  ib::info() << "[TXSQL PARALLEL DDL] Merge sort finished, costs("
             << std::chrono::duration_cast<std::chrono::microseconds>(
                end_time - middle_time).count()
             << ") ms";
#endif /* UNIV_DEBUG_PARALLEL_DDL */

  return err;
}


/* =============================================================================== */
/*                       Parallel Build Btree START                                */
/* =============================================================================== */

class BuildBtrWorker {
public:
  BuildBtrWorker()
    : m_worker_err(DB_SUCCESS),
      m_trx(nullptr),
      m_file(nullptr),
      m_index(nullptr),
      m_stage(nullptr),
      m_btr_bulk(nullptr) {}

  dberr_t init(trx_t *trx, merge_file_t *file, dict_index_t *index,
               dict_table_t *old_table, Flush_observer *observer,
               Alter_stage *stage, bool is_leftmost_subtree);

  void destroy();

  void build_btr_worker_run();
  void join() { m_thread.join(); }
  dberr_t get_worker_err() { return m_worker_err; }
  BtrBulk *get_btr_bulk() { return m_btr_bulk; }
  void set_worker_err(dberr_t worker_err) { m_worker_err = worker_err;}

private:
  dberr_t m_worker_err;
  trx_t *m_trx;
  // IB_thread m_thread;
  std::thread m_thread;
  merge_file_t *m_file;
  dict_index_t *m_index;
  dict_table_t *m_old_table;
  Alter_stage *m_stage;
  BtrBulk *m_btr_bulk;
  AsSubtree m_as_subtree;
};

/** the leftmost path and rightmost path */
struct TreeEdgePath
{
  std::vector<page_id_t> left_path_page_ids;
  std::vector<page_id_t> right_path_page_ids;
};


/** Merge Subtrees into a total tree.
@param[in]  index        index to be created
@param[in]  trx          current transaction
@param[in]  workers      build btree workers
@param[in]  parallel     parallel threads number
@param[in]  max_level    max level among subtrees
@param[in]  mtr          mini-transaction
@return DB_SUCCESS or error code */
dberr_t merge_subtrees(dict_index_t *index, trx_t *trx, BuildBtrWorker *workers,
                       int parallel, ulint max_level, mtr_t* mtr);

/** get edge paths(the left-path and the right path) of each subtree
@param[in]  index        index to be created
@param[in]  workers      build btree workers
@param[in]  parallel     parallel threads number
@param[out] tree_edge_paths   edge paths of each subtree
@param[in]  max_level    max level among subtrees
@param[in]  mtr          mini-transaction
@return DB_SUCCESS or error code */
dberr_t get_tree_edge_paths(dict_index_t *index, BuildBtrWorker *workers, int parallel,
                            TreeEdgePath *tree_edge_paths, ulint max_level, mtr_t *mtr);

/** get the right-most child page id or the left-most child page id
@param[in]  index        index to be created
@param[in]  page_id      current page
@param[in]  leftmost_or_rightmost   true if want left-most child page id, false if want right-most one
@param[in]  mtr          mini-transaction
@param[out] child_page_id   the left-most or right-most child page id
@return DB_SUCCESS or error code */
dberr_t get_child_page_id(dict_index_t *index, page_id_t page_id, bool leftmost_or_rightmost,
                          mtr_t *mtr, page_id_t &child_page_id);

/** link the adjacent paths amoung the subtrees
@param[in]  index        index to be created
@param[in]  trx          current transaction
@param[in]  tree_edge_paths   edge paths of each subtree
@param[in]  parallel     parallel threads number
@param[in]  max_level    max level among subtrees
@param[in]  mtr          mini-transaction
@return DB_SUCCESS or error code */
dberr_t link_adjacent_paths(dict_index_t *index, trx_t *trx, TreeEdgePath *tree_edge_paths,
                           int parallel, ulint max_level, mtr_t *mtr);

/** clear rec_min_flag of block
@param[in]  block        block to be clear flag
@param[in]  comp         page is compact */
void merge_tree_clear_min_rec_flag(buf_block_t *block, bool comp);

/** insert the node_ptrs corresponding to each subtree root page into the index page
@param[in]  index        index to be created
@param[in]  trx          current transaction
@param[in]  tree_edge_paths   edge paths of each subtree
@param[in]  parallel     parallel threads number
@param[in]  max_level    max level among subtrees
@param[in]  mtr          mini-transaction
@return DB_SUCCESS or error code */
dberr_t merge_to_index_root(dict_index_t *index, trx_t *trx, TreeEdgePath *tree_edge_paths,
                            int parallel, ulint max_level, ulint leftmost_level, mtr_t *mtr);

/** compress the total tree merged from these subtrees to lower it
@param[in]  index        index to be created
@param[in]  trx          current transaction
@param[in]  parallel     parallel threads number
@param[in]  initial_root_level    root level before compress
@param[in]  mtr          mini-transaction
@return DB_SUCCESS or error code */
dberr_t compress_tree(dict_index_t *index, trx_t *trx, int parallel, ulint initial_root_level,
                      mtr_t *mtr);

/** get the first child page_no of the index root page
@param[in]  index        index to be created
@param[in]  mtr          mini-transaction
@return page_no of the index root page */
page_no_t compress_tree_get_first_root_child(dict_index_t *index, mtr_t *mtr);

/** the cursor by page_no
@param[in]  index        index to be created
@param[in]  page_no      page_no to get cursor
@param[in]  mtr          mini-transaction
@param[out] cursor       cursor from the page_no
@return DB_SUCCESS or error code */
dberr_t compress_tree_get_cursor_by_page_no(dict_index_t *index, page_no_t page_no,
                                            mtr_t *mtr, btr_cur_t *cursor);

/** get the level of root page of this index
@param[in]  index        index to be created
@param[in]  mtr          mini-transaction
@return DB_SUCCESS or error code */
ulint compress_tree_get_root_level(dict_index_t *index, mtr_t *mtr);

/** parallel build btree
@param[in]  trx          current transaction
@param[in]  index        index to be created
@param[in]  observer     flush observer
@param[in]  old_table    old table of the index
@param[in]  stage        stage to illustrate the progress
@param[in]  parallel_threads   parallel thread number
@return DB_SUCCESS or error code */
dberr_t parallel_build_btree(dict_index_t *index, trx_t *trx, Flush_observer *observer,
                            dict_table_t *old_table, Alter_stage *stage,
                            merge_file_wrapper_t * partitioned_sorted_files,
                            int parallel_threads) {
  dberr_t err = DB_SUCCESS;
  ulint max_level = 0;
  mtr_t merge_mtr;
  mtr_t compress_mtr;
  ut_a(partitioned_sorted_files);

  stage->begin_phase_insert();

  /** Stage 1: build subtrees concurrently */
  BuildBtrWorker *workers = new BuildBtrWorker[parallel_threads];
  for (int t = 0; t < parallel_threads; t++) {
    err = workers[t].init(trx, partitioned_sorted_files[t].file, index,
                          old_table, observer, stage, (t==0));
    if (err != DB_SUCCESS) {
      goto func_exit;
    }
  }

  for (int t = 0; t < parallel_threads; t++) {
    workers[t].join();
  }
  DBUG_EXECUTE_IF(
        "build_btr_worker_fail",
        DBUG_SET("-d,build_btr_worker_fail");
        workers[0].set_worker_err(DB_CORRUPTION););
  for (int t = 0; t < parallel_threads; t++) {
    if (workers[t].get_worker_err() != DB_SUCCESS) {
      err = workers[t].get_worker_err();
      goto func_exit;
    }
  }
  DBUG_EXECUTE_IF(
        "crash_after_build_subtree_before_merge_subtree",
        sql_print_information("Crashing "
                              "crash_after_build_subtree_before_merge_subtree.");
        DBUG_SET("-d,crash_after_build_subtree_before_merge_subtree");
        DBUG_SUICIDE(););

  /* Stage 2: merge subtrees and compress to lower the tree */
  /* 2.1 get max level */
  for (int t = 0; t < parallel_threads; t++) {
    ulint cur_subtree_level = workers[t].get_btr_bulk()->get_root_level();
    if (cur_subtree_level > max_level) {
      max_level = cur_subtree_level;
    }
  }

  /* 2.2 make subtrees same high */
  for (int t = 0; t < parallel_threads; t++) {
    if (workers[t].get_btr_bulk()->get_root_level() < max_level) {
      workers[t].get_btr_bulk()->raise_to_level(max_level);
    }
  }

  /* 2.3 merge subtrees */
  mtr_start(&merge_mtr);
  mtr_set_log_mode(&merge_mtr, MTR_LOG_NO_REDO);
  mtr_set_flush_observer(&merge_mtr, observer);
  err = merge_subtrees(index, trx, workers, parallel_threads, max_level, &merge_mtr);
  mtr_commit(&merge_mtr);
  if (err != DB_SUCCESS) {
    goto func_exit;
  }
  ut_ad(btr_validate_index(index, trx, /*lockout=*/false));
  DBUG_EXECUTE_IF(
        "crash_after_merge_subtree_before_compress_tree",
        sql_print_information("Crashing "
                              "crash_after_merge_subtree_before_compress_tree.");
        DBUG_SET("-d,crash_after_merge_subtree_before_compress_tree");
        DBUG_SUICIDE(););

  /* 2.4 compress tree */
  mtr_start(&compress_mtr);
  mtr_set_log_mode(&compress_mtr, MTR_LOG_NO_REDO);
  mtr_set_flush_observer(&compress_mtr, observer);
  err = compress_tree(index, trx, parallel_threads, max_level + 1, &compress_mtr);
  mtr_commit(&compress_mtr);
  ut_ad(btr_validate_index(index, trx, /*lockout=*/false));
  DBUG_EXECUTE_IF(
        "crash_after_compress_tree",
        sql_print_information("Crashing "
                              "crash_after_compress_tree.");
        DBUG_SET("-d,crash_after_compress_tree");
        DBUG_SUICIDE(););

func_exit:
  row_partition_file_destroy(partitioned_sorted_files, parallel_threads);
  for (int t = 0; t < parallel_threads; t++) {
    workers[t].destroy();
  }
  delete []workers;
  workers = nullptr;
  return err;
}

dberr_t BuildBtrWorker::init(trx_t *trx, merge_file_t *file, dict_index_t *index,
                             dict_table_t *old_table, Flush_observer *observer,
                             Alter_stage *stage, bool is_leftmost_subtree) {
  dberr_t err = DB_SUCCESS;
  m_trx = trx;
  m_file = file;
  m_index = index;
  m_old_table = old_table;
  m_stage = stage;

  m_as_subtree.is_leftmost_subtree = is_leftmost_subtree;
  m_btr_bulk = new BtrBulk(m_index, m_trx->id, observer, &m_as_subtree);
  err = m_btr_bulk->init();
  DBUG_EXECUTE_IF(
		        "build_btr_worker_init_failed",
		        DBUG_SET("-d,build_btr_worker_init_failed");
		        err = DB_OUT_OF_MEMORY;);
  if (err != DB_SUCCESS) {
    return err;
  }

  // m_thread = os_thread_create(parallel_read_thread_key, &BuildBtrWorker::build_btr_worker_run, this);
  // m_thread.start();
  m_thread = std::thread(&BuildBtrWorker::build_btr_worker_run, this);
  return DB_SUCCESS;
}

void BuildBtrWorker::destroy() {
  if (m_btr_bulk) {
    m_btr_bulk->destroy_page_bulks();
    delete m_btr_bulk;
    m_btr_bulk = nullptr;
  }
}

void BuildBtrWorker::build_btr_worker_run() {
  ut::allocator<row_merge_block_t> allocator(mem_key_build_btr);

  mrec_buf_t *buf = nullptr;
  mem_heap_t *heap = nullptr;
  mem_heap_t *tuple_heap = nullptr;
  ulint *offsets = nullptr;
  row_merge_block_t *block = nullptr;
  ulint offsets_i = 1 + REC_OFFS_HEADER_SIZE + dict_index_get_n_fields(m_index);

  heap = mem_heap_create(sizeof(*buf) + offsets_i * sizeof(*offsets), UT_LOCATION_HERE);
  block = allocator.allocate(srv_sort_buf_size);
  buf = static_cast<mrec_buf_t*>(mem_heap_alloc(heap, sizeof(*buf)));
  tuple_heap = mem_heap_create(1000, UT_LOCATION_HERE);
  offsets = static_cast<ulint*>(mem_heap_alloc(heap, offsets_i * sizeof(*offsets)));
  offsets[0] = offsets_i;
  offsets[1] = dict_index_get_n_fields(m_index);

  if (!row_merge_read(m_file->fd, /* offset=*/ 0, block)) {
    m_worker_err = DB_CORRUPTION;
    return;
  }

  const byte *b = block;
  dtuple_t *dtuple = nullptr;
  ulint run = 0;
  ulint cur_tuple_processed = 0;
  static const ulint CHECK_TRX_INTERRUPTED_STEP = 100;

  while (m_worker_err == DB_SUCCESS) {
    const mrec_t *mrec = nullptr;
    b = row_merge_read_rec(block, buf, b, m_index, m_file->fd, &run, &mrec, offsets);
    if (UNIV_UNLIKELY(!b)) {
      if (mrec) {
        m_worker_err = DB_CORRUPTION;
      }
      break;
    }
    dtuple = row_rec_to_index_entry_low(mrec, m_index, offsets, tuple_heap);

    const dict_index_t *old_index = m_old_table->first_index();
    /* If there are externally stored columns. */
    if (dtuple->has_ext()) {
      ut_ad(m_index->is_clustered());
      /* Off-page columns can be fetched safely
      when concurrent modifications to the table
      are disabled. (Purge can process delete-marked
      records, but row_merge_read_clustered_index()
      would have skipped them.)

      When concurrent modifications are enabled,
      row_merge_read_clustered_index() will
      only see rows from transactions that were
      committed before the ALTER TABLE started
      (REPEATABLE READ).

      Any modifications after the
      row_merge_read_clustered_index() scan
      will go through row_log_table_apply().
      Any modifications to off-page columns
      will be tracked by
      row_log_table_blob_alloc() and
      row_log_table_blob_free(). */
      row_merge_copy_blobs(m_trx, old_index, mrec, offsets,
                           dict_table_page_size(m_old_table), dtuple,
                           dict_index_is_sdi(m_index), tuple_heap);
    }

    ut_ad(dtuple_validate(dtuple));

    m_worker_err = m_btr_bulk->insert(dtuple);
    mem_heap_empty(tuple_heap);
    if (UNIV_UNLIKELY(m_worker_err != DB_SUCCESS)) {
      break;
    }
    if (++cur_tuple_processed >= CHECK_TRX_INTERRUPTED_STEP) {
      m_stage->inc(cur_tuple_processed);
      cur_tuple_processed = 0;
      if (trx_is_interrupted(m_trx)) {
        m_worker_err = DB_INTERRUPTED;
        break;
      }
    }
  } // end while

  if (m_worker_err == DB_SUCCESS) {
    m_worker_err = m_btr_bulk->finish(m_worker_err);
  }

  mem_heap_free(tuple_heap);
  mem_heap_free(heap);
  allocator.deallocate(block);
}

dberr_t merge_subtrees(dict_index_t *index, trx_t *trx, BuildBtrWorker *workers,
                       int parallel, ulint max_level, mtr_t* mtr) {
  dberr_t err = DB_SUCCESS;
  TreeEdgePath *tree_edge_paths = new TreeEdgePath[parallel];

  err = get_tree_edge_paths(index, workers, parallel, tree_edge_paths, max_level, mtr);
  if (err != DB_SUCCESS) {
    goto func_exit;
  }

  err = link_adjacent_paths(index, trx, tree_edge_paths, parallel, max_level, mtr);
  if (err != DB_SUCCESS) {
    goto func_exit;
  }

  err = merge_to_index_root(index, trx, tree_edge_paths, parallel, max_level,
                            workers[0].get_btr_bulk()->get_root_level(), mtr);
  if (err != DB_SUCCESS) {
    goto func_exit;
  }

func_exit:
  delete []tree_edge_paths;
  return err;
}

dberr_t get_tree_edge_paths(dict_index_t *index, BuildBtrWorker *workers, int parallel,
                            TreeEdgePath *tree_edge_paths, ulint max_level, mtr_t *mtr) {
  dberr_t err = DB_SUCCESS;
  for (int t = 0; t < parallel; t++) {
    page_id_t root_page_id = {index->space, workers[t].get_btr_bulk()->get_subtree_root_page_no()};
    tree_edge_paths[t].left_path_page_ids.push_back(root_page_id);
    tree_edge_paths[t].right_path_page_ids.push_back(root_page_id);

    page_id_t page_id = root_page_id;
    for (ulint l = max_level; l > 0; l--) {
      page_id_t child_page_id{0, 0};
      /* get leftmost child page id */
      err = get_child_page_id(index, page_id, /*leftmost_or_rightmost*/true, mtr, child_page_id);
      if (err != DB_SUCCESS) {
        goto func_exit;
      }
      page_id = child_page_id;
      tree_edge_paths[t].left_path_page_ids.push_back(page_id);
    }

    page_id = root_page_id;
    for (ulint l = max_level; l > 0; l--) {
      page_id_t child_page_id{0, 0};
      /* get rightmost child page id */
      err = get_child_page_id(index, page_id, /*leftmost_or_rightmost*/false, mtr, child_page_id);
      DBUG_EXECUTE_IF(
          "get_rightmost_child_page_id_failed",
          DBUG_SET("-d,get_rightmost_child_page_id_failed");
          err = DB_CORRUPTION;);
      if (err != DB_SUCCESS) {
        goto func_exit;
      }
      page_id = child_page_id;
      tree_edge_paths[t].right_path_page_ids.push_back(page_id);
    }
  }

func_exit:
  return err;
}

dberr_t get_child_page_id(dict_index_t *index, page_id_t page_id, bool leftmost_or_rightmost,
                          mtr_t *mtr, page_id_t &child_page_id) {
  const page_size_t page_size(dict_table_page_size(index->table));
  page_cur_t page_cursor;

  buf_block_t *block = btr_block_get(page_id, page_size, RW_NO_LATCH, UT_LOCATION_HERE, index, mtr);
  DBUG_EXECUTE_IF(
		        "get_child_page_id_failed",
		        DBUG_SET("-d,get_child_page_id_failed");
		        block = nullptr;);
  if (!block) {
    return DB_CORRUPTION;
  }

  if (leftmost_or_rightmost) {
    page_cur_set_before_first(block, &page_cursor);
    if (page_rec_is_infimum(page_cur_get_rec(&page_cursor))) {
      page_cur_move_to_next(&page_cursor);
    }
  } else {
    page_cur_set_after_last(block, &page_cursor);
    if (page_rec_is_supremum(page_cur_get_rec(&page_cursor))) {
      page_cur_move_to_prev(&page_cursor);
    }
  }

  const auto rec = page_cur_get_rec(&page_cursor);
  mem_heap_t *heap = nullptr;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  auto offsets = offsets_;
  rec_offs_init(offsets_);
  offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &heap);
  auto page_no = btr_node_ptr_get_child_page_no(rec, offsets);
  child_page_id.reset(page_id.space(), page_no);
  if (heap != nullptr) {
    mem_heap_free(heap);
  }
  return DB_SUCCESS;
}

dberr_t link_adjacent_paths(dict_index_t *index, trx_t *trx, TreeEdgePath *tree_edge_paths,
                           int parallel, ulint max_level, mtr_t *mtr) {
  const page_size_t page_size(dict_table_page_size(index->table));
  for (int t = 0; t < parallel - 1; t++) {
    DBUG_EXECUTE_IF(
          "link_adjacent_paths_trx_interrupted",
          DBUG_SET("-d,link_adjacent_paths_trx_interrupted");
          inject_trx_interrupted = true;
          DBUG_SET("+d,trx_is_interrupted_yes"););
    if (trx_is_interrupted(trx)) {
      return DB_INTERRUPTED;
    }
    const std::vector<page_id_t> &tree0_path = tree_edge_paths[t].right_path_page_ids;
    const std::vector<page_id_t> &tree1_path = tree_edge_paths[t+1].left_path_page_ids;
    ut_ad(!(tree0_path.size() != max_level + 1 || tree1_path.size() != max_level + 1));
    for (ulint i = 0; i <= max_level; i++) {
      buf_block_t *left_block =
          btr_block_get(tree0_path[i], page_size, RW_X_LATCH, UT_LOCATION_HERE, index, mtr);
      buf_block_t *right_block =
          btr_block_get(tree1_path[i], page_size, RW_X_LATCH, UT_LOCATION_HERE, index, mtr);
      ut_ad(!(!left_block
          || !right_block
          || (btr_page_get_level(buf_block_get_frame(left_block)) != max_level - i)
          || (btr_page_get_level(buf_block_get_frame(right_block)) != max_level - i)));
      btr_page_set_next(buf_block_get_frame(left_block), buf_block_get_page_zip(left_block),
                        right_block->get_page_no(), mtr);
      btr_page_set_prev(buf_block_get_frame(right_block), buf_block_get_page_zip(right_block),
                        left_block->get_page_no(), mtr);
      /** for compress page, clear REC_INFO_MIN_REC_FLAG.
      for compressed page, REC_INFO_MIN_REC_FLAG will be set when page_zip decompressed
      if it finds page->FIL_PREV == FIL_NULL. This is not true for subtree when creating index.
      So we always clear this flag here. */
      merge_tree_clear_min_rec_flag(right_block, dict_table_is_comp(index->table));
    }
  }
  return DB_SUCCESS;
}

void merge_tree_clear_min_rec_flag(buf_block_t *block, bool comp) {
  rec_t *first_rec = page_rec_get_next(page_get_infimum_rec(buf_block_get_frame(block)));
  ulint info_bits = rec_get_info_bits(first_rec, comp);
  if (info_bits & REC_INFO_MIN_REC_FLAG) {
    if (comp) {
      rec_set_info_bits_new(first_rec, info_bits & (~REC_INFO_MIN_REC_FLAG));
    } else {
      rec_set_info_bits_old(first_rec, info_bits & (~REC_INFO_MIN_REC_FLAG));
    }
  }
}

dberr_t merge_to_index_root(dict_index_t *index, trx_t *trx, TreeEdgePath *tree_edge_paths,
                            int parallel, ulint max_level, ulint leftmost_level, mtr_t *mtr) {
  dberr_t err = DB_SUCCESS;
  /* get index root page */
  const page_size_t page_size(dict_table_page_size(index->table));
  page_id_t index_root_page_id{index->space, index->page};
  buf_block_t *index_root_block =
      btr_block_get(index_root_page_id, page_size, RW_S_LATCH, UT_LOCATION_HERE, index, mtr);
  page_t *index_root_page = buf_block_get_frame(index_root_block);

  /* insert node_ptr(node_key + child page id) into root page */
  mem_heap_t *heap = mem_heap_create(1000, UT_LOCATION_HERE);
  rec_t *cur_rec = page_get_infimum_rec(index_root_page);
  const bool root_page_is_comp = dict_table_is_comp(index->table);
  const page_size_t root_page_size = index_root_block->page.size;

  btr_page_set_level(index_root_page, nullptr, max_level + 1, mtr);

  for (int t = 0; t < parallel; t++) {
    DBUG_EXECUTE_IF(
          "merge_to_index_root_trx_interrupted",
          DBUG_SET("-d,merge_to_index_root_trx_interrupted");
          inject_trx_interrupted = true;
          DBUG_SET("+d,trx_is_interrupted_yes"););
    if (trx_is_interrupted(trx)) {
      err = DB_INTERRUPTED;
      goto func_exit;
    }
    /* prepare node_ptr for each subtree root page */
    page_id_t subtree_root_page_id = tree_edge_paths[t].left_path_page_ids[0];
    buf_block_t *subtree_root_block =
        btr_block_get(subtree_root_page_id, page_size, RW_NO_LATCH, UT_LOCATION_HERE, index, mtr);
    DBUG_EXECUTE_IF(
		        "merge_to_index_root_failed",
		        DBUG_SET("-d,merge_to_index_root_failed");
		        subtree_root_block = nullptr;);
    if (UNIV_UNLIKELY(!subtree_root_block ||
                      subtree_root_block->get_page_id() != subtree_root_page_id)) {
      err = DB_CORRUPTION;
      goto func_exit;
    }
    rec_t *first_rec = page_rec_get_next(page_get_infimum_rec(
                                         buf_block_get_frame(subtree_root_block)));
    dtuple_t *node_ptr = dict_index_build_node_ptr(index, first_rec,
                                         subtree_root_page_id.page_no(), heap, max_level);
    if (t == 0) {
      // make sure the leftmost node has been set with REC_INFO_MIN_REC_FLAG,
      // if the level of the first subtree is greater than 0, it is set when "insert_father",
      // otherwise, set the flag now for the root node.
      if (leftmost_level > 0) {
        ut_a(REC_INFO_MIN_REC_FLAG &
             rec_get_info_bits(first_rec, page_is_comp(buf_block_get_frame(subtree_root_block))));
      }
      else {
        dtuple_set_info_bits(node_ptr, dtuple_get_info_bits(node_ptr) | REC_INFO_MIN_REC_FLAG);
      }
    }
    /* append node_ptr to root_page */
    ulint rec_size = rec_get_converted_size(index, node_ptr);
    if (page_zip_rec_needs_ext(rec_size, root_page_is_comp,
                               dtuple_get_n_fields(node_ptr), root_page_size)) {
      big_rec_t *big_rec = dtuple_convert_big_rec(index, nullptr, node_ptr);
      if (big_rec == nullptr) {
        err = DB_TOO_BIG_RECORD;
        goto func_exit;
      }
      rec_size = rec_get_converted_size(index, node_ptr);
    }
    byte *rec_mem = static_cast<byte*>(mem_heap_alloc(heap, rec_size));
    rec_t *rec = rec_convert_dtuple_to_rec(rec_mem, index, node_ptr);
    ulint *offsets = nullptr;
    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &heap);

    cur_rec = page_cur_insert_rec_low(cur_rec, index, rec, offsets, mtr);

    DBUG_EXECUTE_IF(
            "merge_to_index_root_no_enough_space",
		                DBUG_SET("-d,merge_to_index_root_no_enough_space");
                    cur_rec = NULL;);

    if (!cur_rec) {
      /* no enough space in index root page to insert the node_ptr of subtree root page */
      sql_print_error("no enough space in index root page to merge subtree when parallel ddl enable,"
          "parallel configured: %d, currently merged count: %d. "
          "Turn off parallel ddl, use single thread instead.", parallel, t);
      err = DB_ERROR;
      goto func_exit;
    }
  } // end for parallel

func_exit:
  mem_heap_free(heap);
  return err;
}

dberr_t compress_tree(dict_index_t *index, trx_t *trx, int parallel, ulint initial_root_level,
                      mtr_t *mtr) {
  dberr_t err = DB_SUCCESS;
  mtr_x_lock(dict_index_get_lock(index), mtr, UT_LOCATION_HERE);
  bool compress_succ = false;
  DBUG_EXECUTE_IF(
        "crash_during_compress_tree",
        sql_print_information("Crashing "
                              "crash_during_compress_tree.");
        DBUG_SET("-d,crash_during_compress_tree");
        DBUG_SUICIDE(););
  while (err == DB_SUCCESS) {
    DBUG_EXECUTE_IF(
          "compress_tree_trx_interrupted",
          DBUG_SET("-d,compress_tree_trx_interrupted");
          inject_trx_interrupted = true;
          DBUG_SET("+d,trx_is_interrupted_yes"););
    if (trx_is_interrupted(trx)) {
      err = DB_INTERRUPTED;
      continue;
    }
    btr_cur_t cursor;
    page_no_t page_no = compress_tree_get_first_root_child(index, mtr);
    err = compress_tree_get_cursor_by_page_no(index, page_no, mtr, &cursor);
    if (err != DB_SUCCESS) {
      return err;
    }
    bool compress_recom = btr_cur_compress_recommendation(&cursor, mtr);
    DBUG_EXECUTE_IF(
          "compress_tree_trx_interrupted1",
          DBUG_SET("-d,compress_tree_trx_interrupted1");
          compress_recom = false;
          inject_trx_interrupted = true;
          DBUG_SET("+d,trx_is_interrupted_yes"););
    while (!compress_recom
           || !(compress_succ = btr_compress(&cursor, /*adjust=*/ true, mtr))) {
      if (trx_is_interrupted(trx)) {
        return DB_INTERRUPTED;
      }
      /* if no need to compress at this cursor or try compress fail,
      just go to his right sibling */
      page_t *page = btr_cur_get_page(&cursor);
      page_no_t right_sibling_page_no = btr_page_get_next(page, mtr);
      if (right_sibling_page_no == FIL_NULL) { /* reach right-most */
        return DB_SUCCESS;
      } else {
        /* reset cursor to the right sibling */
        err = compress_tree_get_cursor_by_page_no(index, right_sibling_page_no, mtr, &cursor);
        if (err != DB_SUCCESS) {
          return err;
        }
      }
    } // end while 2

    ulint new_root_level = compress_tree_get_root_level(index, mtr);
    if (new_root_level < initial_root_level) {
      /* since at least one of the subtrees is height-optimal, so now we stop btr_compress */
      return DB_SUCCESS;
    }

    /* After a successful btr_compress, just go to the first child of root page again.
     * This will make sure the current child page is as full as possible before we try to compress
     * at its sibling page. */

  } // end while
  return err;
}

page_no_t compress_tree_get_first_root_child(dict_index_t *index, mtr_t *mtr) {
  const page_size_t page_size(dict_table_page_size(index->table));
  page_id_t index_root_page_id{index->space, index->page};
  buf_block_t *index_root_block =
      btr_block_get(index_root_page_id, page_size, RW_NO_LATCH, UT_LOCATION_HERE, index, mtr);

  page_cur_t page_cursor;
  page_cur_set_before_first(index_root_block, &page_cursor);
  if (page_rec_is_infimum(page_cur_get_rec(&page_cursor))) {
    page_cur_move_to_next(&page_cursor);
  }

  const auto rec = page_cur_get_rec(&page_cursor);
  mem_heap_t *heap = nullptr;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  auto offsets = offsets_;
  rec_offs_init(offsets_);
  offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, UT_LOCATION_HERE, &heap);
  auto page_no = btr_node_ptr_get_child_page_no(rec, offsets);
  if (heap != nullptr) {
    mem_heap_free(heap);
  }
  return page_no;
}

dberr_t compress_tree_get_cursor_by_page_no(dict_index_t *index, page_no_t page_no,
                                            mtr_t *mtr, btr_cur_t *cursor) {
  const page_size_t page_size(dict_table_page_size(index->table));
  page_id_t page_id{index->space, page_no};
  buf_block_t *block = btr_block_get(page_id, page_size, RW_X_LATCH, UT_LOCATION_HERE, index, mtr);
  DBUG_EXECUTE_IF(
		        "compress_tree_get_cursor_by_page_no_failed",
		        DBUG_SET("-d,compress_tree_get_cursor_by_page_no_failed");
		        block = nullptr;);
  if (block == nullptr || block->get_page_id().page_no() != page_no) {
    return DB_CORRUPTION;
  }
  rec_t *rec = page_rec_get_next(page_get_infimum_rec(buf_block_get_frame(block)));
  btr_cur_position(index, rec, block, cursor);
  return DB_SUCCESS;
}

ulint compress_tree_get_root_level(dict_index_t *index, mtr_t *mtr)
{
  const page_size_t page_size(dict_table_page_size(index->table));
  page_id_t page_id{index->space, index->page};
  buf_block_t *block = btr_block_get(page_id, page_size, RW_X_LATCH, UT_LOCATION_HERE, index, mtr);
  const page_t *page = buf_block_get_frame(block);
  return btr_page_get_level(page);
}

/* =============================================================================== */
/*                       Parallel Build Btree END                                  */
/* =============================================================================== */



/** Build indexes on a table by reading a clustered index, creating a temporary
file containing index entries, merge sorting these index entries and inserting
sorted index entries to indexes.
@param[in]	trx		transaction
@param[in]	old_table	table where rows are read from
@param[in]	new_table	table where indexes are created; identical to
old_table unless creating a PRIMARY KEY
@param[in]	online		true if creating indexes online
@param[in]	indexes		indexes to be created
@param[in]	key_numbers	MySQL key numbers
@param[in]	n_indexes	size of indexes[]
@param[in,out]	table		MySQL table, for reporting erroneous key value
if applicable
@param[in]	add_cols	default values of added columns, or NULL
@param[in]	col_map		mapping of old column numbers to new ones, or
NULL if old_table == new_table
@param[in]	add_autoinc	number of added AUTO_INCREMENT columns, or
ULINT_UNDEFINED if none is added
@param[in,out]	sequence	autoinc sequence
@param[in]	skip_pk_sort	whether the new PRIMARY KEY will follow
existing order
@param[in,out]	stage		performance schema accounting object, used by
ALTER TABLE. stage->begin_phase_read_pk() will be called at the beginning of
this function and it will be passed to other functions for further accounting.
@param[in]	add_v		new virtual columns added along with indexes
@param[in]	eval_table	mysql table used to evaluate virtual column
                                value, see innobase_get_computed_value().
@return DB_SUCCESS or error code */
dberr_t row_merge_build_indexes(
    trx_t *trx, dict_table_t *old_table, dict_table_t *new_table, bool online,
    dict_index_t **indexes, const ulint *key_numbers, ulint n_indexes,
    struct TABLE *table, const dtuple_t *add_cols, const ulint *col_map,
    ulint add_autoinc, ddl::Sequence &sequence, bool skip_pk_sort,
    Alter_stage *stage, const dict_add_v_col_t *add_v,
    struct TABLE *eval_table) {
  merge_file_t *merge_files;
  row_merge_block_t *block;
  ulint i;
  dberr_t error;
  int tmpfd = -1;

  Quantilers *quantilers = nullptr;
  std::chrono::steady_clock::time_point start_scan_time;
  ulint max_sample_cnt, sample_step = 0;

  DBUG_TRACE;

  ut_ad(!srv_read_only_mode);
  ut_ad((old_table == new_table) == !col_map);
  ut_ad(!add_cols || col_map);

  interrupt_.store(false);

  stage->begin_phase_read_pk(
      skip_pk_sort && new_table != old_table ? n_indexes - 1 : n_indexes);

  /* Allocate memory for merge file data structure and initialize
  fields */

  ut::allocator<row_merge_block_t> alloc(mem_key_row_merge_sort);

  /* This will allocate "3 * srv_sort_buf_size" elements of type
  row_merge_block_t. The latter is defined as byte. */
  block = alloc.allocate(3 * srv_sort_buf_size);

  if (block == nullptr) {
    return DB_OUT_OF_MEMORY;
  }

  trx_start_if_not_started_xa(trx, true, UT_LOCATION_HERE);

  /* Check if we need a flush observer to flush dirty pages.
  Since we disable redo logging in bulk load, so we should flush
  dirty pages before online log apply, because online log apply enables
  redo logging(we can do further optimization here).
  1. online add index: flush dirty pages right before row_log_apply().
  2. table rebuild: flush dirty pages before row_log_table_apply().

  we use bulk load to create all types of indexes except spatial index,
  for which redo logging is enabled. If we create only spatial indexes,
  we don't need to flush dirty pages at all. */
  bool need_flush_observer = (old_table != new_table);

  for (i = 0; i < n_indexes; i++) {
    if (!dict_index_is_spatial(indexes[i])) {
      need_flush_observer = true;
    }
  }

  Flush_observer *flush_observer = nullptr;
  if (need_flush_observer) {
    flush_observer = ut::new_withkey<Flush_observer>(
        UT_NEW_THIS_FILE_PSI_KEY, new_table->space, trx, stage);
    trx_set_flush_observer(trx, flush_observer);
  }

  merge_files = static_cast<merge_file_t *>(
      ut::malloc_withkey(UT_NEW_THIS_FILE_PSI_KEY, n_indexes * sizeof *merge_files));

  /* Initialize all the merge file descriptors, so that we
  don't call row_merge_file_destroy() on uninitialized
  merge file descriptor */

  for (i = 0; i < n_indexes; i++) {
    merge_files[i].fd = -1;
  }

  /* Reset the MySQL row buffer that is used when reporting duplicate keys. */
  innobase_rec_reset(table);

  size_t parallel_read_threads, parallel_sort_threads;
  check_if_can_use_parallel_ddl(trx, indexes, n_indexes, old_table,
                                new_table, eval_table, add_autoinc, skip_pk_sort,
                                parallel_read_threads, parallel_sort_threads);

  // we must ensure that the total number of sample is not greater than max_sample_cnt
  sample_step = get_sample_step(old_table, indexes, n_indexes,
                                            trx->mysql_thd, max_sample_cnt);
  // there are too little samples that it is not worthy to do parallel ddl.
  if (max_sample_cnt < 10 * parallel_sort_threads) {
    parallel_sort_threads = 1;
  }

  sql_print_information("[TXSQL Parallel DDL] parallel_read_threads with %u threads, "\
                          "parallel_sort_threads with %u threads, sql: %s, "\
                          "sample_step: %u, max_sample_cnt: %u",
                          parallel_read_threads, parallel_sort_threads,
                          trx->mysql_thd->query().str, sample_step, max_sample_cnt);

  if (parallel_sort_threads > 1) {
#ifdef UNIV_DEBUG_PARALLEL_DDL
    ib::info() << "[TXSQL PARALLEL DDL] Parallism(" << parallel_sort_threads
               << ", " << parallel_read_threads
               << ") Sample step(" << sample_step << ") Max sample count("
               << max_sample_cnt << ").";
#endif /* UNIV_DEBUG_PARALLEL_DDL */

    quantilers = new Quantilers();
    quantilers->init(n_indexes, parallel_read_threads, parallel_sort_threads,
                    indexes, sample_step, table, max_sample_cnt / parallel_read_threads); // CHECK cnt/parallel
  }

#ifdef UNIV_DEBUG_PARALLEL_DDL
  start_scan_time = std::chrono::steady_clock::now();
#endif /* UNIV_DEBUG_PARALLEL_DDL */
  if (parallel_read_threads > 1) {
    parallel_scan_context_t ctx;
    ctx.online = online;
    ctx.table = table;
    ctx.eval_table = eval_table;
    ctx.old_table = old_table;
    ctx.new_table = new_table;
    ctx.key_numbers = key_numbers;
    ctx.add_cols = add_cols;
    ctx.add_v = add_v;
    ctx.col_map = col_map;
    ctx.skip_pk_sort = skip_pk_sort;

    error = row_merge_parallel_read_clustered_index(ctx, trx, indexes,
                                n_indexes, merge_files, &tmpfd, stage,
                                parallel_read_threads, quantilers);
  } else {
    /* Read clustered index of the table and create files for
       secondary index entries for merge sort */
    error = row_merge_read_clustered_index(
        trx, table, old_table, new_table, online, indexes,
        merge_files, key_numbers, n_indexes, add_cols, add_v, col_map,
        add_autoinc, sequence, block, skip_pk_sort, &tmpfd, stage, eval_table, quantilers);
  }

  stage->end_phase_read_pk();

  if (error != DB_SUCCESS) {
    goto func_exit;
  }

  // check if there is so many runs in merge_files, if not, do not use parallel ddl
  check_if_have_enough_run(merge_files, n_indexes, indexes, parallel_sort_threads, skip_pk_sort);

  if (quantilers && parallel_sort_threads > 1) {
    quantilers->build_quantiles(parallel_sort_threads);
  }

#ifdef UNIV_DEBUG_PARALLEL_DDL
  ib::info() << "[TXSQL PARALLEL DDL] Parallel scan cluster index finished, costs("
              << std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::steady_clock::now() - start_scan_time
                ).count() << ") ms. Total row count("
            << merge_files[0].fd << ", " << merge_files[0].n_rec << ").";
#endif /* UNIV_DEBUG_PARALLEL_DDL */

  /* Now we have files containing index entries ready for
  sorting and inserting. */

  for (i = 0; i < n_indexes; i++) {
    dict_index_t *sort_idx = indexes[i];

    assert(!dict_index_is_spatial(sort_idx));

    assert(!(indexes[i]->type & DICT_FTS));
    if (merge_files[i].fd >= 0) {

#ifdef UNIV_DEBUG_PARALLEL_DDL
      auto start_merge_sort_time = std::chrono::steady_clock::now();
#endif /* UNIV_DEBUG_PARALLEL_DDL */

      stage->begin_phase_sort(log2(merge_files[i].offset));

      row_merge_dup_t dup = {sort_idx, table, col_map, 0};

      merge_file_wrapper_t *partitioned_sorted_files = nullptr;
      /* When parallel_sort_threads is adjusted to 1, but quantilers is not null,
      it means, in the scan stage, we do not sort merge buffers, thus, we still need the
      patition stage although parallel_sort_threads is equal to 1. */
      if (parallel_sort_threads > 1 || quantilers) {
        partitioned_sorted_files = new merge_file_wrapper_t[parallel_sort_threads];

        Quantiler *quantiler = &(quantilers->quantiler_arr[i]);
        error = partition_and_sort(trx, &dup, &merge_files[i], block,
                                   stage, quantiler, sort_idx, table,
                                   partitioned_sorted_files, skip_pk_sort);

      } else {
        error = row_merge_sort(trx, &dup, &merge_files[i], block, &tmpfd, stage);
      }
#ifdef UNIV_DEBUG_PARALLEL_DDL
      ib::info() << "[TXSQL PARALLEL DDL] Merge sort finished, costs("
                 << std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::steady_clock::now() - start_merge_sort_time).count()
                 << ") ms.";
#endif /* UNIV_DEBUG_PARALLEL_DDL */

      if (error == DB_SUCCESS) {
        DBUG_EXECUTE_IF(
        "row_merge_sort_failed",
        DBUG_SET("-d,row_merge_sort_failed");
        error = DB_CORRUPTION;);
      }

#ifdef UNIV_DEBUG_PARALLEL_DDL
      auto start_build_btree_time = std::chrono::steady_clock::now();
#endif

      if (error == DB_SUCCESS) {
        const int parallel_build_btree_threads = parallel_sort_threads;
        if (parallel_build_btree_threads > 1) {
          error = parallel_build_btree(sort_idx, trx, flush_observer, old_table,
                                       stage, partitioned_sorted_files,
                                       parallel_build_btree_threads);
        } else {
          BtrBulk btr_bulk(sort_idx, trx->id, flush_observer);
          error = btr_bulk.init();
          if (error == DB_SUCCESS) {
            error = row_merge_insert_index_tuples(trx, sort_idx, old_table,
                                                  merge_files[i].fd, block,
                                                  nullptr, &btr_bulk, stage);

            error = btr_bulk.finish(error);
          }
        }
      } else if (parallel_sort_threads > 1){
        row_partition_file_destroy(partitioned_sorted_files, parallel_sort_threads);
      }
      const_cast<dict_index_t *> (sort_idx)->cached_offs_pddl.reset();

#ifdef UNIV_DEBUG_PARALLEL_DDL
      ib::info() << "[TXSQL PARALLEL DDL] Build btree finished, costs(" 
                 << std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - start_build_btree_time).count()
                 << ") ms.";
#endif /* UNIV_DEBUG_PARALLEL_DDL */
    }

    /* Close the temporary file to free up space. */
    row_merge_file_destroy(&merge_files[i]);

    if (error != DB_SUCCESS || !online) {
      /* Do not apply any online log. */
    } else if (old_table != new_table) {
      ut_ad(!sort_idx->online_log);
      ut_ad(sort_idx->online_status == ONLINE_INDEX_COMPLETE);
      if (parallel_sort_threads > 1) {
        flush_observer->flush();
      }
    } else {
      ut_ad(need_flush_observer);

      flush_observer->flush();
      row_merge_write_redo(indexes[i]);

      DEBUG_SYNC_C("row_log_apply_before");
      error = row_log_apply(trx, sort_idx, table, stage);
      DEBUG_SYNC_C("row_log_apply_after");
    }

    if (error != DB_SUCCESS) {
      trx->error_key_num = key_numbers[i];
      goto func_exit;
    }
  } // end for each index

func_exit:

  if (quantilers != nullptr) {
    quantilers->destroy();
    delete quantilers;
  }

  row_merge_file_destroy_low(tmpfd);

  for (i = 0; i < n_indexes; i++) {
    row_merge_file_destroy(&merge_files[i]);
  }

  ut::free(merge_files);

  alloc.deallocate(block);

  if (online && old_table == new_table && error != DB_SUCCESS) {
    /* On error, flag all online secondary index creation
    as aborted. */
    for (i = 0; i < n_indexes; i++) {
      ut_ad(!(indexes[i]->type & DICT_FTS));
      ut_ad(!indexes[i]->is_committed());
      ut_ad(!indexes[i]->is_clustered());

      /* Completed indexes should be dropped as
      well, and indexes whose creation was aborted
      should be dropped from the persistent
      storage. However, at this point we can only
      set some flags in the not-yet-published
      indexes. These indexes will be dropped later
      in row_merge_drop_indexes(), called by
      rollback_inplace_alter_table(). */

      switch (dict_index_get_online_status(indexes[i])) {
        case ONLINE_INDEX_COMPLETE:
          break;
        case ONLINE_INDEX_CREATION:
          rw_lock_x_lock(dict_index_get_lock(indexes[i]), UT_LOCATION_HERE);
          row_log_abort_sec(indexes[i]);
          indexes[i]->type |= DICT_CORRUPT;
          rw_lock_x_unlock(dict_index_get_lock(indexes[i]));
          new_table->drop_aborted = true;
          /* fall through */
        case ONLINE_INDEX_ABORTED_DROPPED:
        case ONLINE_INDEX_ABORTED:
          break;
      }
    }
  }

  DBUG_EXECUTE_IF("ib_index_crash_after_bulk_load", DBUG_SUICIDE(););

  if (flush_observer != nullptr) {
    ut_ad(need_flush_observer);

    DBUG_EXECUTE_IF("ib_index_build_fail_before_flush", error = DB_FAIL;);

    if (error != DB_SUCCESS) {
      flush_observer->interrupted();
    }

    flush_observer->flush();

    ut::delete_(flush_observer);

    if (trx_is_interrupted(trx)) {
      error = DB_INTERRUPTED;
    }

    if (error == DB_SUCCESS && old_table != new_table) {
      for (const dict_index_t *index = new_table->first_index();
           index != nullptr; index = index->next()) {
        row_merge_write_redo(index);
      }
    }
  }

  return error;
}

