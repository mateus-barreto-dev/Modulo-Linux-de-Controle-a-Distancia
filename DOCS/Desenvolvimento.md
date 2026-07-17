# 1. Organização do módulo

O módulo foi desenvolvido como um único módulo de kernel (`ihs_remote.ko`), responsável por concentrar todas as funcionalidades necessárias para o funcionamento do sistema. Essa abordagem foi escolhida para facilitar a comunicação entre os diferentes componentes, reduzir a sobrecarga de sincronização e permitir que todos compartilhassem o mesmo contexto de execução dentro do kernel.

Internamente, o módulo é dividido em cinco blocos principais.

## Emulação de dispositivos de entrada

Este bloco é responsável pela criação dos dispositivos virtuais utilizados para controlar o computador remotamente.

Suas responsabilidades são:

- criação do mouse virtual;
- criação do teclado virtual;
- envio de eventos para o subsistema **Linux Input**.

---

## Comunicação de controle

Este bloco implementa o canal responsável pela comunicação entre o aplicativo Android e o módulo do kernel.

Suas responsabilidades são:

- criação do servidor TCP de controle;
- recebimento dos comandos enviados pelo aplicativo;
- interpretação dos comandos recebidos;
- encaminhamento dos comandos para as rotinas responsáveis por mouse, teclado e gerenciamento da transmissão de vídeo.

---

## Captura da tela

Este bloco realiza a captura das imagens exibidas pelo computador.

Para isso são utilizadas as interfaces fornecidas pelo **Direct Rendering Manager (DRM)**.

Suas responsabilidades são:

- localizar a GPU Intel compatível;
- acessar o framebuffer através da infraestrutura DRM;
- ler continuamente a memória de vídeo;
- converter o formato interno da GPU para um formato adequado à transmissão.

---

## Transmissão de vídeo

Após a captura da tela, este bloco é responsável pelo envio contínuo das imagens para o dispositivo Android.

Suas responsabilidades são:

- criação de um servidor TCP dedicado exclusivamente ao vídeo;
- fragmentação automática dos dados em blocos menores;
- envio sequencial dos quadros capturados;
- controle da taxa de atualização da tela.

---

## Inicialização e encerramento

Este bloco concentra toda a preparação e finalização do módulo.

Durante a inicialização são realizados:

- criação dos dispositivos virtuais;
- inicialização dos servidores TCP;
- criação das threads do kernel;
- localização da GPU.

Durante a remoção do módulo são realizados:

- encerramento das threads;
- fechamento dos sockets;
- remoção da interface `/proc`;
- remoção dos dispositivos virtuais;
- liberação de todos os recursos utilizados.

Essa divisão permitiu manter responsabilidades bem definidas dentro do código, mesmo permanecendo em um único arquivo durante o desenvolvimento inicial.

---

# 2. Inicialização do módulo

Toda a inicialização ocorre na função `create_devices()`, responsável por preparar completamente o ambiente antes que o módulo passe a aceitar conexões.

A sequência de inicialização ocorre na seguinte ordem.

---

## 2.1 Localização da GPU

Inicialmente o módulo procura automaticamente uma GPU Intel presente no sistema através da interface PCI.

Após localizar o dispositivo, obtém a estrutura DRM correspondente, que será utilizada posteriormente para acessar o framebuffer responsável pela captura da tela.

Caso nenhuma GPU compatível seja encontrada, o carregamento do módulo é interrompido.

---

## 2.2 Criação do mouse virtual

Em seguida é criado um dispositivo virtual utilizando o subsistema **Linux Input**.

Durante essa etapa são configurados:

- movimentação relativa dos eixos X e Y;
- roda de rolagem;
- botão esquerdo;
- botão direito.

Após sua configuração, o dispositivo é registrado no kernel, tornando-se um mouse completamente funcional para qualquer aplicação do sistema operacional.

---

## 2.3 Criação do teclado virtual

Após o mouse é criado um teclado virtual.

Todos os códigos de tecla suportados pelo **Linux Input** são habilitados para permitir o envio de qualquer tecla posteriormente.

Assim como ocorre com o mouse, o teclado é registrado no kernel e passa a ser reconhecido pelo sistema operacional como um dispositivo físico.

---

## 2.4 Inicialização do servidor de vídeo

Antes mesmo da conexão do aplicativo Android, o módulo cria um servidor TCP dedicado exclusivamente à transmissão da tela.

Esse servidor permanece aguardando uma conexão na porta configurada para vídeo.

Paralelamente também é criada uma thread responsável pela captura contínua da tela. Inicialmente essa thread apenas aguarda a conexão do cliente, iniciando o envio das imagens somente após o processo de autenticação.

---

## 2.5 Criação da interface `/proc`

É criada uma entrada no sistema de arquivos virtual:

```text
/proc/ihs_remote
```

Essa interface foi utilizada durante o desenvolvimento para permitir testes locais do parser de comandos sem a necessidade de estabelecer conexões pela rede.

Os comandos escritos nesse arquivo seguem exatamente o mesmo protocolo utilizado pelo aplicativo Android.

---

## 2.6 Inicialização do servidor de controle

Na sequência é criado um segundo servidor TCP.

Este servidor é responsável por:

- aceitar conexões do aplicativo;
- autenticar o cliente;
- receber comandos de mouse;
- receber comandos de teclado;
- iniciar a transmissão da tela.

Após sua criação são executadas as etapas tradicionais da API de sockets:

1. criação do socket;
2. configuração da opção `SO_REUSEADDR`;
3. associação à porta TCP (`bind`);
4. início do estado de escuta (`listen`).

---

## 2.7 Criação da thread principal

Por fim é criada a thread responsável pelo servidor de controle.

Essa thread permanece bloqueada aguardando conexões. Quando um cliente estabelece comunicação, ela passa a receber continuamente os comandos enviados pelo aplicativo Android.

Cada comando recebido é encaminhado ao parser, que identifica sua operação e executa a rotina correspondente dentro do módulo.

Após essa etapa, toda a infraestrutura do sistema encontra-se pronta para receber conexões e iniciar o controle remoto do computador.
