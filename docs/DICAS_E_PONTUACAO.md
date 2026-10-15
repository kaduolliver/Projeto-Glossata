# Dicas, assistência e pontuação

A política `identification-v3-assistance` separa assunto e ajuda, mantendo o cálculo exato, o aprendizado após responder e os pontos históricos. Aplicativo 1.1.0 e catálogo 1.0.0 continuam em preparação; esta alteração não é um lançamento público.

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
- Taxonomia exige ancestral científico anterior ao nível solicitado, todas as alternativas conhecidas no nível comparado e pelo menos três positivamente compatíveis. Reino compartilhado pelas quatro é descartado. O número de compatíveis restringe segurança, sem calcular automaticamente dificuldade ou grau.
- Uma associação lexical registrada que corresponda a várias alternativas não identifica uma única resposta; a dica fica indisponível nesse contexto. Mudança de texto, grau, escopo, foto, identidade ou evidência exige nova revisão.

Registros `assistance:...` usam a governança existente e são independentes da revisão científica. Reclassificar a ajuda não apaga o histórico da afirmação. Alterar o texto científico continua invalidando sua própria revisão. O gerador inclui o payload avaliado e seu hash; o domínio confere esse vínculo antes de revelar e ao validar a persistência.

**As 290 avaliações atuais são rascunhos de origem `ai_proposal`.** No Livre, propostas válidas são sinalizadas como provisórias. No Competitivo, somente classificação editorial ou por especialista rastreável, íntegra e não vencida pode habilitar a dica. O motivo da indisponibilidade não expõe o texto oculto. O material científico utilizável após responder continua disponível, independentemente da avaliação de assistência; a governança científica e a identificação do legado permanecem aplicáveis.

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

As 290 propostas requerem revisão humana e confirmação contextual. Por isso, **as dicas atuais do Competitivo ficam indisponíveis**, preservando as partidas e a aprendizagem posterior. Não se presume que os pesos provisórios estejam validados. Pendências científicas e de licenciamento anteriores continuam em [RELATORIO_CONFORMIDADE.md](RELATORIO_CONFORMIDADE.md).

## Persistência e compatibilidade

SQLite migra transacional e idempotentemente para `user_version=3`. Respostas novas guardam versão das regras, as quatro alternativas, máscara, tetos, payloads de avaliação e hashes. A política central verifica graus/tetos, identidade/foto/nível, associações e contrastes documentados nos snapshots, além de totais e acertos. Valores arbitrários, payloads alterados e tetos em dica não revelada são rejeitados. Os hashes demonstram integridade dos bytes, não autenticam um revisor nem tornam um banco editável à prova de fraude.

Registros `identification-v2` e `legacy` mantêm a antiga interpretação da máscara, exclusivamente para histórico. A migração acrescenta snapshots **sem atualizar pontos, acertos ou rankings**. Agregados antigos sem perguntas continuam identificados como legado; não se inventam detalhes. Novas partidas exigem respostas verificáveis, e a leitura de snapshots não depende dos pesos atuais do catálogo.

Rankings competitivos distinguem regras, conteúdo, nível, dificuldade, grupo e tamanho. O fingerprint inclui `education.json` completo: mudanças de assistência, escopo, evidência ou revisão alteram a versão competitiva. O ranking Livre conserva sua chave histórica e resultados; não é uma competição padronizada comparável entre políticas.

## Manutenção e testes

Edite as fontes e execute `build_education.py`, `audit_hint_assistance.py`, `audit_catalog.py --strict`, atualize o manifesto pelo fluxo de versionamento, compile e rode CTest. A CI verifica a reprodução de `AUDITORIA_ASSISTENCIA.json`. Não aprove campos apenas para fazer checks passarem.

Os testes cobrem graus distintos na mesma categoria; etimologia forte/morfologia leve; contraste diagnóstico forte; pendências; oito subconjuntos nas seis ordens com acerto/erro; invariância do teto; 600 perguntas; anúncio da UI; aprendizagem posterior; snapshots; migração, reabertura e isolamento por regras/conteúdo. As revisões das fixtures são somente sintéticas. Compilação, testes e inspeção das capturas reais são relatados na entrega, sem certificação científica ou jurídica.
