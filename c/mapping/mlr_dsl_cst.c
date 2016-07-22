#include "lib/mlr_globals.h"
#include "lib/mlrutil.h"
#include "containers/hss.h"
#include "mlr_dsl_cst.h"
#include "context_flags.h"

// ================================================================
// The Lemon parser in dsls/mlr_dsl_parse.y builds up an abstract syntax tree
// specifically for the CST builder here.
//
// For clearer visuals on what the ASTs look like:
// * See dsls/mlr_dsl_parse.y
// * See reg_test/run's filter -v and put -v outputs, e.g. in reg_test/expected/out
// * Do "mlr -n put -v 'your expression goes here'"
// ================================================================

static mlr_dsl_ast_node_t* get_list_for_block(mlr_dsl_ast_node_t* pnode);

static mlr_dsl_cst_statement_t* alloc_cst_statement(mlr_dsl_ast_node_t* pnode, int type_inferencing, int context_flags);
static mlr_dsl_cst_statement_t* alloc_blank();
static void cst_statement_free(mlr_dsl_cst_statement_t* pstatement);

// ti = type_inferencing
// cf = context_flags
// dfp = do_full_prefixing
static mlr_dsl_cst_statement_t*                  alloc_srec_assignment(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*         alloc_indirect_srec_assignment(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                alloc_oosvar_assignment(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t* alloc_oosvar_from_full_srec_assignment(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t* alloc_full_srec_from_oosvar_assignment(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                            alloc_unset(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                            alloc_emitf(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                        alloc_tee_write(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                       alloc_tee_append(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                      alloc_emitf_write(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                     alloc_emitf_append(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                    alloc_emit_or_emitp(mlr_dsl_ast_node_t* p, int ti, int cf, int dfp);
static mlr_dsl_cst_statement_t*              alloc_emit_or_emitp_write(mlr_dsl_ast_node_t* p, int ti, int cf, int dfp);
static mlr_dsl_cst_statement_t*             alloc_emit_or_emitp_append(mlr_dsl_ast_node_t* p, int ti, int cf, int dfp);
static mlr_dsl_cst_statement_t*             alloc_emit_or_emitp_lashed(mlr_dsl_ast_node_t* p, int ti, int cf, int dfp);
static mlr_dsl_cst_statement_t*       alloc_emit_or_emitp_lashed_write(mlr_dsl_ast_node_t* p, int ti, int cf, int dfp);
static mlr_dsl_cst_statement_t*      alloc_emit_or_emitp_lashed_append(mlr_dsl_ast_node_t* p, int ti, int cf, int dfp);
static mlr_dsl_cst_statement_t*                alloc_conditional_block(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                          alloc_if_head(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                            alloc_while(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                         alloc_do_while(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                         alloc_for_srec(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                       alloc_for_oosvar(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                            alloc_break(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                         alloc_continue(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                           alloc_filter(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                             alloc_dump(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                            alloc_edump(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                       alloc_dump_write(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                      alloc_dump_append(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                            alloc_print(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                           alloc_eprint(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                      alloc_print_write(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                     alloc_print_append(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                           alloc_printn(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                          alloc_eprintn(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                     alloc_printn_write(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                    alloc_printn_append(mlr_dsl_ast_node_t* p, int ti, int cf);
static mlr_dsl_cst_statement_t*                     alloc_bare_boolean(mlr_dsl_ast_node_t* p, int ti, int cf);

static mlr_dsl_cst_statement_t* alloc_if_item(
	mlr_dsl_ast_node_t* pexprnode,
	mlr_dsl_ast_node_t* plistnode,
	int                 type_inferencing,
	int                 context_flags);

static mlr_dsl_cst_statement_vararg_t* mlr_dsl_cst_statement_vararg_alloc(
	char*             emitf_or_unset_srec_field_name,
	rval_evaluator_t* punset_srec_field_name_evaluator,
	rval_evaluator_t* pemitf_arg_evaluator,
	sllv_t*           punset_oosvar_keylist_evaluators);

static sllv_t* allocate_keylist_evaluators_from_oosvar_node(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags);

static void cst_statement_vararg_free(mlr_dsl_cst_statement_vararg_t* pvararg);

static void handle_statement_list_with_break_continue(
	sllv_t*        pcst_statements,
	variables_t*   pvars,
	cst_outputs_t* pcst_outputs);

static void                  handle_srec_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void         handle_indirect_srec_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                handle_oosvar_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void      handle_oosvar_to_oosvar_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void handle_oosvar_from_full_srec_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void handle_full_srec_from_oosvar_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                handle_oosvar_assignment(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_unset(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                        handle_unset_all(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                        handle_tee_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                       handle_tee_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_emitf(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                      handle_emitf_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                     handle_emitf_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_emitp(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                      handle_emitp_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                     handle_emitp_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                             handle_emit(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                       handle_emit_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                      handle_emit_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                     handle_emitp_lashed(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void               handle_emitp_lashed_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void              handle_emitp_lashed_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                      handle_emit_lashed(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                handle_emit_lashed_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void               handle_emit_lashed_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                        handle_emitp_all(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                  handle_emitp_all_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                 handle_emitp_all_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                         handle_emit_all(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                   handle_emit_all_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                  handle_emit_all_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                             handle_dump(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                       handle_dump_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                      handle_dump_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_edump(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_print(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                      handle_print_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                     handle_print_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                           handle_printn(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                     handle_printn_write(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                    handle_printn_append(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                           handle_eprint(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                          handle_eprintn(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                           handle_filter(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                handle_conditional_block(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_while(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                         handle_do_while(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                         handle_for_srec(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                       handle_for_oosvar(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                            handle_break(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                         handle_continue(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                          handle_if_head(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);
static void                     handle_bare_boolean(mlr_dsl_cst_statement_t* s, variables_t* v, cst_outputs_t* o);

static void handle_for_oosvar_aux(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs,
	mlhmmv_value_t           submap,
	sllse_t*                 prest_for_k_names);

static void handle_unset_vararg_oosvar(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs);

static void handle_unset_vararg_full_srec(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs);

static void handle_unset_vararg_srec_field_name(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs);

static void handle_unset_vararg_indirect_srec_field_name(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs);

// ----------------------------------------------------------------
// For mlr filter, which takes a reduced subset of mlr-put syntax:
// * The root node of the AST must be a statement list (as for put).
// * The list must have one child node.
// * That child node must not be a braced statement (begin, end, for, cond, etc.)
// * The child node must evaluate to boolean, although this is fully enforced only
//   during stream processing.

mlr_dsl_ast_node_t* extract_filterable_statement(mlr_dsl_ast_t* pnode, int type_inferencing) {
	mlr_dsl_ast_node_t* proot = pnode->proot;

	if (proot == NULL) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d: null root node.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	if (proot->pchildren->phead == NULL) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d: null left child node.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	if (proot->pchildren->phead->pnext != NULL) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d: extraneous right child node.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	if (proot->type != MD_AST_NODE_TYPE_STATEMENT_LIST) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d:\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		fprintf(stderr,
			"expected node type %s but found %s.\n",
			mlr_dsl_ast_node_describe_type(MD_AST_NODE_TYPE_STATEMENT_LIST),
			mlr_dsl_ast_node_describe_type(proot->type));
		exit(1);
	}

	mlr_dsl_ast_node_t* pleft = proot->pchildren->phead->pvvalue;

	return pleft;
}

// ----------------------------------------------------------------
// Main entry point for AST-to-CST for mlr put.
//
// Example AST (using put -v):
//
// $ mlr -n put -v '#begin{@a=1;@b=2};$m=2;$n=4;end{@y=5;@z=6}'
// AST ROOT:
// list (statement_list):
//     begin (begin):
//         list (statement_list):
//             = (oosvar_assignment):
//                 a (oosvar_name).
//                 1 (strnum_literal).
//             = (oosvar_assignment):
//                 b (oosvar_name).
//                 2 (strnum_literal).
//     = (srec_assignment):
//         m (field_name).
//         2 (strnum_literal).
//     = (srec_assignment):
//         n (field_name).
//         4 (strnum_literal).
//     end (end):
//         list (statement_list):
//             = (oosvar_assignment):
//                 y (oosvar_name).
//                 5 (strnum_literal).
//             = (oosvar_assignment):
//                 z (oosvar_name).
//                 6 (strnum_literal).

mlr_dsl_cst_t* mlr_dsl_cst_alloc(mlr_dsl_ast_t* pnode, int type_inferencing) {
	int context_flags = 0;
	// The root node is not populated on empty-string input to the parser.
	if (pnode->proot == NULL) {
		pnode->proot = mlr_dsl_ast_node_alloc_zary("list", MD_AST_NODE_TYPE_STATEMENT_LIST);
	}

	mlr_dsl_cst_t* pcst = mlr_malloc_or_die(sizeof(mlr_dsl_cst_t));

	if (pnode->proot->type != MD_AST_NODE_TYPE_STATEMENT_LIST) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d:\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		fprintf(stderr,
			"expected root node type %s but found %s.\n",
			mlr_dsl_ast_node_describe_type(MD_AST_NODE_TYPE_STATEMENT_LIST),
			mlr_dsl_ast_node_describe_type(pnode->proot->type));
		exit(1);
	}

	pcst->pbegin_statements = sllv_alloc();
	pcst->pmain_statements  = sllv_alloc();
	pcst->pend_statements   = sllv_alloc();
	mlr_dsl_ast_node_t* plistnode = NULL;
	for (sllve_t* pe = pnode->proot->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pnode = pe->pvvalue;
		switch (pnode->type) {
		case MD_AST_NODE_TYPE_BEGIN:
			plistnode = get_list_for_block(pnode);
			for (sllve_t* pe = plistnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pchild = pe->pvvalue;
				sllv_append(pcst->pbegin_statements, alloc_cst_statement(pchild, type_inferencing,
					context_flags | IN_BEGIN_OR_END));
			}
			break;
		case MD_AST_NODE_TYPE_END:
			plistnode = get_list_for_block(pnode);
			for (sllve_t* pe = plistnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pchild = pe->pvvalue;
				sllv_append(pcst->pend_statements, alloc_cst_statement(pchild, type_inferencing,
					context_flags | IN_BEGIN_OR_END));
			}
			break;
		default:
			sllv_append(pcst->pmain_statements, alloc_cst_statement(pnode, type_inferencing,
				context_flags));
			break;
		}
	}
	return pcst;
}

// ----------------------------------------------------------------
// For begin, end, cond: there must be one child node, of type list.
static mlr_dsl_ast_node_t* get_list_for_block(mlr_dsl_ast_node_t* pnode) {
	if (pnode->pchildren->phead == NULL) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d: null left child node.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	if (pnode->pchildren->phead->pnext != NULL) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d: extraneous right child node.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_ast_node_t* pleft = pnode->pchildren->phead->pvvalue;

	if (pleft->type != MD_AST_NODE_TYPE_STATEMENT_LIST) {
		fprintf(stderr,
			"%s: internal coding error detected in file %s at line %d:\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		fprintf(stderr,
			"expected node type %s but found %s.\n",
			mlr_dsl_ast_node_describe_type(MD_AST_NODE_TYPE_STATEMENT_LIST),
			mlr_dsl_ast_node_describe_type(pleft->type));
		exit(1);
	}
	return pleft;
}

// ----------------------------------------------------------------
void mlr_dsl_cst_free(mlr_dsl_cst_t* pcst) {
	if (pcst == NULL)
		return;
	for (sllve_t* pe = pcst->pbegin_statements->phead; pe != NULL; pe = pe->pnext)
		cst_statement_free(pe->pvvalue);
	for (sllve_t* pe = pcst->pmain_statements->phead; pe != NULL; pe = pe->pnext)
		cst_statement_free(pe->pvvalue);
	for (sllve_t* pe = pcst->pend_statements->phead; pe != NULL; pe = pe->pnext)
		cst_statement_free(pe->pvvalue);
	sllv_free(pcst->pbegin_statements);
	sllv_free(pcst->pmain_statements);
	sllv_free(pcst->pend_statements);

	free(pcst);
}

// ----------------------------------------------------------------
// The parser accepts many things that are invalid, e.g.
// * begin{end{}} -- begin/end not at top level
// * begin{$x=1} -- references to stream records at begin/end
// * break/continue outside of for/while/do-while
// * $x=x -- boundvars outside of for-loop variable bindings
//
// All of the above are enforced here by the CST builder, which takes the parser's output AST as
// input.  This is done (a) to keep the parser from being overly complex, and (b) so we can get much
// more informative error messages in C than in Lemon ('syntax error').
//
// In this file we set up left-hand sides for assignments, as well as right-hand sides for emit and
// unset.  Most right-hand sides are set up in rval_expr_evaluators.c so the context_flags are
// passed through to there as well.

static mlr_dsl_cst_statement_t* alloc_cst_statement(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	switch(pnode->type) {

	case MD_AST_NODE_TYPE_BEGIN:
		fprintf(stderr, "%s: begin statements are only valid at top level.\n", MLR_GLOBALS.bargv0);
		exit(1);
		break;
	case MD_AST_NODE_TYPE_END:
		fprintf(stderr, "%s: end statements are only valid at top level.\n", MLR_GLOBALS.bargv0);
		exit(1);
		break;

	case MD_AST_NODE_TYPE_CONDITIONAL_BLOCK:
		return alloc_conditional_block(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_IF_HEAD:
		return alloc_if_head(pnode, type_inferencing, context_flags);
		break;

	case MD_AST_NODE_TYPE_WHILE:
		return alloc_while(pnode, type_inferencing, context_flags | IN_BREAKABLE);
		break;
	case MD_AST_NODE_TYPE_DO_WHILE:
		return alloc_do_while(pnode, type_inferencing, context_flags | IN_BREAKABLE);
		break;
	case MD_AST_NODE_TYPE_FOR_SREC:
		return alloc_for_srec(pnode, type_inferencing, context_flags | IN_BREAKABLE | IN_BINDABLE);
		break;
	case MD_AST_NODE_TYPE_FOR_OOSVAR:
		return alloc_for_oosvar(pnode, type_inferencing, context_flags | IN_BREAKABLE | IN_BINDABLE);
		break;

	case MD_AST_NODE_TYPE_BREAK:
		if (!(context_flags & IN_BREAKABLE)) {
			fprintf(stderr, "%s: break statements are only valid within for, while, or do-while.\n",
				MLR_GLOBALS.bargv0);
			exit(1);
		}
		return alloc_break(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_CONTINUE:
		if (!(context_flags & IN_BREAKABLE)) {
			fprintf(stderr, "%s: break statements are only valid within for, while, or do-while.\n",
				MLR_GLOBALS.bargv0);
			exit(1);
		}
		return alloc_continue(pnode, type_inferencing, context_flags);
		break;

	case MD_AST_NODE_TYPE_SREC_ASSIGNMENT:
		if (context_flags & IN_BEGIN_OR_END) {
			fprintf(stderr, "%s: assignments to $-variables are not valid within begin or end blocks.\n",
				MLR_GLOBALS.bargv0);
			exit(1);
		}
		return alloc_srec_assignment(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_INDIRECT_SREC_ASSIGNMENT:
		if (context_flags & IN_BEGIN_OR_END) {
			fprintf(stderr, "%s: assignments to $-variables are not valid within begin or end blocks.\n",
				MLR_GLOBALS.bargv0);
			exit(1);
		}
		return alloc_indirect_srec_assignment(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_OOSVAR_ASSIGNMENT:
		return alloc_oosvar_assignment(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_OOSVAR_FROM_FULL_SREC_ASSIGNMENT:
		if (context_flags & IN_BEGIN_OR_END) {
			fprintf(stderr, "%s: assignments from $-variables are not valid within begin or end blocks.\n",
				MLR_GLOBALS.bargv0);
			exit(1);
		}
		return alloc_oosvar_from_full_srec_assignment(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_FULL_SREC_FROM_OOSVAR_ASSIGNMENT:
		if (context_flags & IN_BEGIN_OR_END) {
			fprintf(stderr, "%s: assignments to $-variables are not valid within begin or end blocks.\n",
				MLR_GLOBALS.bargv0);
			exit(1);
		}
		return alloc_full_srec_from_oosvar_assignment(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_UNSET:
		return alloc_unset(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_TEE_WRITE:
		return alloc_tee_write(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_TEE_APPEND:
		return alloc_tee_append(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EMITF:
		return alloc_emitf(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EMITF_WRITE:
		return alloc_emitf_write(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EMITF_APPEND:
		return alloc_emitf_append(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EMITP:
		return alloc_emit_or_emitp(pnode, type_inferencing, context_flags, TRUE);
		break;
	case MD_AST_NODE_TYPE_EMITP_WRITE:
		return alloc_emit_or_emitp_write(pnode, type_inferencing, context_flags, TRUE);
		break;
	case MD_AST_NODE_TYPE_EMITP_APPEND:
		return alloc_emit_or_emitp_append(pnode, type_inferencing, context_flags, TRUE);
		break;
	case MD_AST_NODE_TYPE_EMIT:
		return alloc_emit_or_emitp(pnode, type_inferencing, context_flags, FALSE);
		break;
	case MD_AST_NODE_TYPE_EMIT_WRITE:
		return alloc_emit_or_emitp_write(pnode, type_inferencing, context_flags, FALSE);
		break;
	case MD_AST_NODE_TYPE_EMIT_APPEND:
		return alloc_emit_or_emitp_append(pnode, type_inferencing, context_flags, FALSE);
		break;
	case MD_AST_NODE_TYPE_EMITP_LASHED:
		return alloc_emit_or_emitp_lashed(pnode, type_inferencing, context_flags, TRUE);
		break;
	case MD_AST_NODE_TYPE_EMITP_LASHED_WRITE:
		return alloc_emit_or_emitp_lashed_write(pnode, type_inferencing, context_flags, TRUE);
		break;
	case MD_AST_NODE_TYPE_EMITP_LASHED_APPEND:
		return alloc_emit_or_emitp_lashed_append(pnode, type_inferencing, context_flags, TRUE);
		break;
	case MD_AST_NODE_TYPE_EMIT_LASHED:
		return alloc_emit_or_emitp_lashed(pnode, type_inferencing, context_flags, FALSE);
		break;
	case MD_AST_NODE_TYPE_EMIT_LASHED_WRITE:
		return alloc_emit_or_emitp_lashed_write(pnode, type_inferencing, context_flags, FALSE);
		break;
	case MD_AST_NODE_TYPE_EMIT_LASHED_APPEND:
		return alloc_emit_or_emitp_lashed_append(pnode, type_inferencing, context_flags, FALSE);
		break;
	case MD_AST_NODE_TYPE_FILTER:
		return alloc_filter(pnode, type_inferencing, context_flags);
		break;

	case MD_AST_NODE_TYPE_DUMP:
		return alloc_dump(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EDUMP:
		return alloc_edump(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_DUMP_WRITE:
		return alloc_dump_write(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_DUMP_APPEND:
		return alloc_dump_append(pnode, type_inferencing, context_flags);
		break;

	case MD_AST_NODE_TYPE_PRINT:
		return alloc_print(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EPRINT:
		return alloc_eprint(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_PRINT_WRITE:
		return alloc_print_write(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_PRINT_APPEND:
		return alloc_print_append(pnode, type_inferencing, context_flags);
		break;

	case MD_AST_NODE_TYPE_PRINTN:
		return alloc_printn(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_EPRINTN:
		return alloc_eprintn(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_PRINTN_WRITE:
		return alloc_printn_write(pnode, type_inferencing, context_flags);
		break;
	case MD_AST_NODE_TYPE_PRINTN_APPEND:
		return alloc_printn_append(pnode, type_inferencing, context_flags);
		break;

	default:
		return alloc_bare_boolean(pnode, type_inferencing, context_flags);
		break;
	}
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_blank() {
	mlr_dsl_cst_statement_t* pstatement = mlr_malloc_or_die(sizeof(mlr_dsl_cst_statement_t));

	pstatement->pnode_handler                        = NULL;
	pstatement->pblock_handler                       = NULL;
	pstatement->poosvar_lhs_keylist_evaluators       = NULL;
	pstatement->pemit_keylist_evaluators             = NULL;
	pstatement->num_emit_keylist_evaluators          = 0;
	pstatement->ppemit_keylist_evaluators            = NULL;
	pstatement->srec_lhs_field_name                  = NULL;
	pstatement->psrec_lhs_evaluator                  = NULL;
	pstatement->prhs_evaluator                       = NULL;
	pstatement->poutput_filename_evaluator           = NULL;
	pstatement->pmulti_out                           = NULL;
	pstatement->pmulti_lrec_writer                   = NULL;
	pstatement->poosvar_rhs_keylist_evaluators       = NULL;
	pstatement->pemit_oosvar_namelist_evaluators     = NULL;
	pstatement->pvarargs                             = NULL;
	pstatement->pblock_statements                    = NULL;
	pstatement->pif_chain_statements                 = NULL;
	pstatement->pfor_oosvar_k_names                  = NULL;
	pstatement->for_v_name                           = NULL;
	pstatement->ptype_infererenced_srec_field_getter = NULL;
	pstatement->pbound_variables                     = NULL;

	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_srec_assignment(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 2)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;

	if (pleft->type != MD_AST_NODE_TYPE_FIELD_NAME) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	} else if (pleft->pchildren != NULL) {
		fprintf(stderr, "%s: coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	pstatement->pnode_handler = handle_srec_assignment;
	pstatement->srec_lhs_field_name = pleft->text;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pright, type_inferencing, context_flags);
	return pstatement;
}

// ----------------------------------------------------------------
// $ mlr --from ../data/small put -v '$[@x] = 1'
// list (statement_list):
//     = (indirect_srec_assignment):
//         x (oosvar_name).
//         1 (strnum_literal).

static mlr_dsl_cst_statement_t* alloc_indirect_srec_assignment(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 2)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;

	pstatement->pnode_handler = handle_indirect_srec_assignment;
	pstatement->psrec_lhs_evaluator = rval_evaluator_alloc_from_ast(pleft,  type_inferencing, context_flags);
	pstatement->prhs_evaluator      = rval_evaluator_alloc_from_ast(pright, type_inferencing, context_flags);
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_oosvar_assignment(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;

	if (pleft->type != MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	sllv_t* poosvar_lhs_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pleft, type_inferencing,
		context_flags);

	if (pleft->type == MD_AST_NODE_TYPE_OOSVAR_KEYLIST && pright->type == MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {
		pstatement->pnode_handler = handle_oosvar_to_oosvar_assignment;
		pstatement->poosvar_rhs_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pright,
			type_inferencing, context_flags);
	} else {
		pstatement->pnode_handler = handle_oosvar_assignment;
		pstatement->poosvar_rhs_keylist_evaluators = NULL;
	}

	pstatement->poosvar_lhs_keylist_evaluators = poosvar_lhs_keylist_evaluators;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pright, type_inferencing, context_flags);

	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_oosvar_from_full_srec_assignment(
	mlr_dsl_ast_node_t* pnode, int type_inferencing, int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;

	if (pleft->type != MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	if (pright->type != MD_AST_NODE_TYPE_FULL_SREC) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	pstatement->pnode_handler = handle_oosvar_from_full_srec_assignment;
	pstatement->poosvar_lhs_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pleft,
		type_inferencing, context_flags);
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_full_srec_from_oosvar_assignment(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;

	if (pleft->type != MD_AST_NODE_TYPE_FULL_SREC) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	if (pright->type != MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	pstatement->pnode_handler = handle_full_srec_from_oosvar_assignment;
	pstatement->poosvar_rhs_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pright,
		type_inferencing, context_flags);
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_unset(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	pstatement->pnode_handler = handle_unset;
	pstatement->pvarargs = sllv_alloc();
	for (sllve_t* pe = pnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pnode = pe->pvvalue;

		if (pnode->type == MD_AST_NODE_TYPE_ALL || pnode->type == MD_AST_NODE_TYPE_FULL_OOSVAR) {
			// The grammar allows only 'unset all', not 'unset @x, all, $y'.
			// So if 'all' appears at all, it's the only name. Likewise with '@*'.
			pstatement->pnode_handler = handle_unset_all;

		} else if (pnode->type == MD_AST_NODE_TYPE_FULL_SREC) {
			if (context_flags & IN_BEGIN_OR_END) {
				fprintf(stderr, "%s: unset of $-variables is not valid within begin or end blocks.\n",
					MLR_GLOBALS.bargv0);
				exit(1);
			}
			sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
				NULL,
				NULL,
				NULL,
				NULL));

		} else if (pnode->type == MD_AST_NODE_TYPE_FIELD_NAME) {
			if (context_flags & IN_BEGIN_OR_END) {
				fprintf(stderr, "%s: unset of $-variables is not valid within begin or end blocks.\n",
					MLR_GLOBALS.bargv0);
				exit(1);
			}
			sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
				pnode->text,
				NULL,
				NULL,
				NULL));

		} else if (pnode->type == MD_AST_NODE_TYPE_INDIRECT_FIELD_NAME) {
			if (context_flags & IN_BEGIN_OR_END) {
				fprintf(stderr, "%s: unset of $-variables are not valid within begin or end blocks.\n",
					MLR_GLOBALS.bargv0);
				exit(1);
			}
			sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
				NULL,
				rval_evaluator_alloc_from_ast(pnode->pchildren->phead->pvvalue, type_inferencing, context_flags),
				NULL,
				NULL));

		} else if (pnode->type == MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {
			sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
				NULL,
				NULL,
				NULL,
				allocate_keylist_evaluators_from_oosvar_node(pnode, type_inferencing, context_flags)));

		} else {
			fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
				MLR_GLOBALS.bargv0, __FILE__, __LINE__);
			exit(1);
		}
	}
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_tee_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pfilenode = pnode->pchildren->phead->pvvalue;

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilenode,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	pstatement->pnode_handler = handle_tee_write;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_tee_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pfilenode = pnode->pchildren->phead->pvvalue;

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilenode,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	pstatement->pnode_handler = handle_tee_append;
	return pstatement;
}

// ----------------------------------------------------------------
// Example AST:
// $ mlr -n put -v '@a=$x; @b=$y; emitf @a,@b'
// list (statement_list):
//     = (oosvar_assignment):
//         oosvar_keylist (oosvar_keylist):
//             a (string_literal).
//         x (field_name).
//     = (oosvar_assignment):
//         oosvar_keylist (oosvar_keylist):
//             b (string_literal).
//         y (field_name).
//     emitf (emitf):
//         oosvar_keylist (oosvar_keylist):
//             a (string_literal).
//         oosvar_keylist (oosvar_keylist):
//             b (string_literal).

static mlr_dsl_cst_statement_t* alloc_emitf(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	// Loop over oosvar names to emit in e.g. 'emitf @a, @b, @c'.
	pstatement->pvarargs = sllv_alloc();
	for (sllve_t* pe = pnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pwalker = pe->pvvalue;
		mlr_dsl_ast_node_t* pchild = pwalker->pchildren->phead->pvvalue;
		// This could be enforced in the lemon parser but it's easier to do it here.
		if (pwalker->pchildren->length != 1 || pchild->type != MD_AST_NODE_TYPE_STRING_LITERAL) {
			fprintf(stderr,
				"%s: emitf arguments must be all non-indexed, e.g. @a but not @[\"a\"] or @a[2].\n",
				MLR_GLOBALS.bargv0);
			fprintf(stderr, "Abstract syntax tree for the statement:\n");
			mlr_dsl_ast_node_fprint(pnode, stderr);
			exit(1);
		}
		sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
			pchild->text,
			NULL,
			rval_evaluator_alloc_from_ast(pwalker, type_inferencing, context_flags),
			NULL));
	}

	pstatement->pnode_handler = handle_emitf;
	return pstatement;
}

// 'emitf >  "out", @a'
//   text="emitf", type=emitf_write:
//       text="emitf", type=emitf:
//           text="oosvar_keylist", type=oosvar_keylist:
//               text="a", type=string_literal.
//       text="out", type=strnum_literal.

// xxx code-dedupe opportunities here
static mlr_dsl_cst_statement_t* alloc_emitf_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pnamesnode = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilenode = pnode->pchildren->phead->pnext->pvvalue;

	// Loop over oosvar names to emit in e.g. 'emitf @a, @b, @c'.
	pstatement->pvarargs = sllv_alloc();
	for (sllve_t* pe = pnamesnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pwalker = pe->pvvalue;
		mlr_dsl_ast_node_t* pchild = pwalker->pchildren->phead->pvvalue;
		// This could be enforced in the lemon parser but it's easier to do it here.
		sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
			pchild->text,
			NULL,
			rval_evaluator_alloc_from_ast(pwalker, type_inferencing, context_flags),
			NULL));
	}

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilenode,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	pstatement->pnode_handler = handle_emitf_write;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_emitf_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pnamesnode = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilenode = pnode->pchildren->phead->pnext->pvvalue;

	// Loop over oosvar names to emit in e.g. 'emitf @a, @b, @c'.
	pstatement->pvarargs = sllv_alloc();
	for (sllve_t* pe = pnamesnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pwalker = pe->pvvalue;
		mlr_dsl_ast_node_t* pchild = pwalker->pchildren->phead->pvvalue;
		// This could be enforced in the lemon parser but it's easier to do it here.
		sllv_append(pstatement->pvarargs, mlr_dsl_cst_statement_vararg_alloc(
			pchild->text,
			NULL,
			rval_evaluator_alloc_from_ast(pwalker, type_inferencing, context_flags),
			NULL));
	}

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilenode,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	pstatement->pnode_handler = handle_emitf_append;
	return pstatement;
}

// ----------------------------------------------------------------
// $ mlr -n put -v 'emit @a[2][3], "x", "y", "z"'
// AST ROOT:
// text="list", type=statement_list:
//     text="emit", type=emit:
//         text="oosvar_keylist", type=oosvar_keylist:
//             text="a", type=string_literal.
//             text="2", type=strnum_literal.
//             text="3", type=strnum_literal.
//         text="emit_namelist", type=emit:
//             text="x", type=strnum_literal.
//             text="y", type=strnum_literal.
//             text="z", type=strnum_literal.
//
// $ mlr -n put -v 'emit all, "x", "y", "z"'
// AST ROOT:
// text="list", type=statement_list:
//     text="emit", type=emit:
//         text="all", type=all.
//         text="emit_namelist", type=emit:
//             text="x", type=strnum_literal.
//             text="y", type=strnum_literal.
//             text="z", type=strnum_literal.

static mlr_dsl_cst_statement_t* alloc_emit_or_emitp(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags, int do_full_prefixing)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pkeylist_node  = pnode->pchildren->phead->pvvalue;

	// The grammar allows only 'emit all', not 'emit @x, all, $y'.
	// So if 'all' appears at all, it's the only name.
	if (pkeylist_node->type == MD_AST_NODE_TYPE_ALL || pkeylist_node->type == MD_AST_NODE_TYPE_FULL_OOSVAR) {

		sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
		if (pnode->pchildren->length == 2) {
			mlr_dsl_ast_node_t* pnamelist_node = pnode->pchildren->phead->pnext->pvvalue;
			for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
				sllv_append(pemit_oosvar_namelist_evaluators,
					rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
			}
		}
		pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

		pstatement->pnode_handler = do_full_prefixing
			? handle_emit_all
			: handle_emitp_all;

	} else if (pkeylist_node->type == MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {

		pstatement->pemit_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pkeylist_node,
			type_inferencing, context_flags);

		sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
		if (pnode->pchildren->length == 2) {
			mlr_dsl_ast_node_t* pnamelist_node = pnode->pchildren->phead->pnext->pvvalue;
			for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
				sllv_append(pemit_oosvar_namelist_evaluators,
					rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
			}
		}
		pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

		pstatement->pnode_handler = do_full_prefixing
			? handle_emit
			: handle_emitp;

	} else {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	return pstatement;
}

// ================================================================
// mlr --from ../data/small put -v -q emit  >  "out", @a, "x", "y"
// * text="emit", type=emit_write:
//       text="emit", type=emit:
//           text="oosvar_keylist", type=oosvar_keylist:
//               text="a", type=string_literal.
//           text="emit_namelist", type=emit:
//               text="x", type=strnum_literal.
//               text="y", type=strnum_literal.
//       text="out", type=strnum_literal.

static mlr_dsl_cst_statement_t* alloc_emit_or_emitp_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags, int do_full_prefixing)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pemit_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;

	mlr_dsl_ast_node_t* pkeylist_node = pemit_node->pchildren->phead->pvvalue;

	// The grammar allows only 'emit all', not 'emit @x, all, $y'.
	// So if 'all' appears at all, it's the only name.
	if (pkeylist_node->type == MD_AST_NODE_TYPE_ALL || pkeylist_node->type == MD_AST_NODE_TYPE_FULL_OOSVAR) {

		sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
		if (pemit_node->pchildren->length == 2) {
			mlr_dsl_ast_node_t* pnamelist_node = pemit_node->pchildren->phead->pnext->pvvalue;
			for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
				sllv_append(pemit_oosvar_namelist_evaluators,
					rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
			}
		}
		pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

		pstatement->pnode_handler = do_full_prefixing
			? handle_emit_all_write
			: handle_emitp_all_write;

	} else if (pkeylist_node->type == MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {

		pstatement->pemit_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pkeylist_node,
			type_inferencing, context_flags);

		sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
		if (pemit_node->pchildren->length == 2) {
			mlr_dsl_ast_node_t* pnamelist_node = pemit_node->pchildren->phead->pnext->pvvalue;
			for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
				sllv_append(pemit_oosvar_namelist_evaluators,
					rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
			}
		}
		pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

		pstatement->pnode_handler = do_full_prefixing
			? handle_emit_write
			: handle_emitp_write;

	} else {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_emit_or_emitp_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags, int do_full_prefixing)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pemit_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;

	mlr_dsl_ast_node_t* pkeylist_node = pemit_node->pchildren->phead->pvvalue;

	// The grammar allows only 'emit all', not 'emit @x, all, $y'.
	// So if 'all' appears at all, it's the only name.
	if (pkeylist_node->type == MD_AST_NODE_TYPE_ALL || pkeylist_node->type == MD_AST_NODE_TYPE_FULL_OOSVAR) {

		sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
		if (pemit_node->pchildren->length == 2) {
			mlr_dsl_ast_node_t* pnamelist_node = pemit_node->pchildren->phead->pnext->pvvalue;
			for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
				sllv_append(pemit_oosvar_namelist_evaluators,
					rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
			}
		}
		pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

		pstatement->pnode_handler = do_full_prefixing
			? handle_emit_all_append
			: handle_emitp_all_append;

	} else if (pkeylist_node->type == MD_AST_NODE_TYPE_OOSVAR_KEYLIST) {

		pstatement->pemit_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(pkeylist_node,
			type_inferencing, context_flags);

		sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
		if (pemit_node->pchildren->length == 2) {
			mlr_dsl_ast_node_t* pnamelist_node = pemit_node->pchildren->phead->pnext->pvvalue;
			for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
				mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
				sllv_append(pemit_oosvar_namelist_evaluators,
					rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
			}
		}
		pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

		pstatement->pnode_handler = do_full_prefixing
			? handle_emit_append
			: handle_emitp_append;

	} else {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	return pstatement;
}

// ----------------------------------------------------------------
// $ mlr -n put -v 'emit (@a[2][3], @b[4][5]), "x", "y", "z"'
// text="list", type=statement_list:
//     text="emit", type=emit_lashed:
//         text="lashed_keylists", type=emit_lashed:
//             text="oosvar_keylist", type=oosvar_keylist:
//                 text="a", type=string_literal.
//                 text="2", type=strnum_literal.
//                 text="3", type=strnum_literal.
//             text="oosvar_keylist", type=oosvar_keylist:
//                 text="b", type=string_literal.
//                 text="4", type=strnum_literal.
//                 text="5", type=strnum_literal.
//         text="lashed_namelist", type=emit_lashed:
//             text="x", type=strnum_literal.
//             text="y", type=strnum_literal.
//             text="z", type=strnum_literal.

static mlr_dsl_cst_statement_t* alloc_emit_or_emitp_lashed(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags, int do_full_prefixing)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pkeylists_node  = pnode->pchildren->phead->pvvalue;

	pstatement->num_emit_keylist_evaluators = pkeylists_node->pchildren->length;
	pstatement->ppemit_keylist_evaluators = mlr_malloc_or_die(pstatement->num_emit_keylist_evaluators
		* sizeof(sllv_t*));
	int i = 0;
	for (sllve_t* pe = pkeylists_node->pchildren->phead; pe != NULL; pe = pe->pnext, i++) {
		mlr_dsl_ast_node_t* pkeylist_node = pe->pvvalue;
		pstatement->ppemit_keylist_evaluators[i] = allocate_keylist_evaluators_from_oosvar_node(pkeylist_node,
			type_inferencing, context_flags);
	}

	sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
	if (pnode->pchildren->length == 2) {
		mlr_dsl_ast_node_t* pnamelist_node = pnode->pchildren->phead->pnext->pvvalue;
		for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
			mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
			sllv_append(pemit_oosvar_namelist_evaluators,
				rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
		}
	}
	pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

	pstatement->pnode_handler = do_full_prefixing
		? handle_emit_lashed
		: handle_emitp_lashed;

	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_emit_or_emitp_lashed_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags, int do_full_prefixing)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pemit_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;

	mlr_dsl_ast_node_t* pkeylists_node = pemit_node->pchildren->phead->pvvalue;

	pstatement->num_emit_keylist_evaluators = pkeylists_node->pchildren->length;
	pstatement->ppemit_keylist_evaluators = mlr_malloc_or_die(pstatement->num_emit_keylist_evaluators
		* sizeof(sllv_t*));
	int i = 0;
	for (sllve_t* pe = pkeylists_node->pchildren->phead; pe != NULL; pe = pe->pnext, i++) {
		mlr_dsl_ast_node_t* pkeylist_node = pe->pvvalue;
		pstatement->ppemit_keylist_evaluators[i] = allocate_keylist_evaluators_from_oosvar_node(pkeylist_node,
			type_inferencing, context_flags);
	}

	sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
	if (pemit_node->pchildren->length == 2) {
		mlr_dsl_ast_node_t* pnamelist_node = pemit_node->pchildren->phead->pnext->pvvalue;
		for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
			mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
			sllv_append(pemit_oosvar_namelist_evaluators,
				rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
		}
	}
	pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	pstatement->pnode_handler = do_full_prefixing
		? handle_emit_lashed_write
		: handle_emitp_lashed_write;

	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_emit_or_emitp_lashed_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags, int do_full_prefixing)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pemit_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;

	mlr_dsl_ast_node_t* pkeylists_node = pemit_node->pchildren->phead->pvvalue;

	pstatement->num_emit_keylist_evaluators = pkeylists_node->pchildren->length;
	pstatement->ppemit_keylist_evaluators = mlr_malloc_or_die(pstatement->num_emit_keylist_evaluators
		* sizeof(sllv_t*));
	int i = 0;
	for (sllve_t* pe = pkeylists_node->pchildren->phead; pe != NULL; pe = pe->pnext, i++) {
		mlr_dsl_ast_node_t* pkeylist_node = pe->pvvalue;
		pstatement->ppemit_keylist_evaluators[i] = allocate_keylist_evaluators_from_oosvar_node(pkeylist_node,
			type_inferencing, context_flags);
	}

	sllv_t* pemit_oosvar_namelist_evaluators = sllv_alloc();
	if (pemit_node->pchildren->length == 2) {
		mlr_dsl_ast_node_t* pnamelist_node = pemit_node->pchildren->phead->pnext->pvvalue;
		for (sllve_t* pe = pnamelist_node->pchildren->phead; pe != NULL; pe = pe->pnext) {
			mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
			sllv_append(pemit_oosvar_namelist_evaluators,
				rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
		}
	}
	pstatement->pemit_oosvar_namelist_evaluators = pemit_oosvar_namelist_evaluators;

	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_lrec_writer = multi_lrec_writer_alloc();

	pstatement->pnode_handler = do_full_prefixing
		? handle_emit_lashed_append
		: handle_emitp_lashed_append;

	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_while(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	// Left child node is the AST for the boolean expression.
	// Right child node is the list of statements in the body.
	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;
	sllv_t* pblock_statements = sllv_alloc();

	for (sllve_t* pe = pright->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pbody_ast_node = pe->pvvalue;
		mlr_dsl_cst_statement_t *pstatement = alloc_cst_statement(pbody_ast_node, type_inferencing, context_flags);
		sllv_append(pblock_statements, pstatement);
	}

	pstatement->pnode_handler = handle_while;
	pstatement->pblock_handler = handle_statement_list_with_break_continue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pleft, type_inferencing, context_flags);
	pstatement->pblock_statements = pblock_statements;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_do_while(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	// Left child node is the list of statements in the body.
	// Right child node is the AST for the boolean expression.
	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;
	sllv_t* pblock_statements = sllv_alloc();

	for (sllve_t* pe = pleft->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pbody_ast_node = pe->pvvalue;
		mlr_dsl_cst_statement_t *pstatement = alloc_cst_statement(pbody_ast_node, type_inferencing, context_flags);
		sllv_append(pblock_statements, pstatement);
	}

	pstatement->pnode_handler = handle_do_while;
	pstatement->pblock_handler = handle_statement_list_with_break_continue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pright, type_inferencing, context_flags);
	pstatement->pblock_statements = pblock_statements;
	return pstatement;
}

// ----------------------------------------------------------------
// $ mlr -n put -v 'for (k,v in $*) { $x=1; $y=2 }'
// list (statement_list):
//     for (for-srec):
//         variables (for-variables):
//             k (non_sigil_name).
//             v (non_sigil_name).
//         list (statement_list):
//             = (srec_assignment):
//                 x (field_name).
//                 1 (strnum_literal).
//             = (srec_assignment):
//                 y (field_name).
//                 2 (strnum_literal).

static mlr_dsl_cst_statement_t* alloc_for_srec(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	// Left child node is list of bound variables.
	// Right child node is the list of statements in the body.
	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;

	mlr_dsl_ast_node_t* pknode = pleft->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pvnode = pleft->pchildren->phead->pnext->pvvalue;

	if (streq(pknode->text, pvnode->text)) {
		fprintf(stderr, "%s: duplicate for-loop boundvars \"%s\" and \"%s\".\n",
			MLR_GLOBALS.bargv0, pknode->text, pvnode->text);
		exit(1);
	}
	pstatement->for_srec_k_name = pknode->text;
	pstatement->for_v_name = pvnode->text;

	sllv_t* pblock_statements = sllv_alloc();
	for (sllve_t* pe = pright->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pbody_ast_node = pe->pvvalue;
		sllv_append(pblock_statements, alloc_cst_statement(pbody_ast_node, type_inferencing, context_flags));
	}

	pstatement->pnode_handler = handle_for_srec;
	pstatement->pblock_handler = handle_statement_list_with_break_continue;
	pstatement->pblock_statements = pblock_statements;
	pstatement->pbound_variables = lhmsmv_alloc();
	pstatement->ptype_infererenced_srec_field_getter =
		(type_inferencing == TYPE_INFER_STRING_ONLY)      ? get_srec_value_string_only_aux :
		(type_inferencing == TYPE_INFER_STRING_FLOAT)     ? get_srec_value_string_float_aux :
		(type_inferencing == TYPE_INFER_STRING_FLOAT_INT) ? get_srec_value_string_float_int_aux :
		NULL;
	if (pstatement->ptype_infererenced_srec_field_getter == NULL) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}

	return pstatement;
}

// ----------------------------------------------------------------
// $ mlr -n put -v 'for((k1,k2,k3),v in @a["4"][$5]) { $6 = 7; $8 = 9}'
// AST ROOT:
// list (statement_list):
//     for (for_oosvar):
//         key_and_value_variables (for_variables):
//             key_variables (for_variables):
//                 k1 (non_sigil_name).
//                 k2 (non_sigil_name).
//                 k3 (non_sigil_name).
//             v (non_sigil_name).
//         oosvar_keylist (oosvar_keylist):
//             a (string_literal).
//             4 (strnum_literal).
//             5 (field_name).
//         list (statement_list):
//             = (srec_assignment):
//                 6 (field_name).
//                 7 (strnum_literal).
//             = (srec_assignment):
//                 8 (field_name).
//                 9 (strnum_literal).

static mlr_dsl_cst_statement_t* alloc_for_oosvar(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	// Left child node is list of bound variables.
	//   Left subnode is namelist for key boundvars.
	//   Right subnode is name for value boundvar.
	// Middle child node is keylist for basepoint in the oosvar mlhmmv.
	// Right child node is the list of statements in the body.
	mlr_dsl_ast_node_t* pleft     = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* psubleft  = pleft->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* psubright = pleft->pchildren->phead->pnext->pvvalue;
	mlr_dsl_ast_node_t* pmiddle   = pnode->pchildren->phead->pnext->pvvalue;
	mlr_dsl_ast_node_t* pright    = pnode->pchildren->phead->pnext->pnext->pvvalue;

	pstatement->pfor_oosvar_k_names = slls_alloc();
	int ok = TRUE;
	hss_t* pnameset = hss_alloc();
	for (sllve_t* pe = psubleft->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pnamenode = pe->pvvalue;
		slls_append_with_free(pstatement->pfor_oosvar_k_names, mlr_strdup_or_die(pnamenode->text));
		if (hss_has(pnameset, pnamenode->text)) {
			fprintf(stderr, "%s: duplicate for-loop boundvar \"%s\".\n",
				MLR_GLOBALS.bargv0, pnamenode->text);
			ok = FALSE;
		}
		hss_add(pnameset, pnamenode->text);
	}
	pstatement->for_v_name = psubright->text;
	if (hss_has(pnameset, psubright->text)) {
		fprintf(stderr, "%s: duplicate for-loop boundvar \"%s\".\n",
			MLR_GLOBALS.bargv0, psubright->text);
		ok = FALSE;
	}
	hss_add(pnameset, psubright->text);
	if (!ok) {
		fprintf(stderr, "Boundvars: ");
		for (sllve_t* pe = psubleft->pchildren->phead; pe != NULL; pe = pe->pnext) {
			mlr_dsl_ast_node_t* pnamenode = pe->pvvalue;
			fprintf(stderr, "\"%s\", ", pnamenode->text);
		}
		fprintf(stderr, "\"%s\".\n", psubright->text);
		exit(1);
	}

	hss_free(pnameset);

	pstatement->poosvar_lhs_keylist_evaluators = allocate_keylist_evaluators_from_oosvar_node(
		pmiddle, type_inferencing, context_flags);

	sllv_t* pblock_statements = sllv_alloc();
	for (sllve_t* pe = pright->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pbody_ast_node = pe->pvvalue;
		sllv_append(pblock_statements, alloc_cst_statement(pbody_ast_node, type_inferencing, context_flags));
	}
	pstatement->pblock_statements = pblock_statements;
	pstatement->pbound_variables = lhmsmv_alloc();

	pstatement->pnode_handler = handle_for_oosvar;
	pstatement->pblock_handler = handle_statement_list_with_break_continue;

	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_break(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	pstatement->pnode_handler = handle_break;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_continue(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	pstatement->pnode_handler = handle_continue;
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_conditional_block(mlr_dsl_ast_node_t* pnode,
	int type_inferencing, int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	// Left node is the AST for the boolean expression.
	// Right node is a list of statements to be executed if the left evaluates to true.
	mlr_dsl_ast_node_t* pleft  = pnode->pchildren->phead->pvvalue;
	sllv_t* pblock_statements = sllv_alloc();

	mlr_dsl_ast_node_t* pright = pnode->pchildren->phead->pnext->pvvalue;
	for (sllve_t* pe = pright->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pbody_ast_node = pe->pvvalue;
		mlr_dsl_cst_statement_t *pstatement = alloc_cst_statement(pbody_ast_node, type_inferencing, context_flags);
		sllv_append(pblock_statements, pstatement);
	}

	pstatement->pnode_handler = handle_conditional_block;
	pstatement->pblock_handler = (context_flags & IN_BREAKABLE)
		?  handle_statement_list_with_break_continue
		: mlr_dsl_cst_handle_statement_list;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pleft, type_inferencing, context_flags);
	pstatement->pblock_statements = pblock_statements;
	return pstatement;
}

// ----------------------------------------------------------------
// Example parser-input:
//
//   if (NR == 9) {
//       $x = 10;
//       $x = 11
//   } elif (NR == 12) {
//       $x = 13;
//       $x = 14
//   } else {
//       $x = 15;
//       $x = 16
//   };
//
// Corresponding parser-output AST:
//   if_head (if_head):
//       if (if_item):
//           == (operator):
//               NR (context_variable).
//               9 (strnum_literal).
//           list (statement_list):
//               = (srec_assignment):
//                   x (field_name).
//                   10 (strnum_literal).
//               = (srec_assignment):
//                   x (field_name).
//                   11 (strnum_literal).
//       elif (if_item):
//           == (operator):
//               NR (context_variable).
//               12 (strnum_literal).
//           list (statement_list):
//               = (srec_assignment):
//                   x (field_name).
//                   13 (strnum_literal).
//               = (srec_assignment):
//                   x (field_name).
//                   14 (strnum_literal).
//       else (if_item):
//           list (statement_list):
//               = (srec_assignment):
//                   x (field_name).
//                   15 (strnum_literal).
//               = (srec_assignment):
//                   x (field_name).
//                   16 (strnum_literal).

static mlr_dsl_cst_statement_t* alloc_if_head(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	sllv_t* pif_chain_statements = sllv_alloc();
	for (sllve_t* pe = pnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		// For if and elif:
		// * Left subnode is the AST for the boolean expression.
		// * Right subnode is a list of statements to be executed if the left evaluates to true.
		// For else:
		// * Sole subnode is a list of statements to be executed.
		mlr_dsl_ast_node_t* pitemnode = pe->pvvalue;
		mlr_dsl_ast_node_t* pexprnode = NULL;
		mlr_dsl_ast_node_t* plistnode = NULL;
		if (pitemnode->pchildren->length == 2) {
			pexprnode = pitemnode->pchildren->phead->pvvalue;
			plistnode = pitemnode->pchildren->phead->pnext->pvvalue;
		} else {
			pexprnode = NULL;
			plistnode = pitemnode->pchildren->phead->pvvalue;
		}

		sllv_append(pif_chain_statements, alloc_if_item(pexprnode, plistnode,
			type_inferencing, context_flags));
	}

	pstatement->pnode_handler = handle_if_head;
	pstatement->pblock_handler = (context_flags & IN_BREAKABLE)
		?  handle_statement_list_with_break_continue
		: mlr_dsl_cst_handle_statement_list;
	pstatement->pif_chain_statements = pif_chain_statements;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_if_item(mlr_dsl_ast_node_t* pexprnode,
	mlr_dsl_ast_node_t* plistnode, int type_inferencing, int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	sllv_t* pblock_statements = sllv_alloc();

	for (sllve_t* pe = plistnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pbody_ast_node = pe->pvvalue;
		mlr_dsl_cst_statement_t *pstatement = alloc_cst_statement(pbody_ast_node, type_inferencing, context_flags);
		sllv_append(pblock_statements, pstatement);
	}

	pstatement->pnode_handler = NULL; // handled by the containing if-head evaluator
	pstatement->prhs_evaluator = pexprnode != NULL
		? rval_evaluator_alloc_from_ast(pexprnode, type_inferencing, context_flags) // if-statement or elif-statement
		: rval_evaluator_alloc_from_boolean(TRUE); // else-statement
	pstatement->pblock_statements = pblock_statements;
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_filter(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	mlr_dsl_ast_node_t* pchild = pnode->pchildren->phead->pvvalue;

	pstatement->pnode_handler = handle_filter;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pchild, type_inferencing, context_flags);
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_dump(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	pstatement->pnode_handler = handle_dump;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_edump(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	pstatement->pnode_handler = handle_edump;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_dump_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 1)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pvvalue;
	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_out = multi_out_alloc();
	pstatement->pnode_handler = handle_dump_write;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_dump_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 1)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pvvalue;
	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_out = multi_out_alloc();
	pstatement->pnode_handler = handle_dump_append;
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_print(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 1)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->pnode_handler = handle_print;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_eprint(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 1)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->pnode_handler = handle_eprint;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_print_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 2)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_out = multi_out_alloc();
	pstatement->pnode_handler = handle_print_write;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_print_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 2)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_out = multi_out_alloc();
	pstatement->pnode_handler = handle_print_append;
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_printn(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 1)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->pnode_handler = handle_printn;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_eprintn(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 1)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->pnode_handler = handle_eprintn;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_printn_write(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 2)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_out = multi_out_alloc();
	pstatement->pnode_handler = handle_printn_write;
	return pstatement;
}

static mlr_dsl_cst_statement_t* alloc_printn_append(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	if ((pnode->pchildren == NULL) || (pnode->pchildren->length != 2)) {
		fprintf(stderr, "%s: internal coding error detected in file %s at line %d.\n",
			MLR_GLOBALS.bargv0, __FILE__, __LINE__);
		exit(1);
	}
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();
	mlr_dsl_ast_node_t* pvalue_node = pnode->pchildren->phead->pvvalue;
	mlr_dsl_ast_node_t* pfilename_node = pnode->pchildren->phead->pnext->pvvalue;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pvalue_node, type_inferencing, context_flags);
	pstatement->poutput_filename_evaluator = rval_evaluator_alloc_from_ast(pfilename_node,
		type_inferencing, context_flags);
	pstatement->pmulti_out = multi_out_alloc();
	pstatement->pnode_handler = handle_printn_append;
	return pstatement;
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_t* alloc_bare_boolean(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	mlr_dsl_cst_statement_t* pstatement = alloc_blank();

	pstatement->pnode_handler = handle_bare_boolean;
	pstatement->prhs_evaluator = rval_evaluator_alloc_from_ast(pnode, type_inferencing, context_flags);
	return pstatement;
}

// ----------------------------------------------------------------
static void cst_statement_free(mlr_dsl_cst_statement_t* pstatement) {

	if (pstatement->poosvar_lhs_keylist_evaluators != NULL) {
		for (sllve_t* pe = pstatement->poosvar_lhs_keylist_evaluators->phead; pe != NULL; pe = pe->pnext) {
			rval_evaluator_t* phandler = pe->pvvalue;
			phandler->pfree_func(phandler);
		}
		sllv_free(pstatement->poosvar_lhs_keylist_evaluators);
	}

	if (pstatement->pemit_keylist_evaluators != NULL) {
		for (sllve_t* pe = pstatement->pemit_keylist_evaluators->phead; pe != NULL; pe = pe->pnext) {
			rval_evaluator_t* phandler = pe->pvvalue;
			phandler->pfree_func(phandler);
		}
		sllv_free(pstatement->pemit_keylist_evaluators);
	}

	if (pstatement->ppemit_keylist_evaluators != NULL) {
		for (int i = 0; i < pstatement->num_emit_keylist_evaluators; i++) {
			sllv_t* pemit_keylist_evaluators = pstatement->ppemit_keylist_evaluators[i];
			for (sllve_t* pe = pemit_keylist_evaluators->phead; pe != NULL; pe = pe->pnext) {
				rval_evaluator_t* phandler = pe->pvvalue;
				phandler->pfree_func(phandler);
			}
			sllv_free(pemit_keylist_evaluators);
		}
		free(pstatement->ppemit_keylist_evaluators);
	}

	if (pstatement->psrec_lhs_evaluator != NULL) {
		pstatement->psrec_lhs_evaluator->pfree_func(pstatement->psrec_lhs_evaluator);
	}

	if (pstatement->prhs_evaluator != NULL) {
		pstatement->prhs_evaluator->pfree_func(pstatement->prhs_evaluator);
	}

	if (pstatement->poutput_filename_evaluator != NULL) {
		pstatement->poutput_filename_evaluator->pfree_func(pstatement->poutput_filename_evaluator);
	}

	multi_out_free(pstatement->pmulti_out);
	if (pstatement->pmulti_lrec_writer != NULL) {
		multi_lrec_writer_drain(pstatement->pmulti_lrec_writer);
		multi_lrec_writer_free(pstatement->pmulti_lrec_writer);
	}

	if (pstatement->poosvar_rhs_keylist_evaluators != NULL) {
		for (sllve_t* pe = pstatement->poosvar_rhs_keylist_evaluators->phead; pe != NULL; pe = pe->pnext) {
			rval_evaluator_t* phandler = pe->pvvalue;
			phandler->pfree_func(phandler);
		}
		sllv_free(pstatement->poosvar_rhs_keylist_evaluators);
	}

	if (pstatement->pemit_oosvar_namelist_evaluators != NULL) {
		for (sllve_t* pe = pstatement->pemit_oosvar_namelist_evaluators->phead; pe != NULL; pe = pe->pnext) {
			rval_evaluator_t* phandler = pe->pvvalue;
			phandler->pfree_func(phandler);
		}
		sllv_free(pstatement->pemit_oosvar_namelist_evaluators);
	}

	if (pstatement->pvarargs != NULL) {
		for (sllve_t* pe = pstatement->pvarargs->phead; pe != NULL; pe = pe->pnext)
			cst_statement_vararg_free(pe->pvvalue);
		sllv_free(pstatement->pvarargs);
	}

	if (pstatement->pblock_statements != NULL) {
		for (sllve_t* pe = pstatement->pblock_statements->phead; pe != NULL; pe = pe->pnext)
			cst_statement_free(pe->pvvalue);
		sllv_free(pstatement->pblock_statements);
	}

	if (pstatement->pif_chain_statements != NULL) {
		for (sllve_t* pe = pstatement->pif_chain_statements->phead; pe != NULL; pe = pe->pnext)
			cst_statement_free(pe->pvvalue);
		sllv_free(pstatement->pif_chain_statements);
	}

	if (pstatement->pfor_oosvar_k_names != NULL) {
		slls_free(pstatement->pfor_oosvar_k_names);
	}

	if (pstatement->pbound_variables != NULL) {
		lhmsmv_free(pstatement->pbound_variables);
	}

	free(pstatement);
}

// ----------------------------------------------------------------
static mlr_dsl_cst_statement_vararg_t* mlr_dsl_cst_statement_vararg_alloc(
	char*             emitf_or_unset_srec_field_name,
	rval_evaluator_t* punset_srec_field_name_evaluator,
	rval_evaluator_t* pemitf_arg_evaluator,
	sllv_t*           punset_oosvar_keylist_evaluators)
{
	mlr_dsl_cst_statement_vararg_t* pvararg = mlr_malloc_or_die(sizeof(mlr_dsl_cst_statement_vararg_t));
	pvararg->punset_handler = NULL;
	pvararg->emitf_or_unset_srec_field_name = emitf_or_unset_srec_field_name == NULL
		? NULL : mlr_strdup_or_die(emitf_or_unset_srec_field_name);
	pvararg->punset_oosvar_keylist_evaluators = punset_oosvar_keylist_evaluators;
	pvararg->punset_srec_field_name_evaluator = punset_srec_field_name_evaluator;
	pvararg->pemitf_arg_evaluator             = pemitf_arg_evaluator;

	if (pvararg->punset_oosvar_keylist_evaluators != NULL) {
		pvararg->punset_handler = handle_unset_vararg_oosvar;
	} else if (pvararg->punset_srec_field_name_evaluator != NULL) {
		pvararg->punset_handler = handle_unset_vararg_indirect_srec_field_name;
	} else if (pvararg->emitf_or_unset_srec_field_name != NULL) {
		pvararg->punset_handler = handle_unset_vararg_srec_field_name;
	} else {
		pvararg->punset_handler = handle_unset_vararg_full_srec;
	}

	return pvararg;
}

static void cst_statement_vararg_free(mlr_dsl_cst_statement_vararg_t* pvararg) {
	if (pvararg == NULL)
		return;
	free(pvararg->emitf_or_unset_srec_field_name);

	if (pvararg->punset_srec_field_name_evaluator != NULL) {
		pvararg->punset_srec_field_name_evaluator->pfree_func(pvararg->punset_srec_field_name_evaluator);
	}

	if (pvararg->punset_oosvar_keylist_evaluators != NULL) {
		for (sllve_t* pe = pvararg->punset_oosvar_keylist_evaluators->phead; pe != NULL; pe = pe->pnext) {
			rval_evaluator_t* phandler = pe->pvvalue;
			phandler->pfree_func(phandler);
		}
		sllv_free(pvararg->punset_oosvar_keylist_evaluators);
	}

	if (pvararg->pemitf_arg_evaluator != NULL)
		pvararg->pemitf_arg_evaluator->pfree_func(pvararg->pemitf_arg_evaluator);

	free(pvararg);
}

// ================================================================
// This is for statement lists not recursively contained within a loop body -- including the
// main/begin/end statements.  Since there is no containing loop body, there is no need to check
// for break or continue flags after each statement.
void mlr_dsl_cst_handle_statement_list(
	sllv_t*        pcst_statements,
	variables_t*   pvars,
	cst_outputs_t* pcst_outputs)
{
	for (sllve_t* pe = pcst_statements->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_cst_statement_t* pstatement = pe->pvvalue;
		pstatement->pnode_handler(pstatement, pvars, pcst_outputs);
	}
}

// This is for statement lists recursively contained within a loop body.
// It checks for break or continue flags after each statement.
static void handle_statement_list_with_break_continue(
	sllv_t*        pcst_statements,
	variables_t*   pvars,
	cst_outputs_t* pcst_outputs)
{
	for (sllve_t* pe = pcst_statements->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_cst_statement_t* pstatement = pe->pvvalue;
		pstatement->pnode_handler(pstatement, pvars, pcst_outputs);
		if (loop_stack_get(pvars->ploop_stack) != 0) {
			break;
		}
	}
}

// ----------------------------------------------------------------
static void handle_srec_assignment(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	char* srec_lhs_field_name = pnode->srec_lhs_field_name;
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);

	// Write typed mlrval output to the typed overlay rather than into the lrec (which holds only
	// string values).
	//
	// The rval_evaluator reads the overlay in preference to the lrec. E.g. if the input had
	// "x"=>"abc","y"=>"def" but the previous pass through this loop set "y"=>7.4 and "z"=>"ghi" then an
	// expression right-hand side referring to $y would get the floating-point value 7.4. So we don't need
	// to do lrec_put here, and moreover should not for two reasons: (1) there is a performance hit of doing
	// throwaway number-to-string formatting -- it's better to do it once at the end; (2) having the string
	// values doubly owned by the typed overlay and the lrec would result in double frees, or awkward
	// bookkeeping. However, the NR variable evaluator reads prec->field_count, so we need to put something
	// here. And putting something statically allocated minimizes copying/freeing.
	if (mv_is_present(&val)) {
		lhmsmv_put(pvars->ptyped_overlay, srec_lhs_field_name, &val, FREE_ENTRY_VALUE);
		lrec_put(pvars->pinrec, srec_lhs_field_name, "bug", NO_FREE);
	} else {
		mv_free(&val);
	}
}

// ----------------------------------------------------------------
static void handle_indirect_srec_assignment(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* plhs_evaluator = pnode->psrec_lhs_evaluator;
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;

	mv_t lval = plhs_evaluator->pprocess_func(plhs_evaluator->pvstate, pvars);
	char free_flags;
	char* srec_lhs_field_name = mv_format_val(&lval, &free_flags);

	mv_t rval = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);

	// Write typed mlrval output to the typed overlay rather than into the lrec (which holds only
	// string values).
	//
	// The rval_evaluator reads the overlay in preference to the lrec. E.g. if the input had
	// "x"=>"abc","y"=>"def" but the previous pass through this loop set "y"=>7.4 and "z"=>"ghi" then an
	// expression right-hand side referring to $y would get the floating-point value 7.4. So we don't need
	// to do lrec_put here, and moreover should not for two reasons: (1) there is a performance hit of doing
	// throwaway number-to-string formatting -- it's better to do it once at the end; (2) having the string
	// values doubly owned by the typed overlay and the lrec would result in double frees, or awkward
	// bookkeeping. However, the NR variable evaluator reads prec->field_count, so we need to put something
	// here. And putting something statically allocated minimizes copying/freeing.
	if (mv_is_present(&rval)) {
		lhmsmv_put(pvars->ptyped_overlay, mlr_strdup_or_die(srec_lhs_field_name), &rval,
			FREE_ENTRY_KEY|FREE_ENTRY_VALUE);
		lrec_put(pvars->pinrec, mlr_strdup_or_die(srec_lhs_field_name), "bug", FREE_ENTRY_KEY | FREE_ENTRY_KEY);
	} else {
		mv_free(&rval);
	}

	if (free_flags)
		free(srec_lhs_field_name);
}

// ----------------------------------------------------------------
static void handle_oosvar_assignment(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t rhs_value = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);

	if (mv_is_present(&rhs_value)) {
		int all_non_null_or_error = TRUE;
		sllmv_t* pmvkeys = evaluate_list(pnode->poosvar_lhs_keylist_evaluators, pvars, &all_non_null_or_error);
		if (all_non_null_or_error)
			mlhmmv_put_terminal(pvars->poosvars, pmvkeys, &rhs_value);
		sllmv_free(pmvkeys);
	}
	mv_free(&rhs_value);
}

// ----------------------------------------------------------------
// All assignments produce a mlrval on the RHS and store it on the left -- except if both LHS and RHS
// are oosvars in which case there are recursive copies, or in case of $* on the LHS or RHS.

static void handle_oosvar_to_oosvar_assignment(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int lhs_all_non_null_or_error = TRUE;
	sllmv_t* plhskeys = evaluate_list(pnode->poosvar_lhs_keylist_evaluators, pvars, &lhs_all_non_null_or_error);

	if (lhs_all_non_null_or_error) {
		int rhs_all_non_null_or_error = TRUE;
		sllmv_t* prhskeys = evaluate_list(pnode->poosvar_rhs_keylist_evaluators, pvars, &rhs_all_non_null_or_error);
		if (rhs_all_non_null_or_error) {
			mlhmmv_copy(pvars->poosvars, plhskeys, prhskeys);
		}
		sllmv_free(prhskeys);
	}

	sllmv_free(plhskeys);
}

static void handle_oosvar_from_full_srec_assignment(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int all_non_null_or_error = TRUE;
	sllmv_t* plhskeys = evaluate_list(pnode->poosvar_lhs_keylist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {

		mlhmmv_level_t* plevel = mlhmmv_get_or_create_level(pvars->poosvars, plhskeys);
		if (plevel != NULL) {

			mlhmmv_clear_level(plevel);

			for (lrece_t* pe = pvars->pinrec->phead; pe != NULL; pe = pe->pnext) {
				mv_t k = mv_from_string(pe->key, NO_FREE); // mlhmmv_put_terminal_from_level will copy
				sllmve_t e = { .value = k, .free_flags = 0, .pnext = NULL };
				mv_t* pomv = lhmsmv_get(pvars->ptyped_overlay, pe->key);
				if (pomv != NULL) {
					mlhmmv_put_terminal_from_level(plevel, &e, pomv);
				} else {
					mv_t v = mv_from_string(pe->value, NO_FREE); // mlhmmv_put_terminal_from_level will copy
					mlhmmv_put_terminal_from_level(plevel, &e, &v);
				}
			}

		}
	}
	sllmv_free(plhskeys);
}

static void handle_full_srec_from_oosvar_assignment(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	lrec_clear(pvars->pinrec);
	lhmsmv_clear(pvars->ptyped_overlay);

	int all_non_null_or_error = TRUE;
	sllmv_t* prhskeys = evaluate_list(pnode->poosvar_rhs_keylist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		int error = 0;
		mlhmmv_level_t* plevel = mlhmmv_get_level(pvars->poosvars, prhskeys, &error);
		if (plevel != NULL) {
			for (mlhmmv_level_entry_t* pentry = plevel->phead; pentry != NULL; pentry = pentry->pnext) {
				if (pentry->level_value.is_terminal) {
					char* skey = mv_alloc_format_val(&pentry->level_key);
					mv_t val = mv_copy(&pentry->level_value.u.mlrval);

					// Write typed mlrval output to the typed overlay rather than into the lrec
					// (which holds only string values).
					//
					// The rval_evaluator reads the overlay in preference to the lrec. E.g. if the
					// input had "x"=>"abc","y"=>"def" but a previous statement had set "y"=>7.4 and
					// "z"=>"ghi", then an expression right-hand side referring to $y would get the
					// floating-point value 7.4. So we don't need to lrec_put the value here, and
					// moreover should not for two reasons: (1) there is a performance hit of doing
					// throwaway number-to-string formatting -- it's better to do it once at the
					// end; (2) having the string values doubly owned by the typed overlay and the
					// lrec would result in double frees, or awkward bookkeeping. However, the NR
					// variable evaluator reads prec->field_count, so we need to put something here.
					// And putting something statically allocated minimizes copying/freeing.
					lhmsmv_put(pvars->ptyped_overlay, mlr_strdup_or_die(skey), &val,
						FREE_ENTRY_KEY | FREE_ENTRY_VALUE);
					lrec_put(pvars->pinrec, skey, "bug", FREE_ENTRY_KEY);
				}
			}
		}
	}
	sllmv_free(prhskeys);
}

// ----------------------------------------------------------------
static void handle_unset(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	for (sllve_t* pf = pnode->pvarargs->phead; pf != NULL; pf = pf->pnext) {
		mlr_dsl_cst_statement_vararg_t* pvararg = pf->pvvalue;
		pvararg->punset_handler(pvararg, pvars, pcst_outputs);
	}
}

static void handle_unset_vararg_oosvar(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs)
{
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pvararg->punset_oosvar_keylist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error)
		mlhmmv_remove(pvars->poosvars, pmvkeys);
	sllmv_free(pmvkeys);
}

static void handle_unset_vararg_full_srec(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs)
{
	lrec_clear(pvars->pinrec);
}

static void handle_unset_vararg_srec_field_name(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs)
{
	lrec_remove(pvars->pinrec, pvararg->emitf_or_unset_srec_field_name);
}

static void handle_unset_vararg_indirect_srec_field_name(
	mlr_dsl_cst_statement_vararg_t* pvararg,
	variables_t*                    pvars,
	cst_outputs_t*                  pcst_outputs)
{
	rval_evaluator_t* pevaluator = pvararg->punset_srec_field_name_evaluator;
	mv_t nameval = pevaluator->pprocess_func(pevaluator->pvstate, pvars);
	char free_flags = NO_FREE;
	char* field_name = mv_maybe_alloc_format_val(&nameval, &free_flags);
	lrec_remove(pvars->pinrec, field_name);
	if (free_flags & FREE_ENTRY_VALUE)
		free(field_name);
	mv_free(&nameval);
}

// ----------------------------------------------------------------
static void handle_unset_all(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllmv_t* pempty = sllmv_alloc();
	mlhmmv_remove(pvars->poosvars, pempty);
	sllmv_free(pempty);
}

// ----------------------------------------------------------------
static void handle_tee_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);

	// xxx to-string ...
	// xxx flush param ...
	// xxx put in the overlay ...
	lrec_t* pcopy = lrec_copy(pvars->pinrec);

	// xxx make a list-free API w/ refactor
	sllv_t* poutrecs = sllv_single(pcopy);
	// the writer frees the lrec
	// xxx do pop-off drain in lrec-writers
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);

	sllv_free(poutrecs);
	mv_free(&filename);
}

static void handle_tee_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);

	// xxx to-string ...
	// xxx flush param ...
	// xxx put in the overlay ...
	lrec_t* pcopy = lrec_copy(pvars->pinrec);

	// xxx make a list-free API w/ refactor
	sllv_t* poutrecs = sllv_single(pcopy);
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	// the writer frees the lrec
	// xxx do pop-off drain in lrec-writers

	sllv_free(poutrecs);
	mv_free(&filename);
}

// ----------------------------------------------------------------
static void handle_emitf(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	lrec_t* prec_to_emit = lrec_unbacked_alloc();
	for (sllve_t* pf = pnode->pvarargs->phead; pf != NULL; pf = pf->pnext) {
		mlr_dsl_cst_statement_vararg_t* pvararg = pf->pvvalue;
		char* emitf_or_unset_srec_field_name = pvararg->emitf_or_unset_srec_field_name;
		rval_evaluator_t* pemitf_arg_evaluator = pvararg->pemitf_arg_evaluator;

		// This is overkill ... the grammar allows only for oosvar names as args to emit.  So we could bypass
		// that and just hashmap-get keyed by emitf_or_unset_srec_field_name here.
		mv_t val = pemitf_arg_evaluator->pprocess_func(pemitf_arg_evaluator->pvstate, pvars);

		if (val.type == MT_STRING) {
			// Ownership transfer from (newly created) mlrval to (newly created) lrec.
			lrec_put(prec_to_emit, emitf_or_unset_srec_field_name, val.u.strv, val.free_flags);
		} else {
			char free_flags = NO_FREE;
			char* string = mv_format_val(&val, &free_flags);
			lrec_put(prec_to_emit, emitf_or_unset_srec_field_name, string, free_flags);
		}

	}
	sllv_append(pcst_outputs->poutrecs, prec_to_emit);
}

static void handle_emitf_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);

	lrec_t* prec_to_emit = lrec_unbacked_alloc();
	sllv_t* poutrecs = sllv_alloc();
	for (sllve_t* pf = pnode->pvarargs->phead; pf != NULL; pf = pf->pnext) {
		mlr_dsl_cst_statement_vararg_t* pvararg = pf->pvvalue;
		char* emitf_or_unset_srec_field_name = pvararg->emitf_or_unset_srec_field_name;
		rval_evaluator_t* pemitf_arg_evaluator = pvararg->pemitf_arg_evaluator;

		// This is overkill ... the grammar allows only for oosvar names as args to emit.  So we could bypass
		// that and just hashmap-get keyed by emitf_or_unset_srec_field_name here.
		mv_t val = pemitf_arg_evaluator->pprocess_func(pemitf_arg_evaluator->pvstate, pvars);

		if (val.type == MT_STRING) {
			// Ownership transfer from (newly created) mlrval to (newly created) lrec.
			lrec_put(prec_to_emit, emitf_or_unset_srec_field_name, val.u.strv, val.free_flags);
		} else {
			char free_flags = NO_FREE;
			char* string = mv_format_val(&val, &free_flags);
			lrec_put(prec_to_emit, emitf_or_unset_srec_field_name, string, free_flags);
		}

	}
	sllv_append(poutrecs, prec_to_emit);

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);

	sllv_free(poutrecs);
	mv_free(&filename);
}

static void handle_emitf_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);

	lrec_t* prec_to_emit = lrec_unbacked_alloc();
	sllv_t* poutrecs = sllv_alloc();
	for (sllve_t* pf = pnode->pvarargs->phead; pf != NULL; pf = pf->pnext) {
		mlr_dsl_cst_statement_vararg_t* pvararg = pf->pvvalue;
		char* emitf_or_unset_srec_field_name = pvararg->emitf_or_unset_srec_field_name;
		rval_evaluator_t* pemitf_arg_evaluator = pvararg->pemitf_arg_evaluator;

		// This is overkill ... the grammar allows only for oosvar names as args to emit.  So we could bypass
		// that and just hashmap-get keyed by emitf_or_unset_srec_field_name here.
		mv_t val = pemitf_arg_evaluator->pprocess_func(pemitf_arg_evaluator->pvstate, pvars);

		if (val.type == MT_STRING) {
			// Ownership transfer from (newly created) mlrval to (newly created) lrec.
			lrec_put(prec_to_emit, emitf_or_unset_srec_field_name, val.u.strv, val.free_flags);
		} else {
			char free_flags = NO_FREE;
			char* string = mv_format_val(&val, &free_flags);
			lrec_put(prec_to_emit, emitf_or_unset_srec_field_name, string, free_flags);
		}

	}
	sllv_append(poutrecs, prec_to_emit);

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);

	sllv_free(poutrecs);
	mv_free(&filename);
}

// ----------------------------------------------------------------
static void handle_emitp(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int keys_all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pnode->pemit_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs(pvars->poosvars, pmvkeys, pmvnames, pcst_outputs->poutrecs,
				FALSE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}
	sllmv_free(pmvkeys);
}

static void handle_emitp_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int keys_all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pnode->pemit_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	sllv_t* poutrecs = sllv_alloc();
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs(pvars->poosvars, pmvkeys, pmvnames, poutrecs,
				FALSE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);

	sllmv_free(pmvkeys);
	mv_free(&filename);
}

static void handle_emitp_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int keys_all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pnode->pemit_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	sllv_t* poutrecs = sllv_alloc();
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs(pvars->poosvars, pmvkeys, pmvnames, poutrecs,
				FALSE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);

	sllmv_free(pmvkeys);
	mv_free(&filename);
}

// ----------------------------------------------------------------
static void handle_emit(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int keys_all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pnode->pemit_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs(pvars->poosvars, pmvkeys, pmvnames, pcst_outputs->poutrecs,
				TRUE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}
	sllmv_free(pmvkeys);
}

static void handle_emit_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	int keys_all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pnode->pemit_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs(pvars->poosvars, pmvkeys, pmvnames, poutrecs,
				TRUE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}
	sllmv_free(pmvkeys);
}

static void handle_emit_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	int keys_all_non_null_or_error = TRUE;
	sllmv_t* pmvkeys = evaluate_list(pnode->pemit_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs(pvars->poosvars, pmvkeys, pmvnames, poutrecs,
				TRUE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}
	sllmv_free(pmvkeys);
}

// xxx if to file:
// * mlhmmv_to_lrecs not to pcsv_outputs->poutrecs but to local list
// * hand those off to multi_lrec_writer keyed by filename.
// * multi_lrec_writer needs to have open fp (from write/append) as well as lrec_writer object.
// * multi_lrec_writer needs drain-all on its close method, invokved from mapper_put.

// ----------------------------------------------------------------
static void handle_emitp_lashed(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int keys_all_non_null_or_error = TRUE;
	sllmv_t** ppmvkeys = evaluate_lists(pnode->ppemit_keylist_evaluators, pnode->num_emit_keylist_evaluators,
		pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs_lashed(pvars->poosvars, ppmvkeys, pnode->num_emit_keylist_evaluators, pmvnames,
				pcst_outputs->poutrecs, FALSE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}
	for (int i = 0; i < pnode->num_emit_keylist_evaluators; i++) {
		sllmv_free(ppmvkeys[i]);
	}
	free(ppmvkeys);
}

static void handle_emitp_lashed_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int keys_all_non_null_or_error = TRUE;
	sllmv_t** ppmvkeys = evaluate_lists(pnode->ppemit_keylist_evaluators, pnode->num_emit_keylist_evaluators,
		pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs_lashed(pvars->poosvars, ppmvkeys, pnode->num_emit_keylist_evaluators, pmvnames,
				poutrecs, FALSE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	for (int i = 0; i < pnode->num_emit_keylist_evaluators; i++) {
		sllmv_free(ppmvkeys[i]);
	}
	free(ppmvkeys);
}

static void handle_emitp_lashed_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int keys_all_non_null_or_error = TRUE;
	sllmv_t** ppmvkeys = evaluate_lists(pnode->ppemit_keylist_evaluators, pnode->num_emit_keylist_evaluators,
		pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs_lashed(pvars->poosvars, ppmvkeys, pnode->num_emit_keylist_evaluators, pmvnames,
				poutrecs, FALSE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	for (int i = 0; i < pnode->num_emit_keylist_evaluators; i++) {
		sllmv_free(ppmvkeys[i]);
	}
	free(ppmvkeys);
}

// ----------------------------------------------------------------
static void handle_emit_lashed(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int keys_all_non_null_or_error = TRUE;
	sllmv_t** ppmvkeys = evaluate_lists(pnode->ppemit_keylist_evaluators, pnode->num_emit_keylist_evaluators,
		pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs_lashed(pvars->poosvars, ppmvkeys, pnode->num_emit_keylist_evaluators, pmvnames,
				pcst_outputs->poutrecs, TRUE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}
	for (int i = 0; i < pnode->num_emit_keylist_evaluators; i++) {
		sllmv_free(ppmvkeys[i]);
	}
	free(ppmvkeys);
}

static void handle_emit_lashed_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int keys_all_non_null_or_error = TRUE;
	sllmv_t** ppmvkeys = evaluate_lists(pnode->ppemit_keylist_evaluators, pnode->num_emit_keylist_evaluators,
		pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs_lashed(pvars->poosvars, ppmvkeys, pnode->num_emit_keylist_evaluators, pmvnames,
				poutrecs, TRUE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	for (int i = 0; i < pnode->num_emit_keylist_evaluators; i++) {
		sllmv_free(ppmvkeys[i]);
	}
	free(ppmvkeys);
}

static void handle_emit_lashed_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int keys_all_non_null_or_error = TRUE;
	sllmv_t** ppmvkeys = evaluate_lists(pnode->ppemit_keylist_evaluators, pnode->num_emit_keylist_evaluators,
		pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {
		int names_all_non_null_or_error = TRUE;
		sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &names_all_non_null_or_error);
		if (names_all_non_null_or_error) {
			mlhmmv_to_lrecs_lashed(pvars->poosvars, ppmvkeys, pnode->num_emit_keylist_evaluators, pmvnames,
				poutrecs, TRUE, pcst_outputs->oosvar_flatten_separator);
		}
		sllmv_free(pmvnames);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	for (int i = 0; i < pnode->num_emit_keylist_evaluators; i++) {
		sllmv_free(ppmvkeys[i]);
	}
	free(ppmvkeys);
}

// ----------------------------------------------------------------
static void handle_emitp_all(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		mlhmmv_all_to_lrecs(pvars->poosvars, pmvnames, pcst_outputs->poutrecs,
			FALSE, pcst_outputs->oosvar_flatten_separator);
	}
	sllmv_free(pmvnames);
}

static void handle_emitp_all_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		mlhmmv_all_to_lrecs(pvars->poosvars, pmvnames, poutrecs,
			FALSE, pcst_outputs->oosvar_flatten_separator);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	sllmv_free(pmvnames);
}

static void handle_emitp_all_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		mlhmmv_all_to_lrecs(pvars->poosvars, pmvnames, poutrecs,
			FALSE, pcst_outputs->oosvar_flatten_separator);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	sllmv_free(pmvnames);
}

// ----------------------------------------------------------------
static void handle_emit_all(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		mlhmmv_all_to_lrecs(pvars->poosvars, pmvnames, pcst_outputs->poutrecs,
			TRUE, pcst_outputs->oosvar_flatten_separator);
	}
	sllmv_free(pmvnames);
}

static void handle_emit_all_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		mlhmmv_all_to_lrecs(pvars->poosvars, pmvnames, poutrecs,
			TRUE, pcst_outputs->oosvar_flatten_separator);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_write(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	sllmv_free(pmvnames);
}

static void handle_emit_all_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	sllv_t* poutrecs = sllv_alloc();
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	int all_non_null_or_error = TRUE;
	sllmv_t* pmvnames = evaluate_list(pnode->pemit_oosvar_namelist_evaluators, pvars, &all_non_null_or_error);
	if (all_non_null_or_error) {
		mlhmmv_all_to_lrecs(pvars->poosvars, pmvnames, poutrecs,
			TRUE, pcst_outputs->oosvar_flatten_separator);
	}

	// xxx to-string ...
	// xxx flush param ...
	multi_lrec_writer_append(pnode->pmulti_lrec_writer, poutrecs, filename.u.strv, TRUE/*flush_every_record*/);
	sllv_free(poutrecs);
	mv_free(&filename);

	sllmv_free(pmvnames);
}

// ----------------------------------------------------------------
static void handle_dump(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	mlhmmv_print_json_stacked(pvars->poosvars, FALSE, stdout);
}

static void handle_edump(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	mlhmmv_print_json_stacked(pvars->poosvars, FALSE, stderr);
}

static void handle_dump_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	char ffree_flags;
	char* fval = mv_format_val(&filename, &ffree_flags);

	FILE* outfp = multi_out_get_for_write(pnode->pmulti_out, fval);
	mlhmmv_print_json_stacked(pvars->poosvars, FALSE, outfp);
	if (TRUE) // xxx temp
		fflush(outfp);

	if (ffree_flags)
		free(fval);
	mv_free(&filename);
}

static void handle_dump_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	char ffree_flags;
	char* fval = mv_format_val(&filename, &ffree_flags);

	FILE* outfp = multi_out_get_for_append(pnode->pmulti_out, fval);
	mlhmmv_print_json_stacked(pvars->poosvars, FALSE, outfp);
	if (TRUE) // xxx temp
		fflush(outfp);

	if (ffree_flags)
		free(fval);
	mv_free(&filename);
}

// ----------------------------------------------------------------
static void handle_print(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	char free_flags;
	char* sval = mv_format_val(&val, &free_flags);
	printf("%s\n", sval);
	if (free_flags)
		free(sval);
	mv_free(&val);
}

static void handle_eprint(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	char free_flags;
	char* sval = mv_format_val(&val, &free_flags);
	fprintf(stderr, "%s\n", sval);
	if (free_flags)
		free(sval);
	mv_free(&val);
}

static void handle_print_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	char sfree_flags;
	char* sval = mv_format_val(&val, &sfree_flags);
	char ffree_flags;
	char* fval = mv_format_val(&filename, &ffree_flags);

	FILE* outfp = multi_out_get_for_write(pnode->pmulti_out, fval);
	fprintf(outfp, "%s\n", sval);
	if (TRUE) // xxx temp
		fflush(outfp);

	if (sfree_flags)
		free(sval);
	if (ffree_flags)
		free(fval);
	mv_free(&filename);
	mv_free(&val);
}

static void handle_print_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	char sfree_flags;
	char* sval = mv_format_val(&val, &sfree_flags);
	char ffree_flags;
	char* fval = mv_format_val(&filename, &ffree_flags);

	FILE* outfp = multi_out_get_for_append(pnode->pmulti_out, fval);
	fprintf(outfp, "%s\n", sval);
	if (TRUE) // xxx temp
		fflush(outfp);

	if (sfree_flags)
		free(sval);
	if (ffree_flags)
		free(fval);
	mv_free(&filename);
	mv_free(&val);
}

// ----------------------------------------------------------------
static void handle_printn(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	char free_flags;
	char* sval = mv_format_val(&val, &free_flags);
	printf("%s", sval);
	if (free_flags)
		free(sval);
	mv_free(&val);
}

static void handle_eprintn(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	char free_flags;
	char* sval = mv_format_val(&val, &free_flags);
	fprintf(stderr, "%s", sval);
	if (free_flags)
		free(sval);
	mv_free(&val);
}

static void handle_printn_write(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);

	char free_flags;
	char* sval = mv_format_val(&val, &free_flags);
	char ffree_flags;
	char* fval = mv_format_val(&filename, &ffree_flags);

	FILE* outfp = multi_out_get_for_write(pnode->pmulti_out, fval);
	fprintf(outfp, "%s\n", sval);
	if (TRUE) // xxx temp
		fflush(outfp);

	if (free_flags)
		free(sval);
	if (ffree_flags)
		free(fval);
	mv_free(&filename);
	mv_free(&val);
}

static void handle_printn_append(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;
	rval_evaluator_t* poutput_filename_evaluator = pnode->poutput_filename_evaluator;
	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	mv_t filename = poutput_filename_evaluator->pprocess_func(poutput_filename_evaluator->pvstate, pvars);
	char free_flags;
	char* sval = mv_format_val(&val, &free_flags);
	char ffree_flags;
	char* fval = mv_format_val(&filename, &ffree_flags);

	FILE* outfp = multi_out_get_for_append(pnode->pmulti_out, fval);
	fprintf(outfp, "%s\n", sval);
	if (TRUE) // xxx temp
		fflush(outfp);

	if (free_flags)
		free(sval);
	if (ffree_flags)
		free(fval);
	mv_free(&filename);
	mv_free(&val);
}

// ----------------------------------------------------------------
static void handle_filter(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;

	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	if (mv_is_non_null(&val)) {
		mv_set_boolean_strict(&val);
		*pcst_outputs->pshould_emit_rec = val.u.boolv;
	} else {
		*pcst_outputs->pshould_emit_rec = FALSE;
	}
}

// ----------------------------------------------------------------
static void handle_conditional_block(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;

	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	if (mv_is_non_null(&val)) {
		mv_set_boolean_strict(&val);
		if (val.u.boolv) {
			pnode->pblock_handler(pnode->pblock_statements, pvars, pcst_outputs);
		}
	}
}

// ----------------------------------------------------------------
static void handle_if_head(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	for (sllve_t* pe = pnode->pif_chain_statements->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_cst_statement_t* pitemnode = pe->pvvalue;
		rval_evaluator_t* prhs_evaluator = pitemnode->prhs_evaluator;

		mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
		if (mv_is_non_null(&val)) {
			mv_set_boolean_strict(&val);
			if (val.u.boolv) {
				pnode->pblock_handler(pitemnode->pblock_statements, pvars, pcst_outputs);
				break;
			}
		}
	}
}

// ----------------------------------------------------------------
static void handle_while(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;

	loop_stack_push(pvars->ploop_stack);
	while (TRUE) {
		mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
		if (mv_is_non_null(&val)) {
			mv_set_boolean_strict(&val);
			if (val.u.boolv) {
				pnode->pblock_handler(pnode->pblock_statements, pvars, pcst_outputs);
				if (loop_stack_get(pvars->ploop_stack) & LOOP_BROKEN) {
					loop_stack_clear(pvars->ploop_stack, LOOP_BROKEN);
					break;
				} else if (loop_stack_get(pvars->ploop_stack) & LOOP_CONTINUED) {
					loop_stack_clear(pvars->ploop_stack, LOOP_CONTINUED);
				}
			} else {
				break;
			}
		} else {
			break;
		}
	}
	loop_stack_pop(pvars->ploop_stack);
}

// ----------------------------------------------------------------
static void handle_do_while(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;

	loop_stack_push(pvars->ploop_stack);
	while (TRUE) {
		pnode->pblock_handler(pnode->pblock_statements, pvars, pcst_outputs);
		if (loop_stack_get(pvars->ploop_stack) & LOOP_BROKEN) {
			loop_stack_clear(pvars->ploop_stack, LOOP_BROKEN);
			break;
		} else if (loop_stack_get(pvars->ploop_stack) & LOOP_CONTINUED) {
			loop_stack_clear(pvars->ploop_stack, LOOP_CONTINUED);
			// don't skip the boolean test
		}

		mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
		if (mv_is_non_null(&val)) {
			mv_set_boolean_strict(&val);
			if (!val.u.boolv) {
				break;
			}
		} else {
			break;
		}
	}
	loop_stack_pop(pvars->ploop_stack);
}

// ----------------------------------------------------------------
static void handle_for_srec(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	bind_stack_push(pvars->pbind_stack, pnode->pbound_variables);
	loop_stack_push(pvars->ploop_stack);
	// Copy the lrec for the very likely case that it is being updated inside the for-loop.
	lrec_t* pcopyrec = lrec_copy(pvars->pinrec);
	lhmsmv_t* pcopyoverlay = lhmsmv_copy(pvars->ptyped_overlay);

	for (lrece_t* pe = pcopyrec->phead; pe != NULL; pe = pe->pnext) {

		mv_t mvval = pnode->ptype_infererenced_srec_field_getter(pe, pcopyoverlay);

		mv_t mvkey = mv_from_string_no_free(pe->key);
		lhmsmv_put(pnode->pbound_variables, pnode->for_srec_k_name, &mvkey, FREE_ENTRY_VALUE);
		lhmsmv_put(pnode->pbound_variables, pnode->for_v_name, &mvval, FREE_ENTRY_VALUE);

		pnode->pblock_handler(pnode->pblock_statements, pvars, pcst_outputs);
		if (loop_stack_get(pvars->ploop_stack) & LOOP_BROKEN) {
			loop_stack_clear(pvars->ploop_stack, LOOP_BROKEN);
			break;
		} else if (loop_stack_get(pvars->ploop_stack) & LOOP_CONTINUED) {
			loop_stack_clear(pvars->ploop_stack, LOOP_CONTINUED);
		}
	}
	lhmsmv_free(pcopyoverlay);
	lrec_free(pcopyrec);
	loop_stack_pop(pvars->ploop_stack);
	bind_stack_pop(pvars->pbind_stack);
}

// ----------------------------------------------------------------
static void handle_for_oosvar(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	bind_stack_push(pvars->pbind_stack, pnode->pbound_variables);
	loop_stack_push(pvars->ploop_stack);

	// Evaluate the keylist: e.g. in 'for ((k1, k2), v in @a[3][$4]) { ... }', find the value of $4 for
	// the current record.

	int keys_all_non_null_or_error = FALSE;
	sllmv_t* plhskeylist = evaluate_list(pnode->poosvar_lhs_keylist_evaluators, pvars, &keys_all_non_null_or_error);
	if (keys_all_non_null_or_error) {

		// Locate and copy the submap indexed by the keylist. E.g. in 'for ((k1, k2), v in @a[3][$4]) { ... }', the
		// submap is indexed by ["a", 3, $4].  Copy it for the very likely case that it is being updated inside the
		// for-loop.
		mlhmmv_value_t submap = mlhmmv_copy_submap(pvars->poosvars, plhskeylist);

		if (!submap.is_terminal && submap.u.pnext_level != NULL) {
			// Recurse over the for-k-names, e.g. ["k1", "k2"], on each call descending one level
			// deeper into the submap.  Note there must be at least one k-name so we are assuming
			// the for-loop within handle_for_oosvar_aux was gone through once & thus
			// handle_statement_list_with_break_continue was called through there.

			handle_for_oosvar_aux(pnode, pvars, pcst_outputs, submap, pnode->pfor_oosvar_k_names->phead);

			if (loop_stack_get(pvars->ploop_stack) & LOOP_BROKEN) {
				loop_stack_clear(pvars->ploop_stack, LOOP_BROKEN);
			}
			if (loop_stack_get(pvars->ploop_stack) & LOOP_CONTINUED) {
				loop_stack_clear(pvars->ploop_stack, LOOP_CONTINUED);
			}
		}

		mlhmmv_free_submap(submap);
	}
	sllmv_free(plhskeylist);

	loop_stack_pop(pvars->ploop_stack);
	bind_stack_pop(pvars->pbind_stack);
}

static void handle_for_oosvar_aux(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs,
	mlhmmv_value_t           submap,
	sllse_t*                 prest_for_k_names)
{
	if (prest_for_k_names != NULL) { // Keep recursing over remaining k-names

		if (submap.is_terminal) {
			// The submap was too shallow for the user-specified k-names; there are no terminals here.
		} else {
			// Loop over keys at this submap level:
			for (mlhmmv_level_entry_t* pe = submap.u.pnext_level->phead; pe != NULL; pe = pe->pnext) {
				// Bind the k-name to the entry-key mlrval:
				lhmsmv_put(pnode->pbound_variables, prest_for_k_names->value, &pe->level_key, NO_FREE);
				// Recurse into the next-level submap:
				handle_for_oosvar_aux(pnode, pvars, pcst_outputs, pe->level_value, prest_for_k_names->pnext);

				if (loop_stack_get(pvars->ploop_stack) & LOOP_BROKEN) {
					// Bit cleared in recursive caller
					return;
				} else if (loop_stack_get(pvars->ploop_stack) & LOOP_CONTINUED) {
					loop_stack_clear(pvars->ploop_stack, LOOP_CONTINUED);
				}

			}
		}

	} else { // End of recursion: k-names have all been used up

		if (!submap.is_terminal) {
			// The submap was too deep for the user-specified k-names; there are no terminals here.
		} else {
			// Bind the v-name to the terminal mlrval:
			lhmsmv_put(pnode->pbound_variables, pnode->for_v_name, &submap.u.mlrval, NO_FREE);
			// Execute the loop-body statements:
			pnode->pblock_handler(pnode->pblock_statements, pvars, pcst_outputs);
		}

	}
}

// ----------------------------------------------------------------
static void handle_break(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	loop_stack_set(pvars->ploop_stack, LOOP_BROKEN);
}

// ----------------------------------------------------------------
static void handle_continue(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	loop_stack_set(pvars->ploop_stack, LOOP_CONTINUED);
}

// ----------------------------------------------------------------
static void handle_bare_boolean(
	mlr_dsl_cst_statement_t* pnode,
	variables_t*             pvars,
	cst_outputs_t*           pcst_outputs)
{
	rval_evaluator_t* prhs_evaluator = pnode->prhs_evaluator;

	mv_t val = prhs_evaluator->pprocess_func(prhs_evaluator->pvstate, pvars);
	if (mv_is_non_null(&val))
		mv_set_boolean_strict(&val);
}

// ----------------------------------------------------------------
// Example ASTs, with and without indexing on the left-hand-side oosvar name:
//
// $ mlr put -v '@x[1]["2"][$3][@4]=5' /dev/null
// list (statement_list):
//     = (oosvar_assignment):
//         oosvar_keylist (oosvar_keylist):
//             x (string_literal).
//             1 (strnum_literal).
//             2 (strnum_literal).
//             3 (field_name).
//             oosvar_keylist (oosvar_keylist):
//                 4 (string_literal).
//         5 (strnum_literal).
//
// $ mlr put -v '@x = $y'
// list (statement_list):
//     = (oosvar_assignment):
//         oosvar_keylist (oosvar_keylist):
//             x (string_literal).
//         y (field_name).
//
// $ mlr put -q -v 'emit @v, "a", "b", "c"'
// list (statement_list):
//     emit (emit):
//         oosvar_keylist (oosvar_keylist):
//             v (string_literal).
//         a (strnum_literal).
//         b (strnum_literal).
//         c (strnum_literal).
//
// $ mlr put -q -v 'emit @v[1][2], "a", "b","c"'
// list (statement_list):
//     emit (emit):
//         oosvar_keylist (oosvar_keylist):
//             v (string_literal).
//             1 (strnum_literal).
//             2 (strnum_literal).
//         a (strnum_literal).
//         b (strnum_literal).
//         c (strnum_literal).

// pnode is input; pkeylist_evaluators is appended to.
static sllv_t* allocate_keylist_evaluators_from_oosvar_node(mlr_dsl_ast_node_t* pnode, int type_inferencing,
	int context_flags)
{
	sllv_t* pkeylist_evaluators = sllv_alloc();

	for (sllve_t* pe = pnode->pchildren->phead; pe != NULL; pe = pe->pnext) {
		mlr_dsl_ast_node_t* pkeynode = pe->pvvalue;
		if (pkeynode->type == MD_AST_NODE_TYPE_STRING_LITERAL) {
			sllv_append(pkeylist_evaluators, rval_evaluator_alloc_from_string(pkeynode->text));
		} else {
			sllv_append(pkeylist_evaluators, rval_evaluator_alloc_from_ast(pkeynode, type_inferencing, context_flags));
		}
	}
	return pkeylist_evaluators;
}

// ----------------------------------------------------------------
void mlr_dsl_list_all_keywords_raw(FILE* ostream) {
  printf("filter\n");
  printf("unset\n");
  printf("emit\n");
  printf("emitp\n");
  printf("emitf\n");
  printf("dump\n");
  printf("edump\n");
  printf("print\n");
  printf("eprint\n");
}

static void mlr_dsl_filter_usage(FILE* ostream) {
	fprintf(ostream, "filter: includes/excludes the record in the output record\n");
	fprintf(ostream, "  stream.\n");
	fprintf(ostream, "  Example: %s put 'filter (NR == 2 || $x > 5.4)'.\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Instead of put with 'filter false' you can simply use put -q.  The following\n");
	fprintf(ostream, "  uses the input record to accumulate data but only prints the running sum:\n");
	fprintf(ostream, "  %s put -q '@running_sum += $x * $y; emit @running_sum'.\n", MLR_GLOBALS.bargv0);
}

static void mlr_dsl_unset_usage(FILE* ostream) {
	fprintf(ostream, "unset: clears field(s) from the current record, or an out-of-stream variable.\n");
	fprintf(ostream, "  Example: %s put 'unset $x'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put 'unset $*'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put 'for (k, v in $*) { if (k =~ \"a.*\") { unset $[k] } }'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '...; unset @sums'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '...; unset @sums[\"green\"]'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '...; unset @*'\n", MLR_GLOBALS.bargv0);
}

static void mlr_dsl_emit_usage(FILE* ostream) {
	fprintf(ostream, "emit: inserts an out-of-stream variable into the output record stream. Hashmap\n");
	fprintf(ostream, "  indices present in the data but not slotted by emit arguments are not output.\n");
	fprintf(ostream, "  Example: %s put '... ; emit @sums'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '... ; emit @sums, \"index1\", \"index2\"'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '... ; emit @*, \"index1\", \"index2\"'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Please see http://johnkerl.org/miller/doc for more information.\n");
}

static void mlr_dsl_emitp_usage(FILE* ostream) {
	fprintf(ostream, "emitp: inserts an out-of-stream variable into the output record stream. Hashmap\n");
	fprintf(ostream, "  indices present in the data but not slotted by emitp arguments are output\n");
	fprintf(ostream, "  concatenated with \":\".\n");
	fprintf(ostream, "  Example: %s put '... ; emitp @sums'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '... ; emitp @sums, \"index1\", \"index2\"'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '... ; emitp @*, \"index1\", \"index2\"'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Please see http://johnkerl.org/miller/doc for more information.\n");
}

static void mlr_dsl_emitf_usage(FILE* ostream) {
	fprintf(ostream, "emitf: inserts non-indexed out-of-stream variable(s) side-by-side into the\n");
	fprintf(ostream, "  output record stream.\n");
	fprintf(ostream, "  Example: %s put '... ; emit @a'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put '... ; emit @a, @b, @c'\n", MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Please see http://johnkerl.org/miller/doc for more information.\n");
}

static void mlr_dsl_dump_usage(FILE* ostream) {
	fprintf(ostream, "dump: prints all currently defined out-of-stream variables immediately\n");
	fprintf(ostream, "  to stdout as JSON.\n");
}

static void mlr_dsl_edump_usage(FILE* ostream) {
	fprintf(ostream, "edump: prints all currently defined out-of-stream variables immediately\n");
	fprintf(ostream, "  to stderr as JSON.\n");
}

static void mlr_dsl_print_usage(FILE* ostream) {
	fprintf(ostream, "print: prints expression immediately to stdout.\n");
	fprintf(ostream, "  Example: %s put -q 'print \"The sum of x and y is \".string($x+$y)'.\n",
		MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put -q 'for (k, v in $*) { print string(k) . \" => \" . string(v) }'.\n",
		MLR_GLOBALS.bargv0);
}

static void mlr_dsl_eprint_usage(FILE* ostream) {
	fprintf(ostream, "eprint: prints expression immediately to stderr.\n");
	fprintf(ostream, "  Example: %s put -q 'eprint \"The sum of x and y is \".string($x+$y)'.\n",
		MLR_GLOBALS.bargv0);
	fprintf(ostream, "  Example: %s put -q 'for (k, v in $*) { eprint string(k) . \" => \" . string(v) }'.\n",
		MLR_GLOBALS.bargv0);
}

// Pass function_name == NULL to get usage for all functions.
void mlr_dsl_keyword_usage(FILE* ostream, char* keyword) {
	if (keyword == NULL) {
		mlr_dsl_filter_usage(ostream); fprintf(ostream, "\n");
		mlr_dsl_unset_usage(ostream);  fprintf(ostream, "\n");
		mlr_dsl_emit_usage(ostream);   fprintf(ostream, "\n");
		mlr_dsl_emitp_usage(ostream);  fprintf(ostream, "\n");
		mlr_dsl_emitf_usage(ostream);  fprintf(ostream, "\n");
		mlr_dsl_dump_usage(ostream);   fprintf(ostream, "\n");
		mlr_dsl_edump_usage(ostream);  fprintf(ostream, "\n");
		mlr_dsl_print_usage(ostream);  fprintf(ostream, "\n");
		mlr_dsl_eprint_usage(ostream);
		return;
	}

	if (streq(keyword, "filter")) {
		mlr_dsl_filter_usage(ostream);
	} else if (streq(keyword, "unset")) {
		mlr_dsl_unset_usage(ostream);
	} else if (streq(keyword, "emit")) {
		mlr_dsl_emit_usage(ostream);
	} else if (streq(keyword, "emitp")) {
		mlr_dsl_emitp_usage(ostream);
	} else if (streq(keyword, "emitf")) {
		mlr_dsl_emitf_usage(ostream);
	} else if (streq(keyword, "dump")) {
		mlr_dsl_dump_usage(ostream);
	} else if (streq(keyword, "edump")) {
		mlr_dsl_edump_usage(ostream);
	} else if (streq(keyword, "print")) {
		mlr_dsl_print_usage(ostream);
	} else if (streq(keyword, "eprint")) {
		mlr_dsl_eprint_usage(ostream);
	} else {
		fprintf(ostream, "%s: unrecognized keyword \"%s\".\n", MLR_GLOBALS.bargv0, keyword);
	}
}
