
// 入力文字列
extern char* user_input;

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
extern Token* token;

// 文字列をトークナイズして返す
Token* tokenize( char* p );

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

// 抽象構文木を再帰下降でコード生成
void generate( Node* node );
