# IHS Remote

O **IHS Remote** é um sistema de acesso remoto desenvolvido como projeto da disciplina Interface Hardware/Software. O projeto é composto por um **módulo do kernel Linux**, responsável pelo acesso direto ao hardware do computador, e por um **aplicativo Android**, responsável por controlar o computador e exibir sua tela em tempo real.

---

# Objetivo

Desenvolver um módulo Linux capaz de permitir o controle remoto de um computador através de um dispositivo Android, realizando a comunicação diretamente com o kernel e utilizando os subsistemas nativos do Linux para captura da tela e emulação de dispositivos de entrada.

---

# Funcionalidades

## Módulo Linux

O módulo implementa as seguintes funcionalidades:

- Recebimento de comandos de mouse enviados pelo aplicativo;
- Recebimento de comandos de teclado enviados pelo aplicativo;
- Criação de dispositivos virtuais de mouse e teclado utilizando o Linux Input Subsystem;
- Captura do framebuffer da GPU utilizando a API DRM (Direct Rendering Manager);
- Conversão da imagem para o formato RGB565;
- Transmissão contínua da tela para o aplicativo Android através de sockets TCP;
- Gerenciamento simultâneo dos canais de controle e vídeo utilizando Kernel Threads.

## Aplicativo Android

O aplicativo implementa as seguintes funcionalidades:

- Envio de comandos de mouse;
- Envio de comandos de teclado;
- Recepção dos frames enviados pelo módulo Linux;
- Reconstrução da imagem recebida;
- Exibição da tela do computador em tempo real;
- Interface gráfica para interação do usuário.

---

# Arquitetura

*(Adicionar diagrama da arquitetura.)*

---

# Tecnologias Utilizadas

## Kernel

- Linux Kernel Module
- DRM (Direct Rendering Manager)
- Linux Input Subsystem
- Kernel Threads
- Kernel TCP Sockets
- PCI Subsystem

## Aplicativo

- Java
- Android Studio
- Android SDK
- TCP Sockets

---

# Estrutura do Projeto

```text
IHS-Remote
│
├── kernel/
│   └── ihs_remote.c
│
├── android/
│   └── App Android
│
├── docs/
│
└── README.md
```

---

# Como Compilar

## Módulo Linux

```bash
make clean
make
```

## Aplicativo Android

1. Instale o Android Studio;
2. Clone este repositório;
3. Abra o projeto Android;
4. Conecte um dispositivo Android ou utilize um emulador;
5. Execute o aplicativo.

---

# Como Executar

## Módulo Linux

Carregar o módulo:

```bash
sudo insmod ihs_remote.ko
```

Remover o módulo:

```bash
sudo rmmod ihs_remote
```

Verificar mensagens do kernel:

```bash
dmesg -w
```

## Aplicativo Android

Após iniciar o aplicativo:

1. Informar o endereço IP do computador;
2. Informar a porta TCP;
3. Informar a senha de autenticação;
4. Conectar ao módulo Linux.

---

# Demonstração

*(Adicionar capturas de tela e/ou GIFs demonstrando o funcionamento do sistema.)*

---

# Limitações

- O computador e o dispositivo Android devem estar conectados à mesma rede local.
- Atualmente o módulo foi desenvolvido para GPUs Intel compatíveis com DRM.
- A transmissão da tela utiliza o formato RGB565, sem compressão de imagem.
- O sistema foi desenvolvido para ambiente Linux.

---

# Trabalhos Futuros

- Implementar compressão de imagem (JPEG ou H.264);
- Melhorar o teclado virtual;
- Reduzir a latência da transmissão de vídeo;
- Sincronizar a posição do cursor entre computador e aplicativo;
- Adicionar novos comandos de controle remoto;
- Suporte para múltiplos monitores;
- Suporte a outras GPUs e drivers DRM.
