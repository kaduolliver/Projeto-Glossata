# Dicas, assistência e pontuação

A política atual `identification-v4-progressive` organiza três posições de assistência, com IDs de dica independentes da categoria. Aplicativo 1.6.0 e catálogo 2.4.0 ainda não lançados. A avaliação pedagógica automatizada é o padrão desta versão de testes; não substitui revisão científica, experiência com jogadores ou permissões de imagens. O histórico `identification-v3-assistance`, v2 e legado conserva sua interpretação original.

## Diagnóstico

Antes, `Hint.h` deduzia o teto pela máscara de categorias. Serviço, controlador e validação SQLite reutilizavam essa associação. O painel anunciava o valor isolado da categoria, podendo prometer um ponto depois de outra dica reduzir o acerto a um terço.

| Exemplo do corpus | Avaliação proposta |
| --- | --- |
| *Trametes versicolor*: “de várias cores” | Etimologia forte: pode permitir reconhecer `versicolor` |
| *Ginkgo biloba*: “dois lobos” | Etimologia forte: associação com `biloba` |
| *Rhinoceros unicornis*: “um único chifre” | Etimologia forte: associação com `unicornis` |
| Descrições de faixas, folhas e chifre desses organismos | Morfologia leve: não foi demonstrada informação diagnóstica adicional à foto |
| *Verbascum thapsus*: homenagem a uma localidade | Etimologia leve: contexto histórico sem exclusão demonstrada |
| *Favolaschia claudopus*: referência a favos e ao nome de Lasch | Etimologia forte: uma homenagem também pode fornecer associação escrita reconhecível |

São propostas pedagógicas, sem aprovação humana ou garantia experimental de dificuldade. Não foram inventadas etimologias, características, fontes ou revisores. Os limites fotográficos continuam válidos: *Trametes* e *Rhinoceros* não passam a ter perguntas de espécie apenas por haver etimologia desse nível.

## Política central

As categorias **taxonômica**, **etimológica** e **morfológica** descrevem o assunto. A assistência daquela nota, nível e contexto determina o teto:

| Assistência | Unidades | Apresentação |
| --- | ---: | ---: |
| Leve | 6 | 1,00 |
| Moderada | 3 | 0,50 |
| Forte | 2 | 0,33 |

Sem dica, o teto começa em seis unidades. Erro vale zero. Totais usam inteiros de 64 bits e arredondam somente na apresentação: três acertos de duas unidades somam exatamente um ponto. Acertos, precisão e eficiência são métricas distintas.

Abrir/fechar o painel não altera pontos. Somente revelar uma dica disponível aplica seu teto. Revisitar não cobra novamente. Combinações conservam o menor teto e nunca o aumentam. A pergunta seguinte reinicia com seis unidades. Consulta após responder não modifica pontos registrados. A interface anuncia o teto resultante antes de revelar, inclusive quando outra dica já reduziu a pontuação.

Dicas combinadas podem fornecer ajuda adicional. O menor teto é uma regra explícita mantida nesta correção; não mede ajuda cumulativa e não foi substituído por fórmula experimental.

## Conteúdo, contexto e curadoria

`hint_education.json` contém `assistance`: grau, justificativa, níveis, origem da avaliação, relação com a foto, associação linguística, condições e limitações. Etimologias são notas diferentes por nível. Morfologia mantém referência à ficha e ID da foto. Cada avaliação é vinculada ao organismo, nome, foto/hash, texto e evidência científica correspondente.

- Traduções, decomposição de nomes e homenagens reconhecíveis podem justificar assistência forte sem reproduzir o nome completo. Contexto histórico ou significado pouco transparente pode ajudar menos. A proteção literal é adicional; não classifica semanticamente a ajuda.
- Morfologia genérica ou já visível pode ter assistência leve. Detalhes estruturais/celulares podem acrescentar informação. Uma classificação diagnóstica exige comparação documentada para as quatro alternativas, com nível, compatibilidade, fonte e localização da evidência. Faltar um distrator impede essa dica; ausência de dados não prova incompatibilidade.
- Taxonomia usa apenas ancestrais da fonte original com identidade verificada e nível anterior ao solicitado. A dica leve exige contexto amplo e pelo menos três opções compatíveis. A moderada exige informação mais próxima, duas ou três compatíveis. A forte permite contraste próximo que distingue uma ou duas opções. Todas exigem classificação conhecida e concordante de cada organismo associado a cada alternativa; ausência ou divergência impede a dica. A proximidade e a informação adicional guiam a seleção; contagens são evidência auxiliar e não medem dificuldade humana. Um ancestral já utilizado não é repetido em outro grau.
- Uma associação lexical registrada que corresponda a várias alternativas não identifica uma única resposta; a dica fica indisponível nesse contexto. Mudança de texto, grau, escopo, foto, identidade ou evidência exige nova revisão.

Registros `assistance:...` usam a governança existente e são independentes da revisão científica. Reclassificar a ajuda não apaga o histórico da afirmação. Alterar o texto científico continua invalidando sua própria revisão. O gerador inclui o payload avaliado e seu hash; o domínio confere esse vínculo antes de revelar e ao validar a persistência.

**As 290 avaliações anteriores permanecem rascunhos `ai_proposal`.** O novo corpus deriva 855 receitas de fontes já existentes, sem inventar etimologias ou características. `scripts/review_hints.py --evaluate` registra explicitamente estado `approved_automated`, ator automatizado, processo, versão, justificativa, hash e datas reais. Não preenche `reviewer` humano. O prazo usa `content_policy.json → review_days.assistance`. Reexecutar sem mudança e antes do vencimento é idempotente; gerar `education.json` não aprova conteúdo alterado nem renova datas. Alterações nas fontes, fotos, classificação, política, código avaliador ou revisões invalidam a avaliação. A governança audita essa avaliação separadamente das científicas.

A montagem offline da pergunta valida a nota científica utilizável, organismo, foto/hash, nível e alternativas. Notas de aparência relativas ao organismo podem apoiar níveis superiores com a frase “Sobre este organismo, sem generalizar para todo o grupo”; isso não transfere o caráter à definição de um filo ou classe. Textos de comportamento/ecologia ou estágio inadequado não são promovidos a morfologia. Traduções continuam usando o grau proposto com sua justificativa e proteção contextual; não há inferência semântica universal.

O mantenedor pode revisar sozinho, sem simular revisão independente. Deve registrar responsável/data reais, justificativa, escopo `hint_assistance`, evidência localizada e hash atual pelo fluxo `scripts/content_governance.py record-review`; depois regenerar e auditar. A evidência deve sustentar a ajuda no contexto de nomes e alternativas. Bibliografia, URL ou teste não aprovam o peso pedagógico, e essa revisão não substitui a científica.

## Auditoria e pendências

[AUDITORIA_ASSISTENCIA.json](AUDITORIA_ASSISTENCIA.json) cobre os 122 organismos: 122 avaliações taxonômicas, 64 etimológicas e 104 morfológicas. Registra texto, escopo, justificativa, origem, estado, vínculo, teto anterior/proposto, mudanças, características genéricas/redundantes e conteúdo ausente.

| Resultado | Quantidade |
| --- | ---: |
| Avaliações | 290 |
| Leves / moderadas / fortes propostas | 212 / 57 / 21 |
| Propostas de mudança de teto | 132 |
| Aprovações humanas | 0 |
| Inadequações como morfologia prévia | 4 |

As inadequações são *Manduca sexta* (mistura de lagarta/adulto sem escopo do estágio da foto), *Stegodyphus dumicola* (reunião em teia), *Laterallus rogersi* (incapacidade de voar) e *Pseudoboletus parasiticus* (associação ecológica). Os textos não foram apagados das fontes ou fichas. Não há morfologia forte aprovada no corpus; o suporte a contraste diagnóstico forte é testado com fixtures sintéticas, sem aprovar conteúdo real.

As propostas históricas não foram aprovadas em bloco. As receitas passam pela política automática explícita e por restrições em cada pergunta, nos dois modos. Conteúdo sem apoio suficiente permanece indisponível com motivo, sem excluir o organismo no Livre. O Competitivo prioriza perguntas com três dicas dentro de cada grupo e busca alternativas no mesmo pool em até oito tentativas da política Equilibrada; mantém 20 questões, os grupos e a elegibilidade científica original. Pendências científicas e de licenciamento anteriores continuam em [RELATORIO_CONFORMIDADE.md](RELATORIO_CONFORMIDADE.md).

## Persistência e compatibilidade

A extensão pedagógica usa o snapshot JSON existente. Após a correção de identidade de jogadores, SQLite passa a `user_version=5` (identidade v4 e [pol?tica Unicode v5](NOMES_UNICODE.md)), com [migração transacional e idempotente por ID](IDENTIDADE_JOGADORES.md), sem recálculo dos resultados. Esse número de esquema é independente da versão das regras: em `identification-v4-progressive`, a máscara indica slots, e o snapshot armazena IDs, categorias, textos, fontes/localizadores, avaliação automatizada, tetos, alternativas, regras e versão pedagógica. Nas políticas v3 e v2, a máscara conserva a interpretação por categoria. A política central verifica graus/tetos, identidade/foto/nível, associações e contrastes documentados nos snapshots, além de totais e acertos. Valores arbitrários, payloads alterados e tetos em dica não revelada são rejeitados. Os hashes demonstram integridade dos bytes, não autenticam um revisor nem tornam um banco editável à prova de fraude.

Registros `identification-v2` e `legacy` mantêm a antiga interpretação da máscara, exclusivamente para histórico. A migração acrescenta snapshots **sem atualizar pontos, acertos ou rankings**. Agregados antigos sem perguntas continuam identificados como legado; não se inventam detalhes. Novas partidas exigem respostas verificáveis, e a leitura de snapshots não depende dos pesos atuais do catálogo.

Rankings competitivos distinguem regras, conteúdo, nível, dificuldade, grupo e tamanho. O fingerprint inclui `education.json` completo: mudanças de assistência, escopo, evidência ou revisão alteram a versão competitiva. O ranking Livre conserva sua chave histórica e resultados; não é uma competição padronizada comparável entre políticas.

## Manutenção e testes

Edite `species_profiles.json`/`hint_education.json`/as receitas do preparador; execute explicitamente `review_hints.py --evaluate`, depois `build_education.py`, `audit_hint_assistance.py`, `audit_catalog.py --strict`, atualize o manifesto pelo fluxo de versionamento, compile e rode CTest. A CI verifica a reprodução de `AUDITORIA_ASSISTENCIA.json`. Não aprove campos apenas para fazer checks passarem.

Os testes cobrem graus distintos na mesma categoria; etimologia forte/morfologia leve; contraste diagnóstico forte; pendências; oito subconjuntos nas seis ordens com acerto/erro; invariância do teto; 600 perguntas; anúncio da UI; aprendizagem posterior; snapshots; migração, reabertura e isolamento por regras/conteúdo. As revisões das fixtures são somente sintéticas. Compilação, testes e inspeção das capturas reais são relatados na entrega, sem certificação científica ou jurídica.


## Progressão, exceções e relatos

A ordem é **leve → moderada → forte**, e qualquer dica pode ser escolhida diretamente. Categorias repetidas aparecem quando não há etimologia ou aparência apropriada àquele grau; só há repetição com conteúdos distintos e progressão de informação. A auditoria contextual registra a exceção e as lacunas por organismo/nível. A avaliação automática é uma heurística pedagógica de testes, não garantia de dificuldade percebida; relatos e revisão especializada podem corrigi-la.

**Relatar problema nesta dica** aparece somente para uma dica revelada. O jogador descreve o problema e prepara um rascunho local que pode copiar offline. A abertura do formulário GitHub exige outra ação explícita e não envia a Issue. O rascunho inclui ID público do organismo, nível, dica/texto revelado, categoria, assistência, alternativas, política e hash de avaliação. Não inclui nome científico oculto, resposta correta marcada, outras dicas, jogador, caminhos ou bancos. Consulte [REPORTAR_PROBLEMA.md](REPORTAR_PROBLEMA.md).

A cobertura real por nível, categorias repetidas e motivos de indisponibilidade está em [AUDITORIA_PROGRESSAO_DICAS.json](AUDITORIA_PROGRESSAO_DICAS.json), produzida pelo gerador C++ com seed 42. Os contextos da partida podem variar; isso não é uma promessa de três dicas em todas as combinações possíveis. A lacuna deve permanecer visível quando não houver conteúdo fundamentado, sem reduzir silenciosamente a partida.

### Cobertura de referência em 1.6.0 / catálogo 2.4.0

| Nível | Com três dicas | Outras situações |
| --- | ---: | --- |
| Espécie | 29/40 | 11 com duas |
| Gênero | 80/100 | 17 com duas; 3 com uma |
| Família | 39/100 | 61 com duas |
| Ordem | 23/100 | 76 com duas; 1 com uma |
| Classe | 0/120 | 95 com duas; 25 com uma |
| Filo | 0/121 | 100 com uma; 21 sem conteúdo prévio utilizável |

No subconjunto competitivo (somente fotos de identificação), 64/80 têm três dicas no contexto de referência. A seleção prioriza completude sem remover grupos ou alterar 20 questões. A falta de uma categoria ou de um grau não é resolvida por invenção, aprovação científica automática ou alteração de rótulo. As 21 lacunas de filo incluem fontes ausentes/pendentes e os quatro conteúdos de estágio/comportamento/ecologia inadequados; o organismo continua acessível, e a aprendizagem posterior preserva seu próprio estado científico.
