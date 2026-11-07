%define api.prefix {oxinfra}
%define api.pure full
%locations
%parse-param {oxente::ParserContext* ctx}
%lex-param {oxente::ParserContext* ctx}

%code requires {
#include <string>
#include <memory>
#include <vector>
#include "ast/AstNode.hpp"
#include "ast/ExprNodes.hpp"
#include "ast/StmtNodes.hpp"
#include "common/TypeKind.hpp"
#include "common/Operators.hpp"
#include "common/SourceLocation.hpp"
#include "common/DiagnosticEngine.hpp"

namespace oxente {
  class Scanner;
  struct ParserContext {
    Scanner* scanner;
    DiagnosticEngine* engine;
    std::unique_ptr<ProgramNode> raiz;
  };
}
}

%code {
#include "scanner.hpp"
#include <stdexcept>

int oxinfralex(OXINFRASTYPE* yylval, OXINFRALTYPE* yylloc, oxente::ParserContext* ctx) {
    return ctx->scanner->proximo_significativo(yylval, yylloc);
}

void oxinfraerror(OXINFRALTYPE* loc, oxente::ParserContext* ctx, const char* msg) {
    oxente::SourceLocation sloc{loc ? loc->first_line : 0, loc ? loc->first_column : 0};
    if (ctx && ctx->engine) {
        ctx->engine->emitir(oxente::Stage::Sintatico, sloc, "sintaxe_invalida", msg);
    }
}
}

%union {
    std::string* texto;
    TypeKind tipo_dado;
    ExprNode* expr;
    StmtNode* stmt;
    BlockStmtNode* bloco;
    std::vector<std::unique_ptr<ExprNode>>* lista_expr;
}

%token INDISPONIVEL
%token <texto> IDENT INTEIRO REAL TEXTO NL ESPACO
%token T_INTEIRO TRONXO PAPO RESENHA BOTA BORA ARREMATA
%token VAI_QUE SE_NUM_FOR POIS_PRONTO NUM_PARA ARRODEIA ATE CANSO SIMBORA
%token EVERA PALA AI_DENTO E OU DIGA_AI DESEMBUCHA ABS
%token APERREIO TOMA SEM_FUTURO TENDEU ARRIBA
%token MAIS MENOS VEZES DIVISAO DIV_INTEIRA RESTO POTENCIA
%token MAIOR MENOR MAIOR_IGUAL MENOR_IGUAL IGUAL DIFERENTE
%token ABRE_PAR FECHA_PAR PONTO_VIRGULA

%destructor { delete $$; } <texto>
%destructor { delete $$; } <expr>
%destructor { delete $$; } <stmt>
%destructor { delete $$; } <bloco>
%destructor { delete $$; } <lista_expr>

%type <tipo_dado> tipo
%type <bloco> bloco_instrucoes
%type <stmt> instrucao declaracao atribuicao entrada saida
%type <lista_expr> argumentos
%type <expr> expressao disjuncao conjuncao negacao comparacao soma produto unario potencia primario

%%

programa:
    bloco_instrucoes {
        ctx->raiz = std::make_unique<ProgramNode>(std::unique_ptr<BlockStmtNode>($1));
    }
  ;

linhas_iniciais:
    NL { delete $1; }
  | linhas_iniciais NL { delete $2; }
  ;

bloco_instrucoes:
    bloco_instrucoes NL {
        delete $2;
        $$ = $1;
    }
  | bloco_instrucoes instrucao {
        $$ = $1;
        if ($2) $$->add(std::unique_ptr<StmtNode>($2));
    }
  | linhas_iniciais instrucao {
        $$ = new BlockStmtNode();
        if ($2) $$->add(std::unique_ptr<StmtNode>($2));
    }
  | instrucao {
        $$ = new BlockStmtNode();
        if ($1) $$->add(std::unique_ptr<StmtNode>($1));
    }
  ;

instrucao:
    declaracao   { $$ = $1; }
  | atribuicao   { $$ = $1; }
  | entrada      { $$ = $1; }
  | saida        { $$ = $1; }
  ;

tipo:
    T_INTEIRO { $$ = TypeKind::Inteiro; }
  | TRONXO    { $$ = TypeKind::Tronxo; }
  | PAPO      { $$ = TypeKind::Papo; }
  | RESENHA   { $$ = TypeKind::Resenha; }
  ;

declaracao:
    tipo IDENT BOTA expressao {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new VarDeclStmtNode($1, *$2, std::unique_ptr<ExprNode>($4), loc);
        delete $2;
    }
  ;

atribuicao:
    IDENT BOTA expressao {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new AssignStmtNode(*$1, std::unique_ptr<ExprNode>($3), loc);
        delete $1;
    }
  ;

entrada:
    DIGA_AI ABRE_PAR IDENT FECHA_PAR {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new InputStmtNode(*$3, loc);
        delete $3;
    }
  ;

saida:
    DESEMBUCHA ABRE_PAR argumentos FECHA_PAR {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new OutputStmtNode(std::move(*$3), loc);
        delete $3;
    }
  ;

argumentos:
    expressao {
        $$ = new std::vector<std::unique_ptr<ExprNode>>();
        $$->push_back(std::unique_ptr<ExprNode>($1));
    }
  | argumentos PONTO_VIRGULA expressao {
        $$ = $1;
        $$->push_back(std::unique_ptr<ExprNode>($3));
    }
  ;

expressao:
    disjuncao { $$ = $1; }
  ;

disjuncao:
    disjuncao OU conjuncao {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Ou, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | conjuncao { $$ = $1; }
  ;

conjuncao:
    conjuncao E negacao {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::E, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | negacao { $$ = $1; }
  ;

negacao:
    AI_DENTO negacao {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new UnaryExprNode(UnaryOp::Nao, std::unique_ptr<ExprNode>($2), loc);
    }
  | comparacao { $$ = $1; }
  ;

comparacao:
    soma MAIOR soma {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Maior, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma MENOR soma {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Menor, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma MAIOR_IGUAL soma {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::MaiorIgual, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma MENOR_IGUAL soma {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::MenorIgual, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma IGUAL soma {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Igual, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma DIFERENTE soma {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Diferente, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma { $$ = $1; }
  ;

soma:
    soma MAIS produto {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Soma, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | soma MENOS produto {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Subtracao, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | produto { $$ = $1; }
  ;

produto:
    produto VEZES unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Multiplicacao, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | produto DIVISAO unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Divisao, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | produto DIV_INTEIRA unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::DivisaoInteira, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | produto RESTO unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Modulo, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | unario { $$ = $1; }
  ;

unario:
    MAIS unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new UnaryExprNode(UnaryOp::Positivo, std::unique_ptr<ExprNode>($2), loc);
    }
  | MENOS unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new UnaryExprNode(UnaryOp::Negativo, std::unique_ptr<ExprNode>($2), loc);
    }
  | potencia { $$ = $1; }
  ;

potencia:
    primario POTENCIA unario {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BinaryExprNode(BinaryOp::Potencia, std::unique_ptr<ExprNode>($1), std::unique_ptr<ExprNode>($3), loc);
    }
  | primario { $$ = $1; }
  ;

primario:
    INTEIRO {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new LiteralExprNode(std::stoi(*$1), TypeKind::Inteiro, loc);
        delete $1;
    }
  | REAL {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        std::string s = *$1;
        auto pos = s.find(',');
        if (pos != std::string::npos) s[pos] = '.';
        $$ = new LiteralExprNode(std::stod(s), TypeKind::Tronxo, loc);
        delete $1;
    }
  | EVERA {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new LiteralExprNode(true, TypeKind::Papo, loc);
    }
  | PALA {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new LiteralExprNode(false, TypeKind::Papo, loc);
    }
  | TEXTO {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        std::string s = *$1;
        std::string conteudo;
        if (s.rfind("vei", 0) == 0 && s.size() >= 10 && s.substr(s.size() - 6) == "tendeu") {
            conteudo = s.substr(4, s.size() - 11);
        } else {
            conteudo = s;
        }
        $$ = new LiteralExprNode(conteudo, TypeKind::Resenha, loc);
        delete $1;
    }
  | IDENT {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new IdentifierExprNode(*$1, loc);
        delete $1;
    }
  | ABRE_PAR expressao FECHA_PAR {
        $$ = $2;
    }
  | ABS ABRE_PAR expressao FECHA_PAR {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new UnaryExprNode(UnaryOp::Abs, std::unique_ptr<ExprNode>($3), loc);
    }
  ;

%%
