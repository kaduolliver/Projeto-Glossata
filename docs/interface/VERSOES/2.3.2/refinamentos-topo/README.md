# 2.3.2 — Painéis no topo e rolagem compartilhada

Revisão da [PR #36](https://github.com/kaduolliver/Projeto-Glossata/pull/36) sobre `e02e43bf8bc8f4d2d190402edff3657cdce3524d`. Preserva a composição visual aprovada, o diagnóstico de métricas e todas as evidências da [entrega inicial](../README.md).

Escopo: colocar taxonomia/ranking logo após o cabeçalho, oferecer alternância em telas compactas com contexto em memória e habilitar a rolagem convencional existente em Estatísticas. A documentação multiplayer será uma proposta separada, sem implementação de infraestrutura, mudança de backend, SQLite, dados ou regras. Aplicativo 1.11.0 ainda não lançado e catálogo 2.4.0 preservados.

## Alterações

A ordem passa a ser cabeçalho, painéis taxonômico/ranking, partida atual, desempenho, dicas e histórico. Em larguras úteis de pelo menos 900 pixels, os painéis permanecem lado a lado, com as proporções anteriores e alinhamento superior; cada um acompanha a altura do conteúdo. Em larguras menores, os seletores **Desempenho taxonômico** e **Ranking** exibem um painel de cada vez, logo após o cabeçalho. Modelos, grupos e posições permanecem disponíveis.

`Main.statisticsPresentation.panel` guarda somente a seleção de apresentação em memória. A escolha sobrevive à navegação para outras páginas e ao redimensionamento, sem escrita em banco ou mudança de sessão. Os botões exclusivos permanecem marcados mesmo ao clicar novamente na opção ativa. Tab/Espaço e o foco visível são preservados; quando o redimensionamento oculta o painel focado, o foco passa ao seletor do painel selecionado.

`pageWheelArea` apenas inclui `statistics` no mesmo mecanismo de Início/Explorar. O passo de 110 pixels, animação e redução de movimento são os existentes. Pixels de touchpad e ângulos parciais continuam nativos. A pressão da barra já interrompe a animação compartilhada; não foi criado outro Flickable ou rolagem interna nos painéis. O espaço reservado para a barra permanece igual.

## Planejamento multiplayer

[ESTATISTICAS_MULTIPLAYER.md](../../../../architecture/ESTATISTICAS_MULTIPLAYER.md) separa diagnóstico existente de propostas futuras. Descreve perfis, resultados, rankings e histórico locais; possibilidades de cliente/API/autenticação/banco online; modelo conceitual; validação de resultados; idempotência; sincronização; privacidade e compatibilidade offline. REST, OAuth/OIDC e PostgreSQL são candidatos para avaliação, sem decisão de implementação presumida.

O documento distingue histórico local importado de partida validada pelo servidor. Hash, UUID ou consistência matemática local não tornam um resultado competitivo confiável. Uma futura seleção **Local | Global** é somente proposta; o aplicativo continua apresentando **Ranking local**. Nenhum dado de jogador foi enviado a um serviço multiplayer, e o roadmap principal não foi alterado.

## Rastreabilidade e preservação

Somente `StatisticsView.qml` e `Main.qml` mudam no código da aplicação. Backend, banco, perfis, pontos, dificuldade, dicas, recuperação, catálogo e governança permanecem intactos. Os testes iniciais de Estatísticas são mantidos, com a adaptação de navegação necessária para o novo seletor compacto. As capturas, manifestos e resultados iniciais da PR #36 permanecem nos mesmos arquivos.

Como `Main.qml` integra o shell compartilhado, a regra de captura do projeto exige renovar a imagem real da Home. A captura oficial em `docs/images/tela-inicial.png` passa a mostrar o topo, com manifesto correspondente e sem alterar a Home. A [imagem anterior da Descoberta do Dia](anterior-home.png) e seu [registro original](anterior-home.json), copiados dos bytes de `e02e43b`, são preservados nesta revisão; o README mantém ambos os enquadramentos e o caminho oficial. A diferença entre PNGs vem de enquadramento real do programa, sem edição de pixels.

Verificações, commit de implementação e manifestos desta revisão serão completados após a execução final. As capturas usam QML/controlador reais e SQLite temporário, sem bancos pessoais.
