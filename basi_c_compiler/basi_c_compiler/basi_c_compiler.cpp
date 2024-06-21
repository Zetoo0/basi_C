#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <map>


struct Symbol {
    char symbol;
    const char* type;
};

struct Computing {
    const char* type;
    Symbol child1;
    Symbol child2;
};

struct Assigning {
    const char* type;
    Symbol child1;
    Symbol child2;
};

struct Symreferencing {
    char id;
    const char* type;
};

struct IntConsisting {
    const char* type;
    char id;
};

struct FloatConsisting {
    const char* type;
    char id;
};

typedef enum Token {
    FLOATDCL,
    INTDCL,
    PRINT,
    ID,
    ASSIGN,
    PLUS,
    MINUS,
    INUM,
    FNUM,
    BLANK,
    END
} Token;

struct Tokenazizer {
    Token type;
    char* value;
    int index;
};



struct Tokenazizer scandigit(const char* numstream, int index);
struct Tokenazizer scanner(const char** stream);
struct ASTNode* parseStatement();
struct ASTNode* parseExpression();
struct ASTNode* parseFactor();
struct ASTNode* parseProgram();
struct ASTNode* parseDeclaration();

struct Tokenazizer getNextToken();
struct Tokenazizer peekNextToken();

void printAST(ASTNode* node, int level);
void freeAST(ASTNode* node);

void visit(Symbol sym);
void enterSymbol(char name, const char* type);
const char* lookupSymbol(char name);
const char* consistent(Symbol c1, Symbol c2);
const char* generalize(const char* t1, const char* t2);
void convert(Symbol sym, const char* type);

void visit(Computing n);
void visit(Assigning n);
void visit(Symreferencing n);
void visit(IntConsisting n);
void visit(FloatConsisting n);

void generate(ASTNode* node);

int currTokenIndex;
int tokenCount;
struct Tokenazizer* tokens;
std::map<char, const char*> symTable;

int main() {
    const char* bla = "i g = 1 + 1; f b = g - 2 ;";  // Example input
    struct Tokenazizer t;
    const char* ptr = bla;

    struct Tokenazizer tokenArr[100];
    int i = 0;

    while (*ptr != '\0') {
        printf("I value: %d", i);
        tokenArr[i] = scanner(&ptr);
        //t = scanner(&ptr);
        if (tokenArr[i].type != BLANK) {
            printf("Token Type: %d, Value: %s\n", tokenArr[i].type, tokenArr[i].value);
            /// free(t.value);  // Free the allocated memory
            i++;
        }
    }

    tokens = tokenArr;
    tokenCount = i;
    currTokenIndex = 0;
    printf("Tokens count: %d", tokenCount);
    ASTNode* program = parseProgram();
    printf("\n");
    printAST(program, 0);

    generate(program);

    freeAST(program);
    for (int j = 0; j < tokenCount; j++) {
        free(tokens[j].value);
    }

    return 0;
}

//                  SCANNER
struct Tokenazizer scandigit(const char* numstream) {
    int i = 0;
    struct Tokenazizer token;
    int start = 0; // Mark the start of the number

    // Determine length of the numeric part
    while (isdigit(numstream[i])) {
        i++;
    }
    if (numstream[i] == '.') {
        i++;
        while (isdigit(numstream[i])) {
            i++;
        }
        token.type = FNUM;
    }
    else {
        token.type = INUM;
    }

    // Allocate memory and copy the substring
    token.value = (char*)malloc((i - start + 1) * sizeof(char));
    if (token.value != NULL) {
        strncpy_s(token.value, (i - start + 1), numstream, i - start);
        token.value[i - start] = '\0'; // Null-terminate the string
    }

    return token;
}

struct Tokenazizer scanner(const char** stream_ptr) {
    struct Tokenazizer ans;
    ans.value = NULL;  // Initialize value to NULL
    const char* stream = *stream_ptr;
    int i = 0;

    while (stream[i] == ' ') i++;

    if (stream[i] == ';') {
        ans.type = END;
        ans.value = _strdup(";");
        i++;
    }
    else if (isdigit(stream[i])) {
        ans = scandigit(&stream[i]);
        i += strlen(ans.value); // Advance the pointer by the length of the token value
    }
    else {
        char ch = stream[i++];
        if (isalpha(ch)) {
            switch (ch) {
            case 'f':
                ans.type = FLOATDCL;
                ans.value = _strdup("f");
                break;
            case 'i':
                ans.type = INTDCL;
                ans.value = _strdup("i");
                break;
            case 'p':
                ans.type = PRINT;
                ans.value = _strdup("p");
                break;
            default:
                ans.type = ID;
                ans.value = (char*)malloc(2 * sizeof(char));  // Allocate memory for value
                if (ans.value != NULL) {
                    ans.value[0] = ch;
                    ans.value[1] = '\0';  // Null-terminate the string
                }
                break;
            }
        }
        else {
            switch (ch) {
            case '=':
                ans.type = ASSIGN;
                ans.value = _strdup("=");
                break;
            case '+':
                ans.type = PLUS;
                ans.value = _strdup("+");
                break;
            case '-':
                ans.type = MINUS;
                ans.value = _strdup("-");
                break;
            default:
                ans.type = BLANK;  // Default case for unknown characters
                ans.value = (char*)malloc(2 * sizeof(char));
                if (ans.value != NULL) {
                    ans.value[0] = ch;
                    ans.value[1] = '\0';  // Null-terminate the string
                }
                break;
            }
        }
    }

    *stream_ptr += i; // Advance the original stream pointer
    return ans;
}//---------------------------------------------------------------------------

//                              PARSER
//                               AST

typedef enum {
    NODE_NUMBER,
    NODE_OPERATOR,
    NODE_ID,
    NODE_ASSIGN,
    NODE_EXPRESSION,
    NODE_TERM,
    NODE_FACTOR,
    NODE_PROGRAM,
    NODE_DECL
}NodeType;

typedef struct ASTNode {
    NodeType type;
    Token token;
    char* value;
    ASTNode* left;
    ASTNode* right;
    ASTNode* next;
}ASTNode;


ASTNode* createnode(NodeType type, char* value, Token token) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->token = token;
    node->value = value ? _strdup(value) : NULL;
    node->type = type;
    node->left = NULL;
    node->right = NULL;
    node->next = NULL;
    return node;
}

struct Tokenazizer getNextToken() {
    printf("Gettin next token...\n");
    if (currTokenIndex < tokenCount) {
        return tokens[currTokenIndex++];
    }
    struct Tokenazizer token = { END,NULL,0 };
    return token;
}

struct Tokenazizer peekNextToken() {
    printf("Peeking next token...\n");
    if (currTokenIndex < tokenCount) {
        return tokens[currTokenIndex];
    }
    struct Tokenazizer token = { END,NULL,0 };
    return token;
}

struct ASTNode* parseFactor() {
    struct Tokenazizer token = getNextToken();
    if (token.type == INUM || token.type == FNUM || token.type == ID) {
        return createnode(NODE_FACTOR, token.value, token.type);
    }
    return NULL;
}

struct ASTNode* parseTerm() {
    struct ASTNode* node = parseFactor();
    struct Tokenazizer token = peekNextToken();
    while (token.type == PLUS || token.type == MINUS) {
        getNextToken();
        ASTNode* n_node = createnode(NODE_TERM, token.value, token.type);
        n_node->left = node;
        n_node->right = parseFactor();
        node = n_node;
        token = peekNextToken();
    }
    return node;
}

struct ASTNode* parseExpression() {
    return parseTerm();
}

struct ASTNode* parseDeclaration() {
    struct Tokenazizer declaration = getNextToken();
    struct Tokenazizer id = getNextToken();
    if (id.type != ID) {
        return NULL;
    }
    ASTNode* id_node = createnode(NODE_ID, id.value, id.type);
    printf("Declaration node type: %d\n", declaration.type);
    ASTNode* decl_node = createnode(NODE_DECL, declaration.value, declaration.type);
    decl_node->left = id_node;
    return decl_node;

}

struct ASTNode* parseStatement() {
    printf("parsing statement....");
    struct Tokenazizer t_decl = peekNextToken();
    printf("tooken type: %d", t_decl.type);
    if (t_decl.type == FLOATDCL || t_decl.type == INTDCL) {
        return parseDeclaration();
    }
    struct Tokenazizer t_id = getNextToken();
    if (t_id.type != ID) {
        return NULL;
    }
    struct Tokenazizer t_assign = getNextToken();
    if (t_assign.type != ASSIGN) {
        return NULL;
    }
    ASTNode* left = createnode(NODE_ID, t_id.value, t_id.type);
    ASTNode* right = parseExpression();
    ASTNode* node = createnode(NODE_ASSIGN, t_assign.value, t_assign.type);
    node->left = left;
    node->right = right;

    return node;
}

struct ASTNode* parseProgram() {
    ASTNode* head = createnode(NODE_PROGRAM, NULL, BLANK);
    ASTNode* curr = head;
    while (currTokenIndex < tokenCount) {
        ASTNode* stmt = parseStatement();
        if (stmt != NULL) {
            curr->next = stmt;
            curr = stmt;
        }
        Tokenazizer token = peekNextToken();
        if (token.type == END) {
            getNextToken();
        }
    }
    head->left = parseStatement();
    return head->next;
}

void freeAST(ASTNode* node) {
    if (node == NULL) return;
    freeAST(node->left);
    freeAST(node->right);
    if (node->value != NULL) free(node->value);
    free(node);
}

void printAST(ASTNode* node, int level) {
    while (node != NULL) {
        for (int i = 0; i < level; i++) printf("  ");
        if (node->value != NULL) {
            printf("%s\n", node->value);
        }
        else {
            printf("%d\n", node->type);
        }
        printAST(node->left, level + 1);
        printAST(node->right, level + 1);
        node = node->next;
    }
}

//              SEMANTIC ANALYSIS

void visit(Symbol sym) {
    if (sym.type == "floatdcl") enterSymbol(sym.symbol, "float");
    else enterSymbol(sym.symbol, "integer");
}

void enterSymbol(char name, const char* type) {
    if (symTable[name] == NULL) symTable[name] = type;
    else printf("duplicate declaration");
}

const char* lookupSymbol(char name) {
    return(symTable[name]);
}

//visit methods


void visit(Computing n) {
    n.type = consistent(n.child1, n.child2);
}

void visit(Assigning n) {
    convert(n.child2, n.child1.type);
}

void visit(Symreferencing n) {
    n.type = lookupSymbol(n.id);
}

void visit(IntConsisting n) {
    n.type = "integer";
}

void visit(FloatConsisting n) {
    n.type = "float";
}

const char* consistent(Symbol c1, Symbol c2) {
    const char* m = generalize(c1.type, c2.type);
    convert(c1, m);
    convert(c2, m);
    return m;
}

const char* generalize(const char* t1, const char* t2) {
    const char* ans;
    if (t1 == "float" || t2 == "float") ans = "float";
    else ans = "integer";
    return ans;
}

void convert(Symbol sym, const char* type) {
    if (sym.type == "float" && type == "integer") printf("Illegal type conversion");
    else {
        if (sym.type == "integer" && type == "float") {
            symTable[sym.symbol] = "float";
        }
    }
}

//                  CODE GENERATION

void generate(ASTNode* node) {
    while (node != NULL) {
        switch (node->type) {
        case NODE_PROGRAM:
            generate(node->left);
            break;
        case NODE_DECL:
            printf("DECL %s\n", node->left->value);
            enterSymbol(node->left->value[0], node->token == FLOATDCL ? "float" : "integer");
            break;
        case NODE_ASSIGN:
            generate(node->right);
            printf("STORE %s\n", node->left->value);
            break;
        case NODE_OPERATOR:
            generate(node->left);
            generate(node->right);
            printf("%s\n", node->value);
            break;
        case NODE_NUMBER:
            if (node->token == ID) {
                printf("LOAD %s\n", node->value);
            }
            else {
                printf("PUSH %s\n", node->value);
            }
            break;
        default:
            break;
        }
        node = node->next;
    }
}