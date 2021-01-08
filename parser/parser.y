%{

#include <cstdio>
#include <stdlib.h>
#include "../Constants.h"
#include "Execute.h"
#include "parser.yy.cpp"

int yylex();
void yyerror(const char *);

class Execute;
%}

%union {
    int val_i;
    float val_f;
    char *val_s;
    column_ref* ref_column;
    column_defs* def_column;
    table_def* def_table;
    linked_list* list;
    insert_argu* insert_argu;
    expr_node* expr_node;
    select_argu* select_argu;
    delete_argu* delete_argu;
    update_argu* update_argu;
    table_constraint* t_constraint;
}

/* keyword */
%token QUIT
%token SELECT DELETE UPDATE INSERT
%token CREATE DROP USE SHOW TABLES
%token DATABASES DATABASE TABLE
%token FROM WHERE OPERATOR VALUES SET INTO

/* COLUMN DESCPRIPTION */
%token INT CHAR FLOAT VARCHAR DATE PRIMARY FOREIGN REFERENCES


/* number */
%token <ivalue> VALUE_INT
%token <fvalue> VALUE_FLOAT
%token <string> VALUE_STRING IDENTIFIER VALUE_DATE

/* operator */
%token EQ GT LT GE LE NE

/* aggretation */
%token NOTNULL DESC GROUP LIKE INDEX CHECK IN T_NULL IS AND OR

%type <tree> command
%type <ivalue> type_width
%type <attributesTree> attributes
%type <attributeTree> attribute
%type <identList> tableList column_list column_list_exist
%type <constValuesTree> constvalues
%type <columnsTree> column_decs
%type <columnTree> column_dec
%type <insertValueTree> valueLists
%type <setClauseList> setList
%type <attrType> column_type
%type <ivalue> column_constraints column_constraint
%type <tbOptDec> tb_opt_dec
%type <tbOptDecList> tb_opt_exist tb_opt_decs
%type <expr> expr factor value comparison constvalue whereclause conditions


%%

<program>  : %empty
            | <program><stmt>

<stmt>  : <sysStmt>’;’  {}
         | <dbStmt>’;’  {}
         | <tbStmt>’;’  {}
         | <idxStmt>’;’  {}
		  | <alterStmt>’;’  {}

<tbStmt>  :
           {
                (new Insert($3, $5)).execute();
           }
           | DELETE FROM <tbName> WHERE <whereClause>
           {
                (new Delete($3, $5)).execute();
           }
           | UPDATE <tbName> SET <setClause> WHERE <whereClause>
           {
                (new Update($2, $4, $6)).execute();
           }
           | SELECT <selector> FROM <tableList> WHERE <whereClause>
           {

           }

<type>  : INT (<VALUE_INT>)
         | VARCHAR (<VALUE_INT>)
         | DATE
         | FLOAT
          KINT {$$=AttrType::INT;}
         | KVARCHAR  {$$=AttrType::VARCHAR;}
         | KFLOAT {$$=AttrType::FLOAT;}
         | KDATE {$$=AttrType::DATE;}
         ;

<valueLists>  : ’(’<valueList>’)’
    {
        $$ = new ConstValueLists();
        $$->addConstValues($2);
    }
    | <valueLists>’, ’ ’ (’<valueList>’) ’
    {
        $$->addConstValues($4);
    }

<valueList>  : <value>
| <valueList>’, ’<value>

<value>  : VALUE_INT
		   | VALUE_FLOAT
          | VALUE_STRING
          | NULL

<whereClause>  : <col> <op> <expr>
                {
                    $$ = new Expr($1, $2, $3);
                }
                | <col> IS NULL
                {
                    $$ = new Expr($1, CompOp::IS_OP, new Expr());
                }
                | <col> IS NOT NULL
                {
                    $$ = new Expr($1, CompOp::ISNOT_OP, new Expr());
                }
                | <whereClause> AND <whereClause>
                {
                    $$ = new Expr($1, LogicOp::AND_OP, $3);
                }

<col>  : [<tbName>’.’]<colName>

<op>  : ‘=’
        {
            $$ = CompOp::EQ_OP;
        }
        | ‘<>’
        {
            $$ = CompOp::NE_OP;
        }
        | ‘<=’
        {
            $$ = CompOp::LE_OP;
        }
        | ‘>=’
        {
            $$ = CompOp::GE_OP;
        }
        | ‘<’
        {
            $$ = CompOp::LT_OP;
        }
        | ‘>’
        {
            $$ = CompOp::GT_OP;
        }

<expr>  : <value>
         | <col>

<setClause>  : <colName>’=’<value>
| <setClause>’, ’<colName>’=’<value>

<selector>  : * | <col>[,<col>]*

<tableList>  : <tbName>
| <tableList>’,’<tbName>

<columnList>  : <colName>
            | <columnList>’,’<colName>

<dbName>  : IDENTIFIER

<tbName>  : IDENTIFIER

<colName> : IDENTIFIER


%%


void yyerror(const char *msg) {
    printf("YACC error: %s\n", msg);
}

char start_parse(const char *expr_input)
{
    char ret;
    if(expr_input){
        YY_BUFFER_STATE my_string_buffer = yy_scan_string(expr_input);
        yy_switch_to_buffer( my_string_buffer ); // switch flex to the buffer we just created
        ret = yyparse();
        yy_delete_buffer(my_string_buffer );
    }else{
        ret = yyparse();
    }
    return ret;
}

