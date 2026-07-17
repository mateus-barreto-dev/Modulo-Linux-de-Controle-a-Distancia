# Arquitetura

O projeto **IHS Remote** é composto por dois componentes principais:

- Um módulo do kernel Linux responsável pelo acesso ao hardware do computador;
- Um aplicativo Android responsável pela interação com o usuário.

A comunicação entre ambos ocorre através de dois canais TCP independentes.

---

# Visão Geral

```
                 ┌────────────────────────────┐
                 │      Aplicativo Android    │
                 │                            │
                 │ • Interface gráfica        │
                 │ • Mouse                    │
                 │ • Teclado                  │
                 │ • Exibição da tela         │
                 └─────────────┬──────────────┘
                               │
           TCP 5558            │            TCP 6666
      (Controle)               │            (Vídeo)
                               │
                 ┌─────────────▼──────────────┐
                 │    Módulo Linux (Kernel)   │
                 │                            │
                 │ Parser de comandos         │
                 │ Socket TCP                 │
                 │ Threads                    │
                 │ DRM                        │
                 │ Input Subsystem            │
                 └─────────────┬──────────────┘
                               │
        ┌──────────────────────┴─────────────────────┐
        │                                            │
  Linux Input Subsystem                       DRM / Framebuffer
        │                                            │
 Mouse Virtual                                GPU Intel
 Teclado Virtual                              Tela
```

---

# Organização do Módulo

O módulo foi dividido em cinco grandes responsabilidades.

## 1. Dispositivos Virtuais

Logo após o carregamento do módulo são criados dois dispositivos virtuais utilizando o **Linux Input Subsystem**.

- Mouse virtual
- Teclado virtual

Esses dispositivos permitem que o kernel injete eventos exatamente como se um dispositivo físico estivesse conectado ao computador.

---

## 2. Canal de Controle

O módulo cria um servidor TCP na porta **5558**.

Esse servidor é responsável por receber:

- movimento do mouse;
- cliques;
- scroll;
- pressionamento de teclas;
- autenticação;
- comandos de inicialização da transmissão de vídeo.

Todos os comandos recebidos são interpretados pela função responsável pelo parser e convertidos em eventos do Linux Input Subsystem.

Fluxo:

```
Android

↓

TCP Socket

↓

Parser

↓

Mouse Virtual / Teclado Virtual

↓

Sistema Operacional
```

---

## 3. Captura da Tela

A captura da tela é realizada utilizando a API **DRM (Direct Rendering Manager)**.

O módulo identifica:

- GPU Intel
- CRTC ativo
- Framebuffer ativo

Após localizar o framebuffer, sua memória é mapeada utilizando:

```
drm_gem_vmap()
```

A partir desse momento o módulo possui acesso direto aos pixels da tela.

Fluxo:

```
GPU

↓

Framebuffer

↓

drm_gem_vmap()

↓

Memória Linear
```

---

## 4. Conversão da Imagem

O framebuffer fornecido pelo DRM utiliza o formato **BGRA de 32 bits**.

Para reduzir a quantidade de dados transmitidos, cada pixel é convertido para **RGB565**, reduzindo o tamanho da imagem pela metade.

```
BGRA (32 bits)

↓

Conversão

↓

RGB565 (16 bits)
```

---

## 5. Canal de Vídeo

Após a conversão da imagem, o módulo cria um segundo servidor TCP utilizando a porta **6666**.

A transmissão ocorre continuamente em uma Kernel Thread independente.

Cada quadro é enviado seguindo o formato:

```
+----------------------------+
| largura (4 bytes)          |
+----------------------------+
| altura (4 bytes)           |
+----------------------------+
| bytes por pixel (4 bytes)  |
+----------------------------+
| pixels RGB565              |
+----------------------------+
```

O aplicativo Android utiliza esse cabeçalho para reconstruir corretamente cada frame.

---

# Organização das Threads

O módulo utiliza duas Kernel Threads.

## Thread de Controle

Responsável por:

- aceitar conexões;
- receber comandos;
- interpretar comandos;
- controlar mouse e teclado.

---

## Thread de Vídeo

Responsável por:

- capturar framebuffer;
- converter para RGB565;
- transmitir continuamente a tela.

Essa separação impede que o envio da imagem bloqueie o recebimento dos comandos de controle.

---

# Fluxo Completo

```
Usuário

↓

Aplicativo Android

↓

Socket TCP (5558)

↓

Parser

↓

Input Subsystem

↓

Mouse Virtual / Teclado Virtual

↓

Sistema Operacional



GPU

↓

Framebuffer

↓

DRM

↓

Conversão RGB565

↓

Socket TCP (6666)

↓

Aplicativo Android

↓

Tela do Celular
```

---

# Justificativa da Arquitetura

A arquitetura foi dividida em dois canais independentes:

- **Canal de Controle:** responsável pelo envio dos comandos do usuário.
- **Canal de Vídeo:** responsável exclusivamente pela transmissão da imagem.

Essa divisão permite que comandos de mouse e teclado continuem sendo processados mesmo durante a transmissão contínua dos frames da tela, reduzindo atrasos e simplificando o gerenciamento das conexões.
