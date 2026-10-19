# Arquitetura visual atual

Base: commit `b201e20c2d83fda6d790bc6a96217c0fdc75307e`. A interface de produção é `resources/qml/Main.qml`, embarcada por `resources.qrc` e carregada em `src/app/main.cpp`. `GameController` fornece o estado via contexto `gameController`; `DiscordRichPresence` fornece o estado opcional da presença. As telas são componentes em `resources/qml/views/`; os controles reutilizáveis ficam em `resources/qml/components/`.

`Main.qml` cria uma `ApplicationWindow` de 1240 × 820, mínima de 1000 × 700. Mantém barra lateral, cabeçalho, avisos de recuperação/salvamento e área rolável. A propriedade `page` seleciona um `Component` pelo `Loader`; `navigate`, `openPlay` e os métodos de início/continuação de partida conduzem as transições. `sessionStarted`, `playing` e os estados do controlador determinam para onde “Jogar” retorna.

| Tela (`page`) | Componente | Responsabilidade e entrada principal |
| --- | --- | --- |
| `home` | `HomeView` | Abertura, convite para jogar e resumo do catálogo; entrada pelo menu Início. |
| `categories` | `CategoryView` | Modo Livre/Competitivo, dificuldade das alternativas, acesso rápido e três domínios; entrada por Jogar. |
| `taxonomy` | `TaxonomyView` | Breadcrumbs, fonte/introdução, grupos filhos e quiz elegível no nível selecionado; entrada por cartão ou atalho. |
| `quiz` | `QuizView` | Fotografia, quatro alternativas, pontuação possível e painel de dicas; início ou continuação da partida. |
| `feedback` | `FeedbackView` | Resultado da resposta, aprendizado após responder e avanço; chega após confirmação. |
| `info` | `InfoView` | Ficha da espécie, classificação, tópicos, fontes e estados de revisão; chega pelo feedback. |
| `results` | `ResultsView` | Resumo e próximos caminhos; chega ao concluir a partida. |
| `statistics` | `StatisticsView` | Métricas da sessão, histórico por perfil, grupos e rankings Livre/Competitivo; menu lateral. |
| `achievements` | `AchievementsView` | Conquistas; menu lateral. |
| `settings` | `SettingsView` | Perfis, transições e presença no Discord; menu lateral. |
| `credits` | `CreditsView` | Créditos, fontes e transparência; acessada por Perfis e ajustes. |

Componentes compartilhados: `UiButton`, `NavItem`, `UiIcon`, `TextAction`, `TaxonComboBox` e `StatCard` compõem controles recorrentes; `PlayModeSelector` serve à exploração geral e aos grupos; `ExploreCard`, `FeatheredPhoto`, `SpeciesPortrait` e artes associadas apresentam organismos; `HintPanel` e `EndGameDialog` cobrem decisões durante a partida. A lógica de pontos e disponibilidade das dicas permanece no domínio/controlador, não na aparência desses componentes.

Fluxo principal: Início → Explorar → grupo opcional → Quiz → Feedback → Ficha opcional → próxima questão/Resultado. Menu lateral abre Estatísticas, Conquistas e Perfis; Créditos ficam dentro de Perfis. Uma sessão recuperável aparece em aviso global antes do conteúdo. Voltar de um quiz aciona confirmação de encerramento; algumas transições dependem de checkpoint ou resultado pendente.

## Observações de UX/UI para avaliação posterior

- `credits` não possui entrada em `pageTitles` de `Main.qml`; o cabeçalho usa o título genérico, embora a tela tenha conteúdo específico. O caminho por Ajustes também torna créditos menos visíveis.
- Cores, tamanhos, raios e espaçamentos são declarados repetidamente nas views e nos componentes. Mudanças globais exigem localizar valores em vários arquivos.
- A largura mínima de 1000 px e o `implicitHeight` mínimo de 950 px em Estatísticas exigem rolagem em janelas compactas; as capturas de 1240 × 820 não verificam uso com escala de sistema, leitores de tela ou outras plataformas.
- A seleção da barra lateral destaca “Jogar” em exploração, quiz, feedback, ficha e resultado; ela indica uma área ampla, enquanto o cabeçalho usa o nome de cada estado. Convém avaliar se essa hierarquia comunica claramente o contexto.
- `CategoryView` posiciona filtros e acesso rápido acima dos cartões. A área rolável pode ocultar os cartões inferiores; a versão compacta precisa continuar sendo conferida quando houver mudança de densidade.

Esses itens são observações da implementação e de capturas reais, **não correções nesta subetapa**.
