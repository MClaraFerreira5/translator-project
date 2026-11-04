/* Integração provisória: nenhuma produção da linguagem Oxente. */
%define api.prefix {oxinfra}
%code {
int oxinfralex(void);
void oxinfraerror(const char *) {}
}
%token INDISPONIVEL
%%
infraestrutura: INDISPONIVEL { YYABORT; };
%%
