#include "chibicc.h"

Node *new_node(NodeKind kind)
{
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *node = new_node(kind);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val)
{
  Node *node = new_node(ND_NUM);
  node->val = val;
  return node;
}

void program()
{
  int i = 0;
  while (!at_eof()){
    fn[i] = func(i);
    i++;
  }
  fn[i] = NULL;
}

Node *func(int n)
{
  Token *tok = consume_ident();

  if (tok)
  {
    locals = NULL;
    Node *node = calloc(1, sizeof(Node));
    if (consume("("))
    {
      node->kind = ND_FUNCD;
      strncpy(node->fname, tok->str, tok->len);
      Arg *arg;
      arg = calloc(1, sizeof(Arg));
      node->arg = arg;
      node->argc = 0;
      arg->val = NULL;
      while (!consume(")"))
      {
        arg->val = expr();
        node->argc++;
        if (consume(","))
        {
          arg->next = calloc(1, sizeof(Arg));
          arg = arg->next;
        }
        else
        {
          arg->next = NULL;
        }
      }
    }
    if (consume("{"))
    {
      int i = 0;
      while (!consume("}"))
        code[n][i++] = stmt();
      code[n][i] = NULL;
    }
    return node;
  }
}

Node *stmt()
{
  Node *node, *tmp;

  if (consume("if"))
  {
    expect("(");
    node = new_node(ND_IF);
    node->cond = expr();

    expect(")");
    node->then = stmt();
    node->els = NULL;

    if (consume("else"))
    {
      node->els = stmt();
    }
    return node;
  }

  if (consume("while"))
  {
    expect("(");
    node = new_node(ND_WHILE);
    node->cond = expr();

    expect(")");
    node->then = stmt();
    node->els = NULL;
    return node;
  }

  if (consume("for"))
  {
    expect("(");
    node = new_node(ND_FOR);
    if (!consume(";"))
    { //初期化
      node->init = expr();
      expect(";");
    }
    else
    {
      node->init = NULL;
    }

    if (!consume(";"))
    { //条件式
      node->cond = expr();
      expect(";");
    }
    else
    {
      node->cond = NULL;
    }

    if (!consume(")"))
    { //更新式
      node->post = expr();
      expect(")");
    }
    else
    {
      node->post = NULL;
    }

    node->then = stmt();
    return node;
  }

  if (consume("{"))
  {
    node = new_node(ND_BLOCK);
    tmp = node;
    while (!consume("}"))
    {
      tmp->next = stmt();
      tmp = tmp->next;
    }
    tmp->next = NULL;
    return node;
  }

  if (consume("return"))
  {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RET;
    node->lhs = expr();
  }
  else
  {
    node = expr();
  }

  if (!consume(";"))
    error_at(token->str, " is not `;'");
  return node; 
}

// expr = equality
Node *expr()
{
  return assign();
}

Node *assign()
{
  Node *node = equality();
  for (;;)
  {
    if (consume("="))
    {
      node = new_binary(ND_ASSIGN, node, assign());
    }
    else
    {
      return node;
    }
  }
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality()
{
  Node *node = relational();

  for (;;)
  {
    if (consume("=="))
      node = new_binary(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_binary(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational()
{
  Node *node = add();

  for (;;)
  {
    if (consume("<"))
      node = new_binary(ND_LT, node, add());
    else if (consume("<="))
      node = new_binary(ND_LE, node, add());
    else if (consume(">"))
      node = new_binary(ND_LT, add(), node);
    else if (consume(">="))
      node = new_binary(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add()
{
  Node *node = mul();

  for (;;)
  {
    if (consume("+"))
      node = new_binary(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_binary(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul()
{
  Node *node = unary();

  for (;;)
  {
    if (consume("*"))
      node = new_binary(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_binary(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary()
{
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_binary(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num
Node *primary()
{
  if (consume("("))
  {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok)
  {
    Node *node = calloc(1, sizeof(Node));
    if (consume("("))
    {
      node->kind = ND_FUNC;
      strncpy(node->fname, tok->str, tok->len);
      Arg *arg;
      arg = calloc(1, sizeof(Arg));
      node->arg = arg;
      node->argc = 0;
      arg->val = NULL;
      while (!consume(")"))
      {
        arg->val = expr();
        node->argc++;
        if (consume(","))
        {
          arg->next = calloc(1, sizeof(Arg));
          arg = arg->next;
        }
        else
        {
          arg->next = NULL;
        }
      }

      return node;
    }

    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar != NULL)
    {
      node->offset = lvar->offset;
    }
    else
    {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals != NULL ? locals->offset + 8 : 0;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  return new_num(expect_number());
}