#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// ---------------------------------------------------------
// トークナイザ
// ---------------------------------------------------------

// トークンの種類
typedef enum TokenKind TokenKind;
enum TokenKind
{
    TOKEN_RESERVED, // 記号
    TOKEN_NUMBER,   // 整数トークン
    TOKEN_EOF,      // 入力の終わりを表すトークン
};

// トークン型
typedef struct Token Token;
struct Token
{
    TokenKind kind; // トークンの型
    Token* next;    // 次の入力トークン
    int value;      // kindがTOKEN_NUMBERの場合、その数値
    char* str;      // トークン文字列
};

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

// 現在のトークンが期待してる記号の時はトークンを１つ進め、それ以外はエラー
bool token_consume( char op )
{
    if( token->kind != TOKEN_RESERVED || token->str[0] != op ) return false;
    token = token->next;
    return true;
}

// 現在のトークンが期待してる記号の時はトークンを１つ進め、それ以外はエラー
bool token_expect( char op )
{
    if( token->kind != TOKEN_RESERVED || token->str[0] != op ) token_error(token->str, "'%c'ではありません", op);
    token = token->next;
}

// 現在のトークンが数値の場合トークンを１つ進めてその数値を返し、それ以外はエラー
int token_expect_number()
{
    if( token->kind != TOKEN_NUMBER ) token_error(token->str, "数ではありません");
    int value = token->value;
    token = token->next;
    return value;
}

// 現在のトークンが終端かどうか
bool token_eof()
{
    return token->kind == TOKEN_EOF;
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
        if( strchr("+-*/()", *p) )
        {
            current = new_token(TOKEN_RESERVED, current, p++);
            continue;
        }
        if( isdigit(*p) )
        {
            current = new_token(TOKEN_NUMBER, current, p);
            current->value = strtol(p, &p, 10);
            continue;
        }
        token_error(p, "トークナイズできません");
    }

    new_token(TOKEN_EOF, current, p);

    return head.next;
}

// ---------------------------------------------------------
// 再帰下降構文解析
// ---------------------------------------------------------

// 抽象構文木ノード種類
typedef enum NodeKind NodeKind;
enum NodeKind
{
    NODE_NUM, // 整数
    NODE_ADD, // +
    NODE_SUB, // -
    NODE_MUL, // *
    NODE_DIV, // /
};

// 抽象構文木ノード型
typedef struct Node Node;
struct Node
{
    NodeKind kind; // 種類
    Node* lhs;     // 左辺
    Node* rhs;     // 右辺
    int value;     // kindがNODE_NUMの場合の数値
};

// 抽象構文木ノード生成
Node* new_node( NodeKind kind, Node* lhs, Node* rhs )
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 抽象構文木ノード(数値)生成
Node* new_node_num( int value )
{
    Node* node = calloc(1, sizeof(Node));
    node->kind = NODE_NUM;
    node->value = value;
    return node;
}

// 抽象構文木生成
// -----------------------------------------------
// expr    = mul ("+" mul | "-" mul)*
// mul     = primary ("*" primary | "/" primary)*
// primary = num | "(" expr ")"
// -----------------------------------------------
Node* expr();
Node* mul();
Node* primary();

Node* expr()
{
    Node *node = mul();

    for( ;; )
    {
        if( token_consume('+') )
        {
            node = new_node(NODE_ADD, node, mul());
            continue;
        }
        if( token_consume('-') )
        {
            node = new_node(NODE_SUB, node, mul());
            continue;
        }
        return node;
    }
}

Node* mul()
{
    Node *node = primary();

    for( ;; )
    {
        if( token_consume('*') )
        {
            node = new_node(NODE_MUL, node, primary());
            continue;
        }
        if( token_consume('/') )
        {
            node = new_node(NODE_DIV, node, primary());
            continue;
        }
        return node;
    }
}

Node* primary()
{
    // ( expr )
    if( token_consume('(') )
    {
        Node* node = expr();
        token_expect(')');
        return node;
    }
    // num
    return new_node_num(token_expect_number());
}

// 抽象構文木を再帰下降でコード生成
void generate( Node* node )
{
    if( node->kind == NODE_NUM )
    {
        printf("  push %d\n", node->value);
        return;
    }

    generate(node->lhs);
    generate(node->rhs);

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
    }
    printf("  push rax\n");
}

int main( int argc, char** argv )
{
    if( argc != 2 ) error("引数の個数が正しくありません");

    user_input = argv[1];

    // トークナイズ
    token = tokenize(user_input);

    // 抽象構文木生成
    Node* node = expr();

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
