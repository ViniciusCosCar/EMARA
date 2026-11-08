# EMARA

## Planning

| Nº da Atividade | Designação | Descrição | Início | Fim |
|-----------------|------------|-----------|---------|-----|
| A1 | Dashboard I | Implementação de um servidor web no ESP32-web e desenvolvimento do sistema de definição e envio de trajetórias. | 08-Nov | 14-Nov |
| A2 | Comunicação UART | Desenvolvimento da comunicação série entre ESP32 e o módulo LoRa-E5. | 08-Nov | 14-Nov |
| A3 | Comunicação LoRa | Desenvolvimento da comunicação LoRa entre os dois ESP32 através dos módulos LoRa-E5. | 08-Nov | 21-Nov |
| A4 | Protótipo I | Teste do sistema de comunicação: testar comunicação entre ESP32-web e ESP32-barco. | 22-Nov | 22-Nov |
| A5 | Navegação e Motores I | Desenvolvimento de um sistema básico de controlo dos motores com base numa trajetória predefinida. | 22-Nov | 28-Nov |
| A6 | Navegação e Motores II | Leitura da IMU de 9 eixos e envio da posição GNSS ao ESP32-web + atualização da posição do barco na Dashboard. | 22-Nov | 28-Nov |
| A7 | Protótipo II | Repetir testes da etapa A4 e testar o funcionamento do sistema de navegação desenvolvido na etapa A5. | 29-Nov | 29-Nov |
| A8 | Navegação e Motores III | Desenvolvimento de um sistema de adaptação da trajetória com base nos dados obtidos pela IMU implementada na etapa A5. | 29-Nov | 09-Dec |
| A9 | Dashboard III | Desenvolvimento de uma nova interface gráfica com foco na usabilidade e na apresentação da informação. | 29-Nov | 09-Dec |
| A10 | Produto Final | Aperfeiçoamento estrutural do protótipo e do seu aspeto visual. | 10-Dec | 10-Dec |
| A11 | Validação Final | Repetição da etapa A7 com o sistema aperfeiçoado. | 11-Dec | 12-Dec |

## Trajectory
### Format
#### Example
{
    "lat": 51.505,
    "lng": -0.09
}
