#include "chibicc.h"

int main(int argc, char **argv)
{
  if (argc != 2)
    error("%s: invalid number of arguments", argv[0]);

  // Tokenize and parse.
  user_input = argv[1];
  locals = NULL;
  token = tokenize();
  program();

  // Print out the first half of assembly.
  printf(".intel_syntax noprefix\n");

  for (int i = 0; fn[i]; i++)
  {
    Node *f = fn[i];
    printf("  .global  %s\n", f->fname);
    printf("%s:\n", f->fname);
    // Prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    if (strncmp(f->fname, "main", 4) == 0)
    {
      printf("  sub rsp, %d\n", 256);
    }
    else
    {
      printf("  sub rsp, %d\n", 16);
    }

    int n = f->argc;
    Arg *arg = f->arg;
    while (n--)
    {
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d\n", arg->val->offset);
      switch (n)
      {
      case 0:
        printf("  mov [rax], edi\n");
        break;
      case 1:
        printf("  mov [rax], esi\n");
        break;
      case 2:
        printf("  mov [rax], edx\n");
        break;
      case 3:
        printf("  mov [rax], ecx\n");
        break;
      case 4:
        printf("  mov [rax], r8d\n");
        break;
      case 5:
        printf("  mov [rax], r9d\n");
        break;
      default:
        printf("  pop [rax]\n");
        break;
      }
      arg = arg->next;
    }

    for (int j = 0; code[i][j]; j++)
    {
      gen(code[i][j]);
      printf("  pop rax\n");
    } 
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }

  return 0;
}
