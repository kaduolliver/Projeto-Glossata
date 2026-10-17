# Plano de ação, rastreabilidade e limites

Este documento registra controles implementados e trabalho ainda necessário. **O projeto não está certificado nem integralmente regularizado.** Integridade técnica, evidência de permissão e revisão científica são verificações distintas. O resultado de um teste não substitui análise de direitos ou leitura especializada das fontes.

## Diagnóstico e matriz de ação

A base examinada contém 122 organismos e 120 fotografias distintas. Havia filtros CC0/CC BY, validação de arquivos e revisões por tópico, mas faltavam responsáveis e evidências localizadas para demonstrar revisão humana. As datas antigas são preservadas como dados históricos, sem serem promovidas a datas comprovadas de revisão. A migração identificou 1.660 registros de afirmação/tópico, com 1.568 marcações legadas e os demais pendentes/em análise. Não foi criada nenhuma aprovação humana.

P0 bloqueia redistribuição; P1 exige tratamento científico/editorial prioritário; P2 melhora manutenção e transparência. “Implementado” descreve o controle técnico, não a conclusão da pendência humana.

| Requisito | Situação atual | Lacuna | Prioridade | Ação | Critério de conclusão |
| --- | --- | --- | --- | --- | --- |
| Licença individual por foto | CC0/CC BY conferidos no campo da própria foto | Código da API não comprova sozinho versão ou todas as condições | P0 | `download_photo` reaplica a política antes de rede/cache; confirmação documental separada | Cada foto distribuída com evidência íntegra e versão comprovada |
| Data e evidência da coleta | 120 snapshots mínimos recuperados de cache | Data original de coleta desconhecida | P0 | Preservar `original_collected_at: null`; arquivar sem inventar data; nova captura documentada para revalidação | Data de coleta/captura demonstrada, com lacuna histórica explícita |
| Direitos restritos/ausentes | Bloqueio padrão antes do download | Não há política NC aprovada | P0 | CC BY-NC desativada; exceções por foto, prova, escopo e restrições | Política expressa e grant específico válidos, ou recurso excluído da distribuição |
| Crédito do fotógrafo | Crédito da fonte preservado literalmente | Sete créditos antigos divergem do snapshot da observação; alguns podem vir de outra fonte da mesma foto | P0 | Sinalizar; procurar prova do autor da foto, sem usar o observador como substituto | Divergência resolvida por fonte específica ou crédito corrigido documentalmente |
| Créditos automáticos | JSON, documento e tela usam o mesmo gerador | Confirmações de versão ainda pendentes | P0 | `content_governance.py credits`; checagem de sincronização | Autor/título fornecido, origem, licença e alterações presentes; nenhum nome inventado |
| Ícones e identidade | Arte recebida do mantenedor | Titularidade e autorização de redistribuição não comprovadas | P0 | Inventário com hashes em `asset_rights.json` | Evidência do titular e escopo de distribuição documentados |
| Textos e paráfrases | Corpus legado assistido por IA com bibliografia | Originalidade, direitos de eventuais trechos e apoio às afirmações precisam de revisão | P0/P1 | Revisar por tópico; não considerar URL uma autorização | Declaração de autoria e/ou licença dos trechos, com evidência e localização |
| Research Grade | Filtro obrigatório para novas observações | Duas fotos legadas de Panthera tigris e Lemur catta têm origem Casual | P1, bloqueio de lançamento | Manter legado local com ressalva e registrar achado grave; substituição/revisão explícita, nunca silenciosa | Origem adequada ou exceção científica humana justificada, registrada e compatível com uso |
| Identidade | ID da foto, observação, táxon comunitário e detalhe taxonômico conferidos | Aprovação antiga não equivale a laudo | P1 | Hash por tópico e vínculo à foto; divergência vira proposta | Revisor humano confirma identidade e limites dos ranks usados |
| Fontes por afirmação | Todas as notas têm referência | Localização da evidência e escopo não estão demonstrados nas notas antigas | P1 | Registro humano exige seção/página/registro e o que a fonte sustenta | Cada afirmação aprovada com evidência localizada e escopo correto |
| Ocorrência e distribuição | Fichas preservam textos existentes | Registro pontual não sustenta distribuição de toda a espécie | P1 | Aviso explícito; `claim_scope` obrigatório em novas aprovações | Revisor distingue observação, ocorrência, distribuição conhecida e espécie |
| Classificação | Hierarquia iNaturalist e agrupamento didático separados | Fonte não é consenso universal ou filogenia definitiva | P1 | Sistema/fonte expostos na ficha e política | Divergências taxonômicas documentadas, sem misturar os dois modelos |
| Revisão por tópico | Histórico encadeado, identidade e hash | Ausência de revisões humanas rastreáveis do legado | P1 | Migrar como legado; registrar responsável, data, motivo, evidências e tipo | Aprovação editorial/especialista rastreável; sem simular revisão independente |
| IA | Novo conteúdo sem revisão fica em rascunho | Conteúdo legado precisa de revisão humana | P1 | Geração/HTTP/testes não aprovam; fonte existente não basta | Leitura humana e decisão explícita, com evidência localizada |
| Prazos de revisão | Prazos configuráveis por tópico | Legados sem data não podem receber prazo renovado fictício | P1 | Relatório de pendentes/vencidos; expiração também offline | Pendências triadas; revisão nova registrada em data real |
| Atualização externa | Propostas de links, identificação, taxonomia, conservação e licença | Consulta não equivale a aceitação | P1 | `review_updates.py --online`, limitado e sem mutações | Mantenedor avalia proposta e registra decisão/histórico |
| Histórico de licença | Evidências imutáveis e histórico de confirmação | Mudanças posteriores exigem análise de contexto | P0 | Preservar grant anterior; não presumir revogação automática de CC válida | Análise humana registrada com evidência original preservada |
| Privacidade e APIs | Sem coordenadas/perfis do observador nos snapshots publicados; requisições espaçadas | Outros campos e novos provedores precisam de revisão | P1 | Whitelist e auditoria de campos proibidos; não recuperar locais protegidos | Evidência publicada mínima e termos/limites respeitados |
| Dependências e fontes | Inventário dos arquivos realmente empacotados, versão, pacote e cópias de avisos | Obrigações por módulo, componentes internos, fontes e meios de acesso ao código ainda pendentes | P0 | Inventário por bytes; arquivos desconhecidos e obrigações não documentadas bloqueiam distribuição | Revisão humana de cada componente/versão e cumprimento das obrigações aplicáveis |
| Licenças próprias | Código, textos e coletânea separados | Mantenedor não escolheu a licença desses materiais | Decisão | Opções em `DIREITOS_E_LICENCAS.md`; nenhum LICENSE global criado | Escolha expressa, autoria confirmada e escopo definido, sem relicenciar terceiros |
| Relatos/remoção | Caminho na ficha e procedimento offline | Canal privado do mantenedor ainda não cadastrado | P1/decisão | Issue com IDs públicos; dados privados não devem ser anexados | Canal privado fornecido e procedimento de triagem confirmado |
| Distribuição | Gate separado e fechado por padrão | Evidências atuais ainda insuficientes | P0 | Empacotamento público bloqueia; local exige `-Development` e aviso | Nenhum bloqueio de permissão/obrigação/gravidade alta, com decisões humanas registradas |
| Integração contínua | Testes offline, lint, auditoria e job de distribuição separados | Execução remota depende de enviar o workflow ao GitHub | P1 | Workflow sem publicação de binários/mídias; arquiva só relatórios | Execução remota confirmada; bloqueio de distribuição permanece visível |

## Modelo e preservação de dados

- `resources/compliance/review_baseline.json` congela hashes e estados anteriores. É uma migração idempotente, não um novo conjunto de aprovações. Não deve ser refeito para aprovar alterações.
- `resources/compliance/permissions/*.json` contém snapshots mínimos, com hash no nome, origem, data de arquivamento e data de coleta quando demonstrada. Cache antigo não recebe retroativamente uma data de coleta.
- `resources/data/photo_provenance.json` separa o crédito literal, origem específica, ID da observação, ID da foto, versão, evidências, bytes e alterações. Títulos não fornecidos permanecem `null`.
- `review_history.jsonl` registra decisões declaradas humanas encadeadas; alterações em conteúdo/fonte/foto exigem outra revisão. `permission_history.jsonl` preserva confirmações e o registro anterior. As cadeias verificam consistência interna, sem assinatura digital, autenticação de revisor ou prova independente. `audit-history --base SHA_COMPLETO` compara os prefixos com uma referência Git selecionada e preservada separadamente; uma cadeia totalmente recalculada pode passar sem essa referência. Consulte [garantias, âncoras e confiança](AUDITORIA_E_CONFIANCA.md).
- `education.json` mantém as informações legadas com aviso, sem chamá-las de revisão por especialista. As flags antigas `approved`/`verified` são entradas históricas de compatibilidade; a autoridade atual é `governance` e o histórico rastreável. Novas notas e alterações sem decisão humana ficam indisponíveis como pistas confirmadas.
- Perfis, partidas, ranking e SQLite dos jogadores não são alterados por esses scripts. IDs do catálogo são preservados. A migração de governança é em arquivos de conteúdo; não cria uma migração no banco dos jogadores.
- Os modos Livre e Competitivo continuam disponíveis localmente com a cobertura anterior, explicitamente legada. Isso não autoriza redistribuir o pacote ou afirmar que as identificações têm revisão especializada. O gate de lançamento trata as pendências separadamente.

## Operação offline e manutenção

Na raiz do projeto:

```powershell
python scripts/content_governance.py migrate  # Apenas a primeira vez; não renova datas
python scripts/build_education.py
python scripts/content_governance.py credits
python scripts/audit_catalog.py --strict
python scripts/review_updates.py             # Relatório offline; não usa rede
ctest --test-dir build --output-on-failure
```

`--strict` verifica integridade e política técnica. Os campos de licenciamento e ciência permanecem separados no JSON. As métricas históricas `verified_etymology`/`verified_morphology` significam disponibilidade de conteúdo legado, não número de revisões científicas demonstradas.

```powershell
python scripts/review_updates.py --online --max-checks 25
```

A consulta externa é explícita, com até 25 verificações de observações, 25 de táxons e 25 de referências. O resultado é uma proposta em `build/update-proposals/`; não troca textos, imagens, licenças nem datas de revisão. Links indisponíveis, ausência de conteúdo ou sucesso HTTP são sinais para investigar, não conclusões científicas. Não foram feitas consultas em massa nesta implementação. `refine_photos.py` mantém uma entrada compatível, agora somente de propostas.

Os prazos em `content_policy.json` podem ser ajustados por tópico. Registros sem data comprovada ficam pendentes/legados, não artificialmente recentes. Uma instalação antiga deixa de usar aprovações rastreáveis vencidas mesmo sem internet.

## Registrar uma revisão humana

Escolha um tópico (`topic:1:taxonomy`, `topic:1:photo`, `topic:1:education`, etc.) ou afirmação (`species:1:distribution`, etc.) no relatório. Use o `content_sha256` atual; não reutilize um hash após editar conteúdo. Um único mantenedor pode fazer revisão editorial; só registre especialista quando houver esse responsável e sua competência documentada.

Crie uma decisão JSON real com campos:

```json
{
  "state": "approved_editorial",
  "actor_type": "human",
  "reviewer": "IDENTIFICADOR_PUBLICO_DO_REVISOR_REAL",
  "review_kind": "editorial",
  "reviewed_at": "DATA_REAL_DA_REVISAO_EM_ISO_8601_COM_FUSO",
  "rationale": "JUSTIFICATIVA_DA_DECISAO_REAL",
  "content_sha256": "HASH_ATUAL_DO_REGISTRO_REVISADO",
  "claim_scope": "known_distribution",
  "evidence": [{"url": "URL_REAL_DA_FONTE", "locator": "SECAO_PAGINA_OU_REGISTRO_REAL", "supports": "AFIRMACAO_QUE_A_FONTE_SUSTENTA"}]
}
```

Isso é um esquema ilustrativo, **não uma revisão pronta para submissão**. Preencha somente depois de ler a evidência. Para especialista, `approved_expert`, `review_kind: specialist` e `expertise` também são necessários. Não use IA como responsável humano nem preencha datas por inferência.

```powershell
python scripts/content_governance.py record-review --key species:1:distribution --decision caminho/decisao-real.json
python scripts/build_education.py
python scripts/audit_catalog.py --strict
```

Para confirmação de permissão, `confirm-permission --decision ...` exige foto e bytes vinculados ao hash, versão demonstrada, responsável, data, justificativa, snapshots íntegros e prova localizada da versão (`license_version_evidence`: path, sha256, source_url, locator). Evidência nova deve ter coleta real; o campo original desconhecido é preservado. Não basta inserir `4.0` no JSON ou mudar uma flag. A confirmação é uma declaração humana verificável nos arquivos, não um parecer jurídico automático.

## Preparar pacote e avaliar lançamento

```powershell
# Uso local autorizado de desenvolvimento; continua offline e preserva os bancos
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Development
powershell -ExecutionPolicy Bypass -File scripts/validate_package.ps1

# Fluxo de distribuição: falha enquanto faltarem permissões/obrigações
python scripts/audit_catalog.py --distribution
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
python scripts/distribution.py check --package package
```

O modo de desenvolvimento não libera direitos nem deve ser usado para publicar um pacote. Ele grava `LOCAL_DEVELOPMENT_ONLY.txt`, créditos, evidências, avisos de dependências e diagnóstico na pasta `compliance`. A ausência de um executável no Git não resolve por si só direitos sobre mídias já presentes no repositório; o mantenedor deve avaliar a exposição existente, inclusive histórico público, sem apagá-lo automaticamente.

O inventário associa arquivos empacotados aos pacotes instalados por hash, copia avisos disponíveis e deixa as obrigações pendentes. Arquivos sem dono, licença/aviso sem prova, componentes novos e falta de acesso documentado ao código correspondente não devem receber aprovação por passarem no smoke test. Revisar também componentes internos do Qt e condições do canal de distribuição.

A compilação registra `build/build-manifest.json` após vincular o executável. O gate do pacote confere os bytes do programa e os arquivos-fonte/recursos correspondentes; metadados atuais não liberam um executável antigo que ainda incorpore outros recursos. Esse manifesto é controle de integridade do build, não assinatura, permissão ou revisão científica.

Decisões de dependências usam `python scripts/distribution.py record-review --package package --decision ...`. A decisão deve vincular `package` e `content_sha256` atuais do componente, `actor_type: human`, responsável, data, justificativa, arquivo público de evidência e SHA-256. `source_provision` documenta `method` (`bundled_source`, `written_offer` ou, quando aplicável, `not_required`), prova do fornecimento (`source_delivery`: path/sha256) e instruções de substituição (`replacement_instructions`: path/sha256). O código não aceita dispensa automática para declarações GPL/LGPL. Decisões anteriores permanecem em `dependency_reviews.jsonl`; alterações de versão/bytes/avisos deixam de corresponder ao hash revisado.

Somente evidências públicas mínimas devem ficar em `resources/compliance`: esse diretório é versionado e exportado ao pacote. Contratos, dados de contato privados e provas pessoais devem ficar fora dele; registre no projeto apenas resumo público adequado e sua referência controlada. O inventário e os hashes não detectam por si só todos os dados pessoais em texto livre, nem avaliam a suficiência jurídica de uma autorização.

## Decisões e revisão humana ainda necessárias

1. Confirmar direitos da arte temporária ou substituí-la por material com permissão demonstrada. Não há nova arte aprovada nesta etapa.
2. Revalidar as versões e datas das permissões das fotos e resolver os sete créditos divergentes. Preservar evidência de grants válidos anteriores ao analisar mudanças.
3. Revisar originalidade dos textos, o apoio de cada afirmação, escopo de distribuição/conservação e fotografias Casual. Não remover espécies ou alterar conteúdo científico silenciosamente.
4. Registrar responsável público por tópico. Nomear um especialista somente se ele realmente tiver revisado; revisão editorial por uma pessoa continua sendo revisão editorial.
5. Escolher licenças apenas dos materiais próprios; decidir expressamente se haverá alguma política NC e como controlar sua distribuição. O padrão CC0/CC BY continua ativo.
6. Revisar dependências/versões, avisos, código correspondente e instruções de substituição/religação. Completar validação em Windows limpo.
7. Cadastrar um canal privado real para titulares e pesquisadores. Não foi inventado endereço de contato.

Fontes normativas e de provedor, consultadas para esta etapa, estão em [Direitos e licenças](DIREITOS_E_LICENCAS.md). O relatório de validação e as limitações da entrega ficam em [RELATORIO_CONFORMIDADE.md](RELATORIO_CONFORMIDADE.md).
