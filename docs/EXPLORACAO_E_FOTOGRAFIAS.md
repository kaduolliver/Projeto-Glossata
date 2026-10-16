# Exploração científica e fotografias dos cartões

O aplicativo 1.5.0 mantém Bacteria, Archaea e Eukaryota como portas principais. Animalia e Fungi continuam em Opisthokonta, e Plantae no sentido do backbone adotado em Archaeplastida. Os atalhos abrem os mesmos identificadores. Consulte as [fontes e divergências filogenéticas](CLASSIFICACAO_FILOGENETICA.md).

Os controles **Alternativas no modo Livre**, seu seletor e **Competitivo · 20 questões** voltam à ordem anterior à expansão fotográfica: na seleção principal, depois da introdução e antes do acesso rápido; dentro dos grupos, depois da introdução científica e antes dos controles de identificação e dos grupos filhos. Alinhamento, espaçamento e comportamento de PlayModeSelector são preservados. Nenhuma regra, pontuação ou conquista muda.

## Cobertura e limitações

O catálogo 2.3.0 contém **109 fotografias de apresentação**: **39 CC0 1.0** e **70 CC BY** (2.0, 3.0 ou 4.0, conforme o arquivo). A auditoria percorre **todos os 623 cartões**: 622 nós científicos, inclusive ranks intermediários e espécies, mais o misto.

| Estado | Quantidade |
| --- | ---: |
| Grupos com associação fotográfica explícita | 533 |
| Cartão misto com fotografia | 1 |
| Grupos com ilustração temporária e pendência justificada | 89 |
| Cartões sem fotografia ou justificativa | 0 |

Isso documenta toda a cobertura, **não significa que todos os cartões já tenham foto**. O [relatório por cartão](AUDITORIA_FOTOGRAFIAS.json) identifica cada nó, rank, imagem ou motivo da pendência. As lacunas correspondem a ramos de 28 organismos sem imagem preparada. Ausência de arquivo adequado nas páginas consultadas não demonstra inexistência de fotografias.

O manifesto editável `resources/data/exploration_photos.json` v2 guarda `photos`, `bindings`, `mixed_photo`, `pending` e `selection_notes`. A preparação pode associar um representante aos seus ancestrais, gravando cada vínculo explicitamente. O aplicativo não propaga a imagem de um ancestral para todos os descendentes. Reutilização exige pertencimento ao caminho taxonômico do organismo ou escopo externo com justificativa e fonte. Havendo vários representantes, a ferramenta distribui as novas associações entre eles, preservando escolhas anteriores.

As oito fotos originais foram obtidas em resolução maior. Há representantes específicos de animais, plantas, fungos, algas, mixomicetos e bactérias. Bacteria e Archaea usam micrografias. Discoba recebe uma micrografia eletrônica de *Trypanosoma brucei brucei*, em Euglenozoa/Kinetoplastea segundo [Adl et al. 2019, tabelas 1 e 2](https://doi.org/10.1111/jeu.12691). Archaea e Discoba continuam sem organismos jogáveis. *Physarum polycephalum* não é atribuído a *P. psittacinum*, que tem foto própria. Larvas e estruturas microscópicas são identificadas nas notas.

Exclusões documentadas incluem:

- *Turbinaria ornata*: arquivo representava um coral (Cnidaria), homônimo da alga.
- *Hassallia byssoidea*: descrição dizia fungo e a imagem não sustentava a identificação como cianobactéria; arquivo descartado.
- *Hydnora esculenta*: herbário pouco legível e campo de autoria com URL, sem crédito fotográfico demonstrável.
- Culturas, filmes bacterianos, fichas de herbário e montagens: não preparados como micrografias celulares ou retratos identificáveis.
- Arquivos com metadados sensíveis, licença incompatível, ilustrações ou resolução inadequada: recusados durante a seleção/preparação.

Os **122 organismos e suas 120 fotos do quiz permanecem intactos**, assim como IDs, taxonomia, fichas, dicas, revisões, regras e bancos dos jogadores. Fotos de apresentação não concedem elegibilidade no quiz.

## Renderização

`ExploreCard.qml` coloca `FeatheredPhoto.qml` no fundo à direita, com **42% da largura e 100% da altura**, inclusive no misto. Dimensões e proporções dos cartões permanecem. O texto ocupa a esquerda, com quebra de nomes longos.

O Canvas usa escala máxima equivalente a `PreserveAspectCrop`, sem distorção ou barras vazias. `focus_x` e `focus_y` (0 a 1) definem o ponto focal; o padrão é central. A foto começa transparente e se torna opaca em direção à direita, sem desaparecer novamente na extremidade. O recorte acompanha os cantos arredondados externos e o contorno de foco fica por cima. Há somente uma máscara horizontal, sem animação contínua ou efeito exclusivo de GPU. Pintura ocorre ao carregar, redimensionar ou alterar o foco. [Canvas/Context2D](https://doc.qt.io/qt-6/qml-qtquick-context2d.html).

As imagens reduzidas vêm do Commons, com solicitação de até 640 × 640 e validação do bitmap (mínimo 160, máximo 1024 pixels por lado; a fonte pode usar tamanhos padronizados maiores). Os arquivos somam **18.373.503 bytes**, sem dependência nova no executável. Tudo é embarcado no QRC. Falha de arquivo/evidência mantém a ilustração. Pendências aparecem na descrição acessível e na dica de foco/ponteiro.

## Descoberta e preparação

`scripts/exploration_media.py` reutiliza os contratos, adaptador Commons e transporte/cache de provedores na descoberta. `exploration_discovery.py` isola preparação de vínculos e inventário. Não modifica candidatos científicos ou dados de partidas.

```powershell
# Auditoria offline de todos os cartões
python scripts/exploration_media.py

# Descoberta de propostas em lotes dos organismos atuais e representantes externos
python scripts/exploration_media.py --discover --online --offset 0 --limit 90
python scripts/exploration_media.py --discover --online --offset 90 --limit 34

# Próxima página e busca CC0 para representantes ainda não preparados
python scripts/exploration_media.py --discover --online --offset 0 --limit 90 --search-page 1
python scripts/exploration_media.py --discover --online --offset 0 --limit 90 --cc0-search

# Replay: sem rede e sem renovar a data da coleta
python scripts/exploration_media.py --discover --offset 0 --limit 90

# Plano explícito, após conferir identidade e adequação da fotografia
python scripts/exploration_media.py --collect-plan build/plano-fotografias.json --online
python scripts/exploration_media.py --complete-coverage --sync-resources
python scripts/content_governance.py credits
```

Uma entrada do plano contém `id` local, `file` do Commons, `subject`, `kind` e `catalog_species_id`, ou `scope` externo com nós, justificativa e fonte. Pode incluir `identification_category` (demonstrada pela API antes do download), `retry_files`, notas e ponto focal. Resultado de busca, nome literal ou categoria não substitui inspeção visual ou curadoria científica.

Descoberta usa páginas de oito resultados, até três páginas e lotes limitados. `--cc0-search` usa [haswbstatement](https://commons.wikimedia.org/wiki/Help:Search), sem dispensar licença por arquivo. Metadados são pedidos em pequenos lotes conforme [Imageinfo](https://www.mediawiki.org/wiki/API:Imageinfo). Acesso serial por ferramenta, intervalo mínimo de 1,1 segundo, orçamento e interrupção por 429, sem retries persistentes ou agendamento.

Cache e propostas ficam em `build/exploration-discovery`; preserve-os antes de limpar build. O manifesto publicado conserva recibos das consultas que justificam pendências. Preparação em checkout sem cache não apaga essas evidências. Falhas de seleção preservam os arquivos válidos anteriores e ficam no relatório; `--sync-resources` inventaria exclusivamente fotos verificadas, sem liberar os demais recursos.

## Direitos e rastreabilidade

A licença **do arquivo específico** deve ser CC0/CC BY com versão/URL demonstradas, autoria indicada e sem restrições. A verificação precede o download. SA, ND, NC, ausência e licença desconhecida são recusadas neste fluxo. Observador/carregador não é presumido fotógrafo. Uma referência bibliográfica ou licença geral do site não autoriza outra imagem. [Condições de reuso](https://commons.wikimedia.org/wiki/Commons:Reusing_content_outside_Wikimedia).

`resources/compliance/exploration/` conserva metadados selecionados, IDs de arquivo/revisão, URLs de origem/download, SHA-1 original informado, crédito, licença e coleta real. O manifesto acrescenta SHA-256 local/da evidência, dimensões, bytes, modificações e triagem de IA. `asset_rights.json`, `credits.json`, [PHOTO_CREDITS.md](PHOTO_CREDITS.md), interface e pacote usam esses registros. CC0 recebe crédito editorial. Não há sugestão de endosso ou revisão jurídica presumida.

Não se arquivam coordenadas, dados protegidos ou páginas integrais. Imagens com metadados sensíveis são rejeitadas, sem remoção silenciosa durante auditoria. Trocas exigem novas evidências/hashes; não sobrescreva lançamento publicado. Git preserva o histórico. Bloqueios legados de distribuição permanecem.

## Verificação e revisão humana

Testes Python verificam todos os nós, licença por arquivo, identidade incompatível, crédito/evidência adulterados, lacunas sem motivo, foco inválido, recusa anterior ao download, isolamento offline e inventário idempotente. O teste C++ percorre a árvore inteira e exige imagem verificada ou motivo para cada nó.

Testes QML usam componentes reais no renderizador software: transparência, altura integral, largura de 42%, cantos e carregamento para cartões normais e mistos. A navegação é capturada em 1000 × 700, 1240 × 820 e 1920 × 1080, com ordem dos controles verificada. Capturas ficam em `build/tests/previews`; isso não valida todos os drivers de GPU ou escalas de DPI.

Execute testes Python, compilação, CTest, qmllint, auditoria estrita, créditos e metadados de versão. A CI guarda a auditoria fotográfica sem consultar conteúdo externo. Associações continuam **rascunhos de IA ligados à fonte e à inspeção das imagens**, sem especialista ou revisão humana independente simulados. A curadoria deve conferir identidade, representatividade, recorte e qualidade pedagógica. Licença evidenciada e teste técnico não certificam regularização integral. [Comunicar erro ou remoção](REPORTAR_PROBLEMA.md).

Validação local desta etapa: **9/9 suítes CTest**, incluindo **16 testes Python de fotografias**, compilação, qmllint e auditoria estrita aprovados. A captura real de exploração está em `docs/images/exploracao.png`, com proveniência em `exploracao.json`, e a tela inicial foi recapturada pelo programa. O pacote local passou na inicialização com PATH isolado, usando banco temporário. Isso não representa teste em instalação limpa do Windows. O gate público continua bloqueado por pendências legadas, sem tag ou publicação.

## Ampliação da etapa integrada

Catálogo 2.3.0: **109 imagens**, **534/623 cartões (85,71%)**, **89 pendências**, ganho de 30 cartões com 11 novos arquivos e 2.461.743 bytes. Os 623 registros agora incluem `special:mixed`; os 533 bindings são apenas grupos científicos. A linha de base 2.2.0 tinha 98 fotos, 504 cartões com foto e 119 pendências. Novos representantes superiores usam classificação GBIF arquivada por hash, sem representar a espécie pendente. Cartões de espécies recusam representantes de outro organismo, mesmo com scope manual. CLI acrescenta `--by-search --by-version 2.0`/`4.0` e `--discovery-plan` para propostas adicionais; nenhum filtro de busca substitui permissão individual. Os recibos e descartes desta etapa estão na auditoria.
