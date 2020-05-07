#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
    TK_RESERVED, // 記号
    TK_NUMBER,   // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token* next;    // 次の入力トークン
    int value;      // kindがTK_NUMBERの場合、その数値
    char* str;      // トークン文字列
};

// 入力文字列
char* user_input;

// 現在着目しているトークン
Token* token;

// エラーを報告するための関数
void error( char* fmt, ... )
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラー箇所を指して報告する関数
void error_at( char* at, char* fmt, ... )
{
    va_list ap;
    int N = at - user_input;
    va_start(ap, fmt);
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", N, ""); // N個の空白文字を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 現在のトークンが期待してる記号の時はトークンを１つ進め、それ以外はエラー
bool token_consume( char op )
{
    if( token->kind != TK_RESERVED || token->str[0] != op ) return false;
    token = token->next;
    return true;
}

// 現在のトークンが期待してる記号の時はトークンを１つ進め、それ以外はエラー
bool token_expect( char op )
{
    if( token->kind != TK_RESERVED || token->str[0] != op ) error_at(token->str, "'%c'ではありません",op);
    token = token->next;
}

// 現在のトークンが数値の場合トークンを１つ進めてその数値を返し、それ以外はエラー
int token_expect_number()
{
    if( token->kind != TK_NUMBER ) error_at(token->str, "数ではありません");
    int value = token->value;
    token = token->next;
    return value;
}

// 現在のトークンが終端かどうか
bool token_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを生成
Token* new_token( TokenKind kind, Token* current, char* str )
{
    Token* token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->str = str;
    current->next = token;
    return token;
}

// 文字列pをトークナイズして返す
Token* tokenize( char* p )
{
    Token head;
    head.next = NULL;
    Token* current = &head;
    while( *p )
    {
        if( isspace(*p) )
        {
            p++;
            continue;
        }
        if( *p == '+' || *p == '-' )
        {
            current = new_token(TK_RESERVED, current, p++);
            continue;
        }
        if( isdigit(*p) )
        {
            current = new_token(TK_NUMBER, current, p);
            current->value = strtol(p, &p, 10);
            continue;
        }
        error("トークナイズできません");
    }
    new_token(TK_EOF, current, p);
    return head.next;
}

int main( int argc, char** argv )
{
    if( argc != 2 ) error("引数の個数が正しくありません");

    user_input = argv[1];
    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  mov rax, %d\n", token_expect_number());

    while( !token_eof() )
    {
        if( token_consume('+') )
        {
            printf("  add rax, %d\n", token_expect_number());
            continue;
        }
        token_expect('-');
        printf("  sub rax, %d\n", token_expect_number());
    }

    printf("  ret\n");
    return 0;
}
