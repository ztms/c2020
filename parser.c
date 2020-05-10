#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    return head.next;
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

char* token_consume_ident()
{
    if( token->kind != TOKEN_IDENT ) return NULL;
    char* ident = token->str;
    token = token->next;
    return ident;
}

// ---------------------------------------------------------
// 抽象構文木
// ---------------------------------------------------------

// 木ノード生成
Node* new_node( NodeKind kind, Node* lhs, Node* rhs )
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 木ノード(数値)生成
Node* new_node_num( int value )
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_NUM;
    node->value = value;
    return node;
}

// 木ノード(ローカル変数)生成
Node* new_node_lvar( char* ident )
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_LVAR;
    node->offset = (ident[0] - 'a' + 1) * 8;
    return node;
}

// program = statement*
Statement* program()
{
    Statement head;
    Statement* current = &head;

    head.next = NULL;

    while( !token_eof() )
    {
        Statement* st = calloc(1, sizeof(Statement));
        st->node = statement();
        current->next = st;
        current = st;
    }
    return head.next;
}

// statement = expression ";"
Node* statement()
{
    Node *node = expression();
    token_expect(";");
    return node;
}

// expression = equal
Node* expression()
{
    return assign();
}

// assign = equal ("=" assign)?
Node* assign()
{
    Node *node = equal();

    if( token_consume("=") )
    {
        node = new_node(NODE_ASSIGN, node, assign());
    }
    return node;
}

// equal = less ("==" less | "!=" less)*
Node* equal()
{
    Node *node = less();

    for( ;; )
    {
        if( token_consume("==") )
        {
            node = new_node(NODE_EQ, node, less());
            continue;
        }
        if( token_consume("!=") )
        {
            node = new_node(NODE_NE, node, less());
            continue;
        }
        return node;
    }
}

// less = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* less()
{
    Node *node = add();

    for( ;; )
    {
        if( token_consume("<") )
        {
            node = new_node(NODE_LT, node, add());
            continue;
        }
        if( token_consume("<=") )
        {
            node = new_node(NODE_LE, node, add());
            continue;
        }
        if( token_consume(">") )
        {
            node = new_node(NODE_LT, add(), node);
            continue;
        }
        if( token_consume(">=") )
        {
            node = new_node(NODE_LE, add(), node);
            continue;
        }
        return node;
    }
}

// add = mul ("+" mul | "-" mul)*
Node* add()
{
    Node *node = mul();

    for( ;; )
    {
        if( token_consume("+") )
        {
            node = new_node(NODE_ADD, node, mul());
            continue;
        }
        if( token_consume("-") )
        {
            node = new_node(NODE_SUB, node, mul());
            continue;
        }
        return node;
    }
}

// mul = unary ("*" unary | "/" unary)*
Node* mul()
{
    Node *node = unary();

    for( ;; )
    {
        if( token_consume("*") )
        {
            node = new_node(NODE_MUL, node, unary());
            continue;
        }
        if( token_consume("/") )
        {
            node = new_node(NODE_DIV, node, unary());
            continue;
        }
        return node;
    }
}

// unary = ("+" | "-")? unary | primary
Node* unary()
{
    // +x は x に置き換え
    if( token_consume("+") ) return unary();
    // -x は 0-x に置き換え
    if( token_consume("-") ) return new_node(NODE_SUB, new_node_num(0), unary());
    // x
    return primary();
}

// primary = num | ident | "(" expression ")"
Node* primary()
{
    // ( expr )
    if( token_consume("(") )
    {
        Node* node = expression();
        token_expect(")");
        return node;
    }
    // ident
    char* ident = token_consume_ident();
    if( ident )
    {
        return new_node_lvar(ident);
    }
    // num
    return new_node_num(token_expect_number());
}

