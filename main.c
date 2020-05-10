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
    Node* node = expression();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 再帰下降でコード生成
    generate(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
