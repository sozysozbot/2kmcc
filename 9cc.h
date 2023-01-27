typedef int BinaryOperation;

enum ExprKind {
    EK_Number,
    EK_Operator,
    EK_Identifier,
    EK_Call,
};

typedef struct Expr {
    BinaryOperation binary_op;
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

typedef struct LVar LVar;

// ローカル変数の型
struct LVar {
    LVar *next;  // 次の変数かNULL
    char *name;  // 変数の名前
    int len;     // 名前の長さ
    int offset;  // RBPからのオフセット
};

extern LVar *locals;

typedef int TokenKind;

typedef struct Token {
    TokenKind kind;
    int value;
    char *identifier_name;
} Token;

// prototype declaration
int isDigit(char c);
int intLength(char *str);
int parseInt(char *str);
Expr *parseMultiplicative();
Expr *parseAdditive();
Expr *parseExpr();
Expr *parseUnary();
void parseProgram();
Expr *parseAssign();
Stmt *parseFor();
Stmt *parseStmt();
FuncDef *parseFunction();

void CodegenStmt(Stmt *stmt);

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
