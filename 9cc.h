typedef int BinaryOperation;

enum ExprKind
{
  EK_Number,
  EK_Operator,
  EK_Identifier,
};

typedef struct Expr
{
  BinaryOperation binary_op;
  enum ExprKind expr_kind;
  int value;
  struct Expr *first_child;
  struct Expr *second_child;
  char* name;
} Expr;

enum StmtKind{
  SK_AndThen,
  SK_Expr,
  SK_Return,
  SK_If,
  SK_While,
  SK_For,
};

typedef struct Stmt
{
  enum StmtKind stmt_kind;
  
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
  LVar *next; // 次の変数かNULL
  char* name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};

extern LVar *locals;

typedef int TokenKind;

typedef struct Token
{
  TokenKind kind;
  int value;
  char* identifier_name;
} Token;

// prototype declaration
int isDigit(char c);
int intLength(char *str);
int parseInt(char *str);
Expr *parseMultiplicative(Token **ptrptr, Token *token_end);
Expr *parseAdditive(Token **ptrptr, Token *token_end);
Expr *parseExpr(Token **ptrptr, Token *token_end);
Expr *parseUnary(Token **ptrptr, Token *token_end);
Stmt *parseProgram(Token **ptrptr, Token *token_end);
Expr *parseAssign(Token **ptrptr, Token *token_end);
Stmt *parseFor(Token **ptrptr,Token *token_end);
Stmt *parseStmt(Token **ptrptr, Token *token_end);

void Codegen(Stmt* stmt);

int tokenize(char *str);
LVar *findLVar(char *name);
LVar *insertLVar(char *name);
LVar *lastLVar();
int is_alnum(char c);


void EvaluateExprIntoRax(Expr *expr);

extern Token tokens[1000];