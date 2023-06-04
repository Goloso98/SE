# SE
Trabalho de smart home para a cadeira de sistemas embutidos

## esp8266
Na pasta `esp8266` encontra-se o projeto que corre nos arduinos.
Foi usado PlatformIO, onde se existem build flags para alterar o `roomid` ou o sensor a utilizar.

## rpi
O sistema foi montado em docker para facilitar o deploy, isto nao inclui credenciais, sendo necessario 
configura-las primeiro.
Existe tambem um script em python extra para a gestao do audio na casa, que se
encontra dentro da pasta `docker/audio`.
Para alem disso, o ficheiro `flow.json`, sao as configuracoes do nodered.

