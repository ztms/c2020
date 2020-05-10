#include "cmz.h"

// ---------------------------------------------------
// 抽象構文木を再帰下降でアセンブルコード生成
// ---------------------------------------------------

// 代入左辺の特殊コード
void assemble_lvar( Node* node )
{
    if( node->kind != NODE_LVAR ) error("代入の左辺が変数ではありません");

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

// 抽象構文木ひとつコード生成
void assemble_tree( Node* node )
{
    switch( node->kind )
    {
        case NODE_NUM:
            printf("  push %d\n", node->value);
            return;
        case NODE_LVAR:
            assemble_lvar(node);
            printf("  pop rax\n");
            printf("  mov rax, [rax]\n");
            printf("  push rax\n");
            return;
        case NODE_ASSIGN:
            assemble_lvar(node->lhs);
            assemble_tree(node->rhs);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            printf("  push rdi\n");
            return;
    }

    assemble_tree(node->lhs);
    assemble_tree(node->rhs);

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

// 全体コード生成
void assemble( Expression* expression )
{
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // コード生成
    for( Expression* ex=expression; ex; ex=ex->next )
    {
        assemble_tree(ex->node);
        // 式の評価結果としてスタックに１つ値が残っている
        // スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}
