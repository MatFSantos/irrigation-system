# Instruções de uso

Para uso do software, siga os seguintes passos:

- **1°:** clone o repositório para o seu computador.

    - Ao abrir o projeto com o **VSCode**, a extensão do **CMake** irá criar a pasta ``build`` para você com os arquivos de compilação.

    - Caso não seja gerada a pasta, crie uma pasta com nome `build` e execute o seguinte comando dentro da pasta criada:
        
        ``cmake ..``

        O comando acima irá criar os arquivos de compilação.
        
        > Você pode também seguir para o passo 2 sem executar o passo anterior

- **2°:** execute a compilação do firmware usando a extensão do ***Raspberry Pi Pico*** do ***VSCode***.

A partir daqui, seu firmware já está compilado e pronto para uso, e só depende de onde será usado.

## Execução no ambiente de simulação Wokwi

Para utilizar o **firmware* no ambiente de simulação, basta ter a extensão do **Wokwi* instalado no seu **VSCode**, além de ter configurado a sua licença.

Suprindo os requisitos citados acima, basta clicar no arquivo diagram.json, e a simulação já abrirá com o esquemático pronto.

**Clique no botão de play, no canto superior esquerdo e simule!**

## Execução na *BitDogLab*

Siga os passos a seguir para poder executar seu firmware na placa *BitDogLab*:

- **1°:** coloque o seu ***Raspberry*** em modo de ***bootsel***, clicando no botão branco na placa e reiniciando a mesma.

- **2°:** copie o arquivo `.uf2` que foi gerado na pasta `build` para o seu ***Raspberry*** (ele aparecerá como um armazenamento externo, um Pen-Drive com nome de RPI-RP2).

    - Após isso, o seu código já vai está rodando na sua plaquinha ***BitDogLab***.

- **3°:** Está pronto, dê os comandos para executar as funcionalidades clicando nas teclas do seu teclado.

## Instruções para uso do firmware

O firmware que o código gera, simula um controle automatizado de irrigação de plantas. Por meio de um sensor de umidade do solo, o firmware monitora a umidade no local e, caso abaixe a um certo nível (abaixo de 30%), a bomba é ligada automaticamente, permanecendo ligada até que o solo apresente uma umidade superior a 40%.

Um LED vermelho foi empregado para ser um aviso visual de que a umidade do solo está abaixo de 30%, e um LED azul é um aviso visual de que a bomba de irrigação está ligada.

Dois botões também são usados, um para que o usuário possa colocar o sistema em modo manual, e o outro para assinar a bomba (em caso de uso do sistema em modo manual). Um som é emitido para avisar que o modo manual foi ativado/desativado.

Além do sensor de umidade do solo, é usado um sensor de temperatura, para monitorar a temperatura ambiente, para posteriores ***features*** que façam uso de tal recurso. Ambos os sensores são simulados com os potenciômetros, e foram usados um sensor LM35 e um sensor comum de umidade como base para montar o software simulado.

O LM35, como tem uma sensibilidade baixa, foi usado um shift na leitura de 1.4 *Volts* para que a leitura seja o mais real possível com base na placa ***BitDogLab***.

O Display OLED é usado para mostrar as medidas, status da *bomba* e do ***modo manual*** e também para mostrar avisos, como uma mensagem quando a umidade está abaixo de 30%.

A Matriz 5x5 é usada para mostrar um rosto que indica, de forma visual, quando a umidade está em um nível bom (com um sorriso em verde), em um nível intermediário (rosto neutro em branco) e em um nível baixo (rosto triste em vermelho).

## Vídeo Ensaio

Clique em ***[link do video]()*** para visualizar o vídeo ensaio do projeto.
