typedef int BinaryOperation;
typedef int UnaryOperation;
typedef int BaseType;

typedef struct Type {
    BaseType ty;
    struct Type *ptr_to;
} Type;

enum ExprKind {
    EK_Number,
    EK_BinaryOperator,
    EK_UnaryOperator,
    EK_Identifier,
    EK_Call,
};

typedef struct Expr {
    BinaryOperation op;
    enum ExprKind expr_kind;
    int value;
    struct Expr *first_child;
    struct Expr *second_child;
    struct Expr **func_args;
    int func_arg_len;
    char *name;
} Expr;

typedef int StmtKind;

typedef struct FuncDef {
    struct Stmt *content;
    char *name;
    char **params;
    int param_len;
    char **lvar_names_start;
    char **lvar_names_end;
} FuncDef;

typedef struct Stmt {
    StmtKind stmt_kind;
    struct Expr *expr;
    struct Expr *expr1;
    struct Expr *expr2;
    struct Stmt *first_child;
    struct Stmt *second_child;
    struct Stmt *third_child;
} Stmt;

typedef struct LVar {
    struct LVar *next;
    char *name;
    int offset_from_rbp;
} LVar;

typedef int TokenKind;

typedef struct Token {
    TokenKind kind;
    int value;
    char *identifier_name;
} Token;

Expr *parseMultiplicative(void);
Expr *parseAdditive(void);
Expr *parseExpr(void);
Expr *parseUnary(void);
void parseProgram(void);
Expr *parseAssign(void);
Stmt *parseFor(void);
Stmt *parseStmt(void);
FuncDef *parseFunction(void);

void CodegenFunc(FuncDef *funcdef);

int tokenize(char *str);
LVar *findLVar(char *name);
LVar *insertLVar(char *name);
LVar *lastLVar();
int is_alnum(char c);

void EvaluateExprIntoRax(Expr *expr);

extern Token all_tokens[1000];
extern FuncDef *all_funcdefs[100];

int enum2(int a, int b);
int enum3(int a, int b, int c);
int enum4(int a, int b, int c, int d);
