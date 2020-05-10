#include <stdio.h>
#include "cmz.h"

// ---------------------------------------------------
// 抽象構文木を再帰下降でコード生成
// ---------------------------------------------------

void codegen_lval( Node* node )
{
    if( node->kind != NODE_LVAR ) error("代入の左辺が変数ではありません");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void codegen( Node* node )
{
    switch( node->kind )
    {
        case NODE_NUM:
            printf("  push %d\n", node->value);
            return;
        case NODE_LVAR:
            codegen_lval(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case NODE_ASSIGN:
            codegen_lval(node->lhs);
            codegen(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
    }

    codegen(node->lhs);
    codegen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch( node->kind )
    {
        case NODE_ADD:
            printf("  add rax, rdi\n");
            break;
        case NODE_SUB:
            printf("  sub rax, rdi\n");
            break;
        case NODE_MUL:
            printf("  imul rax, rdi\n");
            break;
        case NODE_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
        case NODE_EQ:
            printf("  cmp rax, rdi\n");
            printf("  sete al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_NE:
            printf("  cmp rax, rdi\n");
            printf("  setne al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_LT:
            printf("  cmp rax, rdi\n");
            printf("  setl al\n");
            printf("  movzb rax, al\n");
            break;
        case NODE_LE:
            printf("  cmp rax, rdi\n");
            printf("  setle al\n");
            printf("  movzb rax, al\n");
            break;
    }
    printf("  push rax\n");
}

