# Implementação Apache 2.0 — Projeto Glossata

Intervenção em **2026-10-17**, branch `feat/licenciamento-apache-2`, sobre `b48ab87` (PR #10 integrado). Escopo: concessão do código próprio e documentação/verificação administrativa. Não constitui certificação jurídica/científica nem liberação do pacote. A integração depende de revisão do Pull Request pelo mantenedor; não há merge automático.

## Decisão e auditoria de origem

O mantenedor autorizou expressamente Apache-2.0 e forneceu **Carlos Eduardo Lima de Oliveira** como criador original. O nome em NOTICE vem dessa solicitação, não de inferência de usuário GitHub. Não se presumiu autoria exclusiva, cessão de contribuições ou direitos sobre materiais externos; não foram inventados avisos nominais de copyright/anos, assinaturas ou revisores.

Foram consultados plano, inventário, proposta, direitos por camada, relatório histórico de conformidade, prontidão de expansão, README/CONTRIBUTING, política, inventário de recursos, proveniência/permissões e inventário Qt/MSYS2 do pacote local. O inventário anterior permanece intacto como snapshot histórico; SHA-256 `a7d0115d0b6ecd0192193c01223d6cecd64962079197306600a7f594331c7e0e`.

A inspeção da árvore de código e de avisos de origem (`SPDX`, copyright, licenças, atribuições e referências a código copiado/adaptado), com histórico Git, não identificou biblioteca vendorizada ou contribuição com termos externos na enumeração. O histórico declara duas identidades de autor de commit: `kaduolliver` e `Carlos Eduardo Lima de Oliveira`; isso não autentica autores nem comprova titularidade. Não há auditoria de contratos/emprego/coautoria desconhecida. A concessão cobre os direitos disponíveis sobre o código próprio, com investigação e limites documentados, sem afirmar que todo o repositório é propriedade do mantenedor.

## Concessão implementada e exclusões

| Item | Resultado |
| --- | --- |
| LICENSE | Texto oficial completo e inalterado da ASF, incluindo apêndice; SHA-256 `cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30` |
| NOTICE | Criador original, repositório oficial, distinção de contribuidores, reconhecimento sem cessão/exclusividade ou endosso e sem imposição adicional de UI |
| Código abrangido | **155 arquivos explicitamente enumerados** em [ESCOPO_LICENCIAMENTO.json](ESCOPO_LICENCIAMENTO.json), explicados em [ESCOPO_LICENCIAMENTO.md](ESCOPO_LICENCIAMENTO.md) |
| Código misto pendente | `scripts/build_education.py` inteiro excluído da concessão integral, por redação educativa ainda com direitos/originalidade pendentes |
| Conteúdo externo/misto | Textos educativos/científicos e dados reproduzidos/carregados por código não recebem Apache por incorporação. Fotografias, evidências, fixtures, documentação, capturas e derivados não enumerados estão excluídos |
| Identidade visual | Nome/logotipo/ícones separados; não se afirma registro de marca, nem se concede automaticamente direito marcário |
| Código próprio versus dependências | QML próprio enumerado; QML/recursos Qt copiados no pacote não abrangidos. Licenças Qt/MSYS2 e avisos individuais preservados |
| Política | `own_material_licenses.code = "Apache-2.0"` na política e no inventário de recursos; `texts`/`collection` continuam `null`. Referência à autorização administrativa e ao escopo, sem eventos de revisão científica/fotográfica |
| SPDX | Identificador no validador de inventário; avaliada e dispensada inserção em massa. Sem cabeçalhos sobre fotos/evidências/dados, gerador misto ou arquivos visuais vinculados às capturas |

O propósito gratuito/educacional e sem fins lucrativos da iniciativa oficial permanece. Apache permite utilização, modificação e redistribuição **inclusive comercial** do código coberto, nos termos oficiais, sem exceções inseridas pelo projeto. Atribuições aplicáveis seguem o §4, contribuições futuras o §5, marcas o §6. Não se cria CLA ou transferência de copyright; não se presume assentimento retroativo a uma licença futura.

## Preservação e pendências

Os 122 organismos, IDs, 120 arquivos de foto de quiz, 109 fotos de exploração, textos/dados científicos, créditos, hashes/evidências fotográficas e registros de curadoria foram preservados. Não houve download de fotos, modificação de imagem, contato com terceiros ou aprovação automática. Código de domínio, regras, persistência e QML não foram alterados. Não se abriu banco de jogador para teste.

Persistem **120 pendências documentais de fotos de quiz**, **sete divergências de crédito a conferir**, **sete achados de outros materiais**, **109 revisões científicas de fotos de exploração pendentes** e os **dois bloqueios científicos** (`707161720`, *Panthera tigris*; `659755043`, *Lemur catta*). Essas contagens não são prova automática de violações. Casual/cativeiro é conflito com a política documentada, não demonstração isolada de identificação incorreta. As evidências anteriores foram reutilizadas sem consultas desnecessárias.

O pacote local continua inventariado com **23 componentes**, 87 avisos registrados e **78 achados cumulativos** de obrigações/rastreabilidade/fontes. Confirmada a presença local de `share/icu/78.3/LICENSE`, SHA-256 `e55522d81edc687a341a4411e0776e54ca654e90147f354a90458aaced4116af`, fora do diretório coletado; o pacote continua sem esse aviso. Correção do coletor e revisão das obrigações são **trabalho separado**, sem aprovação fictícia ou mudança de licença ICU pela Apache. Não foi exportado ou publicado executável.

Política/inventário embarcados mudaram administrativamente; o manifesto foi regenerado pelo fluxo existente, completando **2.4.0 ainda não lançado**. O hash da política inteira participa da avaliação automatizada das dicas: por isso foi executado `review_hints.evaluate` e regenerada educação, sem alterar receitas, textos, assistência ou fontes. A avaliação continua `approved_automated`, com data real **2026-10-17T18:15:51.335812+00:00**, sem aprovação científica/humana. Seu hash anterior foi preservado em `asset_rights.inventory_history`, mantendo o estado de direitos pendente. A comparação estrutural confirmou alterações somente nos metadados de avaliação/inputs e na projeção correspondente, preservando todos os demais tópicos/revisões.

Aplicativo **1.7.0 em desenvolvimento**, sem incremento artificial, tag ou release. O hash novo identifica a política que acompanha esse build; resultados históricos não são recalculados e permanecem em suas identidades de conteúdo/regras anteriores. Nenhum campo de pontuação ou perfil foi alterado.

## Validações

| Verificação executada localmente | Resultado e limite |
| --- | --- |
| Configuração CMake/Ninja e compilação Release | Aprovadas; recursos administrativos recompilados, sem exportar pacote |
| CTest completo | **26/26 suítes aprovadas, 178,24 s**; bancos temporários, sem banco de jogador real |
| Direitos/inventário e regressões da concessão | **35 testes aprovados**, nove novos; fixtures sintéticas para texto oficial adulterado/restrição adicional, NOTICE incompleto, mídia/terceiros, novo código sem decisão, conteúdo misto pendente, política divergente e ausência de efeitos nos gates/revisões |
| `licensing_inventory.py --check-code` | Texto oficial, atribuição, escopo enumerado e vínculo da decisão consistentes; sem comprovação automática de titularidade |
| `audit_catalog.py --strict` | 122 organismos, zero erros técnicos; elegibilidade e corpus preservados |
| Regeneração comparada em memória | Educação/créditos correspondem às fontes. Notas, receitas, textos, assistência e fontes científicas idênticos à base, exceto metadados da avaliação automatizada explicitamente reexecutada |
| Comparação com Git-base completo | Apenas cinco recursos alterados: política, inventário, manifesto e metadados das dicas/educação. Fotos, branding, provas, fontes e créditos sem diff; histórico anterior do inventário preservado |
| `recover-permissions` / `audit-history --base b48ab8700f210558d97394007db9773e6e892c73` | Estado consistente; três ledgers sem eventos novos e baseline SHA-256 `97b500f9b18539e9b67d631f53fddcc4a272ae499173d5a494719f3e9c6667a6` preservado |
| `release.py check`, `project_status.py check --build build` | Fontes/manifestações e documentação consistentes; aplicativo 1.7.0 e catálogo 2.4.0 continuam não lançados |
| Validação de proteção local, exploração, provedores, assistência, filogenia, atualizações e qmllint | Todos com saída 0; manutenção de conteúdo offline, sem consultas externas de fotos ou alterações de classificação |
| Documentos/índice | Links relativos existentes, UTF-8 e diff sem erros de whitespace; auditoria de arquivos versionados/índice aplicada antes de commit |
| Gate de fonte | **Bloqueado**, zero erros técnicos, 120 fotos, sete outros materiais e dois bloqueios científicos |
| `distribution.py check --package package` | **Recusado com código 2**: mesmos achados, 78 de dependências e um erro de pacote/binário desatualizado. Não se tentou fazer o gate passar |

Logs locais em `build/apache-check-*.log`, `build/apache-checks.json`, `build/apache-preservation.json`, `build/Testing/Temporary/LastTest.log` e relatórios de auditoria. Não são artefatos de distribuição. A CI remota **`offline-validation`** foi mantida e ampliada com o check de escopo; seu resultado deve ser conferido no commit final do PR, separadamente de `distribution-permissions`. Não foi realizada inspeção visual manual ou teste de pacote numa instalação Windows limpa; a interface permaneceu sem mudanças, sem necessidade de nova captura.

## Manutenção e próxima etapa

1. Conferir o escopo e eventuais direitos de empregador/coautores/trechos externos ainda não identificados. Novos arquivos exigem origem declarada e inclusão explícita ou pendência, com `licensing_inventory.py --check-code`.
2. Separar o gerador misto de seu conteúdo educativo e revisar originalidade/fontes; escolher licenças de textos/organização somente mediante autorização própria.
3. Completar evidências e decisões legítimas por foto/material e os dois bloqueios científicos, sem usar LICENSE como permissão de mídia.
4. Resolver a coleta ICU e as obrigações reais de Qt/MSYS2, inclusive fontes/avisos/recombinação e componentes internos efetivamente distribuídos.
5. Antes de qualquer lançamento, transportar LICENSE/NOTICE e o escopo com o código coberto e cumprir os avisos externos; executar todos os gates e obter autorização específica. Esta PR não altera o empacotador nem autoriza distribuição.

Históricos de preparação permanecem datados, com atualização administrativa separada. Integridade técnica não certifica titularidade, validade das permissões ou exatidão científica. `distribution-permissions` continua distinto de `offline-validation`, com seus bloqueios preservados.
