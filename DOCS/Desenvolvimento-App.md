# 1. Arquitetura Geral do Aplicativo

## 1.1 Objetivo do Aplicativo

O aplicativo Android foi desenvolvido para atuar como a interface de comunicação entre o usuário e o módulo Linux executado no computador.

Seu principal objetivo é permitir que um dispositivo móvel controle remotamente o computador através da rede local, enviando comandos de mouse e teclado enquanto recebe continuamente a imagem da tela capturada pelo módulo.

Diferentemente do módulo Linux, responsável pela interação direta com o hardware do computador, o aplicativo concentra-se na captura das interações do usuário, na comunicação com o kernel e na exibição da tela remota.

---

## 1.2 Organização Geral da Aplicação

Toda a lógica do aplicativo encontra-se centralizada na classe `MainActivity`, responsável por inicializar a interface gráfica, capturar as interações do usuário e gerenciar a comunicação com o módulo Linux.

Internamente o aplicativo pode ser dividido em cinco blocos principais:

### Interface gráfica

Responsável pela construção da interface apresentada ao usuário.

São disponibilizados:

- área sensível ao toque utilizada como touchpad;
- botão para abertura do teclado virtual;
- botão para encerramento da conexão;
- campo oculto utilizado para capturar os caracteres digitados pelo teclado do Android.

---

### Captura das interações

Toda interação realizada pelo usuário é convertida em eventos de alto nível.

Entre eles estão:

- movimentação do dedo;
- gestos de múltiplos toques;
- duplo toque;
- toque prolongado;
- digitação pelo teclado virtual.

Esses eventos são posteriormente convertidos para comandos compreendidos pelo módulo Linux.

---

### Gerenciamento da comunicação

Uma conexão TCP é estabelecida com o computador.

Essa conexão permanece ativa durante toda a execução do aplicativo e é responsável pelo envio dos comandos gerados pela interface.

Para evitar bloqueios da interface gráfica, toda a comunicação é executada em uma thread independente.

---

### Gerenciamento da fila de comandos

Os comandos produzidos pelas diversas partes da interface não são enviados imediatamente.

Todos eles são inseridos em uma fila (`BlockingQueue`), que funciona como intermediária entre a interface gráfica e a thread responsável pela comunicação.

Essa abordagem desacopla completamente a geração dos comandos do processo de transmissão pela rede.

---

### Conversão de comandos

Antes do envio, cada interação do usuário é convertida para o protocolo textual definido pelo projeto.

Exemplos:

```text
MOVE 15 -8
```

```text
WHEEL 2
```

```text
KEY 30
```

```text
LBC
```

```text
RBC
```

Esses comandos são enviados diretamente ao módulo Linux, que interpreta e executa a ação correspondente.

---

## 1.3 Fluxo de Funcionamento

O funcionamento do aplicativo ocorre conforme a sequência abaixo.

1. O usuário inicia o aplicativo.

2. A interface gráfica é carregada.

3. É criada uma thread responsável pela comunicação TCP.

4. O aplicativo estabelece conexão com o módulo Linux.

5. O processo de autenticação é realizado.

6. O usuário começa a interagir com a tela do dispositivo.

7. Cada gesto é convertido em um comando do protocolo.

8. O comando é inserido na fila de transmissão.

9. A thread de comunicação remove os comandos da fila e os envia ao computador.

10. O módulo Linux interpreta cada comando recebido e executa a operação correspondente.

Essa arquitetura mantém a interface sempre responsiva, mesmo durante transmissões contínuas de comandos.

---

# 2. Interface Gráfica

## 2.1 Estrutura da Interface

A interface foi construída utilizando um único arquivo XML (`activity_main.xml`) baseado em um `ConstraintLayout`.

Essa escolha permitiu posicionar todos os componentes de forma simples e eficiente, ocupando praticamente toda a área útil da tela.

Os principais componentes presentes na interface são:

- título da aplicação;
- área sensível ao toque;
- botão para abertura do teclado virtual;
- botão para encerramento da conexão;
- campo oculto para captura do teclado.

Toda a lógica associada a esses componentes é implementada na classe `MainActivity`.

---

## 2.2 Área de Controle (Touchpad)

A maior parte da tela funciona como um touchpad virtual.

Todos os eventos de toque são capturados através de um `OnTouchListener` associado ao layout principal.

Durante a movimentação do dedo são calculadas as diferenças entre a posição atual e a posição anterior, produzindo deslocamentos relativos nos eixos X e Y.

Esses deslocamentos são posteriormente convertidos em comandos do tipo:

```text
MOVE dx dy
```

Quando dois dedos são detectados simultaneamente, a mesma área passa a representar a roda de rolagem do mouse, permitindo o envio de comandos de scroll.

---

## 2.3 Botão do Teclado

O aplicativo utiliza um botão flutuante para abrir o teclado virtual do Android.

Ao ser pressionado, esse botão transfere o foco para um campo de texto oculto (`EditText`) e solicita ao sistema operacional a exibição do teclado.

Essa estratégia permite reutilizar o teclado nativo do Android sem a necessidade de implementar um teclado próprio.

Os caracteres digitados são capturados imediatamente e convertidos em comandos de teclado enviados ao computador.

---

## 2.4 Botão de Desconexão

Outro botão flutuante é responsável pelo encerramento da sessão.

Quando acionado, ele realiza o fechamento ordenado da conexão TCP, interrompe a thread responsável pela comunicação e encerra a atividade atual.

Esse procedimento evita conexões pendentes e libera corretamente todos os recursos utilizados pelo aplicativo.

---

## 2.5 Área de Captura do Teclado

Embora invisível para o usuário, existe um componente `EditText` utilizado exclusivamente para capturar os caracteres digitados.

Sempre que um caractere é inserido, um `TextWatcher` identifica a nova entrada.

O caractere é convertido para o código correspondente do teclado Linux e imediatamente enviado para a fila de comandos.

Após o envio, o conteúdo do campo é apagado automaticamente, permitindo a captura contínua dos próximos caracteres.

---

## 2.6 Detecção dos Gestos

Os gestos mais complexos são processados através da classe `GestureDetector`.

Entre os gestos reconhecidos destacam-se:

- duplo toque, utilizado para simular um clique esquerdo;
- toque prolongado, utilizado para simular um clique direito.

Esses gestos geram comandos específicos do protocolo:

```text
LBC
```

```text
RBC
```

A utilização do `GestureDetector` simplificou significativamente o reconhecimento dessas interações, evitando a implementação manual de algoritmos de detecção de gestos.

# 3. Comunicação com o Módulo Linux

## 3.1 Estabelecimento da Conexão TCP

A comunicação entre o aplicativo Android e o módulo Linux é realizada utilizando sockets TCP.

Durante a inicialização do aplicativo é criada uma thread dedicada exclusivamente à comunicação com o computador. Essa thread estabelece uma conexão TCP com o endereço IP e a porta configurados no módulo Linux.

No projeto atual, o endereço IP e a porta encontram-se definidos diretamente no código:

Após a criação do socket é obtido um objeto `PrintWriter`, responsável pelo envio das mensagens utilizando protocolo textual.

Toda a comunicação é realizada utilizando essa conexão enquanto o aplicativo permanecer em execução.

---

## 3.2 Processo de Autenticação

Logo após estabelecer a conexão, o aplicativo realiza um processo simples de autenticação.

Esse procedimento consiste no envio da mensagem:

```text
AUTH IHSREMOTE2026
```

O módulo Linux verifica essa mensagem antes de aceitar qualquer comando enviado pelo aplicativo.

Caso a autenticação seja bem-sucedida, a conexão permanece ativa.

Caso contrário, o módulo encerra imediatamente a comunicação.

Embora simples, esse mecanismo impede que dispositivos desconhecidos enviem comandos diretamente ao computador.

---

## 3.3 Gerenciamento da Comunicação

Após a autenticação, a thread de comunicação permanece executando continuamente.

Seu funcionamento consiste em remover comandos da fila de transmissão e enviá-los imediatamente ao módulo Linux.

O envio ocorre através da chamada:

```java
writer.println(sendCMD);
```

Como a thread permanece bloqueada aguardando novos elementos na fila (`BlockingQueue`), não há utilização desnecessária da CPU quando o usuário não está interagindo com a interface.

Essa abordagem também evita que operações de rede bloqueiem a interface gráfica do Android.

---

## 3.4 Fila de Comandos

Todas as interações realizadas pelo usuário são inicialmente armazenadas em uma fila do tipo `BlockingQueue`.

Sua declaração é realizada da seguinte forma:

```java
private final BlockingQueue<String> fila =
        new LinkedBlockingQueue<>();
```

Sempre que algum gesto é reconhecido, o comando correspondente é inserido nessa fila através da função:

```java
offerCommand(String command)
```

Essa função encapsula a operação:

```java
fila.offer(command);
```

A utilização dessa estrutura oferece diversas vantagens:

- desacopla a interface gráfica da comunicação em rede;
- garante que os comandos sejam enviados na ordem correta;
- elimina problemas de concorrência entre múltiplos eventos de toque;
- simplifica significativamente a sincronização entre threads.

---

## 3.5 Envio dos Comandos

Todos os comandos seguem um protocolo textual simples, definido especificamente para este projeto.

Alguns exemplos são:

```text
MOVE 15 -4
```

```text
WHEEL -2
```

```text
KEY 30
```

```text
DOWN 42
```

```text
UP 42
```

```text
LBC
```

```text
RBC
```

Cada mensagem representa uma operação específica que será interpretada pelo parser implementado no módulo Linux.

A utilização de comandos em formato textual simplificou o desenvolvimento, facilitou os testes utilizando ferramentas de rede e tornou o protocolo facilmente extensível.

---

## 3.6 Encerramento da Conexão

Quando o usuário pressiona o botão de desconexão, o aplicativo realiza o encerramento ordenado da comunicação.

As seguintes operações são executadas:

- fechamento do `PrintWriter`;
- fechamento do socket TCP;
- interrupção da thread de comunicação;
- encerramento da atividade (`finish()`).

Esse procedimento garante que todos os recursos sejam liberados corretamente e evita conexões pendentes no módulo Linux.

---

# 4. Captura das Interações do Usuário

## 4.1 Detecção dos Gestos

Toda a interação do usuário ocorre sobre a área principal da interface.

Os eventos de toque são capturados utilizando um `OnTouchListener`, enquanto gestos mais complexos são reconhecidos através da classe `GestureDetector`.

Essa combinação permite detectar diferentes tipos de interação utilizando uma única superfície de controle.

---

## 4.2 Movimento do Mouse

Quando apenas um dedo encontra-se sobre a tela, o aplicativo interpreta o movimento como deslocamento do cursor.

A posição atual do dedo é continuamente comparada com sua posição anterior.

Os deslocamentos relativos são calculados pelas expressões:

```java
dX = posX - lastX;
dY = posY - lastY;
```

Esses valores são enviados para a função:

```java
moverMouse(dX, dY);
```

Para reduzir o envio de comandos desnecessários, pequenos movimentos são ignorados.

Somente deslocamentos superiores a dois pixels geram comandos para o computador.

---

## 4.3 Cliques

Os cliques do mouse são implementados utilizando gestos específicos.

Um duplo toque é interpretado como um clique esquerdo.

Nesse caso é enviado o comando:

```text
LBC
```

Um toque prolongado é interpretado como um clique direito.

Nesse caso é enviado:

```text
RBC
```

A utilização desses gestos elimina a necessidade de botões adicionais na interface, tornando a navegação mais natural.

---

## 4.4 Scroll

Quando dois dedos são posicionados simultaneamente sobre a tela, o aplicativo altera automaticamente o modo de operação.

Nesse modo, a média da posição vertical dos dois dedos passa a representar o deslocamento da roda do mouse.

A diferença entre duas posições consecutivas produz um valor proporcional ao scroll.

Esse valor é convertido em comandos como:

```text
WHEEL 3
```

ou

```text
WHEEL -2
```

permitindo realizar rolagens tanto para cima quanto para baixo.

---

## 4.5 Entrada pelo Teclado Virtual

A digitação utiliza o teclado nativo do Android.

Quando o usuário pressiona o botão correspondente, um campo de texto invisível recebe o foco e solicita ao sistema operacional a abertura do teclado virtual.

Cada caractere digitado é capturado por um `TextWatcher`.

Esse caractere é convertido para o código correspondente do teclado Linux através da função:

```java
apertarTecla(char c)
```

Após a conversão, o comando é inserido na fila de transmissão.

O campo de texto é imediatamente limpo para permitir a captura contínua dos próximos caracteres.

---

## 4.6 Conversão dos Gestos em Comandos

Após reconhecer uma interação do usuário, o aplicativo converte essa ação para o protocolo utilizado pelo módulo Linux.

A conversão é realizada pelas funções:

- `moverMouse()`;
- `moverScroll()`;
- `apertarTecla()`;
- reconhecimento de gestos através do `GestureDetector`.

Cada função gera uma mensagem textual compatível com o parser implementado no kernel.

Essa separação entre captura de gestos e geração dos comandos tornou o código mais organizado, facilitando futuras expansões do protocolo sem alterar a lógica da interface.

# 5. Captura dos Gestos

A interação do usuário com o computador remoto é realizada através da captura dos gestos executados sobre a tela do dispositivo Android. Para isso, o aplicativo utiliza dois mecanismos complementares fornecidos pelo framework Android: o `OnTouchListener`, responsável por capturar continuamente a posição dos dedos sobre a tela, e o `GestureDetector`, utilizado para reconhecer gestos mais complexos, como duplo toque e pressionamento longo.

A área principal da aplicação funciona como um touchpad virtual. Todo evento de toque recebido é processado e convertido em ações equivalentes às realizadas em um mouse convencional.

Os gestos atualmente suportados são:

- movimentação de um dedo para controlar o cursor do mouse;
- gesto com dois dedos para controlar a rolagem da roda do mouse;
- duplo toque para executar o clique esquerdo;
- pressionamento longo para executar o clique direito.

Durante o processamento dos eventos, o aplicativo acompanha continuamente a posição atual e a posição anterior dos dedos na tela, permitindo calcular os deslocamentos necessários para mover o cursor ou realizar a rolagem.

Essa abordagem tornou o controle bastante semelhante ao touchpad encontrado em notebooks, proporcionando uma utilização intuitiva sem exigir hardware adicional.

---

# 6. Conversão dos Gestos em Comandos

Após reconhecer um gesto, o aplicativo converte a ação realizada pelo usuário para o protocolo textual utilizado na comunicação com o módulo Linux.

Cada tipo de interação gera um comando específico, que posteriormente é inserido na fila de transmissão responsável pela comunicação TCP.

Os principais comandos utilizados são:

| Gesto | Comando enviado |
|--------|-----------------|
| Movimento do dedo | `MOVE dx dy` |
| Rolagem com dois dedos | `WHEEL valor` |
| Duplo toque | `LBC` |
| Pressionamento longo | `RBC` |
| Pressionar tecla | `KEY código` |
| Manter tecla pressionada | `DOWN código` |
| Soltar tecla | `UP código` |

No caso da movimentação do cursor, o aplicativo calcula a diferença entre a posição atual e a posição anterior do dedo. Esse deslocamento relativo é enviado ao computador na forma do comando:

```text
MOVE dx dy
```

Como o módulo Linux utiliza um mouse virtual baseado em movimentação relativa, não é necessário transmitir coordenadas absolutas da tela.

Para a rolagem, o deslocamento vertical médio dos dois dedos é convertido em um valor inteiro proporcional ao movimento realizado.

Já os eventos de clique são representados por comandos simples (`LBC` e `RBC`), reduzindo o volume de dados transmitidos.

Os comandos relacionados ao teclado utilizam códigos compatíveis com o subsistema Linux Input. Antes do envio, cada caractere digitado é convertido para seu respectivo keycode através da função `apertarTecla()`, que realiza o mapeamento entre caracteres ASCII e códigos de tecla reconhecidos pelo kernel.

Todos os comandos produzidos são inseridos em uma fila (`BlockingQueue<String>`), permitindo que a interface gráfica permaneça completamente independente da comunicação de rede. 
Uma thread dedicada é responsável por retirar continuamente os comandos dessa fila e transmiti-los ao computador, garantindo que as interações do usuário permaneçam responsivas mesmo em situações de atraso na rede.

# 7. Tratamento de Eventos

Grande parte do funcionamento do aplicativo consiste em capturar as interações realizadas pelo usuário na tela do celular e convertê-las em comandos compreendidos pelo módulo Linux.

Para isso, foram utilizados diferentes mecanismos disponibilizados pela API do Android, permitindo reconhecer movimentos, toques, gestos e entradas de teclado.

---

## 7.1 Eventos de Toque

Todos os eventos de toque são capturados através do método:

```java
setOnTouchListener(...)
```

Esse método recebe continuamente os eventos gerados pela tela sensível ao toque do dispositivo.

Cada evento contém informações como:

- posição dos dedos;
- quantidade de dedos sobre a tela;
- tipo do evento (pressionar, mover ou soltar).

Essas informações são utilizadas para determinar qual ação deverá ser executada.

---

## 7.2 Movimentação do Mouse

Quando apenas um dedo está sobre a tela, o aplicativo interpreta esse movimento como o deslocamento do cursor do mouse.

A cada atualização são calculadas as diferenças entre a posição atual e a posição anterior:

```text
ΔX = posição atual X − posição anterior X
ΔY = posição atual Y − posição anterior Y
```

Esses deslocamentos são enviados ao módulo utilizando o comando:

```text
MOVE dx dy
```

O módulo recebe esses valores e utiliza o subsistema Linux Input para movimentar o cursor.

Foi adotado um pequeno limite mínimo de deslocamento para evitar o envio de comandos causados por pequenas oscilações naturais do toque.

---

## 7.3 Rolagem (Scroll)

Quando dois dedos permanecem simultaneamente sobre a tela, o aplicativo altera automaticamente o modo de interpretação dos eventos.

Nesse caso é calculada a posição média entre os dois dedos.

A variação vertical dessa média representa a direção e a intensidade da rolagem.

O comando enviado ao módulo possui o formato:

```text
WHEEL valor
```

O módulo converte esse valor em eventos da roda do mouse utilizando o Linux Input.

---

## 7.4 Gestos

Gestos mais complexos são reconhecidos utilizando a classe:

```java
GestureDetector
```

Essa classe permite identificar automaticamente diferentes tipos de interação sem que seja necessário implementar manualmente toda a lógica de reconhecimento.

Os gestos utilizados foram:

- duplo toque;
- toque longo.

---

## 7.5 Clique Esquerdo

O gesto de duplo toque é interpretado como um clique do botão esquerdo do mouse.

Quando detectado, o aplicativo envia:

```text
LBC
```

O módulo recebe esse comando e gera um clique esquerdo através do dispositivo virtual.

---

## 7.6 Clique Direito

O gesto de pressionar a tela durante um período maior (Long Press) representa um clique com o botão direito.

O comando enviado é:

```text
RBC
```

Após receber esse comando, o módulo gera um clique direito utilizando o mouse virtual.

---

## 7.7 Entrada de Texto

Para permitir a digitação, foi utilizado um `EditText` invisível presente na interface.

Quando o usuário pressiona o botão do teclado, esse campo recebe o foco e o teclado virtual do Android é aberto.

Cada caractere digitado dispara um evento do tipo:

```java
TextWatcher
```

O caractere é então convertido para o respectivo código de tecla Linux e enviado ao módulo.

Após o envio, o conteúdo do campo é imediatamente apagado para que a próxima tecla possa ser capturada individualmente.

Essa abordagem simplifica o processamento e reduz o volume de dados enviados pela rede.

---

## 7.8 Encerramento da Conexão

O aplicativo possui um botão dedicado ao encerramento da sessão.

Ao ser pressionado são executadas as seguintes etapas:

- interrupção da thread de comunicação;
- fechamento do `PrintWriter`;
- fechamento do socket TCP;
- encerramento da Activity.

Esse procedimento garante que todos os recursos sejam liberados corretamente e evita conexões pendentes tanto no Android quanto no módulo Linux.

---

# 8. Gerenciamento das Threads

O aplicativo foi desenvolvido utilizando múltiplas threads para impedir que operações de rede bloqueassem a interface gráfica.

Essa separação permite manter a aplicação responsiva mesmo durante o envio contínuo de comandos.

---

## 8.1 Thread Principal (UI Thread)

A Thread Principal é responsável exclusivamente pela interface gráfica.

Nela são executadas atividades como:

- renderização da interface;
- captura dos eventos de toque;
- abertura do teclado virtual;
- atualização dos componentes visuais.

Nenhuma operação de rede é realizada nessa thread, evitando travamentos da aplicação.

---

## 8.2 Thread de Comunicação

Logo após a criação da Activity é iniciada uma thread dedicada à comunicação com o computador.

Essa thread realiza:

- abertura do socket TCP;
- autenticação junto ao módulo Linux;
- espera por novos comandos;
- envio contínuo dos comandos presentes na fila.

O funcionamento ocorre continuamente enquanto a aplicação permanece conectada.

---

## 8.3 Fila de Comandos

A comunicação entre a interface gráfica e a thread responsável pela rede é realizada através de uma estrutura do tipo:

```java
LinkedBlockingQueue<String>
```

Sempre que algum evento ocorre na interface, o comando correspondente é colocado nessa fila.

Exemplo:

```text
MOVE 12 -8
```

ou

```text
KEY 30
```

A thread de comunicação permanece bloqueada aguardando novos elementos na fila.

Quando um comando é inserido, ele é imediatamente transmitido ao módulo Linux.

Essa abordagem elimina problemas de concorrência e garante que os comandos sejam enviados exatamente na ordem em que foram produzidos.

---

## 8.4 Sincronização

A utilização da `LinkedBlockingQueue` elimina a necessidade de mecanismos explícitos de sincronização como mutexes ou semáforos.

A própria estrutura garante:

- acesso concorrente seguro;
- bloqueio automático quando não existem comandos;
- liberação automática quando novos comandos são adicionados.

Isso simplifica significativamente o código e reduz a possibilidade de condições de corrida.

---

## 8.5 Encerramento das Threads

Quando o usuário encerra a conexão, a thread de comunicação é interrompida.

Durante esse processo são fechados:

- o socket TCP;
- o `PrintWriter`;
- a própria thread.

Após a liberação dos recursos, a Activity é finalizada, encerrando completamente o aplicativo.

# 9. Encerramento do Desenvolvimento

O desenvolvimento do aplicativo Android teve como objetivo fornecer uma interface simples e intuitiva para o controle remoto de um computador executando Linux, mantendo toda a lógica de baixo nível concentrada no módulo do kernel.

Durante o projeto foram implementadas funcionalidades para:

- estabelecimento da conexão TCP com o computador;
- autenticação do cliente;
- captura dos gestos realizados pelo usuário;
- conversão desses gestos em comandos do protocolo definido;
- envio contínuo dos comandos de controle;
- abertura do teclado virtual do Android;
- envio de caracteres individuais para o computador;
- gerenciamento da conexão durante toda a execução da aplicação.

A arquitetura adotada procurou separar claramente as responsabilidades da aplicação. Enquanto a interface gráfica é responsável apenas pela interação com o usuário, uma thread independente realiza toda a comunicação de rede, permitindo que a interface permaneça responsiva mesmo durante longos períodos de utilização.

Além disso, a utilização de uma fila de comandos permitiu desacoplar completamente a geração dos eventos de entrada do envio efetivo pela rede, reduzindo problemas relacionados à concorrência entre múltiplas ações do usuário.

O aplicativo foi desenvolvido para operar em conjunto com o módulo Linux, seguindo rigorosamente o protocolo de comunicação definido para o projeto. Dessa forma, novas funcionalidades podem ser adicionadas futuramente sem alterações significativas na arquitetura geral da aplicação.

Ao término do desenvolvimento, o aplicativo tornou-se capaz de controlar remotamente o computador de maneira transparente ao usuário, constituindo a interface responsável pela interação entre o dispositivo móvel e o módulo implementado no kernel Linux.
