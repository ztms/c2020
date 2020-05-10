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
    tokenize(user_input);

    // 抽象構文木生成
    Expression* expression = syntax();

    // アセンブルコード出力
    assemble(expression);

    return 0;
}
