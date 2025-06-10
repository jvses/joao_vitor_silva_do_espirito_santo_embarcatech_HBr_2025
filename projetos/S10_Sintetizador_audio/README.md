
# Projetos de Sistemas Embarcados - EmbarcaTech 2025

Autor: **João Vitor Silva do Espirito Santo**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

## Sintetizador de áudio com Buzzers

Este programa embarcado na BitDogLab grava 22 segundos de áudio ao apertar o botão A e então o reproduz e desenha a onda sonora da gravação no OLED ao apertar o botão B.

Se preferível pelo usuário ele pode alterar os parâmetros no código para reduzir a precisão da captação de som para ter mais tempo de gravação ou então aumentar a precisão, mas com menor tempo de som.

Essas mudanças podem ser feitas alterando os valores dos MACROS no arquivo audio_proc.h. Mas tome cuidado para não dar overflow. A memória é limitada e a gravação é armazenada com um buffer de tipo uint16_t de tamanho (MIC_SAMPLES_RATE * DURATION_SEC) então cuidado para não extrapolar. Eu testei com uma amostragem de 22K amostras por segundo e 5 segundos de gravação e rodou tranquilo, mesmo com tratamento de áudio.

```C++
#define MIC_SAMPLES_RATE 5000 // 10.000 amostras por segundo 10kHz
#define DURATION_SEC 22          // 5 segundos de gravação 
```

Por padrão a filtragem para tirar ruído está desativada pois não vi diferença no áudio com ou sem o tratamento de áudio após a gravação.

E a opção de tratamento de áudio for ativada no código ela pode levar um tempo até ser concluída e ela irá sobrescrever o Buffer da gravação original. Se for tratar o áudio descomente o trecho na main que ele usará um LED azul para sinalizar que está processando o áudio. 
```
	// led_put_color(0,0,1);
	// filtrar_audio();
```
Lembre-se que quanto maior for o buffer que guarda o som mais tempo vai levar para processar. Outro fator de qualidade que aumenta o tempo de processamento é a ordem do filtro `#define FILTER_ORDER 50` que você pode editar no arquivo audio_proc.h também. Em 50 ou 100 a filtragem já é boa. Aumentar para 200 demora muito o processamento e não se vê tanta diferença no resultado final.

Após mudanças(caso o usuário quiser) é só recompilar e embarcar na BitDogLab que irá funcionar.


## 📜 Licença
GNU GPL-3.0.

