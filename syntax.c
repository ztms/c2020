#include "cmz.h"

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

// 抽象構文木生成
// ------------------------------------------------------------
// syntax     = expression*
// expression = assign ";"
// assign     = equal ("=" assign)?
// equal      = less ("==" less | "!=" less)*
// less       = add ("<" add | "<=" add | ">" add | ">=" add)*
// add        = mul ("+" mul | "-" mul)*
// mul        = unary ("*" unary | "/" unary)*
// unary      = ("+" | "-")? unary | primary
// primary    = num | ident | "(" assign ")"
// ------------------------------------------------------------
Node* expression();
Node* assign();
Node* equal();
Node* less();
Node* add();
Node* mul();
Node* unary();
Node* primary();

// syntax = expression*
Expression* syntax()
{
    Expression head;
    Expression* ep = &head;

    head.next = NULL;

    while( !token_eof() )
    {
        Expression* ex = calloc(1, sizeof(Expression));
        ex->node = expression();
        ep->next = ex;
        ep = ex;
    }
    return head.next;
}

// expression = assign ";"
Node* expression()
{
    Node *node = assign();
    token_expect(";");
    return node;
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

// primary = num | ident | "(" assign ")"
Node* primary()
{
    // ( expr )
    if( token_consume("(") )
    {
        Node* node = assign();
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

