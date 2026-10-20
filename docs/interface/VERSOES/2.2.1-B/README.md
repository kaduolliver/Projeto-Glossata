# 2.2.1-B — Descoberta do Dia e Indicadores Dinâmicos

Subetapa exclusiva da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22), baseada na `main` em `27294a18ffd0d1e533dcdeb47b44ee0011e97b35` (PR #30 integrado). A versão identifica a revisão visual; aplicativo 1.7.0 e catálogo 2.4.0 continuam em desenvolvimento, sem lançamento. Commit final e PR: pendentes de criação.

Telas afetadas: Início, com Descoberta do Dia abaixo do carrossel e três indicadores do catálogo. O carrossel, seu diálogo de créditos, Explorar, Jogar e os textos científicos existentes são preservados. `StatCard` recebe uma descrição opcional, vazia nos usos anteriores.

## Decisões

- Seleção offline: IDs positivos únicos em ordem crescente, índice igual ao dia civil local (`QDate::toJulianDay`) módulo quantidade elegível. A mesma data e o mesmo catálogo/revisões produzem a mesma descoberta em outra execução. Não há repetição em datas consecutivas quando o conjunto permanece igual e tem pelo menos dois organismos; com apenas um, a repetição é inevitável. Mudanças reais do catálogo ou perda de elegibilidade podem alterar a seleção no mesmo dia; conteúdo inválido não é conservado para sustentar a estabilidade visual.
- Elegibilidade: regra existente `approvedForQuiz(species, "species")`, fotografia com papel `identification`, nó científico da espécie navegável e resumo de `EducationalContent.profile` com revisão editorial/especializada utilizável e referência HTTPS. Não se exige um conjunto de quatro alternativas. Fotografias de apoio e textos legados sem revisão editorial utilizável não viram descobertas. O resumo é reutilizado integralmente, sem reescrita ou transferência de aprovação.
- Indicadores: IDs únicos registrados; IDs de domínios presentes nos caminhos desses organismos; organismos com fotografia de identificação utilizável segundo a regra acima. O último é uma contagem de organismos, não de arquivos, e exclui imagens de apoio. A utilização local não comprova liberação para distribuição nem versão de licença ausente.
- `TaxonomyIndex` produz uma projeção coerente do catálogo; `GameController` publica propriedades com notificação somente ao mudar. Um timer de 30 segundos relê a data local e a projeção, inclusive as revisões dos textos, e a ativação do aplicativo atualiza imediatamente. O catálogo de produção é um snapshot imutável carregado pelo adaptador SQLite: uma atualização do conteúdo instalado é refletida na próxima inicialização. Não há importação em execução, monitoramento de arquivos ou novo banco.
- A ação da descoberta abre o nó exato da espécie no contexto Explorar existente, sem iniciar ou modificar partida. A interface mantém créditos locais, licença conforme declarada, fonte e aviso de revisão editorial/especializada, além de estado sem conteúdo elegível.

## Verificação e capturas

Verificação em 20/10/2026: compilação Release com Qt 6.11.2/GCC 16.2.0 em Windows 11, saída isolada em `build/2.2.1-B-verified`, sem encerrar o aplicativo habitual ou abrir bancos pessoais.

- **26/26 suítes CTest aprovadas**, em 303,81 s. [Domínio/aplicação](verificacoes/core-results.txt): 56 casos aprovados; [interface Qt/QML](verificacoes/ui-test-results.txt): 11 casos aprovados, sem avisos.
- Seleção estável, reinicialização no mesmo dia, timer com relógio injetado, mudança e retorno de data, ano bissexto, rotação sem repetição consecutiva, um/zero elegíveis, IDs duplicados, alteração dos dados e exclusão por foto, identificação, revisão vencida/legada e fonte ausente. Fixtures de revisão são sintéticas, apenas em memória; não geram decisões nos dados distribuídos.
- Interface: três resoluções, fonte e resumo correspondentes ao registro, estado vazio, fonte científica em itálico, contraste de textos, Tab/Shift+Tab, foco trazido à área visível, acesso direto a Explorar/créditos e retomada de quiz sem mudar foto/alternativas. O acesso leva ao nó taxonômico da espécie já disponível em Explorar; não cria outra ficha nem reformula a tela.
- Regressões da PR #30: quatro slides, metadados sincronizados, crossfade de 600 ms, intervalos reais de oito segundos, pausas/retomadas, mouse/teclado, redução de movimento e sessão preservada.
- Qmllint de todos os QML, auditoria estrita do catálogo, escopo Apache, integridade de release/captura, metadados do projeto e higiene do repositório aprovados. A verificação técnica não concede revisão especializada ou licença.
- Comparação com a base: `resources/data`, `resources/images` e `resources/compliance` idênticos; 122 organismos, 855 receitas e todos os 1.053 eventos de governança preservados. Aplicativo 1.7.0 e catálogo 2.4.0 continuam não lançados, sem alteração de identidade do catálogo.
- Gate de distribuição continua **bloqueado**, com zero erros técnicos, 120 pendências de fotografias e 275 bloqueios científicos. Nenhum gate foi contornado e nenhuma release/tag foi criada.

### Valores observados

| Indicador | Valor | Significado |
| --- | ---: | --- |
| Organismos catalogados | 122 | IDs positivos únicos carregados do catálogo |
| Domínios representados | 2 | Bacteria e Eukaryota; Archaea sem organismo não entra na contagem |
| Cobertura fotográfica | 20 | Organismos com foto de identificação utilizável no app; imagens de apoio não entram |

Os 122 registros de fotografias tecnicamente válidos e os 40 elegíveis no nível espécie no relatório geral não são esta métrica: o indicador também exige as revisões existentes e exclui o papel `supporting`. Não conta arquivos, fotos de exploração/carrossel nem os representa como novas identificações.

Nas capturas de 20/10/2026, a descoberta é *Panthera onca* (onça-pintada), foto `742746729`, por William Stephens conforme o crédito da fonte. A licença continua CC BY declarada, com versão não comprovada; a nova apresentação não altera esse registro. O resumo e sua fonte Animal Diversity Web são reutilizados da nota editorial existente, com escopo, hash e revisão preservados no [contexto do manifesto](capturas.json).

### Capturas reais

| Estado | 1000 × 700 | 1240 × 820 | 1920 × 1080 |
| --- | --- | --- | --- |
| Início da página, com carrossel preservado | [Captura](capturas/home-1000x700.png) | [Captura](capturas/home-1240x820.png) | [Captura](capturas/home-1920x1080.png) |
| Descoberta e indicadores, após rolagem | [Captura](capturas/discovery-1000x700.png) | [Captura](capturas/discovery-1240x820.png) | [Captura](capturas/discovery-1920x1080.png) |

São seis PNGs selecionados da janela Qt real, produzidos por `dailyDiscoveryNavigationIndicatorsAndCaptures`, com QML/controlador de produção, SQLite temporário, Discord desativado e movimento desativado apenas para os quadros estáticos. O relógio injetado é restaurado à data local real antes de capturar. A rolagem enquadra a nova seção, mantendo foco de teclado no botão. Não há montagem, retoque ou imagem simulada. As regressões exercitam separadamente as animações ativas.

[capturas.json](capturas.json) registra horários UTC reais dos arquivos, dimensões, hashes dos PNGs, fontes, testes, binários e contexto dos dados. A [captura oficial do README](../../../images/tela-inicial.png) foi refeita por `scripts/capture_home.py`, com o executável de produção, e tem [registro independente](../../../images/tela-inicial.json). O procedimento agora inclui também os arquivos de seleção/educação nos hashes das fontes da Home. Versões visuais anteriores permanecem intactas.

Comandos utilizados, a partir da raiz com Qt no PATH:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$PWD/build/2.2.1-B-verified"
cmake --build build -j 4
ctest --test-dir build --output-on-failure
python scripts/capture_home.py --binary build/2.2.1-B-verified/projeto_glossata.exe
python scripts/audit_catalog.py --strict
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/content_governance.py audit-history --base 27294a18ffd0d1e533dcdeb47b44ee0011e97b35
python scripts/release.py check
python scripts/distribution.py check
```

O CTest grava os quadros em `build/tests/previews/daily-discovery/`. O manifesto arquiva os seis quadros listados, sem transformar pixels; a captura oficial usa o procedimento próprio acima. Qmllint foi executado para todos os arquivos de `resources/qml` com `--unqualified disable`.

Limites: renderizador Qt software/offscreen, escala 1 e 96 DPI. Outras GPUs, sistemas, escalas e leitores de tela não foram ensaiados. Datas de validade das notas são verificadas pelo mecanismo existente; fotos e taxonomia usam os estados do snapshot carregado. A fotografia proporcional da descoberta preserva o arquivo e seus créditos; a licença do código não relicencia fotos, textos ou identidade visual.
