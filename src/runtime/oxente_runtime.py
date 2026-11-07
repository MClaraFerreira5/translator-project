import sys
import math

def _ox_erro(codigo: str, mensagem: str, linha: int = None):
    loc = f"linha {linha}: " if linha else ""
    sys.stderr.write(f"oxente [execução]: {loc}erro '{codigo}': {mensagem}\n")
    sys.exit(1)

# 1. Formatação de Saída (desembucha)
def _ox_formatar_valor(v):
    if isinstance(v, bool):
        return "evera" if v else "pala"
    if isinstance(v, float):
        # 3 casas decimais com vírgula: ex 7.5 -> 7,500
        return f"{v:.3f}".replace(".", ",")
    return str(v)

def _ox_desembucha(*args):
    saida = "".join(_ox_formatar_valor(a) for a in args)
    print(saida)

# 2. Entrada (diga_ai)
def _ox_diga_ai(tipo: str, linha: int = None):
    try:
        texto = input()
    except EOFError:
        _ox_erro("entrada_ausente", "Fim de entrada inesperado ao tentar ler dados.", linha)

    texto_limpo = texto.strip()

    if tipo == "inteiro":
        try:
            return int(texto_limpo)
        except ValueError:
            _ox_erro("entrada_invalida", f"Esperava número inteiro, mas recebeu '{texto}'.", linha)

    elif tipo == "tronxo":
        try:
            # Aceita tanto vírgula quanto ponto na entrada
            return float(texto_limpo.replace(",", "."))
        except ValueError:
            _ox_erro("entrada_invalida", f"Esperava número real (tronxo), mas recebeu '{texto}'.", linha)

    elif tipo == "papo":
        if texto_limpo == "evera":
            return True
        elif texto_limpo == "pala":
            return False
        else:
            _ox_erro("entrada_invalida", f"Esperava 'evera' ou 'pala', mas recebeu '{texto}'.", linha)

    elif tipo == "resenha":
        return texto

    else:
        _ox_erro("tipo_desconhecido", f"Tipo '{tipo}' não suportado na leitura.", linha)

# 3. Operações Aritméticas e Truncamento
def _ox_div(a, b, linha: int = None):
    if b == 0:
        _ox_erro("dividiu_por_nada", "Divisão real por zero.", linha)
    return float(a) / float(b)

def _ox_div_int(a: int, b: int, linha: int = None):
    if b == 0:
        _ox_erro("dividiu_por_nada", "Divisão inteira por zero.", linha)
    # Truncamento em direção a zero (igual C++: int(a / b))
    # Em Python: int(a / b) trunca em direção a zero para positivos e negativos
    return int(a / b)

def _ox_mod(a: int, b: int, linha: int = None):
    if b == 0:
        _ox_erro("dividiu_por_nada", "Resto de divisão por zero.", linha)
    # Coerente com o quociente truncado: a = (q * b) + r  =>  r = a - (q * b)
    q = int(a / b)
    return a - (q * b)

def _ox_pot(a, b, linha: int = None):
    try:
        resultado = math.pow(a, b)
        if math.isnan(resultado) or math.isinf(resultado):
            _ox_erro("potencia_invalida", "Resultado de potência fora dos limites reais.", linha)
        return float(resultado)
    except (ValueError, OverflowError):
        _ox_erro("potencia_invalida", "Operação de potência inválida ou estouro de limite.", linha)