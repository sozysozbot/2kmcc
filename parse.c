#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

Expr *numberexpr(int value)
{
  Expr *numberexp = calloc(1, sizeof(Expr));
  numberexp->value = value;
  numberexp->expr_kind = EK_Number;
  return numberexp;
}

Expr *identifierexpr(char *name)
{
  Expr *numberexp = calloc(1, sizeof(Expr));
  numberexp->name = name;
  numberexp->expr_kind = EK_Identifier;
  return numberexp;
}

Expr *binaryExpr(Expr *first_child, Expr *second_child, BinaryOperation binaryop)
{
  Expr *newexp = calloc(1, sizeof(Expr));
  newexp->first_child = first_child;
  newexp->expr_kind = EK_Operator;
  newexp->binary_op = binaryop;
  newexp->second_child = second_child;
  return newexp;
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Expr *parseRelational(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  Expr *result = parseAdditive(&tokens, token_end);

  for (; tokens < token_end;)
  {
    Token maybe_relational = *tokens;
    switch (maybe_relational.kind)
    {
    case TK_Greater:
    {
      tokens++;
      Expr *numberexp = parseAdditive(&tokens, token_end);
      result = binaryExpr(result, numberexp, '>');
      break;
    }
    case TK_GreaterEqual:
    {
      tokens++;
      Expr *numberexp = parseAdditive(&tokens, token_end);
      result = binaryExpr(result, numberexp, '>' * 256 + '=');
      break;
    }
    case TK_Less:
    {
      tokens++;
      Expr *numberexp = parseAdditive(&tokens, token_end);
      // swap children of operator node
      result = binaryExpr(numberexp, result, '>');
      break;
    }
    case TK_LessEqual:
    {
      tokens++;
      Expr *numberexp = parseAdditive(&tokens, token_end);
      // swap children of operator node
      result = binaryExpr(numberexp, result, '>' * 256 + '=');
      break;
    }
    default:
      *ptrptr = tokens;
      return result;
      break;
    }
  }
  *ptrptr = tokens;
  return result;
}
// equality   = relational ("==" relational | "!=" relational)*
Expr *parseEquality(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  Expr *result = parseRelational(&tokens, token_end);

  for (; tokens < token_end;)
  {
    Token maybe_relational = *tokens;
    switch (maybe_relational.kind)
    {
    case TK_Equal:
    {
      tokens++;
      Expr *numberexp = parseRelational(&tokens, token_end);
      result = binaryExpr(result, numberexp, '=' * 256 + '=');
      break;
    }
    case TK_NotEqual:
    {
      tokens++;
      Expr *numberexp = parseRelational(&tokens, token_end);
      result = binaryExpr(result, numberexp, '!' * 256 + '=');
      break;
    }
    default:
      *ptrptr = tokens;
      return result;
      break;
    }
  }
  *ptrptr = tokens;
  return result;
}
// primary    = num | ident | "(" expr ")"
Expr *parsePrimary(Token **ptrptr, Token *token_end)
{
  Token *maybe_number = *ptrptr;
  if (maybe_number >= token_end)
  {
    fprintf(stderr, "Expected: number, but got EOF");
    exit(1);
  }
  if (maybe_number->kind == TK_Number)
  {
    *ptrptr += 1;
    return numberexpr(maybe_number->value);
  }
  else if (maybe_number->kind == TK_Identifier)
  {
    *ptrptr += 1;
    return identifierexpr(maybe_number->identifier_name);
  }
  else
  {
    Token *maybe_leftparenthesis = maybe_number;
    if (maybe_leftparenthesis->kind == TK_LeftParenthesis)
    {
      *ptrptr += 1;
      Expr *expr = parseExpr(ptrptr, token_end);
      Token *maybe_rightparenthesis = *ptrptr;
      if (maybe_rightparenthesis->kind != TK_RightParenthesis)
      {
        fprintf(stderr, "Expected: right parenthesis. Token Kind:%d", maybe_rightparenthesis->kind);
        exit(1);
      }
      *ptrptr += 1;
      return expr;
    }
    fprintf(stderr, "Expected: number. Token Kind:%d", maybe_number->kind);
    exit(1);
  }
}
// unary   = ("+" | "-")? primary
Expr *parseUnary(Token **ptrptr, Token *token_end)
{
  Token *maybe_unary = *ptrptr;
  if (maybe_unary >= token_end)
  {
    fprintf(stderr, "Expected: number, but got EOF");
    exit(1);
  }
  if (maybe_unary->kind == TK_Plus)
  {
    *ptrptr += 1;
    return parsePrimary(ptrptr, token_end);
  }
  if (maybe_unary->kind == TK_Minus)
  {
    *ptrptr += 1;
    return binaryExpr(numberexpr(0), parsePrimary(ptrptr, token_end), '-');
  }
  return parsePrimary(ptrptr, token_end);
}

// expr       = equality
Expr *parseExpr(Token **ptrptr, Token *token_end)
{
  return parseAssign(ptrptr, token_end);
}
Expr *parseAssign(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  Expr *result = parseEquality(&tokens, token_end);
  if (tokens->kind == TK_Assign)
  {
    tokens++;
    Expr *newresult = parseAssign(&tokens, token_end);
    *ptrptr = tokens;
    return binaryExpr(result, newresult, '=');
  }
  *ptrptr = tokens;
  return result;
}

Expr *parseOptionalExprAndToken(Token **ptrptr, Token *token_end, enum TokenKind target)
{
  Token *tokens = *ptrptr;
  if (tokens->kind == target)
  {
    tokens++;
    *ptrptr = tokens;
    return NULL;
  }
  Expr *expr = parseExpr(&tokens, token_end);
  if (tokens->kind == target)
  {
    tokens++;
    *ptrptr = tokens;
    return expr;
  }
  fprintf(stderr, "expected TokenKind#%d after optional expression but did not find one", target);
  exit(-1);
}

Stmt *parseFor(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  tokens++;

  if (tokens->kind == TK_LeftParenthesis)
  {
    tokens++;
  }
  else
  {
    fprintf(stderr, "expected left parenthesis got %d\n", tokens->kind);
    exit(1);
  }
  Expr *exprs[3] = {NULL, NULL, NULL};
  exprs[0] = parseOptionalExprAndToken(&tokens, token_end, TK_Semicolon);
  exprs[1] = parseOptionalExprAndToken(&tokens, token_end, TK_Semicolon);
  exprs[2] = parseOptionalExprAndToken(&tokens, token_end, TK_RightParenthesis);

  Stmt *stmt = malloc(sizeof(Stmt));
  stmt->stmt_kind = SK_For;
  stmt->first_child = NULL;
  stmt->second_child = NULL;
  stmt->third_child = NULL;
  stmt->expr = exprs[0];
  stmt->expr1 = exprs[1];
  stmt->expr2 = exprs[2];

  Stmt *statement = parseStmt(&tokens, token_end);
  stmt->second_child = statement;

  *ptrptr = tokens;

  return stmt;
}
// statement= expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "while" "(" expr ")" stmt ?
//        | for?
Stmt *parseStmt(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  int is_return = 0;
  int is_if = 0;
  int is_while = 0;

  if (tokens->kind == TK_Return)
  {
    tokens++;
    is_return = 1;
  }
  if (tokens->kind == TK_IF)
  {
    tokens++;
    is_if = 1;
    if (tokens->kind == TK_LeftParenthesis)
    {
      tokens++;
    }
    else
    {
      fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
      exit(1);
    }
  }
  if (tokens->kind == TK_WHILE)
  {
    tokens++;
    is_while = 1;
    if (tokens->kind == TK_LeftParenthesis)
    {
      tokens++;
    }
    else
    {
      fprintf(stderr, "expected right parenthesis got %d\n", tokens->kind);
      exit(1);
    }
  }
  if (tokens->kind == TK_FOR)
  {
    Stmt *stmt = parseFor(&tokens, token_end);
    *ptrptr = tokens;
    return stmt;
  }
  Expr *expr = parseExpr(&tokens, token_end);
  {
    Token maybe_operator = *tokens;
    if (is_if || is_while)
    {
      if (maybe_operator.kind == TK_RightParenthesis)
      {
        tokens++;
      }
      else
      {
        fprintf(stderr, "expected right parenthesis got %d\n", maybe_operator.kind);
        exit(1);
      }
    }
    else
    {
      if (maybe_operator.kind == TK_Semicolon)
      {
        tokens++;
      }
      else
      {
        fprintf(stderr, "no semicolon after expr. kind=%d\n", maybe_operator.kind);
        exit(1);
      }
    }
  }
  Stmt *stmt = malloc(sizeof(Stmt));
  stmt->stmt_kind = SK_Expr;
  stmt->first_child = NULL;
  stmt->second_child = NULL;
  stmt->third_child = NULL;
  stmt->expr = expr;
  if (is_if)
  {
    stmt->stmt_kind = SK_If;
    {
      Stmt *statement = parseStmt(&tokens, token_end);
      stmt->second_child = statement;
    }
    Token maybe_else = *(tokens);
    if (maybe_else.kind == TK_ELSE)
    {
      tokens++;
      Stmt *statement1 = parseStmt(&tokens, token_end);
      stmt->third_child = statement1;
    }
  }
  if (is_while)
  {
    stmt->stmt_kind = SK_While;
    Stmt *statement = parseStmt(&tokens, token_end);
    stmt->second_child = statement;
  }
  if (is_return)
  {
    stmt->stmt_kind = SK_Return;
  }
  *ptrptr = tokens;
  return stmt;
}
// program = (expr ";")*
Stmt *parseProgram(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  Stmt *result = parseStmt(&tokens, token_end);

  for (; tokens < token_end;)
  {
    Stmt *statement = parseStmt(&tokens, token_end);
    Stmt *newexp = calloc(1, sizeof(Stmt));
    newexp->first_child = result;
    newexp->stmt_kind = SK_AndThen;
    newexp->second_child = statement;
    result = newexp;
  }
  *ptrptr = tokens;
  return result;
}

// mul = unary ("*" unary | "/" unary)*
Expr *parseMultiplicative(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  Expr *result = parseUnary(&tokens, token_end);

  for (; tokens < token_end;)
  {
    Token maybe_operator = *tokens;
    switch (maybe_operator.kind)
    {
    case TK_Number:
    {
      fprintf(stderr, "Expected operator got Number");
      exit(1);
    }
    case TK_Mul:
    {
      tokens++;
      Expr *numberexp = parseUnary(&tokens, token_end);
      result = binaryExpr(result, numberexp, '*');
      // ptr++;
      break;
    }
    case TK_Div:
    {
      tokens++;
      Expr *numberexp = parseUnary(&tokens, token_end);
      result = binaryExpr(result, numberexp, '/');
      // ptr++;
      break;
    }
    default:
      *ptrptr = tokens;
      return result;
      break;
    }
  }
  *ptrptr = tokens;
  return result;
}

// additive = mul ("+" mul | "-" mul)*
Expr *parseAdditive(Token **ptrptr, Token *token_end)
{
  Token *tokens = *ptrptr;
  if (token_end == tokens)
  {
    fprintf(stderr, "No token found");
    exit(1);
  }
  Expr *result = parseMultiplicative(&tokens, token_end);

  for (; tokens < token_end;)
  {
    Token maybe_operator = *tokens;
    switch (maybe_operator.kind)
    {
    case TK_Number:
    {
      fprintf(stderr, "Expected operator got Number");
      exit(1);
    }
    case TK_Minus:
    {
      tokens++;
      Expr *numberexp = parseMultiplicative(&tokens, token_end);
      result = binaryExpr(result, numberexp, '-');
      // ptr++;
      break;
    }
    case TK_Plus:
    {
      tokens++;
      Expr *numberexp = parseMultiplicative(&tokens, token_end);
      result = binaryExpr(result, numberexp, '+');
      // ptr++;
      break;
    }
    default:
      *ptrptr = tokens;
      return result;
      break;
    }
  }
  *ptrptr = tokens;
  return result;
}
