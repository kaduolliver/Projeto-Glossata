# Relatório da implementação — 15/10/2026

> **Atualização administrativa — 2026-10-17:** Apache-2.0 foi autorizada somente para o [código próprio enumerado](ESCOPO_LICENCIAMENTO.md), com LICENSE/NOTICE. A ausência de licença descrita nesta etapa histórica era correta à época. Textos/coletânea, mídias, marca, dependências e pendências científicas permanecem separados; nenhum resultado histórico de auditoria foi reescrito.

> Registro histórico desta etapa. A investigação INV-08 de 17/10/2026 está em [SANEAMENTO_DIREITOS.md](SANEAMENTO_DIREITOS.md). Corrigida abaixo uma inversão documental de IDs: o catálogo e as evidências sempre associaram `707161720` a *Panthera tigris* e `659755043` a *Lemur catta*. A correção não altera associações ou revisões científicas.

Atualização filogenética: aplicativo **1.2.0**, catálogo **2.0.0**, ainda não lançados. Os 122 registros foram projetados para Bacteria (20) e Eukaryota (102); Archaea permanece vazio. Há 62 Opisthokonta, 20 Archaeplastida, 10 Amoebozoa e 10 SAR; Discoba não tem representantes. IDs, fotos e metadados de direitos foram comparados integralmente com o registro anterior. Os 122 hashes de revisão da classificação de origem permanecem iguais; os 122 enquadramentos novos são rascunhos com revisão independente. Não houve concessão de licença nem revisão humana simulada.

A migração do catálogo é transacional, mantém snapshot das linhas anteriores e rejeita versões futuras desconhecidas sem rebaixamento. O histórico de jogadores conserva seu esquema e pontuações; o hash de conteúdo distingue o novo ranking competitivo. A [documentação da classificação](CLASSIFICACAO_FILOGENETICA.md) e a [auditoria por organismo](AUDITORIA_FILOGENETICA.json) registram fontes, divergências e limites. Os testes incluem rollback, reabertura, preservação das 122 identidades e fotos, rastreabilidade de revisão, ranks ausentes e navegação real. Permanecem os bloqueios de direitos/curadoria registrados abaixo; nenhuma tag, push ou release integra esta etapa.

## Histórico anterior da governança

Atualização de dicas: `identification-v3-assistance` separa categoria de assistência e preserva pontos antigos com migração SQLite v3 e snapshots verificáveis. Foram auditadas 290 avaliações para os 122 organismos, com 132 mudanças de teto propostas e quatro inadequações como morfologia prévia. Todas são rascunhos de origem IA: não houve aprovação humana. Dicas provisórias são sinalizadas no Livre e ficam indisponíveis no Competitivo. Texto científico, fontes, organismos e fotografias permanecem preservados. Consulte [DICAS_E_PONTUACAO.md](DICAS_E_PONTUACAO.md) e [AUDITORIA_ASSISTENCIA.json](AUDITORIA_ASSISTENCIA.json). Não houve alteração de licença, push ou lançamento nesta etapa.

Atualização de versionamento: aplicativo **1.1.0 em desenvolvimento**, catálogo independente **1.0.0 ainda não lançado**. Os dados, revisões, mídias ativas e evidências são identificados em `resources/data/catalog_manifest.json` e vinculados ao executável por `build/build-manifest.json`. O acervo histórico local fora do catálogo continua auditado separadamente; não é dependência do manifesto embarcado. `CHANGELOG.md`, `CATALOGO_CHANGELOG.md` e `VERSIONAMENTO.md` documentam o histórico e os bloqueios. A captura real de 1240×820 está em `docs/images/tela-inicial.png`, com origem documentada; não contém dados pessoais de jogadores. A validação desta atualização inclui seis suítes CTest, 13 testes específicos de lançamento, lint e auditoria técnica. Nenhuma tag, push ou release foi criada: os bloqueios de distribuição permanecem.

Foram implementados controles e uma matriz de ações, **sem certificação jurídica ou científica**. A versão local mantém os 122 organismos, 120 fotografias distintas, navegação, fichas, dicas, modos Livre/Competitivo e dados dos jogadores. Nenhum banco pessoal foi alterado pelos scripts de governança; os testes usam bancos temporários.

## Entregas concluídas

- Política CC0/CC BY reaplicada antes de qualquer download direto, inclusive por chamadas auxiliares. Licença de observação não autoriza foto. URL/ID, autoria disponível, foto associada, Research Grade, táxon comunitário e detalhe taxonômico são conferidos na importação.
- Exceções dependem de autorização por foto, escopo e evidência, com política NC explícita quando cabível. Nada foi ativado como exceção nesta entrega.
- Migração idempotente de 1.660 afirmações/tópicos para baseline legada. Nenhum responsável, data original, título ou aprovação humana foi inventado. A data real de arquivamento/migração consta nos arquivos, separada da coleta desconhecida.
- 120 snapshots mínimos de evidência preservados, com hashes e identificadores, sem coordenadas nem perfis do observador. Sete divergências de crédito foram sinalizadas, sem correção por inferência.
- Créditos gerados em JSON, documento e interface. O pacote local inclui os créditos e as evidências offline. Alterações, licenças, origens específicas e ausência de versão comprovada são expostas.
- Histórico humano encadeado por tópico, declaração de responsável humano, tipo editorial/especialista, data, escopo, justificativa, localização da evidência e hash do conteúdo. Modificações relevantes exigem nova revisão; a expiração também é aplicada offline.
- Novo conteúdo, inclusive assistido por IA, fica em rascunho até revisão documentada. As flags históricas `approved`/`verified` não são promovidas a revisão humana.
- Consultas de manutenção agora geram propostas, sem substituir conteúdo publicado ou renovar datas. Verificações possíveis incluem indisponibilidade de referências, identificação comunitária, taxonomia, conservação e licença. Não foi feita uma atualização científica em massa na rede nesta etapa.
- Auditoria de recursos antigos, imagens, textos, ícones, fontes e inventário de dependências por bytes. Foram identificados 146 arquivos de mídia fora do manifesto de recursos atual, com permissão ainda não demonstrada; não entram no executável, mas exigem avaliação antes de distribuir o repositório/arquivo-fonte.
- Fichas mostram estado, responsável/data ausentes quando não demonstrados, fontes por tópico, limitações e caminho de correção/remoção. Há tela de créditos offline em Perfis e ajustes.
- Gate de distribuição separado da integridade técnica. Empacotamento padrão bloqueia; `-Development` destina-se somente a teste local, com aviso e relatórios. O gate não publica nem envia arquivos a terceiros.
- Workflow de CI com testes offline, geração reprodutível, lint, auditoria e job separado de distribuição. As ações foram fixadas em commits oficiais. Não houve execução remota nem publicação no GitHub nesta tarefa.

## Evidência de validação

| Verificação | Resultado e limite |
| --- | --- |
| Compilação Release | Concluída; valida compilação, não o mérito científico |
| CTest | Seis suítes, zero falhas; execução completa da correção de dicas em 33,42 s |
| GlossataTests | Regras, perfis/ranking, migrações e expiração de revisão offline |
| GlossataInterfaceTests | Fluxos de jogo, revisão, 20 questões competitivas e nova tela de créditos |
| GlossataDiscordTests | Protocolo em servidores locais fictícios; não publica atividade real |
| GlossataImportTests | 14 testes offline, incluindo preservação de identidades/textos e geração reprodutível |
| GlossataGovernanceTests | 22 testes offline: licença individual, bloqueio antes da rede, prova de permissão, privacidade JSON/EXIF/XMP, crédito, legado, revisão humana, expiração, alteração de hierarquia/nome, histórico e publicação/execução desatualizada; revisão pedagógica independente e invalidada por mudanças |
| GlossataReleaseTests | 13 testes offline: SemVer, versões independentes, dados/evidências alterados, cache local fora do catálogo, captura desatualizada, PNG no commit visual, árvore limpa, gates obrigatórios e ausência de tag em caso de falha |
| qmllint | Sem avisos após os ajustes |
| Auditoria técnica | Zero erros técnicos detectados; 122 fotos tecnicamente válidas |
| Cobertura local | 40 espécies; 100 registros em gênero/família/ordem; 120 em classe; 121 em filo/reino; 80 no pool competitivo |
| Gate de distribuição | Deve falhar com código 2 enquanto houver as pendências abaixo; esse resultado é esperado |
| Pacote local | Iniciou com PATH isolado e banco temporário, código 0; executável corresponde ao manifesto do build |
| Inventário empacotado | 1.346 arquivos de execução/mídia analisados; 23 pacotes de terceiros identificados, nenhum arquivo de terceiro sem dono identificado por hash; avisos disponíveis copiados. Obrigações continuam pendentes |

Artefatos reproduzíveis: `build/catalog-quality.json`, `build/distribution-audit.json`, `build/update-proposals/*.json`, `build/tests/core-results.txt`, `build/tests/ui-test-results.txt`, `build/tests/previews/`, `build/package-smoke.json` e `package/compliance/`. Esses caminhos gerados não são versionados. O pacote local deve ser validado novamente quando seus bytes/dependências mudarem.

## Pendências verificadas e decisões que continuam humanas

| Achado | Consequência/ação necessária |
| --- | --- |
| 120 fotos sem versão e data original de licença demonstradas; confirmação humana ausente | Completar prova localizada da versão e captura real; preservar a lacuna original e evidência anterior |
| Sete créditos divergem do snapshot da observação | Conferir outra fonte específica da mesma foto; não presumir que o observador é autor |
| Panthera tigris, foto 707161720, e Lemur catta, foto 659755043, com origem Casual (IDs corrigidos documentalmente em 17/10/2026) | Revisão científica prioritária; achados graves bloqueiam lançamento. Uso local é legado sinalizado |
| 1.568 registros legados, 91 pendentes e um em análise | Nenhuma revisão científica humana rastreável criada nesta etapa; completar revisão por tópico e afirmação |
| Arte temporária sem cadeia de direitos comprovada | Mantenedor deve documentar autorização ou escolher substituição apropriada |
| Corpus textual sem revisão de originalidade/direitos concluída | Bibliografia não autoriza copiar; conferir paráfrases, eventuais trechos e autoria |
| 146 mídias antigas fora do executável atual | Não presumir liberação por não serem mais utilizadas; avaliar distribuição do código/histórico público |
| Dependências com inventário e avisos, mas obrigações sem conclusão humana | Revisar módulos/versões/componentes internos, fornecimento de código, substituição e canal de distribuição |
| Licença dos materiais próprios não escolhida | Decisão expressa do mantenedor por camada; nenhum LICENSE global criado |
| Canal privado para documentos de titularidade ausente | Cadastrar contato real; não anexar dados privados em Issues públicas |

As evidências do cache não demonstram a autoria real de cada fotografia, a validade jurídica de cada grant ou a exatidão de cada afirmação. Hashes demonstram correspondência de bytes, não titularidade ou verdade científica. Uma cadeia de eventos local não autentica uma pessoa nem substitui revisão independente. Permanecem necessários revisão humana, cumprimento das obrigações de cada distribuição e teste em instalação Windows limpa. O plano detalhado e os critérios de conclusão estão em [CONFORMIDADE.md](CONFORMIDADE.md).

A inclusão dos metadados de governança altera o hash de conteúdo do Competitivo. Os recordes da versão anterior permanecem armazenados em sua configuração/versão; não são misturados com a nova chave de ranking. Isso não apaga partidas ou perfis.

A auditoria também procura GPS e números de série comuns em EXIF de JPEG/eXIf de PNG e tags GPS de XMP, sem imprimir seus valores ou regravar fotos. Nenhum achado desse detector ocorreu nos arquivos atuais. Isso não substitui análise completa de metadados, inspeção de locais identificáveis na própria imagem ou triagem de dados pessoais em texto livre. Após a execução completa do CTest, controles adicionais do gate foram verificados novamente pelas suítes de importação/governança e a ficha offline foi revalidada pela suíte de interface.
