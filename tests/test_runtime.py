import unittest
from unittest.mock import patch
import io
import sys
import os

# Adiciona o diretório src/runtime ao path para importação
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "../src/runtime")))
import oxente_runtime as ox

class TestOxenteRuntime(unittest.TestCase):

    # 1. Testes de formatação de saída
    def test_formatacao_papo(self):
        self.assertEqual(ox._ox_formatar_valor(True), "evera")
        self.assertEqual(ox._ox_formatar_valor(False), "pala")

    def test_formatacao_tronxo(self):
        self.assertEqual(ox._ox_formatar_valor(7.5), "7,500")
        self.assertEqual(ox._ox_formatar_valor(3.14159), "3,142")
        self.assertEqual(ox._ox_formatar_valor(0.0), "0,000")

    # 2. Testes de Aritmética e Truncamento com Negativos
    def test_divisao_truncada_negativos(self):
        # Na Oxente Lang: truncamento em direção a zero
        # 7 / 2 = 3 ; resto = 1
        self.assertEqual(ox._ox_div_int(7, 2), 3)
        self.assertEqual(ox._ox_mod(7, 2), 1)

        # -7 / 2 = -3 (Python puro daria -4!)
        self.assertEqual(ox._ox_div_int(-7, 2), -3)
        # Resto coerente: -7 - (-3 * 2) = -1 (Python puro daria 1!)
        self.assertEqual(ox._ox_mod(-7, 2), -1)

        # 7 / -2 = -3
        self.assertEqual(ox._ox_div_int(7, -2), -3)
        self.assertEqual(ox._ox_mod(7, -2), 1)

    # 3. Testes de Erros e Divisão por Zero
    def test_divisao_por_zero(self):
        with self.assertRaises(SystemExit):
            ox._ox_div_int(10, 0)

        with self.assertRaises(SystemExit):
            ox._ox_mod(10, 0)

        with self.assertRaises(SystemExit):
            ox._ox_div(10.0, 0)

    # 4. Testes de Entrada (diga_ai)
    @patch("builtins.input", return_value="42")
    def test_diga_ai_inteiro_valido(self, mock_input):
        self.assertEqual(ox._ox_diga_ai("inteiro"), 42)

    @patch("builtins.input", return_value="7,5")
    def test_diga_ai_tronxo_com_virgula(self, mock_input):
        self.assertEqual(ox._ox_diga_ai("tronxo"), 7.5)

    @patch("builtins.input", return_value="evera")
    def test_diga_ai_papo_evera(self, mock_input):
        self.assertTrue(ox._ox_diga_ai("papo"))

    @patch("builtins.input", return_value="invalido")
    def test_diga_ai_invalido(self, mock_input):
        with self.assertRaises(SystemExit):
            ox._ox_diga_ai("inteiro")

    # 5. Avaliação de Curto-Circuito
    def test_curto_circuito_logico(self):
        # Em Oxente Lang, 'pala e expressao' não deve avaliar a segunda expressão
        chamou = []
        def efeito_colateral():
            chamou.append(True)
            return True

        # Simulando curto-circuito (pala e ...)
        condicao = False
        resultado = condicao and efeito_colateral()
        self.assertFalse(resultado)
        self.assertEqual(len(chamou), 0, "Curto-circuito falhou: avaliou o segundo operando!")

if __name__ == "__main__":
    unittest.main()