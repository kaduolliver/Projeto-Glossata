# Investigação de direitos e associações científicas — INV-08

> Nota administrativa de 2026-10-17: a expressão “open source” no contexto histórico abaixo descreve a intenção do projeto, sem demonstrar licença própria já concedida. O código é público e a escolha permanece pendente. Consulte o [plano de licenciamento](PLANO_LICENCIAMENTO.md); as evidências, contagens e pendências desta investigação foram preservadas.

Investigação local e consultas públicas em **17/10/2026**, partindo de `9c43bb10bcfcc32e57d9e6019703734fa5f11325`. O projeto é educacional, open source, sem fins lucrativos e experimental; isso não é tratado como licença automática de conteúdo de terceiros. Há um mantenedor, sem equipe externa de curadoria. Não se criou autorização, especialista, consentimento ou revisão humana.

O [relatório estruturado por fotografia/material](SANEAMENTO_DIREITOS.json) é um **snapshot histórico desta intervenção**, com campos de fonte, táxon, crédito registrado, licença declarada, comprovação, data real, hashes, divergências, prioridade, superfícies e ações. Não é uma nova fonte de permissões. O estado atual deve ser regenerado pelo comando abaixo, usando os mesmos validadores e ledgers do projeto.

## Diagnóstico reexecutado

| Escopo | Resultado nesta investigação | Interpretação |
| --- | --- | --- |
| Fotos distintas do quiz | 120, associadas aos 122 organismos | Preservadas imagens, IDs, táxons e créditos. |
| Fotos com pendência documental de permissão | **120**, como no INV-08 original | Faltam versão comprovada por arquivo, data/evidência de coleta aplicável e confirmação humana rastreável. Não são 120 violações comprovadas. |
| Crédito divergente do snapshot | **7** | Categoria distinta das sete pendências de outros materiais. Conferir fonte específica; não alterar autoria por inferência. |
| Outros materiais inventariados com pendências | **7**, como no diagnóstico original | Três arquivos de branding e quatro materiais de texto/geração educativa; autoria/originalidade/escopo de cessão ainda dependem do mantenedor. |
| Bloqueios científicos de maior gravidade | **2**, os mesmos IDs | Origem Casual incompatível com o filtro Research Grade vigente; não é prova de identificação taxonômica incorreta. |
| Fotos de apresentação | **109**, sem achados de permissão no auditor específico | Evidências de licença por arquivo existentes; 109 revisões científicas em estado `draft`, sem presumir aprovação especialista. |
| Outras mídias atualmente no Git | **5** | Três capturas documentais e duas fixtures de teste. Superfície adicional, não incluída artificialmente nas sete pendências históricas. |
| Mídias locais fora do bundle | **109**, não rastreadas no Git atual | Arquivos locais/ignorados não são fotos publicadas na árvore atual. O histórico Git anterior continua sendo uma superfície distinta. |
| Dependências do pacote local existente | **23 componentes / 1.346 arquivos inventariados** | Avaliação específica do pacote, sem presumir correspondência com o executável atual. |

A comparação histórica vem de `docs/INVESTIGACAO_EVIDENCIAS.json`, não de números fixados no gerador. O [relatório de conformidade anterior](RELATORIO_CONFORMIDADE.md) invertia os nomes associados aos dois IDs; a correção documental está identificada, sem modificar o catálogo ou reescrever eventos.

## Evidências públicas efetivamente reunidas

Foram obtidos snapshots minimizados para **120/120 fotos**, com data real da consulta e vínculo à observação/fotografia. O código retornado pela fonte foi **CC BY em 113 fotos e CC0 em sete**; 118 observações estavam Research Grade e duas Casual. Os táxons/associações consultados não produziram novos bloqueios científicos além dos dois existentes. Nenhum reconhecimento visual por IA foi usado para conceder aprovação.

A API consultada **não forneceu versão de licença por arquivo** nesses snapshots. O `4.0` já declarado no catálogo não foi transformado em prova de versão. As novas coletas permanecem propostas fora de `resources/`; não preenchem retroativamente a data original nem renovam revisão humana. `registered_permission_state`, pendências e gate mantêm o estado oficial; `public_proposal` contém observação externa para análise.

Nas sete fotos CC0, a API devolveu `attribution: "no rights reserved"`; isso é aviso de licença, não nome de autor. O relatório preserva o crédito do catálogo e o campo literal da fonte separadamente. A diferença não prova que os nomes registrados estejam errados. IDs envolvidos: `231974984`, `27005174`, `338534567`, `407087233`, `58089762`, `581394987` e `68055096`. Conferir a autoria da foto específica, sem atribuí-la automaticamente ao observador nem substituir nomes pelo aviso.

O iNaturalist distingue licenças de observações e mídias: consultar a licença de cada fotografia, conforme a [orientação oficial de uso de fotos](https://help.inaturalist.org/en/support/solutions/articles/151000169918). O fluxo prioriza CC0/CC BY pelo validador existente; SA, ND, NC, licença ausente e todos os direitos reservados exigem análise da política/escopo, sem troca automática de licença. Uma mudança posterior é proposta de análise, não revogação automática de uma concessão CC válida anterior. Obrigações de crédito e modificações dependem da licença e versão comprovadas; consulte [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) sem presumir que essa versão vale para todo arquivo.

## Dois casos científicos prioritários

| Foto / táxon registrado | Observação e evidência consultada | Divergência e ação |
| --- | --- | --- |
| `707161720` — *Panthera tigris*, organismo 125 | [Observação 386235634](https://www.inaturalist.org/observations/386235634), táxon 41967, comunidade 41967; a foto está associada à observação. `quality_grade=casual`, `captive=true`, um registro `wild/agree=false`. | O táxon retornado coincide com o esperado. A sinalização de cativeiro contextualiza a ausência de Research Grade; permanece necessária revisão humana da associação e adequação pedagógica ao escopo do quiz. |
| `659755043` — *Lemur catta*, organismo 128 | [Observação 361508293](https://www.inaturalist.org/observations/361508293), táxon 43617, comunidade 43617; a foto está associada à observação. `quality_grade=casual`, `captive=true`, dois registros `wild/agree=false`. | Mesma incompatibilidade com a política de origem, sem evidência coletada de troca de táxon. Manter bloqueio e documentar a decisão humana necessária. |

Os registros `wild` são avaliações da observação; não são votos independentes de especialistas e podem incluir avaliações automatizadas da plataforma. Não se arquivaram os identificadores de quem votou. O iNaturalist explica que organismos cativos/cultivados não atendem aos critérios de observação verificável/Research Grade: [DQA oficial](https://help.inaturalist.org/en/support/solutions/articles/151000169936). A condição Casual não demonstra sozinha erro de espécie, ausência de licença ou inadequação para todo uso educativo.

Não se alterou o filtro Research Grade nem a elegibilidade existente. Registrar uma revisão isolada não elimina automaticamente o achado de origem. Qualquer futura decisão de usar exceção científica, substituir mídia ou alterar política precisa de justificativa, evidências, autorização apropriada e alteração explícita revisável, preservando snapshots anteriores. Os bloqueios permanecem.

## Arquitetura e uso

`scripts/rights_investigation.py` é uma extensão de manutenção chamada por `review_updates.py`, não um sistema de aprovação paralelo. Reutiliza `release_gate`, `credits`, `permission_issues`, `source_findings`, `changes_for_photo`, política de licenças, estados científicos, hashes, minimização e IDs da fila de `curation_requests.py`. Não há comando de aplicação/promoção. As solicitações existentes podem ser localizadas pelos IDs estáveis do relatório, sem criar novas Issues ou contatos nesta intervenção.

```powershell
# Diagnóstico offline; consulta zero fontes externas.
python scripts/review_updates.py --rights-report

# Inclui auditoria somente de leitura do pacote existente.
python scripts/review_updates.py --rights-report --package package

# Consulta pública limitada às duas fotos prioritárias.
python scripts/review_updates.py --rights-report --online --max-checks 1 --photo 707161720 --photo 659755043

# Investiga o catálogo, reutilizando snapshots locais já obtidos.
python scripts/review_updates.py --rights-report --online --max-checks 25 --package package
```

O resultado atual fica em `build/rights-investigation.json`; cada execução também é arquivada pelo mecanismo existente em `build/update-proposals/`. Snapshots e recibos de consultas ficam em `rights-evidence/` e `rights-queries/` nesse diretório. O snapshot histórico versionado inclui os objetos minimizados das propostas, permitindo conferir seus hashes mesmo sem o cache local. Os caminhos `build/` referem-se à coleta original, não a arquivos automaticamente promovidos ao pacote.

Sucesso do comando significa geração da triagem, inclusive quando há pendências. Não equivale à aprovação de `distribution.py check`, que mantém seu próprio resultado e código de saída. A CI técnica gera esse relatório offline e guarda o artefato; o job de permissões continua separado.

Rede é opt-in: GET público por IDs numéricos, lotes de até dez observações, orçamento **total** de 1 a 120 chamadas por execução (`--max-checks`, padrão 25). `PublicClient` mantém HTTPS, destinos/portas autorizados, proteção DNS/redirecionamentos, timeout, limite de 4 MB por JSON e intervalo mínimo de 1,1 s. Respostas grandes/incompletas permitem subdividir lotes dentro do mesmo orçamento, sem elevar os limites; recusas e falhas são registradas. HTTP 401/403/429 ou erro de serviço encerra a rodada, sem tentar contornar restrições. Foram encontradas respostas grandes/incompletas reais; as tentativas não foram descritas como coleta bem-sucedida.

Os snapshots íntegros são reutilizados com sua data original; `--refresh` solicita nova coleta explícita. Não há renovação implícita de datas científicas, e um snapshot antigo não é uma consulta atual. A [política oficial da API](https://www.inaturalist.org/pages/api+recommended+practices) recomenda aproximadamente uma requisição por segundo, lotes de IDs e ausência de autenticação quando desnecessária. Não são baixadas fotos/HTML, usados tokens ou contatadas pessoas. Campos ocultos, coordenadas, perfis, contatos e identificadores de avaliadores não entram nos snapshots; propostas com metadados potencialmente privados são omitidas, registrando apenas a omissão no recibo.

## Superfícies e dependências

Cada recurso diferencia presença no checkout, rastreamento no Git atual, entrada do QRC e cópias soltas com bytes iguais no pacote. QRC indica o que uma compilação de fonte incluiria, **não comprova o conteúdo do executável existente**. Não se extraiu o bundle do pacote. As três capturas documentais são obras derivadas a conferir por contribuições/atribuições; as duas fixtures requerem registro de geração/autoria e não são organismos do catálogo.

O pacote local foi avaliado **sem exportar, alterar ou copiar avisos**. Os 23 componentes continuam sem revisão de obrigações rastreável e vinculada ao inventário; nove têm ausência de documentação de fornecimento de fonte nas verificações existentes. O relatório separa componentes, licenças declaradas, URLs upstream, avisos/hashes, obrigações e fornecimento/substituição. Declarações do pacote não são parecer sobre todos os componentes internos do Qt. Os bytes/avisos inventariados não apresentaram os erros de alteração específicos, mas o manifesto do executável foi marcado `packaged_binary_or_compiled_sources_stale`: o pacote não comprova o estado atual do código.

Não se escolheu licença dos materiais próprios nem se atribuiu automaticamente a licença do código às mídias. Branding fornecido pelo usuário e textos assistidos por IA ainda exigem análise de autoria, originalidade e escopo da concessão; bibliografia não autoriza reprodução. Consulte [direitos por camada](DIREITOS_E_LICENCAS.md).

## Procedimento humano

1. Abrir a linha pelo ID, conferir prioridade e distinguir pendência documental, contradição da fonte e revisão científica. P0 indica associação/condição relevante para impedir distribuição; P1, documentação/atribuição/material; P2/P3, representatividade e outros itens sem presumir infração.
2. Conferir a fotografia, observação, táxon e referências específicas. Para autoria, licença/versão e alterações, reunir prova localizada por arquivo; não substituir observador por fotógrafo. Conservar lacunas históricas e evidências anteriores.
3. Revisar o snapshot minimizado, seus hashes, a data e a licença da mídia. Somente após análise arquivar evidência pública adequada pelo mecanismo de permissões existente; não copiar contratos, dados pessoais ou contatos para diretórios exportados.
4. Preparar a decisão com responsável real, data real, justificativa, escopo e prova localizada. Os modelos `curation_requests.py decision-template` permanecem incompletos; usar número real de Issue existente quando optar pelo bridge. `confirm-permission`/`record-review` e `distribution.py record-review` continuam sendo os únicos fluxos de decisão aplicáveis. Não completar campos para apenas satisfazer testes.
5. Conferir o binding atual da foto/componente e os eventos anteriores, registrar a decisão legítima e regenerar educação/créditos pelo fluxo documentado. Mudança de dados exige manifesto/histórico do catálogo apropriados. Revisão editorial por um mantenedor não deve ser descrita como parecer independente de especialista ou jurídico.
6. Reexecutar auditorias e `distribution.py check`; com pacote, também `check --package package`. Resolver origem e obrigações por evidência, sem excluir achados/evidências, enfraquecer checks ou marcar autorização fictícia. A preparação e o acompanhamento de pedidos não enviam mensagens; contato depende de autorização separada.

## Validação e entrega

Verificações locais em 17/10/2026:

- Configuração CMake/Ninja Release e compilação incremental concluídas. CTest completo: **25/25 suítes aprovadas em 126,51 s**.
- Após os ajustes finais, quatro suítes de governança/proteção/metadados/investigação foram aprovadas novamente em 5,73 s. A suíte nova tem **14 testes** com fixtures sintéticas, clientes simulados e diretórios temporários.
- Os 14 testes foram executados também nos dois intérpretes locais, Python 3.11 e 3.14, após corrigir a diferença de fechamento de `HTTPError` sem resposta entre essas versões. Todas as categorias de recursos têm prioridade explícita, incluindo dependências e mídias locais fora do bundle.
- Os testes cobrem licença/crédito ausentes, contradições, licença da observação independente da foto, mudança posterior sem revogação automática, Casual/cativeiro, foto removida, táxon comunitário incompatível, ausência de aprovação apesar de Research Grade/URL, minimização, hashes, cache adulterado, IDs de resposta, limites/lotes/HTTP 429, geração determinística, superfícies QRC/pacote e obrigações de dependências.
- Auditoria estrita dos 122 organismos, checks de metadados/build, proposta de proteção, `release.py check`, regra de captura no histórico e qmllint passaram. O diff contra a base não altera `VERSION.txt`, `src/`, `resources/data/`, `resources/compliance/`, QML ou créditos oficiais. Não houve mudança visual nem necessidade de captura nova.
- `distribution.py check` retornou **2 (bloqueado)**: 120 pendências de foto, sete achados de materiais e dois bloqueios científicos; zero erros técnicos na governança do código-fonte. `check --package package` também retornou **2**, acrescentando 78 achados de obrigações/vínculos das dependências e o erro de manifesto desatualizado. O diagnóstico esperado foi verificado, não convertido em sucesso de distribuição.
- Coleta pública real para as 120 fotos, arquivada de forma minimizada no relatório histórico; dados privados não foram publicados. As verificações automatizadas da CI foram incorporadas localmente, **sem execução remota da CI** nesta intervenção.

Nenhuma decisão real, autorização ou banco de jogador foi alterado. O relatório preserva as limitações das fontes e não certifica regularização científica/jurídica. Gate de distribuição preservado, sem push, tag, release, contato externo ou publicação de executáveis/catálogos.
