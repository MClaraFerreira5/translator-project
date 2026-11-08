#include "semantic/SymbolTable.hpp"
#include "semantic/ScopeVisitor.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/ExprNodes.hpp"
#include <iostream>
#include <cassert>

using namespace oxente;

int main() {
    std::cout << "[TESTE ESCOPO] Iniciando testes de escopo..." << std::endl;

    // 1. Armazenamento de nome, tipo, localização e escopo
    {
        SymbolTable tabela;
        SourceLocation loc{10, 5};
        assert(tabela.inserir("x", TypeKind::Inteiro, loc));
        
        const Symbol* s = tabela.buscar("x");
        assert(s != nullptr);
        assert(s->nome == "x");
        assert(s->tipo == TypeKind::Inteiro);
        assert(s->loc.line == 10 && s->loc.column == 5);
        assert(s->nivel_escopo == 0);
        assert(s->inicializado == true);
    }

    // 2. Proibição de sombreamento (mesmo em escopo interno)
    {
        SymbolTable tabela;
        assert(tabela.inserir("a", TypeKind::Tronxo, {1, 1}));
        
        tabela.entrar_escopo(); // abre bloco interno
        // Tentar redeclarar 'a' no bloco interno deve falhar
        assert(!tabela.inserir("a", TypeKind::Tronxo, {2, 1}));
        tabela.sair_escopo();
    }

    // 3. Reutilização de nomes em blocos independentes após saída do escopo
    {
        SymbolTable tabela;
        
        // Primeiro bloco independente
        tabela.entrar_escopo();
        assert(tabela.inserir("temp", TypeKind::Inteiro, {1, 1}));
        assert(tabela.buscar("temp") != nullptr);
        tabela.sair_escopo(); // temp saiu de visibilidade

        assert(tabela.buscar("temp") == nullptr);

        // Segundo bloco independente (pode redeclarar 'temp')
        tabela.entrar_escopo();
        assert(tabela.inserir("temp", TypeKind::Resenha, {5, 1}));
        const Symbol* s = tabela.buscar("temp");
        assert(s != nullptr);
        assert(s->tipo == TypeKind::Resenha);
        tabela.sair_escopo();
    }

    // 4. Validação com AST Manual: inicializador usa a própria variável antes de declarada
    // Ex: "inteiro x bota x + 1" (deve falhar)
    {
        DiagnosticEngine engine("", false); // não lança exceção para inspecionarmos
        ScopeVisitor visitor(&engine);

        // x bota x + 1
        auto varX = std::make_unique<IdentifierExprNode>("x", SourceLocation{1, 16});
        auto lit1 = std::make_unique<LiteralExprNode>(1, TypeKind::Inteiro, SourceLocation{1, 20});
        auto soma = std::make_unique<BinaryExprNode>(BinaryOp::Soma, std::move(varX), std::move(lit1));
        auto decl = std::make_unique<VarDeclStmtNode>(TypeKind::Inteiro, "x", std::move(soma), SourceLocation{1, 1});

        decl->accept(visitor);
        assert(visitor.houve_erro); // Falhou porque 'x' ainda não existia
    }

    // 5. Validação com AST Manual: Escopo do cabeçalho de 'arrodeia'
    {
        ScopeVisitor visitor;

        // arrodeia (inteiro i bota 0; i < 10; i bota i + 1) bora { ... }
        auto declI = std::make_unique<VarDeclStmtNode>(TypeKind::Inteiro, "i", 
                     std::make_unique<LiteralExprNode>(0, TypeKind::Inteiro));
        auto cond = std::make_unique<BinaryExprNode>(BinaryOp::Menor,
                    std::make_unique<IdentifierExprNode>("i"),
                    std::make_unique<LiteralExprNode>(10, TypeKind::Inteiro));
        auto inc = std::make_unique<AssignStmtNode>("i",
                   std::make_unique<BinaryExprNode>(BinaryOp::Soma,
                   std::make_unique<IdentifierExprNode>("i"),
                   std::make_unique<LiteralExprNode>(1, TypeKind::Inteiro)));
        auto corpo = std::make_unique<BlockStmtNode>();

        auto forStmt = std::make_unique<ForStmtNode>(std::move(declI), std::move(cond), 
                                                     std::move(inc), std::move(corpo));

        forStmt->accept(visitor);
        assert(!visitor.houve_erro);

        // Fora do arrodeia, 'i' NÃO pode ser visível!
        assert(visitor.tabela.buscar("i") == nullptr);
    }

    std::cout << "[SUCESSO] Todos os critérios de escopo foram validados!" << std::endl;
    return 0;
}