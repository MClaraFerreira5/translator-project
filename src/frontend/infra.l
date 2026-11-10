/* Scanner reentrante: sem estado global de leitura. */
%option noyywrap nounput noinput nodefault reentrant bison-bridge bison-locations
%option prefix="oxinfra" extra-type="oxente::ContextoLexico*"
%x TEXTO_EST COMENTARIO LINHA
%{
#include "scanner.hpp"
#include <unordered_map>
#define YYSTYPE OXINFRASTYPE
#define YYLTYPE OXINFRALTYPE
#define YY_USER_ACTION yyextra->avancar(yylloc, yytext, yyleng);
#define YY_INPUT(buffer, resultado, maximo) do { \
  resultado = static_cast<int>(std::fread(buffer, 1, maximo, yyin)); \
  if (std::ferror(yyin)) throw std::runtime_error("falha na leitura da entrada"); \
} while (false)
// O retorno é o tipo do token declarado no Bison; texto carrega o lexema.
// YY_USER_ACTION já preencheu yylloc com o intervalo de origem. O consumidor
// assume o std::string: o harness usa RAII e o adaptador/parser libera ou move.
#define EMITIR(tipo) do { \
  yylval->texto = new std::string(yytext, yyleng); \
  return tipo; \
} while (false)
static int palavra(const std::string& texto) {
  static const std::unordered_map<std::string, int> palavras = {
    {"inteiro", T_INTEIRO},
    {"tronxo", TRONXO},
    {"papo", PAPO},
    {"resenha", RESENHA},
    {"bota", BOTA},
    {"bora", BORA},
    {"arremata", ARREMATA},
    {"vai_que", VAI_QUE},
    {"se_num_for", SE_NUM_FOR},
    {"pois_pronto", POIS_PRONTO},
    {"num_para", NUM_PARA},
    {"arrodeia", ARRODEIA},
    {"ate", ATE},
    {"canso", CANSO},
    {"simbora", SIMBORA},
    {"evera", EVERA},
    {"pala", PALA},
    {"ai_dento", AI_DENTO},
    {"e", E},
    {"ou", OU},
    {"diga_ai", DIGA_AI},
    {"desembucha", DESEMBUCHA},
    {"abs", ABS},
    {"aperreio", APERREIO},
    {"toma", TOMA},
    {"sem_futuro", SEM_FUTURO},
    {"tendeu", TENDEU},
    {"arriba", ARRIBA}
  };
  auto it = palavras.find(texto);
  return it == palavras.end() ? IDENT : it->second;
}
%}
ID [A-Za-z_][A-Za-z0-9_]*
QUEBRA (\r\n|\n)
%%
 /* Estados exclusivos: texto acumula o literal; comentários descartam conteúdo. */
<INITIAL>"vei"                                      {
  yyextra->abertura = *yylloc;
  yyextra->texto = "vei";
  BEGIN(TEXTO_EST);
}
<INITIAL>"primeiramente"                            {
  yyextra->abertura = *yylloc;
  BEGIN(LINHA);
}
<INITIAL>"espia"                                    {
  yyextra->abertura = *yylloc;
  BEGIN(COMENTARIO);
}

 /* Palavras completas, números e espaços fora dos estados exclusivos. */
<INITIAL>{ID}                                       { EMITIR(palavra(std::string(yytext, yyleng))); }
<INITIAL>[0-9]+(,[0-9]+)?{ID}                       {
  yyextra->erro(*yylloc,
      "caso pendente na especificação: número adjacente a identificador");
}
<INITIAL>[0-9]+,[0-9]+                              { EMITIR(REAL); }
<INITIAL>[0-9]+                                     { EMITIR(INTEIRO); }
<INITIAL>[ \t]+                                     { EMITIR(ESPACO); }

 /* Quebras físicas viram NL também nos comentários para preservar términos.
   Só o comentário de linha termina aqui; texto rejeita quebras em regra própria. */
<INITIAL,LINHA,COMENTARIO>{QUEBRA}                  {
  if (YY_START == LINHA) BEGIN(INITIAL);
  EMITIR(NL);
}

 /* Operadores e delimitadores: o Flex escolhe a maior correspondência. */
<INITIAL>"+"                                        { EMITIR(MAIS); }
<INITIAL>"-"                                        { EMITIR(MENOS); }
<INITIAL>"*"                                        { EMITIR(VEZES); }
<INITIAL>"/"                                        { EMITIR(DIVISAO); }
<INITIAL>"//"                                       { EMITIR(DIV_INTEIRA); }
<INITIAL>"%"                                        { EMITIR(RESTO); }
<INITIAL>"**"                                       { EMITIR(POTENCIA); }
<INITIAL>">"                                        { EMITIR(MAIOR); }
<INITIAL>"<"                                        { EMITIR(MENOR); }
<INITIAL>">="                                       { EMITIR(MAIOR_IGUAL); }
<INITIAL>"<="                                       { EMITIR(MENOR_IGUAL); }
<INITIAL>"=="                                       { EMITIR(IGUAL); }
<INITIAL>"!="                                       { EMITIR(DIFERENTE); }
<INITIAL>"("                                        { EMITIR(ABRE_PAR); }
<INITIAL>")"                                        { EMITIR(FECHA_PAR); }
<INITIAL>";"                                        { EMITIR(PONTO_VIRGULA); }

 /* Texto: tendeu só fecha como palavra completa após espaço ou tabulação. */
<TEXTO_EST>{ID}                                     {
  std::string parte(yytext, yyleng);
  if (yyextra->texto == "vei")
    yyextra->erro(*yylloc, "esperado espaço ou tabulação após vei");
  if (parte == "tendeu" && (yyextra->texto.back() == ' ' || yyextra->texto.back() == '\t')) {
    const auto& t = yyextra->texto;
    if (t.size() <= 4 || t[4] == ' ' || t[4] == '\t' ||
        t[t.size()-2] == ' ' || t[t.size()-2] == '\t')
      yyextra->erro(yyextra->abertura, "caso pendente na especificação: texto vazio ou espaços de borda");
    yyextra->texto += parte;
    yylloc->first_line = yyextra->abertura.first_line;
    yylloc->first_column = yyextra->abertura.first_column;
    yylval->texto = new std::string(yyextra->texto);
    BEGIN(INITIAL);
    return TEXTO;
  }
  yyextra->texto += parte;
}
<TEXTO_EST>"\\"                                     {
  yyextra->erro(*yylloc,
      "caso pendente na especificação: escape em texto");
}
<TEXTO_EST>{QUEBRA}                                 {
  yyextra->erro(yyextra->abertura,
      "caso pendente na especificação: texto multilinha");
}
<TEXTO_EST>[\x20-\x7e\t]                            {
  if (yyextra->texto == "vei" && yytext[0] != ' ' && yytext[0] != '\t')
    yyextra->erro(*yylloc, "esperado espaço ou tabulação após vei");
  yyextra->texto.append(yytext, yyleng);
}

 /* Comentários: arriba fecha o bloco; ambos os estados ignoram ASCII imprimível. */
<COMENTARIO>{ID}                                    {
  if (std::string(yytext, yyleng) == "arriba") BEGIN(INITIAL);
}
<LINHA,COMENTARIO>[\x20-\x7e\t]                     {
  /* Conteúdo ignorado; localização já avançou. */
}

 /* Erros comuns a todos os estados: CR isolado e bytes sem regra válida.
   No EOF, cada estado informa o fechamento que falta; INITIAL encerra a leitura. */
<INITIAL,LINHA,COMENTARIO,TEXTO_EST>\r              {
  yyextra->erro(*yylloc,
      "caso pendente na especificação: CR isolado");
}
<INITIAL,LINHA,COMENTARIO,TEXTO_EST>.               {
  yyextra->erro(*yylloc,
      "caractere não reconhecido: " + oxente::representar(std::string(yytext, yyleng)));
}
<TEXTO_EST><<EOF>>                                  {
  yyextra->erro(yyextra->abertura,
      "texto não terminado: esperado tendeu");
}
<COMENTARIO><<EOF>>                                 {
  yyextra->erro(yyextra->abertura,
      "comentário não terminado: esperado arriba");
}
<LINHA><<EOF>>                                      {
  yyextra->erro(yyextra->abertura,
      "caso pendente na especificação: comentário de linha no EOF");
}
<INITIAL><<EOF>>                                    {
  *yylloc = {yyextra->linha, yyextra->coluna, yyextra->linha, yyextra->coluna};
  yylval->texto = nullptr;
  return 0;
}
%%
