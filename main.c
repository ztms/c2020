#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "cmz.h"

// 入力文字列
char* user_input;

// エラー終了
void error( char* fmt, ... )
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

int main( int argc, char** argv )
{
    if( argc != 2 ) error("引数の個数が正しくありません");

    user_input = argv[1];

    // トークナイズ
    token = tokenize(user_input);

    // 抽象構文木生成
    Statement* statement = program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // プロローグ
    // 変数26個分の領域を確保
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, 208\n");

    // 再帰下降でコード生成
    for( Statement* st=statement; st; st=st->next )
    {
        codegen(st->node);
        // 式の評価結果としてスタックに１つ値が残っている
        // スタックが溢れないようにポップしておく
        printf("  pop rax\n");
    }

    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return 0;
}
