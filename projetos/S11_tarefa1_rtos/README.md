
# Tarefa: Roteiro de FreeRTOS #1 - EmbarcaTech 2025

Autor: **João Vitor Silva do Espirito Santo**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Brasília, Junho de 2025

### Funcionamento

Nesse código são criadas 3 Tasks. Uma para alternância dos LEDs Vermelho, Verde e Azul, uma para tocar o Buzzer por um tempo bem curto e um para a checagem dor botões apertados que irão desabilitar  um dos serviços. O botão A desativa os LEDs enquanto ele estiver apertado e o botão B desativa o Buzzer enquanto estiver pressionado.

#### Desafios Extras feitos

* Mudança das prioridades
> Para um funcionamento melhor do programa foi feito uma mudança na prioridade da Task responsável pela suspensão e retomada das outras duas tasks, ou seja, a Task dos botões recebeu uma prioridade maior.
Sem ela como prioritária há uma chance de atraso maior na suspensão dos outros serviços. Quiçá a competição pela atenção do processador entre essas 3 faça com que demore demais até ela ser atendida e a ergonomia do aparelho para uso humano seja comprometida.


### Reflexões

* Se todas as tarefas tiverem mesma prioridade isso compromete um pouco o comportamento esperado, principalmente se a tarefa usada para a suspensão das outras tiver a mesma hierarquia. Se uma tarefa depende de outra, a dependente tem prioridade menor. Quando elas têm prioridades iguais o tempo dedicado a ambas são os mesmos o que, ao testar, é perceptível uma alteração na responsividade do sistema e o atraso na percepção dos botões seria mais perceptível para o usuário comum, a suspensão ou resumo das tasks também iriam demorar e o tempo entre bips do buzzer ficaria menos consistente.
* A task de maior consumo seria aquela com menor tempo de delay e espera, que é a task checadora dos botões. Portanto por ser requisitada muitas vezes e com prioridade hierárquica ela consome mais. A 2° provavelmente é a task do Buzzer devido aos controles PWM exigir mais do sistema para controlar ele. Apesar de ele ficar muito tempo em Delay os momentos que são ativadas as vibrações para o bip há um esforço da CPU para processar isso. O 3° é provavelmente a task de LEDs, a troca do estado de saída de 3 pinos é algo muito mais fácil de fazer e de menor esforço da CPU. Além de ser o mais simples de ser feito ele tem uma pausa de 500ms até a próxima alteração.
* Os riscos envolvidos a não usar o polling com prioridade na hierarquia ocasiona nos defeitos descritos no primeiro tópico de reflexão. A latência aumenta, a CPU é mais exigida, eventos são perdidos e os tempos de resposta ficam comprometidos.

## 📜 Licença
GNU GPL-3.0.
