# PR #33 — alinhamento do botão Iniciar partida

Ajuste exclusivamente de apresentação sobre `c914270ca359bf7cd1d750ad6b97e5adaccafbe0`, na mesma [PR #33](https://github.com/kaduolliver/Projeto-Glossata/pull/33) e branch `feat/issue-22-play-2.2.3`. Aplicativo 1.9.0 ainda não lançado; catálogo 2.4.0 preservado. Sem merge, release ou avanço de subetapa.

Código e testes em [557afce](https://github.com/kaduolliver/Projeto-Glossata/commit/557afceff027ff8cc1ebb36bac13559cdc9b1640). As capturas e a execução direcionada correspondem às mesmas fontes finais.

Em `TaxonomyView.qml`, a dificuldade ocupa uma coluna flexível e tem texto centralizado nas janelas amplas. Isso permite à grade usar toda a largura interna, com o seletor à esquerda e o botão Iniciar partida alinhado à direita. O botão mantém sua largura intrínseca e a centralização vertical. Em 1000×700, continua na segunda linha, sem sobreposição. A mensagem de organismos elegíveis conserva o alinhamento abaixo dos controles. Não há posicionamento absoluto nem alteração de altura do painel, modelos, regras, dificuldade ou ação de início.

O teste existente `compactPlayConfigurationAndContextualSources` agora confere a borda direita do botão contra a área interna do painel, a largura intrínseca do botão e o alinhamento esquerdo do contador. Mantém as verificações de centralização vertical, ausência de sobreposição, seleção de níveis, dificuldade, grupos vazios/limitados, fontes contextuais, foco, mouse, teclado e redução de movimento nas três resoluções.

As [capturas e a suíte completa da rodada anterior](../configuracao-compacta/README.md) permanecem intactas.

## Validação e capturas

Execução direcionada de `compactPlayConfigurationAndContextualSources`, `playConfigurationReturnsToSelectionContext` e `playSelectionNavigationAndCaptures`: **cinco casos aprovados**, incluindo inicialização/limpeza, sem falhas. Verifica alinhamento nas três resoluções, retorno ao contexto e início Livre, Competitivo e misto. [Log](verificacoes/play-start-alignment.txt) e [resultados](verificacoes/resultados.json). A suíte CTest completa da rodada anterior está preservada; não foi repetida para este ajuste de alinhamento.

Build Release, lint do QML alterado, escopo de licenciamento do código, metadados/histórico de versões, status do projeto, whitespace e [smoke do executável](verificacoes/smoke-test.txt) passaram. O [manifesto](capturas.json) vincula as três novas capturas reais às fontes, executáveis, dimensões, horários e hashes. Bancos temporários e Discord desativado; sem montagem ou edição das fotografias.

| Resolução | Configuração atualizada |
| --- | --- |
| 1000×700 | [Abrir](capturas/configure-1000x700.png) |
| 1240×820 | [Abrir](capturas/configure-1240x820.png) |
| 1920×1080 | [Abrir](capturas/configure-1920x1080.png) |

Windows 11, Qt 6.11.2 software/offscreen, escala Qt 1. Eventos de mouse/teclado pelo QtTest; sem ensaio de GPU física ou leitor de tela. Regras, dados, licenças e governança permanecem intactos. A alteração não afeta as fontes da Home nem exige nova captura dessa tela.
