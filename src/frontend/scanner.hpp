#pragma once

#include "infra.tab.h"
#include <cstdio>
#include <stdexcept>
#include <string>
#include <memory>

// Forward declarations da AST
class ProgramNode;

namespace oxente {
// Posições com base 1, fim exclusivo; tabulação avança uma coluna de origem.
struct ErroLexico : std::runtime_error {
  OXINFRALTYPE local;
  ErroLexico(OXINFRALTYPE posicao, const std::string& mensagem)
      : std::runtime_error(mensagem), local(posicao) {}
};
struct ContextoLexico {
  int linha = 1, coluna = 1;
  OXINFRALTYPE abertura{1, 1, 1, 1};
  std::string texto;
  void avancar(OXINFRALTYPE* local, const char* dados, int tamanho);
  [[noreturn]] void erro(OXINFRALTYPE local, const std::string& mensagem) const;
};
// O consumidor é dono de valor.texto em cada token não EOF; deve liberá-lo.
// O parser Bison usa %destructor; futuras ações devem mover/copiar o valor.
class Scanner {
 public:
  explicit Scanner(FILE* entrada);
  ~Scanner();
  Scanner(const Scanner&) = delete;
  Scanner& operator=(const Scanner&) = delete;

  int proximo(OXINFRASTYPE* valor, OXINFRALTYPE* local);
  int proximo_significativo(OXINFRASTYPE* valor, OXINFRALTYPE* local);
  void* instancia() const { return scanner_; }

 private:
  ContextoLexico contexto_;
  void* scanner_ = nullptr;
};

const char* nome_token(int token);
std::string representar(const std::string& texto);

std::unique_ptr<ProgramNode> parse(Scanner& scanner, DiagnosticEngine* engine = nullptr);
}  // namespace oxente
