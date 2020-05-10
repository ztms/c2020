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
    TOKEN_RESERVED, // 予約語
    TOKEN_NUMBER,   // 整数
    TOKEN_EOF,      // 終端
};

// トークン型
typedef struct Token Token;
struct Token
{
    TokenKind kind; // 種類
    Token* next;    // 次のトークン
    char* str;      // トークン文字列
    int len;        // トークン文字列長
    int value;      // 整数トークンの場合、その数値
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
    if( !token_consume(op) ) token_error(token->str, "'%c'ではありません", op);
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

// 現在のトークンが終端かどうか
bool token_eof()
{
    return token->kind == TOKEN_EOF;
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
    if( strchr("+-*/()<>", *p) ) return 1;
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
        int len = find_token_reserved(p);
        if( len )
        {
            current = new_token(TOKEN_RESERVED, current, p, len);
            p += len;
            continue;
        }
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
    NODE_EQ,  // ==
    NODE_NE,  // !=
    NODE_LT,  // <
    NODE_LE,  // <=
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
// ------------------------------------------------------------
// expression = equal
// equal      = less ("==" less | "!=" less)*
// less       = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? unary | primary
// primary    = num | "(" expression ")"
// ------------------------------------------------------------
Node* expression();
Node* equal();
Node* less();
Node* add();
Node* mul();
Node* unary();
Node* primary();

// expression = equal
Node* expression()
{
    return equal();
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

// primary = num | "(" expr ")"
Node* primary()
{
    // ( expr )
    if( token_consume("(") )
    {
        Node* node = expression();
        token_expect(")");
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
