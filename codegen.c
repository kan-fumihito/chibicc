#include "chibicc.h"

int lcount = 0;
//
// Code generator
//
void gen_lval(Node *node)
{
  if (node->kind != ND_LVAR)
    error("assign lvalue is not variable");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_arg(Arg *arg, int n, int argc)
{
  if (argc == n)
  {
    return;
  }
  gen_arg(arg->next, n + 1, argc);

  gen(arg->val);
  if (n > 5)
  {
    return;
  }
  printf("  pop rax\n");
  switch (n)
  {
  case 0:
    printf("  mov edi, eax\n");
    break;
  case 1:
    printf("  mov esi, eax\n");
    break;
  case 2:
    printf("  mov edx, eax\n");
    break;
  case 3:
    printf("  mov ecx, eax\n");
    break;
  case 4:
    printf("  mov r8d, eax\n");
    break;
  case 5:
    printf("  mov r9d, eax\n");
    break;
  default:
    break;
  }
}

void gen(Node *node)
{
  int jflag;
  Node *tmp;
  Arg *arg;
  if (node->kind == ND_RET)
  {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

  switch (node->kind)
  {
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;

  case ND_IF:
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    jflag = lcount;
    lcount++;
    if (node->els == NULL)
    {
      printf("  je  .Lend%d\n", jflag);
      gen(node->then);
      printf(".Lend%d:\n", jflag);
    }
    else
    {
      printf("  je  .Lelse%d\n", jflag);
      gen(node->then);
      printf("  je  .Lend%d\n", jflag);
      printf(".Lelse%d:\n", jflag);
      gen(node->els);
      printf(".Lend%d:\n", jflag);
    }

    return;
  case ND_WHILE:
    jflag = lcount;
    lcount++;

    printf(".Lstart%d:\n", jflag);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .Lend%d\n", jflag);
    gen(node->then);

    printf("  pop rax\n");
    printf("  jmp .Lstart%d\n", jflag);
    printf(".Lend%d:\n", jflag);
    return;

  case ND_FOR:
    jflag = lcount;
    lcount++;

    if (node->init != NULL)
    {
      gen(node->init);
    }

    printf(".Lstart%d:\n", jflag);
    if (node->cond != NULL)
    {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", jflag);
    }

    gen(node->then);
    if (node->post)
    {
      gen(node->post);
    }
    printf("  pop rax\n");

    printf("  jmp .Lstart%d\n", jflag);
    printf(".Lend%d:\n", jflag);
    return;
  case ND_BLOCK:
    node = node->next;
    while (node != NULL)
    {
      gen(node);
      printf("  pop rax\n");
      node = node->next;
    }
    return;

  case ND_FUNC:
    arg = node->arg;
    if (node->argc > 6)
    {
      jflag = lcount;
      lcount++;
      if (node->argc % 2 == 0)
      {
        printf("  mov rax, rsp\n");
        printf("  and rax, 0x0f\n");
        printf("  cmp rax, 0\n");
        printf("  jne  .Lalign%d\n", jflag);
        if (arg->val != NULL)
        {
          gen_arg(arg, 0, node->argc);
        }
        printf("  mov rax, 0\n");
        printf("  call  %s\n", node->fname);
        printf("  add rsp, %d\n", (node->argc - 6) * 8);
        printf("  push  rax\n");
        printf("  jmp .Lendalign%d\n", jflag);
        printf(".Lalign%d:\n", jflag);
        printf("  sub rsp, 8\n");
        if (arg->val != NULL)
        {
          gen_arg(arg, 0, node->argc);
        }
        printf("  mov rax, 0\n");
        printf("  call  %s\n", node->fname);
        printf("  add rsp, %d\n", (node->argc - 6) * 8 + 8);
        printf("  push  rax\n");
        printf(".Lendalign%d:\n", jflag);
      }
      else
      {
        printf("  mov rax, rsp\n");
        printf("  and rax, 0x0f\n");
        printf("  cmp rax, 0\n");
        printf("  je  .Lalign%d\n", jflag);
        if (arg->val != NULL)
        {
          gen_arg(arg, 0, node->argc);
        }
        printf("  mov rax, 0\n");
        printf("  call  %s\n", node->fname);
        printf("  add rsp, %d\n", (node->argc - 6) * 8);
        printf("  push  rax\n");
        printf("  jmp .Lendalign%d\n", jflag);
        printf(".Lalign%d:\n", jflag);
        printf("  sub rsp,8\n");
        if (arg->val != NULL)
        {
          gen_arg(arg, 0, node->argc);
        }
        printf("  mov rax, 0\n");
        printf("  call  %s\n", node->fname);
        printf("  add rsp, %d\n", (node->argc - 6) * 8 + 8);
        printf("  push  rax\n");
        printf(".Lendalign%d:\n", jflag);
      }
    }
    else
    {
      /*
      if (arg->val != NULL)
      {
        gen_arg(arg, 0, node->argc);
      }
      printf("  call  %s\n", node->fname);
      printf("  push  rax\n");
      */

      jflag = lcount;
      lcount++;

      printf("  mov rax, rsp\n");
      printf("  and rax, 0x0f\n");
      printf("  cmp rax, 0\n");
      printf("  jne  .Lalign%d\n", jflag);
      if (arg->val != NULL)
      {
        gen_arg(arg, 0, node->argc);
      }
      printf("  mov rax, 0\n");
      printf("  call  %s\n", node->fname);
      printf("  push  rax\n");
      printf("  jmp .Lendalign%d\n", jflag);
      printf(".Lalign%d:\n", jflag);
      printf("  sub rsp,8\n");
      if (arg->val != NULL)
      {
        gen_arg(arg, 0, node->argc);
      }
      printf("  mov rax, 0\n");
      printf("  call  %s\n", node->fname);
      printf("  add rsp, %d\n", 8);
      printf("  push  rax\n");
      printf(".Lendalign%d:\n", jflag);
    }
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}