#include "semantic/SymbolTable.hpp"
#include "semantic/ScopeVisitor.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/ExprNodes.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include "frontend/scanner.hpp"

using namespace oxente;

namespace {
void exigir(bool condicao, const std::string& mensagem) {
    if (!condicao) throw std::runtime_error(mensagem);
}

struct FecharArquivo {
    void operator()(FILE* arquivo) const { if (arquivo) std::fclose(arquivo); }
};

struct CapturarErros {
    std::ostringstream saida;
    std::streambuf* anterior = std::cerr.rdbuf(saida.rdbuf());
    ~CapturarErros() { std::cerr.rdbuf(anterior); }
};

class VisitanteOrdenado : public ScopeVisitor {
public:
    std::vector<std::string> nomes;
    using ScopeVisitor::ScopeVisitor;
    using ScopeVisitor::visit;
    void visit(IdentifierExprNode& node) override {
        nomes.push_back(node.nome);
        ScopeVisitor::visit(node);
    }
};
}

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

    // 6. O segundo argumento de saída também passa pela resolução de nomes.
    {
        std::unique_ptr<FILE, FecharArquivo> arquivo(std::tmpfile());
        exigir(arquivo != nullptr, "Falha ao criar fonte de teste");
        const std::string codigo = "inteiro x bota 1\ndesembucha(x; ausente)";
        const auto escritos = std::fwrite(codigo.data(), 1, codigo.size(), arquivo.get());
        exigir(escritos == codigo.size(), "Falha ao escrever fonte de teste");
        std::rewind(arquivo.get());
        Scanner scanner(arquivo.get());
        auto ast = parse(scanner);
        exigir(ast != nullptr, "Programa de saída rejeitado pelo parser");
        DiagnosticEngine engine("saida.oxe", false);
        VisitanteOrdenado visitor(&engine);
        CapturarErros captura;
        ast->accept(visitor);
        exigir(visitor.houve_erro && engine.tem_erros(), "Segundo argumento não foi verificado");
        exigir(visitor.nomes == std::vector<std::string>{"x", "ausente"}, "Ordem de visita incorreta");
        const auto mensagem = captura.saida.str();
        exigir(mensagem.find("saida.oxe:2:15:") != std::string::npos, "Localização incorreta");
        exigir(mensagem.find("nome_nao_declarado") != std::string::npos, "Causa incorreta");
        exigir(mensagem.find("ausente") != std::string::npos, "Nome ausente no diagnóstico");
    }

    // 7. Mantém o construtor de um argumento e percorre vários na ordem da AST.
    {
        VisitanteOrdenado visitor;
        exigir(visitor.tabela.inserir("x", TypeKind::Inteiro, {1, 1}), "Falha ao declarar x");
        exigir(visitor.tabela.inserir("y", TypeKind::Inteiro, {2, 1}), "Falha ao declarar y");
        OutputStmtNode unica(std::make_unique<IdentifierExprNode>("x"));
        unica.accept(visitor);
        std::vector<std::unique_ptr<ExprNode>> argumentos;
        argumentos.push_back(std::make_unique<IdentifierExprNode>("y"));
        argumentos.push_back(std::make_unique<IdentifierExprNode>("x"));
        OutputStmtNode multipla(std::move(argumentos));
        multipla.accept(visitor);
        exigir(!visitor.houve_erro, "Nome válido rejeitado");
        exigir(visitor.nomes == std::vector<std::string>{"x", "y", "x"}, "Ordem de visita incorreta");
    }

    std::cout << "[SUCESSO] Todos os critérios de escopo foram validados!" << std::endl;
    return 0;
}
