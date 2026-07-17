# Instalação

Este documento descreve como configurar o ambiente necessário para executar o projeto **IHS Remote**.

---

# Requisitos

## Sistema Operacional

- Ubuntu 24.04 LTS (ou distribuição Linux compatível)
- Kernel Linux 6.x

## Hardware

- Computador com GPU Intel compatível com DRM
- Dispositivo Android
- Ambos conectados à mesma rede local

---

# Dependências

Antes de compilar o módulo, instale as dependências necessárias.

```bash
sudo apt update
```

```bash
sudo apt install build-essential
```

```bash
sudo apt install linux-headers-$(uname -r)
```

Verifique se os headers do kernel foram instalados corretamente.

```bash
ls /usr/src/linux-headers-$(uname -r)
```

---

# Compilando o Módulo

Entre na pasta do módulo.

```bash
cd kernel
```

Remova arquivos de compilações anteriores.

```bash
make clean
```

Compile o módulo.

```bash
make
```

Ao final da compilação deverá existir o arquivo:

```text
ihs_remote.ko
```

---

# Carregando o Módulo

Carregue o módulo utilizando:

```bash
sudo insmod ihs_remote.ko
```

Verifique se o módulo foi carregado.

```bash
lsmod | grep ihs_remote
```

Também é possível verificar as mensagens do kernel.

```bash
dmesg -w
```

---

# Removendo o Módulo

Para remover o módulo:

```bash
sudo rmmod ihs_remote
```

---

# Compilando o Aplicativo Android

## Requisitos

- Android Studio
- Android SDK
- Java 17 (ou versão compatível)

Clone o repositório.

```bash
git clone <URL_DO_REPOSITORIO>
```

Abra a pasta do projeto Android no Android Studio.

Aguarde o Gradle baixar todas as dependências.

Conecte um dispositivo Android ou utilize um emulador.

Execute o aplicativo pressionando **Run**.

---

# Executando o Sistema

## 1. Carregue o módulo

```bash
sudo insmod ihs_remote.ko
```

---

## 2. Execute o aplicativo Android

Ao abrir o aplicativo:

- informe o endereço IP do computador;
- informe a porta **5558**;
- informe a senha de autenticação.

---

## 3. Conecte

Após a autenticação:

- o canal de controle será estabelecido;
- o canal de vídeo será criado automaticamente;
- a tela do computador começará a ser transmitida.

---

# Verificação

Caso tudo esteja funcionando corretamente:

- o módulo aparecerá em `lsmod`;
- o aplicativo exibirá a tela do computador;
- os comandos de mouse e teclado serão executados normalmente.

---

# Solução de Problemas

## Erro ao compilar

Verifique se os headers do kernel correspondem exatamente à versão do kernel em execução.

```bash
uname -r
```

---

## Não consegue carregar o módulo

Verifique as mensagens do kernel.

```bash
dmesg
```

---

## O aplicativo não conecta

Verifique:

- se computador e celular estão na mesma rede;
- se o endereço IP informado está correto;
- se o módulo foi carregado;
- se a porta 5558 está disponível.

---

## A tela não aparece

Verifique:

- se a GPU utilizada é compatível com DRM;
- se o framebuffer foi localizado corretamente;
- as mensagens exibidas em `dmesg`.
