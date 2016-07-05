#ifndef LREC_WRITERS_H
#define LREC_WRITERS_H
#include "cli/quoting.h"
#include "output/lrec_writer.h"

lrec_writer_t*  lrec_writer_alloc(char* fmtdesc, char* ors, char* ofs, char* ops,
	int headerless_csv_output, int oquoting,
	int left_align_pprint,
	int right_justify_xtab_value,
	char* json_flatten_separator,
	int quote_json_values_always,
	int stack_json_output_vertically,
	int wrap_json_output_in_outer_list);

lrec_writer_t* lrec_writer_csv_alloc(char* ors, char* ofs, quoting_t oquoting, int headerless_csv_output);
lrec_writer_t* lrec_writer_csvlite_alloc(char* ors, char* ofs, int headerless_csv_output);
lrec_writer_t* lrec_writer_markdown_alloc(char* ors);
lrec_writer_t* lrec_writer_dkvp_alloc(char* ors, char* ofs, char* ops);
lrec_writer_t* lrec_writer_json_alloc(int stack_vertically, int wrap_json_output_in_outer_list,
	int quote_json_values_always, char* json_flatten_separator);
lrec_writer_t* lrec_writer_nidx_alloc(char* ors, char* ofs);
lrec_writer_t* lrec_writer_pprint_alloc(char* ors, char ofs, int left_align);
lrec_writer_t* lrec_writer_xtab_alloc(char* ofs, char* ops, int right_justify_value);

#endif // LREC_WRITERS_H
