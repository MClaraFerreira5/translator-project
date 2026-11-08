#include "scanner.hpp"
#include <new>

int oxinfralex_init_extra(oxente::ContextoLexico*, void**);
int oxinfralex_destroy(void*);
void oxinfraset_in(FILE*, void*);
int oxinfralex(OXINFRASTYPE*, OXINFRALTYPE*, void*);
namespace oxente {
void ContextoLexico::avancar(OXINFRALTYPE* local, const char* dados, int tamanho) {
  local->first_line = linha;
  local->first_column = coluna;
  for (int i = 0; i < tamanho; ++i) {
    if (dados[i] == '\r' && i + 1 < tamanho && dados[i+1] == '\n') ++i;
    if (dados[i] == '\n') { ++linha; coluna = 1; }
    else ++coluna;
  }
  local->last_line = linha;
  local->last_column = coluna;
}
void ContextoLexico::erro(OXINFRALTYPE local, const std::string& mensagem) const {
  throw ErroLexico(local, mensagem);
}
Scanner::Scanner(FILE* entrada) {
  if (!entrada) throw std::invalid_argument("entrada nula para scanner");
  if (oxinfralex_init_extra(&contexto_, &scanner_) != 0) throw std::bad_alloc();
  oxinfraset_in(entrada, scanner_);
}
Scanner::~Scanner() { oxinfralex_destroy(scanner_); }
int Scanner::proximo(OXINFRASTYPE* valor, OXINFRALTYPE* local) {
  valor->texto = nullptr;
  return oxinfralex(valor, local, scanner_);
}
const char* nome_token(int token) {
  switch (token) {
    case 0: return "EOF";
    case IDENT: return "IDENT";
    case INTEIRO: return "INTEIRO";
    case REAL: return "REAL";
    case TEXTO: return "TEXTO";
    case NL: return "NL";
    case ESPACO: return "ESPACO";
    case T_INTEIRO: return "T_INTEIRO";
    case TRONXO: return "TRONXO";
    case PAPO: return "PAPO";
    case RESENHA: return "RESENHA";
    case BOTA: return "BOTA";
    case BORA: return "BORA";
    case ARREMATA: return "ARREMATA";
    case VAI_QUE: return "VAI_QUE";
    case SE_NUM_FOR: return "SE_NUM_FOR";
    case POIS_PRONTO: return "POIS_PRONTO";
    case NUM_PARA: return "NUM_PARA";
    case ARRODEIA: return "ARRODEIA";
    case ATE: return "ATE";
    case CANSO: return "CANSO";
    case SIMBORA: return "SIMBORA";
    case EVERA: return "EVERA";
    case PALA: return "PALA";
    case AI_DENTO: return "AI_DENTO";
    case E: return "E";
    case OU: return "OU";
    case DIGA_AI: return "DIGA_AI";
    case DESEMBUCHA: return "DESEMBUCHA";
    case ABS: return "ABS";
    case APERREIO: return "APERREIO";
    case TOMA: return "TOMA";
    case SEM_FUTURO: return "SEM_FUTURO";
    case TENDEU: return "TENDEU";
    case ARRIBA: return "ARRIBA";
    case MAIS: return "MAIS";
    case MENOS: return "MENOS";
    case VEZES: return "VEZES";
    case DIVISAO: return "DIVISAO";
    case DIV_INTEIRA: return "DIV_INTEIRA";
    case RESTO: return "RESTO";
    case POTENCIA: return "POTENCIA";
    case MAIOR: return "MAIOR";
    case MENOR: return "MENOR";
    case MAIOR_IGUAL: return "MAIOR_IGUAL";
    case MENOR_IGUAL: return "MENOR_IGUAL";
    case IGUAL: return "IGUAL";
    case DIFERENTE: return "DIFERENTE";
    case ABRE_PAR: return "ABRE_PAR";
    case FECHA_PAR: return "FECHA_PAR";
    case PONTO_VIRGULA: return "PONTO_VIRGULA";
    default: return "DESCONHECIDO";
  }
}
std::string representar(const std::string& texto) {
  std::string saida = "\"";
  const char* hexa = "0123456789ABCDEF";
  for (unsigned char c : texto) {
    switch (c) {
      case '\n': saida += "\\n"; break;
      case '\r': saida += "\\r"; break;
      case '\t': saida += "\\t"; break;
      case '\\': saida += "\\\\"; break;
      case '"': saida += "\\\""; break;
      default:
        if (c < 32 || c >= 127) { saida += "\\x"; saida += hexa[c >> 4]; saida += hexa[c & 15]; }
        else saida += static_cast<char>(c);
    }
  }
  return saida + "\"";
}

int Scanner::proximo_significativo(OXINFRASTYPE* valor, OXINFRALTYPE* local) {
  for (;;) {
    int tok = proximo(valor, local);
    switch (tok) {
      case 0:
        // EOF encerra a última instrução: o parser sempre recebe uma quebra final.
        if (!quebra_final_pendente_) return 0;
        quebra_final_pendente_ = false;
        valor->texto = new std::string();
        return NL;
      case NL:
        quebra_final_pendente_ = false;
        return tok;  // A ação da gramática libera o lexema.
      case IDENT:
      case INTEIRO:
      case REAL:
      case TEXTO:
        quebra_final_pendente_ = true;
        return tok;  // As ações da gramática movem ou liberam o lexema.
      default:
        // Palavras reservadas e operadores não têm valor semântico no parser.
        delete valor->texto;
        valor->texto = nullptr;
        if (tok == ESPACO) break;
        quebra_final_pendente_ = true;
        return tok;
    }
  }
}

std::unique_ptr<ProgramNode> parse(Scanner& scanner, DiagnosticEngine* engine) {
  ParserContext ctx{&scanner, engine, nullptr, std::nullopt};
  int resultado = ::oxinfraparse(&ctx);
  if (ctx.erro) {
    // A pilha do Bison já foi liberada. O contrato desta API é retornar
    // nullptr para erros de entrada, inclusive com motor que lança exceção.
    ctx.raiz.reset();
    if (engine) {
      try {
        engine->emitir(ctx.erro->etapa, ctx.erro->loc,
                       ctx.erro->causa_tecnica, ctx.erro->mensagem);
      } catch (const DiagnosticException&) {
        return nullptr;
      }
    }
    return nullptr;
  }
  if (resultado != 0) {
    return nullptr;
  }
  return std::move(ctx.raiz);
}
}  // namespace oxente
