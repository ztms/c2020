#include "cmz.h"

// ---------------------------------------------------------
// トークナイザ
// ---------------------------------------------------------

// 現在着目しているトークン
Token* token;

// エラー箇所を示して終了
void token_error( char* at, char* fmt, ... )
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

// 新しいトークンを生成
Token* new_token( TokenKind kind, Token* current, char* str, int len )
{
    Token* token = calloc(1, sizeof(Token));
    token->kind = kind;
    token->str = str;
    token->len = len;
    current->next = token;
    return token;
}

// 文字列の先頭が予約語トークンだった場合、その文字列長を返し、それ以外は0を返す
int find_token_reserved( char* p )
{
    // 2文字
    if( memcmp("==", p, 2)==0 ) return 2;
    if( memcmp("!=", p, 2)==0 ) return 2;
    if( memcmp("<=", p, 2)==0 ) return 2;
    if( memcmp(">=", p, 2)==0 ) return 2;
    // 1文字
    if( ispunct(*p) ) return 1;
    return 0;
}

// 文字列をトークナイズして返す
Token* tokenize( char* p )
{
    Token head;
    Token* current = &head;

    head.next = NULL;

    while( *p )
    {
        if( isspace(*p) )
        {
            p++;
            continue;
        }
        // 予約語
        int len = find_token_reserved(p);
        if( len )
        {
            current = new_token(TOKEN_RESERVED, current, p, len);
            p += len;
            continue;
        }
        // 識別子
        if( 'a' <= *p && *p <= 'z' )
        {
            current = new_token(TOKEN_IDENT, current, p, 1);
            p++;
            continue;
        }
        // 整数
        if( isdigit(*p) )
        {
            char* q = p;
            int value = strtol(p, &p, 10);
            int len = p - q;
            current = new_token(TOKEN_NUMBER, current, p, len);
            current->value = value;
            continue;
        }
        token_error(p, "トークナイズできません");
    }

    new_token(TOKEN_EOF, current, p, 0);

    token = head.next;
    return token;
}

// 現在のトークンが終端かどうか
bool token_eof()
{
    return token->kind == TOKEN_EOF;
}

// 現在のトークンが期待してる記号の時はトークンを１つ進め、進んだかどうかを返却
bool token_consume( char* op )
{
    if( token->kind != TOKEN_RESERVED ) return false;
    if( token->len != strlen(op) ) return false;
    if( memcmp(op, token->str, token->len) ) return false;
    token = token->next;
    return true;
}

// 現在のトークンが期待してる記号の時はトークンを１つ進め、それ以外はエラー終了
bool token_expect( char* op )
{
    if( !token_consume(op) ) token_error(token->str, "'%s'ではありません", op);
    return true;
}

// 現在のトークンが数値の場合トークンを１つ進めてその数値を返し、それ以外はエラー
int token_expect_number()
{
    if( token->kind != TOKEN_NUMBER ) token_error(token->str, "数ではありません");
    int value = token->value;
    token = token->next;
    return value;
}

// 現在のトークンが識別子ならトークンを１つ進めてその文字列を返し、それ以外はエラー
char* token_consume_ident()
{
    if( token->kind != TOKEN_IDENT ) return NULL;
    char* ident = token->str;
    token = token->next;
    return ident;
}

