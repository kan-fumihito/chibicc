#ifndef CHIBICC_H
#define CHIBICC_H

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Tokenizer
//

typedef enum
{
  TK_RESERVED, // Keywords or punctuators
  TK_IDENT,    // Identifier
  TK_NUM,      // Integer literals
  TK_EOF,      // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token
{
  TokenKind kind; // Token kind
  Token *next;    // Next token
  int val;        // If kind is TK_NUM, its value
  char *str;      // Token string
  int len;        // Token length
};

typedef struct LVar LVar;
struct LVar
{
  LVar *next; // pointer is next variable
  char *name; // name of variable
  int len;    // length of name
  int offset; // offset from RBP
};

LVar *locals;

// Input program
char *user_input;

// Current token
Token *token;

// Reports an error and exit.
void error(char *fmt, ...);
// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...);
// Consumes the current token if it matches `op`.
bool consume(char *op);
// Consumes the current token if it matches variable.
Token *consume_ident(void);
// Ensure that the current token is `op`.
void expect(char *op);
// Ensure that the current token is TK_NUM.
int expect_number();
// Return whether the current token is TK_EOF.
bool at_eof();
// Find variable name.
LVar *find_lvar(Token *tok);
// Create a new token and add it as the next token of `cur`.
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
// Compare p and q based strlen(q).
bool startswith(char *p, char *q);
// Return length of variable name
int len_var(char *p);
// Tokenize `user_input` and returns new tokens.
Token *tokenize();

//
// Parser
//

typedef enum
{
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_NUM,    // Integer
  ND_ASSIGN, // =
  ND_LVAR,   // Local Variable
  ND_FUNC,
  ND_RET, // Return
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_FUNCD,
} NodeKind;

typedef struct Arg Arg;

// AST node type
typedef struct Node Node;
struct Node
{
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side

  // if
  Node *cond;
  Node *then;
  Node *els;

  // for
  Node *init;
  Node *post;

  // block{}
  Node *next;

  int val;    // Used if kind == ND_NUM
  int offset; // Used if kind == ND_LVAR

  char fname[16];
  Arg *arg;
  int argc;
};
struct Arg
{
  Node *val;
  Arg *next;
};

Node *code[10][100];
Node *fn[10];

Node *new_node(NodeKind kind);
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
Node *new_num(int val);

void program();
Node *func(int n);
Node *stmt();
Node *expr();
Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void gen_lval(Node *node);
void gen(Node *node);

#endif // !CHIBICC_H