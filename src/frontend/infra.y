/* Contrato léxico real; parser provisório, sem produções Oxente (OX-05). */
%define api.prefix {oxinfra}
%define api.pure full
%locations
%parse-param {void* scanner}
%lex-param {void* scanner}
%code requires {
#include <string>
}
%code {
int oxinfralex(OXINFRASTYPE*, OXINFRALTYPE*, void*);
void oxinfraerror(OXINFRALTYPE*, void*, const char*) {}
}
%union { std::string* texto; }
%token INDISPONIVEL
%token <texto> IDENT INTEIRO REAL TEXTO NL ESPACO
%token <texto> T_INTEIRO TRONXO PAPO RESENHA BOTA BORA ARREMATA
%token <texto> VAI_QUE SE_NUM_FOR POIS_PRONTO NUM_PARA ARRODEIA ATE CANSO SIMBORA
%token <texto> EVERA PALA AI_DENTO E OU DIGA_AI DESEMBUCHA ABS
%token <texto> APERREIO TOMA SEM_FUTURO TENDEU ARRIBA
%token <texto> MAIS MENOS VEZES DIVISAO DIV_INTEIRA RESTO POTENCIA
%token <texto> MAIOR MENOR MAIOR_IGUAL MENOR_IGUAL IGUAL DIFERENTE
%token <texto> ABRE_PAR FECHA_PAR PONTO_VIRGULA
%destructor { delete $$; } <texto>
%%
infraestrutura: INDISPONIVEL { YYABORT; };
%%
