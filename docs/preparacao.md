# Preparação do ambiente

## Linux

Pré-requisitos: compilador C++17 com `std::to_chars` para `double` (GCC 11+),
CMake 3.16+, Make ou Ninja, Flex 2.6+, Bison 3.0+ e Python 3.

Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install git g++ cmake make flex bison m4 python3
```

Fedora:

```bash
sudo dnf install git gcc-c++ cmake make flex bison m4 python3
```

Na raiz do checkout:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

`sh scripts/build-e-testes.sh` reúne esses passos (testes ativos por padrão)
e aceita outra pasta de build como argumento. A CI em `.github/workflows/ci.yml`
instala dependências no Ubuntu e chama esse script em pushes para `main` e PRs.
Em uma pasta de build reaproveitada, confira se `BUILD_TESTING` continua ativo.

CMake rejeita build na árvore fonte. Use uma pasta nova para recompilar do zero;
os fontes Flex/Bison e o header do runtime embutido ficam em `build/generated/`.
Não versione os artefatos. Python é necessário aos testes e à execução do programa
gerado; o tradutor pode ser compilado com `-DBUILD_TESTING=OFF` sem Python.
Consulte a [suíte](test-suite.md) e o [início rápido](../README.md#início-rápido).

## Windows: WSL 2 com Ubuntu

No Windows 10 atualizado ou Windows 11, use o WSL 2 com Ubuntu para executar
as mesmas ferramentas Linux do projeto. Esta é a rota documentada para Windows;
os comandos abaixo não exigem uma instalação nativa de Flex/Bison no PowerShell.

1. Abra o PowerShell **como administrador** e instale o WSL:

   ```powershell
   wsl --install -d Ubuntu
   ```

2. Reinicie o computador se solicitado. Abra o aplicativo **Ubuntu** e conclua
   a criação do usuário e senha Linux. Se o WSL já estiver instalado, confira
   a versão no PowerShell com `wsl --list --verbose`; se Ubuntu estiver na
   versão 1, execute `wsl --set-version Ubuntu 2`.

3. No terminal **Ubuntu/WSL**, instale as dependências e clone o projeto na pasta
   pessoal Linux (prefira `~/projetos` a `/mnt/c`):

   ```bash
   sudo apt-get update
   sudo apt-get install git g++ cmake make flex bison m4 python3
   mkdir -p ~/projetos
   cd ~/projetos
   git clone https://github.com/MClaraFerreira5/translator-project.git
   cd translator-project
   git switch main
   cmake -S . -B build
   cmake --build build --parallel 2
   ctest --test-dir build --output-on-failure
   ./build/oxente --help
   ```

   Se já tiver um clone no WSL, entre nele e use `git fetch origin` antes de
   selecionar a branch, preservando suas alterações locais. Para repositório
   privado, autentique o Git conforme o acesso da sua conta GitHub.

Os comandos de build e execução devem continuar no Ubuntu/WSL, não no
PowerShell. Com VS Code, instale a extensão WSL e abra o checkout nesse ambiente.
Consulte a [instalação oficial do WSL](https://learn.microsoft.com/windows/wsl/install)
se a virtualização ou instalação do Ubuntu exigir configuração adicional.
O build foi validado localmente em Linux; a instalação do WSL no Windows não foi executada
neste ambiente de desenvolvimento. Em build nativo com MSYS2, fora da rota
documentada, o teste `lexer_harness` falha com "CR isolado"; os demais passam.
