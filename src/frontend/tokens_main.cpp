#include "scanner.hpp"
#include <iostream>
#include <memory>

namespace {
struct FecharArquivo { void operator()(FILE* arquivo) const { std::fclose(arquivo); } };
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Uso: oxente-tokens arquivo.oxe\n";
    return 2;
  }
  std::unique_ptr<FILE, FecharArquivo> entrada(std::fopen(argv[1], "rb"));
  if (!entrada) {
    std::cerr << "Não foi possível abrir a entrada\n";
    return 3;
  }
  try {
    oxente::Scanner scanner(entrada.get());
    // Inspeção bruta: inclui ESPACO e não sintetiza NL antes do EOF.
    for (;;) {
      OXINFRASTYPE valor{};
      OXINFRALTYPE local{};
      int token = scanner.proximo(&valor, &local);
      std::unique_ptr<std::string> texto(valor.texto);
      std::cout << oxente::nome_token(token) << ' ' << local.first_line << ':'
                << local.first_column << '-' << local.last_line << ':' << local.last_column
                << ' ' << oxente::representar(texto ? *texto : "") << '\n';
      if (token == 0) break;
    }
    if (std::ferror(entrada.get())) {
      std::cerr << "Falha na leitura\n";
      return 3;
    }
  } catch (const oxente::ErroLexico& erro) {
    std::cerr << argv[1] << ':' << erro.local.first_line << ':' << erro.local.first_column
              << ": erro léxico: " << erro.what() << '\n';
    return 1;
  } catch (const std::exception& erro) {
    std::cerr << "Falha no scanner: " << erro.what() << '\n';
    return 3;
  }
}
