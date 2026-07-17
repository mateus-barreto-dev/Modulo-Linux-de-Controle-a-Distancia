# Protocolo de Comunicação

## Visão Geral

A comunicação entre o aplicativo Android e o módulo Linux é realizada através de **dois canais TCP independentes**.

| Porta | Finalidade |
|--------|------------|
| 5558 | Controle (Mouse, Teclado e Autenticação) |
| 6666 | Transmissão da Tela |

A separação entre os canais permite que o envio contínuo dos frames da tela não interfira na responsividade dos comandos enviados pelo usuário.

---

# Canal de Controle (TCP 5558)

Após estabelecer a conexão TCP, o cliente deve realizar a autenticação.

## Autenticação

Formato:

```text
AUTH senha
```

Exemplo:

```text
AUTH senha
```

Caso a autenticação seja aceita, o cliente pode iniciar a negociação do canal de vídeo.

---

## Inicialização do Canal de Vídeo

Formato:

```text
CONNECT_UDP <IP_DO_CELULAR> <PORTA>
```

> **Observação:** Apesar do nome do comando permanecer como `CONNECT_UDP`, a implementação atual utiliza **TCP** para transmissão do vídeo.

Exemplo:

```text
CONNECT_UDP 192.168.0.25 6666
```

Resposta do módulo:

```text
UDP_OK
```

Após essa resposta o aplicativo abre uma nova conexão TCP na porta **6666**.

---

# Comandos Suportados

## Movimento do Mouse

Formato

```text
MOVE dx dy
```

Exemplo

```text
MOVE 15 -8
```

Onde

- dx → deslocamento horizontal
- dy → deslocamento vertical

---

## Clique Esquerdo

```text
LBC
```

---

## Clique Direito

```text
RBC
```

---

## Scroll

```text
WHEEL valor
```

Exemplo

```text
WHEEL 3
```

---

## Pressionar uma Tecla

```text
KEY codigo
```

Exemplo

```text
KEY 30
```

O código corresponde ao código do Linux Input Subsystem.

---

## Pressionar (Segurar) uma Tecla

```text
DOWN codigo
```

Exemplo

```text
DOWN 42
```

---

## Soltar uma Tecla

```text
UP codigo
```

Exemplo

```text
UP 42
```

---

## Digitação de Texto

```text
TEXT texto
```

Exemplo

```text
TEXT hello
```

---

# Canal de Vídeo (TCP 6666)

Após o handshake no canal de controle, o aplicativo estabelece uma segunda conexão TCP.

O módulo transmite continuamente os frames da tela.

Cada frame possui dois blocos:

## Cabeçalho

| Campo | Tamanho |
|--------|----------|
| Largura | 4 bytes |
| Altura | 4 bytes |
| Bytes por Pixel | 4 bytes |

Total:

```text
12 bytes
```

Todos os valores são enviados em **Little Endian**.

---

## Dados da Imagem

Logo após o cabeçalho são enviados os pixels da imagem.

Formato:

```text
RGB565
```

Cada pixel ocupa:

```text
2 bytes
```

Quantidade total de bytes:

```text
largura × altura × 2
```

---

# Fluxo da Comunicação

```text
Android
      │
      │ Conecta TCP 5558
      ▼
Kernel

AUTH senha
────────────────────────►

CONNECT_UDP IP 6666
────────────────────────►

UDP_OK
◄────────────────────────

Android abre TCP 6666

◄────────────────────────
Frame 1

◄────────────────────────
Frame 2

◄────────────────────────
Frame 3

...
```

---

# Tratamento de Desconexão

Caso qualquer conexão seja encerrada:

- o socket correspondente é fechado;
- a thread responsável é finalizada;
- o módulo permanece aguardando uma nova conexão.

---

# Observações

- O protocolo é baseado em mensagens de texto para o canal de controle.
- Cada comando deve ser enviado em uma linha, finalizada por `\n`.
- A transmissão de vídeo utiliza um protocolo binário próprio para reduzir overhead.
- O formato RGB565 foi escolhido por reduzir pela metade o volume de dados transmitidos em comparação ao formato BGRA de 32 bits.
