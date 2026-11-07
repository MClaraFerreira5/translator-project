#include "scanner.hpp"
#include <iostream>
#include <memory>
#include <vector>

namespace {
struct Esperado { int tipo; std::string texto; int linha, coluna, fim_linha, fim_coluna; };
int casos = 0;
void exigir(bool condicao, const std::string& mensagem) {
  if (!condicao) throw std::runtime_error(mensagem);
}
struct FecharArquivo { void operator()(FILE* arquivo) const { std::fclose(arquivo); } };
using Arquivo = std::unique_ptr<FILE, FecharArquivo>;
Arquivo fonte(const std::string& texto) {
  Arquivo arquivo(std::tmpfile());
  exigir(arquivo != nullptr, "tmpfile falhou");
  exigir(std::fwrite(texto.data(), 1, texto.size(), arquivo.get()) == texto.size(), "escrita falhou");
  std::rewind(arquivo.get());
  return arquivo;
}
void conferir(const std::string& entrada, const std::vector<Esperado>& esperados) {
  auto arquivo = fonte(entrada);
  oxente::Scanner scanner(arquivo.get());
  for (const auto& esperado : esperados) {
    OXINFRASTYPE valor{}; OXINFRALTYPE local{};
    int tipo = scanner.proximo(&valor, &local);
    std::unique_ptr<std::string> texto(valor.texto);
    const auto contexto = "entrada " + oxente::representar(entrada) + ", esperado " + oxente::nome_token(esperado.tipo);
    exigir(tipo == esperado.tipo, contexto + ", recebido " + oxente::nome_token(tipo));
    exigir((texto ? *texto : "") == esperado.texto, contexto + ": lexema diferente");
    exigir(local.first_line == esperado.linha && local.first_column == esperado.coluna &&
           local.last_line == esperado.fim_linha && local.last_column == esperado.fim_coluna,
           contexto + ": posição diferente");
  }
  OXINFRASTYPE valor{}; OXINFRALTYPE local{};
  int tipo = scanner.proximo(&valor, &local);
  std::unique_ptr<std::string> texto(valor.texto);
  exigir(tipo == 0, "tokens inesperados ao final");
  ++casos;
}
void erro(const std::string& entrada, int linha, int coluna, const std::string& trecho) {
  auto arquivo = fonte(entrada);
  oxente::Scanner scanner(arquivo.get());
  try {
    for (;;) {
      OXINFRASTYPE valor{}; OXINFRALTYPE local{};
      int tipo = scanner.proximo(&valor, &local);
      std::unique_ptr<std::string> texto(valor.texto);
      if (!tipo) break;
    }
  } catch (const oxente::ErroLexico& e) {
    exigir(e.local.first_line == linha && e.local.first_column == coluna, "posição incorreta do erro");
    exigir(std::string(e.what()).find(trecho) != std::string::npos, "diagnóstico incorreto");
    ++casos;
    return;
  }
  throw std::runtime_error("erro esperado, recebido sucesso: " + oxente::representar(entrada));
}
}  // namespace

int main() {
  try {
    const std::vector<std::pair<std::string, int>> palavras = {
      {"inteiro", T_INTEIRO}, {"tronxo", TRONXO}, {"papo", PAPO}, {"resenha", RESENHA},
      {"bota", BOTA}, {"bora", BORA}, {"arremata", ARREMATA}, {"vai_que", VAI_QUE},
      {"se_num_for", SE_NUM_FOR}, {"pois_pronto", POIS_PRONTO}, {"num_para", NUM_PARA},
      {"arrodeia", ARRODEIA}, {"ate", ATE}, {"canso", CANSO}, {"simbora", SIMBORA},
      {"evera", EVERA}, {"pala", PALA}, {"ai_dento", AI_DENTO}, {"e", E}, {"ou", OU},
      {"diga_ai", DIGA_AI}, {"desembucha", DESEMBUCHA}, {"abs", ABS},
      {"aperreio", APERREIO}, {"toma", TOMA}, {"sem_futuro", SEM_FUTURO},
      {"tendeu", TENDEU}, {"arriba", ARRIBA}
    };
    for (const auto& palavra : palavras) {
      int fim = static_cast<int>(palavra.first.size()) + 1;
      conferir(palavra.first, {{palavra.second, palavra.first, 1, 1, 1, fim}, {0, "", 1, fim, 1, fim}});
    }
    for (const std::string nome : {"arrodeia_extra", "Arrodeia", "inteiro2", "vei_extra", "tendeu_extra",
                                 "espia_extra", "arriba_extra", "primeiramente_extra", "_total", "Idade", "IDADE",
                                 "int", "float", "bool", "string", "funcao", "devolve", "void", "liso", "balaio"})
      conferir(nome, {{IDENT, nome, 1, 1, 1, static_cast<int>(nome.size()) + 1}});
    const std::vector<std::pair<std::string, int>> operadores = {
      {"+", MAIS}, {"-", MENOS}, {"*", VEZES}, {"/", DIVISAO}, {"//", DIV_INTEIRA},
      {"%", RESTO}, {"**", POTENCIA}, {">", MAIOR}, {"<", MENOR}, {">=", MAIOR_IGUAL},
      {"<=", MENOR_IGUAL}, {"==", IGUAL}, {"!=", DIFERENTE}, {"(", ABRE_PAR},
      {")", FECHA_PAR}, {";", PONTO_VIRGULA}
    };
    for (const auto& op : operadores)
      conferir(op.first, {{op.second, op.first, 1, 1, 1, static_cast<int>(op.first.size()) + 1}});
    conferir("***///>=<=!=", {{POTENCIA,"**",1,1,1,3},{VEZES,"*",1,3,1,4},
      {DIV_INTEIRA,"//",1,4,1,6},{DIVISAO,"/",1,6,1,7},{MAIOR_IGUAL,">=",1,7,1,9},
      {MENOR_IGUAL,"<=",1,9,1,11},{DIFERENTE,"!=",1,11,1,13}});
    conferir("0;007;7,5000;-3", {{INTEIRO,"0",1,1,1,2},{PONTO_VIRGULA,";",1,2,1,3},
      {INTEIRO,"007",1,3,1,6},{PONTO_VIRGULA,";",1,6,1,7},{REAL,"7,5000",1,7,1,13},
      {PONTO_VIRGULA,";",1,13,1,14},{MENOS,"-",1,14,1,15},{INTEIRO,"3",1,15,1,16}});
    std::string grande(400, '9');
    conferir(grande, {{INTEIRO,grande,1,1,1,401}});
    conferir(grande + "," + grande, {{REAL,grande + "," + grande,1,1,1,802}});
    conferir("vei Ola mundo tendeu", {{TEXTO,"vei Ola mundo tendeu",1,1,1,21}});
    conferir("vei\tOi\ttendeu)", {{TEXTO,"vei\tOi\ttendeu",1,1,1,14},{FECHA_PAR,")",1,14,1,15}});
    const std::string texto = "vei primeiramente espia arriba tendeu_extra tendeu";
    conferir(texto, {{TEXTO,texto,1,1,1,static_cast<int>(texto.size())+1}});
    conferir("\t \r\n\n0", {{ESPACO,"\t ",1,1,1,3},{NL,"\r\n",1,3,2,1},{NL,"\n",2,1,3,1},
      {INTEIRO,"0",3,1,3,2},{0,"",3,2,3,2}});
    conferir("0\n", {{INTEIRO,"0",1,1,1,2},{NL,"\n",1,2,2,1},{0,"",2,1,2,1}});
    conferir("", {{0,"",1,1,1,1}});
    conferir("0 primeiramente fim\r\n2", {{INTEIRO,"0",1,1,1,2},{ESPACO," ",1,2,1,3},
      {NL,"\r\n",1,20,2,1},{INTEIRO,"2",2,1,2,2}});
    conferir("espia\r\narriba_extra\narriba\r\n0", {{NL,"\r\n",1,6,2,1},{NL,"\n",2,13,3,1},
      {NL,"\r\n",3,7,4,1},{INTEIRO,"0",4,1,4,2}});
    conferir("espia espia arriba", {}); // Sem aninhamento.
    erro("@",1,1,"caractere não reconhecido");
    erro("\n\r\n\t@",3,2,"caractere não reconhecido");
    erro("espia\r\narriba\r\n@",3,1,"caractere não reconhecido");
    erro("\r\nvei Ola",2,1,"texto não terminado");
    erro("\n espia\r\n",2,2,"comentário não terminado");
    erro("7.5",1,2,"caractere não reconhecido");
    erro("7,",1,2,"caractere não reconhecido");
    erro(",5",1,1,"caractere não reconhecido");
    erro("=",1,1,"caractere não reconhecido");
    erro("\"",1,1,"caractere não reconhecido");
    erro(std::string("\0",1),1,1,"caractere não reconhecido");
    erro(std::string("\xC3\xA1"),1,1,"caractere não reconhecido");
    // Limitações explícitas, sem apresentar decisões pendentes como linguagem inválida.
    erro("vei tendeu",1,1,"caso pendente");
    erro("vei  Oi tendeu",1,1,"caso pendente");
    erro("vei Oi  tendeu",1,1,"caso pendente");
    erro("vei Oi\n",1,1,"caso pendente");
    erro("vei Oi\\n tendeu",1,7,"caso pendente");
    erro("primeiramente fim",1,1,"caso pendente");
    erro("2valor",1,1,"caso pendente");
    erro("7,5nome",1,1,"caso pendente");
    erro("\r",1,1,"caso pendente");
    // Instâncias independentes e parser provisório rejeitando vazio e tokens reais.
    auto a = fonte("arrodeia"), b = fonte("0");
    oxente::Scanner primeiro(a.get()), segundo(b.get());
    OXINFRASTYPE valor{}; OXINFRALTYPE local{};
    exigir(primeiro.proximo(&valor,&local) == ARRODEIA, "primeiro scanner"); delete valor.texto;
    exigir(segundo.proximo(&valor,&local) == INTEIRO && local.first_column == 1, "segundo scanner"); delete valor.texto;
    ++casos;
    for (const std::string entrada : {"", "inteiro"}) {
      auto arquivo = fonte(entrada); oxente::Scanner scanner(arquivo.get());
      oxente::ParserContext ctx{&scanner, nullptr, nullptr};
      exigir(oxinfraparse(&ctx) != 0, "parser não pode aceitar programas vazios ou incompletos");
      ++casos;
    }
    std::cout << casos << " casos léxicos aprovados\n";
  } catch (const std::exception& e) {
    std::cerr << "Falha após " << casos << " casos: " << e.what() << '\n'; return 1;
  }
}
