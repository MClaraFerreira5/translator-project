/* Integração provisória, sem reconhecimento de tokens Oxente. */
%option noyywrap nounput noinput prefix="oxinfra"
%{
#include "infra.tab.h"
%}
%%
(.|\n) return INDISPONIVEL;
<<EOF>> return 0;
%%
