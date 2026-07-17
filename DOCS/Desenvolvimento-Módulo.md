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

# 3. Desenvolvimento do Aplicativo Android

## 3.1 Estrutura Geral

O aplicativo Android foi desenvolvido utilizando a linguagem Java e possui como principal objetivo servir como interface entre o usuário e o módulo Linux executado no computador.

Sua arquitetura foi organizada em torno de uma única Activity (`MainActivity`), responsável por concentrar tanto a interface gráfica quanto os mecanismos de comunicação com o módulo.

Durante sua execução, o aplicativo mantém dois canais de comunicação independentes:

- um canal TCP para envio dos comandos de controle;
- um canal TCP exclusivo para recepção contínua das imagens da tela.

Além disso, o aplicativo implementa uma fila de comandos (`BlockingQueue`), responsável por desacoplar a captura dos eventos da interface da transmissão efetiva pela rede, evitando bloqueios na interface do usuário.

Internamente, o aplicativo é dividido em sete componentes principais:

- Interface gráfica;
- Comunicação TCP de controle;
- Recepção do fluxo de vídeo;
- Reconstrução das imagens;
- Captura de gestos;
- Conversão dos gestos em comandos;
- Controle do teclado virtual.

Essa divisão permitiu separar responsabilidades e facilitar futuras expansões do sistema.

---

## 3.2 Interface Gráfica

A interface foi construída utilizando componentes nativos do Android.

O principal elemento da tela é um componente `ImageView`, responsável por exibir continuamente os quadros recebidos do computador.

Além da área de visualização da tela remota, foram adicionados três botões de ação rápida:

- Conectar ao computador;
- Abrir o teclado virtual;
- Encerrar a conexão.

Durante o processo de conexão é apresentado um diálogo solicitando:

- endereço IP do computador;
- porta TCP;
- senha de autenticação.

Após a autenticação bem sucedida, a interface passa a exibir continuamente a tela do computador, enquanto todos os gestos realizados sobre ela são convertidos em comandos enviados ao módulo.

---

## 3.3 Comunicação TCP

A comunicação entre o aplicativo e o módulo ocorre através de dois sockets TCP independentes.

O primeiro socket é utilizado exclusivamente para o envio de comandos de controle.

Após estabelecer a conexão, o aplicativo realiza um processo de autenticação enviando uma senha previamente configurada.

Uma vez autenticado, o aplicativo informa ao módulo seu endereço IP e a porta utilizada para a recepção do fluxo de vídeo.

Todos os comandos de mouse e teclado são inseridos em uma fila (`BlockingQueue`), sendo enviados continuamente por uma thread dedicada à comunicação.

Essa abordagem evita bloqueios na interface gráfica e garante que os comandos sejam transmitidos na mesma ordem em que foram gerados pelo usuário.

---

## 3.4 Recepção da Tela

Após o processo de autenticação, o aplicativo estabelece uma segunda conexão TCP destinada exclusivamente à transmissão da tela.

O módulo envia continuamente uma sequência composta por:

- um cabeçalho contendo largura, altura e formato da imagem;
- os pixels correspondentes ao quadro capturado.

Uma thread independente permanece bloqueada aguardando novos quadros.

Sempre que um novo frame é recebido, seus dados são armazenados em buffers previamente alocados, reduzindo a quantidade de alocações dinâmicas durante a execução.

Essa estratégia contribui para diminuir o tempo gasto pelo coletor de lixo (Garbage Collector) e melhora a estabilidade da transmissão.

---

## 3.5 Reconstrução da Imagem

Os pixels enviados pelo módulo chegam ao aplicativo no formato RGB565, exatamente na mesma organização utilizada pela GPU.

Assim que um quadro é recebido, o aplicativo percorre todos os pixels realizando duas etapas principais.

Na primeira etapa ocorre a reorganização espacial dos pixels, reconstruindo a imagem original a partir da organização em tiles utilizada pela memória gráfica.

Na segunda etapa cada pixel RGB565 é convertido para o formato ARGB8888 utilizado internamente pelo Android.

Após essa conversão é criado um objeto `Bitmap`, que é imediatamente exibido no componente `ImageView` da interface.

Todo esse processo ocorre para cada quadro recebido, permitindo que a tela do computador seja atualizada continuamente.

---

## 3.6 Captura dos Gestos

A interação do usuário ocorre diretamente sobre a área de exibição da tela remota.

Os eventos de toque são capturados utilizando os mecanismos nativos do Android (`MotionEvent` e `GestureDetector`).

Os seguintes gestos foram implementados:

- movimento de um dedo para controlar o cursor do mouse;
- toque duplo para clique esquerdo;
- toque prolongado para clique direito;
- movimento com dois dedos para realizar a rolagem da roda do mouse.

Além desses gestos, foi implementado um teclado virtual utilizando um campo de texto invisível, permitindo capturar os caracteres digitados pelo usuário.

Essa abordagem elimina a necessidade de desenvolver um teclado personalizado.

---

## 3.7 Conversão dos Gestos em Comandos

Após a captura dos gestos, o aplicativo converte cada interação em comandos pertencentes ao protocolo definido pelo sistema.

Por exemplo:

- movimentação do dedo → `MOVE dx dy`
- rolagem → `WHEEL valor`
- clique esquerdo → `LBC`
- clique direito → `RBC`
- pressionamento de tecla → `KEY código`

Os comandos são inseridos em uma fila de transmissão, garantindo que sejam enviados exatamente na ordem em que ocorreram.

No caso do teclado, cada caractere digitado é convertido para o código correspondente do Linux Input, permitindo que o módulo gere exatamente os mesmos eventos que um teclado físico produziria.

Essa camada de tradução torna o aplicativo independente da implementação interna do módulo Linux, permitindo que apenas o protocolo de comunicação seja compartilhado entre ambos.

---

# 4. Decisões de Projeto

## 4.1 Por que Linux Kernel Module?

A principal exigência do projeto era realizar a comunicação diretamente com o hardware do computador.

Implementar a solução como um módulo do kernel permite acesso direto aos subsistemas internos do Linux, eliminando a necessidade de softwares intermediários em espaço de usuário.

Além disso, essa abordagem possibilitou utilizar diretamente o subsistema Linux Input para emular dispositivos de entrada e o DRM para acessar a memória gráfica da GPU.

Outra vantagem é que toda a lógica crítica permanece centralizada no kernel, reduzindo a latência entre o recebimento dos comandos e sua execução.

---

## 4.2 Por que DRM?

A captura da tela poderia ser realizada por diferentes mecanismos disponíveis no Linux.

Entretanto, o DRM (Direct Rendering Manager) oferece acesso direto ao framebuffer utilizado pela GPU, permitindo capturar a imagem exatamente como ela é armazenada na memória de vídeo.

Essa abordagem evita depender de ambientes gráficos específicos (como X11 ou Wayland) e permite capturar a tela independentemente da aplicação em execução.

Além disso, o acesso ocorre inteiramente dentro do kernel, atendendo aos requisitos do projeto.

---

## 4.3 Por que Input Subsystem?

O Linux Input é o subsistema responsável por gerenciar todos os dispositivos de entrada do sistema operacional.

Ao criar dispositivos virtuais utilizando essa infraestrutura, o módulo passa a ser reconhecido pelo sistema exatamente como um mouse e um teclado físicos.

Dessa forma, qualquer aplicação do computador recebe os eventos normalmente, sem necessidade de adaptações ou bibliotecas adicionais.

Essa abordagem também mantém total compatibilidade com o restante do sistema operacional.

---

## 4.4 Por que duas conexões TCP?

O sistema utiliza duas conexões TCP independentes.

A primeira é dedicada aos comandos de controle, que possuem pequeno volume de dados e exigem baixa latência.

A segunda é utilizada exclusivamente para o envio contínuo das imagens da tela.

Separar essas duas funcionalidades evita que grandes transmissões de vídeo atrasem comandos de mouse ou teclado.

Além disso, a utilização de conexões independentes simplifica a implementação das threads responsáveis por cada serviço.

---

## 4.5 Por que RGB565?

Inicialmente as imagens eram transmitidas utilizando 32 bits por pixel (BGRA8888), exatamente como armazenadas pela GPU.

Entretanto, essa abordagem produzia um elevado volume de dados, reduzindo significativamente a taxa de atualização da tela.

A conversão para RGB565 reduz cada pixel de quatro bytes para apenas dois bytes.

Com isso, o volume de dados transmitido é reduzido pela metade, permitindo aumentar a taxa de quadros sem perda perceptível de qualidade para o objetivo do projeto.

A conversão para ARGB8888 é realizada apenas no aplicativo Android, imediatamente antes da exibição.

---

## 4.6 Por que utilizar Kernel Threads?

Algumas operações realizadas pelo módulo executam continuamente durante toda a sua vida útil.

Entre elas destacam-se:

- o servidor responsável pelos comandos de controle;
- a captura e transmissão contínua da tela.

Executar essas tarefas utilizando Kernel Threads permite que cada serviço opere de forma independente e bloqueante, sem impedir a execução dos demais componentes do módulo.

Essa abordagem simplifica o código, melhora a organização da aplicação e facilita o gerenciamento das conexões estabelecidas com o aplicativo Android.

# 5. Dificuldades Encontradas

Durante o desenvolvimento do projeto diversos desafios foram encontrados, principalmente por se tratar de uma aplicação executada inteiramente no espaço de kernel e que interage diretamente com componentes internos do sistema operacional.

As principais dificuldades são descritas a seguir.

---

## 5.1 Captura da Tela

A maior dificuldade encontrada durante o desenvolvimento foi descobrir uma forma confiável de capturar a tela diretamente do kernel.

Inicialmente foram estudadas alternativas utilizando o framebuffer tradicional (`fbdev`), porém essa abordagem mostrou-se limitada, pois muitos sistemas atuais utilizam o DRM (Direct Rendering Manager) como principal infraestrutura gráfica.

Após diversos testes, optou-se por utilizar diretamente a infraestrutura DRM, obtendo acesso ao framebuffer associado ao CRTC principal.

Outra dificuldade foi compreender a organização interna dos objetos DRM, como:

- CRTC;
- Primary Plane;
- Framebuffer;
- GEM Objects.

Foi necessário estudar a documentação do subsistema DRM e analisar diversos exemplos presentes no código-fonte do kernel Linux para compreender o caminho até os pixels armazenados na memória de vídeo.

---

## 5.2 Organização do Framebuffer

Mesmo após conseguir acessar a memória gráfica, a imagem obtida inicialmente apresentava completamente corrompida.

Durante os testes foi observado que a GPU Intel não armazenava os pixels de forma linear na memória.

Em vez disso, a memória encontrava-se organizada em pequenos blocos (tiles), utilizados para melhorar o desempenho do hardware.

Foi necessário analisar cuidadosamente o padrão de organização da memória e desenvolver um algoritmo capaz de reconstruir corretamente a posição de cada pixel.

Após a reorganização espacial dos pixels, tornou-se possível reconstruir corretamente a imagem no aplicativo Android.

---

## 5.3 Comunicação Kernel ↔ Android

Outro desafio importante foi estabelecer uma comunicação estável entre o módulo Linux e o aplicativo Android.

Inicialmente foi considerada a utilização de apenas uma conexão TCP para transmitir comandos e imagens.

Entretanto, verificou-se que o grande volume de dados das imagens causava atrasos perceptíveis no envio dos comandos de mouse e teclado.

Como solução, optou-se pela utilização de duas conexões TCP independentes:

- uma dedicada aos comandos de controle;
- outra dedicada exclusivamente à transmissão da tela.

Também foi necessário implementar um mecanismo simples de autenticação e um processo de sincronização para que a transmissão da tela somente fosse iniciada após o estabelecimento da conexão de controle.

---

## 5.4 Sincronização das Threads

O módulo utiliza múltiplas Kernel Threads executando simultaneamente.

Uma thread permanece aguardando conexões de controle, enquanto outra realiza continuamente a captura e transmissão das imagens.

Durante o desenvolvimento foram encontrados problemas relacionados ao encerramento dessas threads quando o módulo era removido.

Inicialmente algumas chamadas bloqueantes impediam o encerramento correto do módulo, ocasionando erros durante a remoção.

A solução adotada consistiu em fechar previamente os sockets responsáveis pelas conexões, fazendo com que as chamadas bloqueantes retornassem imediatamente e permitissem o encerramento seguro das threads utilizando `kthread_stop()`.

Essa abordagem tornou o carregamento e descarregamento do módulo significativamente mais robusto.

---

# 6. Resultados Obtidos

Ao final do desenvolvimento foi possível implementar um sistema funcional capaz de controlar remotamente um computador Linux diretamente através de um aplicativo Android.

O módulo desenvolvido foi capaz de:

- criar um mouse virtual utilizando o Linux Input;
- criar um teclado virtual utilizando o Linux Input;
- receber comandos enviados pela rede;
- interpretar os comandos recebidos;
- capturar continuamente a tela utilizando o DRM;
- transmitir as imagens em tempo real para o aplicativo Android.

O aplicativo Android, por sua vez, foi capaz de:

- estabelecer conexão com o módulo;
- autenticar o usuário;
- enviar comandos de mouse e teclado;
- receber continuamente as imagens transmitidas pelo computador;
- reconstruir corretamente os quadros recebidos;
- exibir a tela remota em tempo real.

Durante os testes foi possível controlar o computador integralmente utilizando apenas o celular, incluindo movimentação do cursor, cliques, rolagem da roda do mouse, digitação e visualização da tela.

Os objetivos inicialmente propostos para o projeto foram, portanto, alcançados com sucesso.

---

# 7. Trabalhos Futuros

Embora o sistema desenvolvido seja plenamente funcional, diversas melhorias podem ser implementadas em trabalhos futuros.

Entre elas destacam-se:

- implementação de compressão de imagem para reduzir o volume de dados transmitidos;
- utilização de protocolos de transmissão mais adequados para vídeo em tempo real, como UDP com mecanismos próprios de recuperação de perdas;
- suporte a múltiplos monitores;
- implementação de escalonamento automático da resolução transmitida conforme a largura de banda disponível;
- melhoria do teclado virtual, incluindo suporte completo a caracteres especiais e atalhos do sistema operacional;
- adição de novos gestos para facilitar a interação com o computador;
- autenticação mais robusta utilizando criptografia e troca segura de chaves;
- transmissão protegida por TLS;
- adaptação para GPUs de outros fabricantes, como AMD e NVIDIA;
- desenvolvimento de uma interface gráfica mais completa para o aplicativo Android;
- otimizações adicionais visando reduzir a latência entre a captura da tela e sua exibição no dispositivo móvel.

Essas melhorias podem ampliar a compatibilidade do sistema, aumentar seu desempenho e torná-lo mais adequado para aplicações em ambientes reais.
