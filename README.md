# EMARA

## Planning
| Nº | Designação da atividade | Descrição | Início | Fim |
|---:|---|---|---|---|
| 1 | Análise do estado da arte | Revisão da literatura e análise comparativa das soluções existentes. | 21/10/2026 | 30/10/2026 |
| 2 | Inventário de materiais | Identificação dos materiais necessários e realização dos respetivos pedidos. | 21/10/2026 | 25/10/2026 |
| 3 | Dashboard I | Implementação de um servidor web no ESP32 e desenvolvimento do sistema de definição e envio de trajetórias. | 21/10/2026 | 25/10/2026 |
| 4 | Sistema de comunicação série | Desenvolvimento da comunicação série entre o ESP32 e o Wio-E5 mini da estação de base. | 26/10/2026 | 31/10/2026 |
| 5 | Sistema de comunicação LoRa | Desenvolvimento da comunicação LoRa entre o Wio-E5 mini da estação de base e o Wio-E5 mini do barco. | 01/11/2026 | 14/11/2026 |
| 6 | Protótipo I | Teste do envio de trajetórias e comunicação do dashboard e do Wio-E5 mini da estação de base. | 15/11/2026 | 15/11/2026 |
| 7 | Sistema de navegação e motores I | Desenvolvimento de um sistema básico de controlo dos motores com base numa trajetória predefinida. | 16/11/2026 | 22/11/2026 |
| 8 | Sistema de sensores | Desenvolvimento do software necessário para a aquisição e leitura dos dados dos sensores. | 16/11/2026 | 22/11/2026 |
| 9 | Dashboard II | Implementação da representação textual e gráfica dos dados obtidos pelos sensores. | 23/11/2026 | 24/11/2026 |
| 10 | Protótipo II | Integração e teste do envio de trajetórias do dashboard para o barco, do controlo dos motores e da transmissão dos dados desde os sensores até ao ESP32. | 25/11/2026 | 28/11/2026 |
| 11 | Sistema de navegação e motores II | Desenvolvimento de um sistema de adaptação da trajetória com base na direção e intensidade da correnteza. | 29/11/2026 | 09/12/2026 |
| 12 | Dashboard III | Melhorias da interface do dashboard, com foco na usabilidade e na apresentação da informação. | 29/11/2026 | 09/12/2026 |
| 13 | Produto final | Melhorias da robustez estrutural do protótipo e do seu aspeto visual. | 10/12/2026 | 10/12/2026 |
| 14 | Validação final | Repetição dos testes do Protótipo II com o sistema aperfeiçoado. | 11/12/2026 | 12/12/2026 |


## Trajectory
### Format
#### Example
{
    "lat": 51.505,
    "lng": -0.09
}
