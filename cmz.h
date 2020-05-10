#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 入力文字列
extern char* user_input;

// エラー終了
void error( char* fmt, ... );

// ---------------------------------------------------------
// トークナイズ
// ---------------------------------------------------------

// トークンの種類
typedef enum TokenKind TokenKind;
enum TokenKind
{
    TOKEN_EOF,      // 終端
    TOKEN_RESERVED, // 予約語
    TOKEN_NUMBER,   // 整数
    TOKEN_IDENT,    // 識別子
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

// 現在のトークン
extern Token* token;

// 文字列をトークナイズして返す
Token* tokenize( char* p );
// 現在のトークンが終端かどうか
bool token_eof();
// 現在のトークンが期待してる記号の時はトークンを１つ進め、進んだかどうかを返却
bool token_consume( char* op );
// 現在のトークンが期待してる記号の時はトークンを１つ進め、それ以外はエラー終了
bool token_expect( char* op );
// 現在のトークンが数値の場合トークンを１つ進めてその数値を返し、それ以外はエラー
int token_expect_number();
// 現在のトークンが識別子ならトークンを１つ進めてその文字列を返し、それ以外はエラー
char* token_consume_ident();

// ---------------------------------------------------------
// 抽象構文木
// ---------------------------------------------------------

// 木ノード種類
typedef enum NodeKind NodeKind;
enum NodeKind
{
    NODE_NUM,    // 整数
    NODE_ADD,    // +
    NODE_SUB,    // -
    NODE_MUL,    // *
    NODE_DIV,    // /
    NODE_EQ,     // ==
    NODE_NE,     // !=
    NODE_LT,     // <
    NODE_LE,     // <=
    NODE_ASSIGN, // = 代入
    NODE_LVAR,   // ローカル変数
};

// 木ノード型
typedef struct Node Node;
struct Node
{
    NodeKind kind; // 種類
    Node* lhs;     // 左辺
    Node* rhs;     // 右辺
    int value;     // 種類が整数の場合の数値
    int offset;    // 種類がローカル変数の場合に使う
};

// 式リスト
typedef struct Expression Expression;
struct Expression
{
    Expression* next;
    Node* node;
};

// 抽象構文木生成
Expression* syntax();

// ---------------------------------------------------------
// 再帰下降構文解析
// ---------------------------------------------------------

// 全体コード生成
void assemble( Expression* expression );

