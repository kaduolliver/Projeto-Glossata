# Evolução e auditoria integrada — Projeto Glossata

> Relatório histórico desta etapa, sem atualização retroativa de versões/resultados. Para fontes e contagem de testes atuais, consulte [ESTADO_DO_PROJETO.md](ESTADO_DO_PROJETO.md).

Execução sequencial em 16/10/2026. Aplicativo **1.6.0**, catálogo **2.4.0**, ambos **não lançados**. Código em C++17/Qt 6/QML, SQLite e execução offline preservados. Não houve push, release, tag, reescrita de histórico ou acesso a bancos reais de jogadores.

## Entregas por etapa

| Área | Resultado e evidência |
| --- | --- |
| GitHub e Git | `6cf7dc6`: logs remotos confirmaram Git ausente; dependência MSYS2/preflight corrigidos. Auditor de blobs do índice, hook e testes adversariais impedem artefatos/formatos conhecidos de segredos. [Diagnóstico](AUDITORIA_REPOSITORIO.md). Nenhuma nova execução remota foi alegada |
| Fotografias | `b3e99e4`: 11 novos arquivos, 98 → 109 fotos. Associações científicas 503 → 533; acrescentando o misto, 504 → 534/623 cartões (80,90% → 85,71%). Pendências 119 → 89. Todos os 623 cartões têm associação válida ou justificativa. [Auditoria](AUDITORIA_FOTOGRAFIAS.json) |
| Rastreabilidade | Evidência Commons por arquivo; representantes alternativos superiores vinculados à classificação GBIF arquivada e hash. Espécies recusam identidade diferente. Créditos 39 CC0 1.0 e 70 CC BY; novas fotos com licenças demonstradas na coleta. [Créditos](PHOTO_CREDITS.md) |
| Recursos e cartões | Fotos somam 18.373.503 bytes (+2.461.743). Componente existente mantém 42% à direita, altura integral, crop, gradiente horizontal, foco e cantos arredondados. Filtros não foram deslocados. Sem downloads durante partidas |
| Dicas | `d7c523e`: 855 receitas para os 122 organismos, estado `approved_automated` explicitamente pedagógico; 290 propostas v3 preservadas como rascunhos. Quatro conteúdos inadequados não foram promovidos a morfologia. [Avaliações](AUDITORIA_ASSISTENCIA.json) |
| Assistência | 581 contextos elegíveis de referência, 1.162 dicas disponíveis e 581 posições com lacuna. 171 contextos completos; 171 registram repetição de categoria (conteúdo distinto). Competitivo: 64/80 organismos com três dicas no cenário de referência. [Cobertura por organismo e nível](AUDITORIA_PROGRESSAO_DICAS.json) |
| Pontuação | Domínio central mantém unidades inteiras 6/3/2, seleção direta por ID/slot, menor teto, reconsulta gratuita e erro zero. Interface anuncia teto efetivo. Atalhos legados por categoria não devem revelar outra dica ao serem repetidos |
| Histórico | Snapshots v4 guardam texto, categoria, teto/grau, opções, fonte/localizador, identidade/foto, avaliação e conteúdo. V3/v2/legado conservam suas políticas; não houve ALTER TABLE ou recálculo de resultados. Rankings distinguem regras/conteúdo internamente no mesmo programa |
| Comunidade | Relato somente para dica revelada: rascunho local preenchível/copiável; navegador e envio dependem do jogador. Sem respostas marcadas, outras dicas, perfis ou bancos. [Procedimento](REPORTAR_PROBLEMA.md) |

A política automática valida identidade/foto/nível, fontes científicas utilizáveis, contexto conhecido de todas as alternativas, associação linguística documentada, conteúdo distinto e proximidade dos ancestrais. Ela não faz inferência semântica universal, não mede dificuldade humana e não concede revisão científica. Datas não são renovadas pelo gerador; mudanças nas fontes/política/implementação ou vencimento exigem avaliação explícita. O prazo é configurável em `review_days.assistance`.

## Limitações e decisões humanas

- A classificação pedagógica é uma heurística de testes. Relatos e curadoria podem corrigir seus graus e textos; não se simula avaliação independente.
- Não há três dicas em todos os contextos. No nível de filo, 21 organismos não têm conteúdo prévio utilizável; ficam acessíveis no Livre com lacuna explícita. Conteúdo de organismo usado em níveis superiores é marcado como individual, sem definir todo o grupo. As alternativas sorteadas podem modificar a disponibilidade.
- A prioridade competitiva ocorre dentro dos grupos e usa busca limitada de alternativas no pool existente, sem reduzir as 20 questões ou remover organismos/grupos por falta de dicas.
- As 89 pendências fotográficas não foram preenchidas com ilustrações, espécies erradas, culturas/diagramas não representativos ou licenças presumidas. As associações visuais novas não equivalem a curadoria por especialista.
- O gate de distribuição continua bloqueado: 120 fotos legadas com permissão incompletamente documentada, sete divergências de crédito, sete pendências de direitos de outros materiais (incluindo o novo corpus), 146 mídias históricas fora do bundle, dois bloqueios de fonte científica e obrigações das dependências. O pacote contém 23 componentes inventariados com 78 achados de obrigações/rastreabilidade ainda pendentes (incluindo nove faltas de documentação de fornecimento de fontes). Não são 120 licenças comprovadamente incompatíveis. Zero erros técnicos no diagnóstico atual não elimina essas pendências.
- Decisões sobre licenças dos materiais próprios, revisão especializada, saneamento de direitos e publicação permanecem com o mantenedor. Nenhuma licença global, autor, data retroativa ou autorização foi inventada.

## Verificações

A etapa 1 passou em 10/10 suítes (51,76 s), a etapa 2 em 10/10 (64,43 s), e a etapa 3 em 11/11 (58,90 s), antes dos respectivos commits. A validação integrada repetida detectou uma suposição antiga no teste QML: clicar sempre na terceira dica quando qualquer uma estava disponível. O teste agora seleciona uma dica efetivamente oferecida e passou três execuções consecutivas (63,13 s). A revisão do caminho legado também impede que repetir a categoria revele automaticamente o próximo ID da mesma categoria; teste de regressão acrescentado.

### Resultado integrado final

| Comando/verificação executada | Resultado |
| --- | --- |
| `cmake --build build --parallel 4` | Build Release no ambiente existente, Qt 6.11.2/GCC 16.2.0; aplicativo 1.6.0 e catálogo 2.4.0 |
| `ctest --test-dir build --output-on-failure` | **11/11**, 58,03 s; suíte C++ 48 casos aprovados, suíte QML 5 casos aprovados |
| QML `--repeat until-fail:3` | Três execuções consecutivas aprovadas (63,13 s), além do CTest final |
| `review_hints.py`, `build_education.py`, `content_governance.py credits`, `audit_hint_assistance.py` e `git diff --exit-code` dos derivados | Política válida; reprodução idêntica sem rede ou renovação de avaliação |
| `audit_catalog.py --strict`, `exploration_media.py` e `qmllint` em todos os QML | Integridade e renderização aprovadas; 122 organismos/120 fotos originais preservados; fotos de apresentação com zero erros |
| Comparação por Git dos bytes originais | Preservados espécies, taxonomia, revisões/baseline, notas de origem e cada uma das 120 fotos do quiz |
| `scientific_catalog.py coverage` e `review_updates.py` | Cobertura/pendências e propostas locais geradas; nada publicado ou renovado |
| `capture_home.py`, `release.py check`, `release.py check-history --base 3bcbd4f` | Captura real 1240×820, perfil temporário; metadados e histórico aprovados. Pixels da tela inicial permaneceram idênticos; registro de captura atualizado |
| `audit_repository.py`, hook e `git diff --check` | Nenhum artefato proibido ou padrão conhecido de segredo no índice; sem erros de whitespace |
| `package.ps1 -Development` e `validate_package.ps1` | Pacote canônico local atualizado e marcado para desenvolvimento. Smoke com PATH isolado: saída 0, stderr vazio, sem dependências carregadas do MSYS2 externo |
| `distribution.py check --package package` | **Bloqueio legítimo mantido**, saída do gate 2. Zero erros técnicos não equivale à liberação de distribuição |

O empacotador avisou sobre ausência de catálogos de tradução Qt e de `dxcompiler.dll`/`dxil.dll`. O smoke realizado usa renderização software; não é um teste de GPU física ou de instalação limpa do Windows. Os textos próprios em português permanecem disponíveis. Não se declara certificação jurídica ou revisão científica independente.

### Histórico local

- `6cf7dc6` — Git/CI, proteção do índice e diagnóstico de direitos.
- `b3e99e4` — cobertura fotográfica e representantes superiores rastreáveis.
- `d7c523e` — avaliação automática, progressão, snapshots, relatos e documentação.
- `a16530e` — reconsulta por categoria preservada e teste de interface adaptado às dicas disponíveis.

Este relatório e o encerramento do registro operacional são documentação posterior à validação. Não houve push, tag ou release; o pacote continua restrito ao desenvolvimento local enquanto os gates exigirem decisões/evidências humanas.

[Registro operacional e retomada](PLANO_EVOLUCAO.md). Os relatórios gerados de execução ficam em build, fora do Git; documentos e evidências científicas/licenças necessárias são preservados no repositório.
