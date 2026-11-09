%define api.prefix {oxinfra}
%define api.pure full
%locations
%parse-param {oxente::ParserContext* ctx}
%lex-param {oxente::ParserContext* ctx}

%code requires {
#include <string>
#include <memory>
#include <vector>
#include <optional>
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
    std::optional<Diagnostic> erro;

    // Registrar aqui e emitir depois do retorno do Bison permite executar
    // os %destructor antes de qualquer exceção do motor de diagnósticos.
    void registrar_erro(Stage etapa, SourceLocation loc,
                        const std::string& causa, const std::string& mensagem) {
      if (!erro) erro.emplace(etapa, loc, causa, mensagem);
    }
  };
}
}

%code {
#include "scanner.hpp"
#include <stdexcept>
#include <cmath>

int oxinfralex(OXINFRASTYPE* yylval, OXINFRALTYPE* yylloc, oxente::ParserContext* ctx) {
    try {
        return ctx->scanner->proximo_significativo(yylval, yylloc);
    } catch (const oxente::ErroLexico& erro) {
        ctx->registrar_erro(oxente::Stage::Lexico,
            {erro.local.first_line, erro.local.first_column}, "erro_lexico", erro.what());
        // O token de erro faz o Bison liberar a pilha sem emitir outro erro.
        return OXINFRAerror;
    }
}

void oxinfraerror(OXINFRALTYPE* loc, oxente::ParserContext* ctx, const char* msg) {
    oxente::SourceLocation sloc{loc ? loc->first_line : 0, loc ? loc->first_column : 0};
    if (ctx) {
        ctx->registrar_erro(oxente::Stage::Sintatico, sloc, "sintaxe_invalida",
            std::string(msg) == "syntax error" ? "Entrada fora da gramatica." : msg);
    }
}

static LiteralExprNode* literal_numerico(oxente::ParserContext* ctx,
        const std::string& texto, TypeKind tipo, oxente::SourceLocation loc) {
    try {
        if (tipo == TypeKind::Inteiro) {
            return new LiteralExprNode(std::stoi(texto), tipo, loc);
        }
        std::string decimal = texto;
        const auto virgula = decimal.find(',');
        if (virgula != std::string::npos) decimal[virgula] = '.';
        double valor = std::stod(decimal);
        if (std::isfinite(valor)) return new LiteralExprNode(valor, tipo, loc);
    } catch (const std::out_of_range&) {
        // A representação atual é a da AST (int/double), não uma nova regra.
    } catch (const std::invalid_argument&) {
        // O lexer só fornece formatos numéricos reconhecidos.
    }
    ctx->registrar_erro(oxente::Stage::Lexico, loc,
        "literal_fora_da_representacao",
        "Literal numerico fora da representacao atual da AST.");
    return nullptr;
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
%type <bloco> instrucoes corpo bloco senao_opcional
%type <stmt> instrucao simples condicional enquanto para inicio_para declaracao atribuicao entrada saida
%type <lista_expr> argumentos
%type <expr> expressao disjuncao conjuncao negacao comparacao soma produto unario potencia primario

%%

programa:
    instrucoes {
        ctx->raiz = std::make_unique<ProgramNode>(std::unique_ptr<BlockStmtNode>($1), $1->loc);
    }
  | linhas instrucoes {
        ctx->raiz = std::make_unique<ProgramNode>(std::unique_ptr<BlockStmtNode>($2), $2->loc);
    }
  ;

linhas:
    NL { delete $1; }
  | linhas NL { delete $2; }
  ;

instrucoes:
    instrucao {
        $$ = new BlockStmtNode($1->loc);
        $$->add(std::unique_ptr<StmtNode>($1));
    }
  | instrucoes instrucao {
        $$ = $1;
        $$->add(std::unique_ptr<StmtNode>($2));
    }
  ;

/* Cada instrução consome as quebras que a encerram; o scanner entrega uma
   quebra final quando o arquivo termina sem ela. Assim, depois de arremata e
   das linhas em branco, o próximo token decide se o condicional continua. */
instrucao:
    simples linhas { $$ = $1; }
  | condicional    { $$ = $1; }
  | enquanto       { $$ = $1; }
  | para           { $$ = $1; }
  ;

simples:
    declaracao   { $$ = $1; }
  | atribuicao   { $$ = $1; }
  | entrada      { $$ = $1; }
  | saida        { $$ = $1; }
  | CANSO {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new BreakStmtNode(loc);
    }
  | SIMBORA {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new ContinueStmtNode(loc);
    }
  ;

bloco:
    BORA linhas corpo ARREMATA linhas {
        $$ = $3;
        $$->loc = oxente::SourceLocation{@1.first_line, @1.first_column};
    }
  ;

corpo:
    %empty { $$ = new BlockStmtNode(); }
  | instrucoes { $$ = $1; }
  ;

condicional:
    VAI_QUE ABRE_PAR expressao FECHA_PAR bloco senao_opcional {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new IfStmtNode(std::unique_ptr<ExprNode>($3), std::unique_ptr<BlockStmtNode>($5),
                            std::unique_ptr<BlockStmtNode>($6), loc);
    }
  ;

/* se_num_for vira um IfStmtNode como único comando do senao_bloco anterior. */
senao_opcional:
    %empty { $$ = nullptr; }
  | POIS_PRONTO bloco { $$ = $2; }
  | SE_NUM_FOR ABRE_PAR expressao FECHA_PAR bloco senao_opcional {
        oxente::SourceLocation loc{@1.first_line, @1.first_column};
        $$ = new BlockStmtNode(loc);
        $$->add(std::make_unique<IfStmtNode>(std::unique_ptr<ExprNode>($3),
                                             std::unique_ptr<BlockStmtNode>($5),
                                             std::unique_ptr<BlockStmtNode>($6), loc));
    }
  ;

enquanto:
    NUM_PARA ATE ABRE_PAR expressao FECHA_PAR bloco {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new WhileStmtNode(std::unique_ptr<ExprNode>($4), std::unique_ptr<BlockStmtNode>($6), loc);
    }
  ;

para:
    ARRODEIA ABRE_PAR inicio_para PONTO_VIRGULA ATE expressao PONTO_VIRGULA atribuicao FECHA_PAR bloco {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        $$ = new ForStmtNode(std::unique_ptr<StmtNode>($3), std::unique_ptr<ExprNode>($6),
                             std::unique_ptr<StmtNode>($8), std::unique_ptr<BlockStmtNode>($10), loc);
    }
  ;

inicio_para:
    declaracao { $$ = $1; }
  | atribuicao { $$ = $1; }
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
        std::unique_ptr<std::string> texto($1);
        $$ = literal_numerico(ctx, *texto, TypeKind::Inteiro, loc);
        // YYABORT não destrói os valores da produção corrente: texto é RAII.
        if (!$$) YYABORT;
    }
  | REAL {
        oxente::SourceLocation loc{@$.first_line, @$.first_column};
        std::unique_ptr<std::string> texto($1);
        $$ = literal_numerico(ctx, *texto, TypeKind::Tronxo, loc);
        if (!$$) YYABORT;
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
