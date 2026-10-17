# Preparação de licenciamento do Projeto Glossata

Preparação técnica e documental em **2026-10-17**, a partir de `09aa3bdf342982d7425bd6200da87fbabca91495` (PR #9 integrado). Branch: `docs/preparacao-licenciamento`. **Nenhuma licença do código foi escolhida ou ativada. Não foi criado `LICENSE`.** Esta análise organiza evidências e opções; não constitui parecer jurídico definitivo nem comprovação de titularidade.

O código está publicamente acessível, com intenção de evolução open source. Publicidade, gratuidade e finalidade educativa não substituem uma concessão expressa. A [definição da OSI](https://opensource.org/osd) distingue acesso ao código de condições que permitem redistribuição e derivados, inclusive usos comerciais. O GitHub também distingue [repositório público e licença de software](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository); visualizar/forkar pela plataforma não equivale a obter uma licença geral para todos os materiais.

## 1. Inventário de materiais e limites de autoria

O [inventário reproduzível por arquivo e componente](INVENTARIO_LICENCIAMENTO.json) enumera os caminhos rastreados, seus bytes SHA-256, categoria e referências aos registros existentes. Exclui somente seu próprio arquivo para evitar autorreferência. Classificação por pasta é uma descrição operacional: **não confirma autoria, originalidade ou titularidade**. Autores Git, datas de commit e participação de IA não demonstram sozinhos capacidade para licenciar uma contribuição.

| Categoria | Caminhos e exemplos | Situação e providência |
| --- | --- | --- |
| Código do aplicativo, testes e configuração | `src/`, `tests/*.cpp`, CMake, workflows e configuração | Sem licença própria selecionada; conferir origem de trechos, contribuições e titulares antes de delimitar a concessão |
| Scripts de manutenção | `scripts/*.py`, `*.ps1`, `package/run_glossata_game.cmd` | Licença própria pendente; o gerador `build_education.py` também contém texto educativo e exige análise dessa camada |
| Componentes QML do projeto | `resources/qml/` | Código declarativo e desenho da interface com titularidade a conferir; não confundir com QML do Qt copiado no pacote |
| Documentação do projeto | README, contexto, procedimentos, formulários e Markdown | Textos locais ainda sem política de licença própria; citações e contribuições de terceiros permanecem separadas |
| Textos educativos editáveis | `species_profiles.json`, `hint_education.json`; introduções no gerador | Autoria/originalidade e condições de reprodução pendentes; referência bibliográfica não autoriza copiar a redação da fonte |
| Dados científicos externos e sínteses | `species.json`, `taxonomy.json`, `phylogeny.json`, manifestos de fontes | Fatos, organização, sínteses próprias e direitos/termos de bases precisam ser distinguidos; a licença da mídia não cobre o dataset |
| Fotografias de terceiros | 120 fotos distintas do quiz e 109 fotos de apresentação | Licenças e evidências individuais; manter autores/créditos e modificações. A licença futura do código não as substitui |
| Identidade visual | PNG/ICO em `resources/branding/` | Titular, processo de criação e escopo de distribuição pendentes; não presumir cessão pelo envio do arquivo |
| Dependências externas | DLLs, plugins, QML e imagens do Qt no pacote local | 23 componentes inventariados, fora da escolha de licença própria; nenhuma biblioteca vendorizada foi identificada na árvore rastreada por esse classificador |
| Gerados e derivados | `education.json`, créditos, manifestos, relatórios, provas, capturas e fixtures | Herança das camadas de origem; geração, hash ou captura não concedem novos direitos sobre os elementos reproduzidos |

Arquivos administrativos de revisão/política são classificados como documentação/configuração ou projeções de governança; não são declarados dados externos por estarem em `resources/data`. O JSON preserva as categorias separadas mesmo quando não há arquivos de uma categoria no Git. O inventário do pacote é outro escopo: **código Qt distribuído não é código próprio do Glossata**.

Capturas públicas de tela também reproduzem mídia/identidade visual. Os registros, hashes e pendências de `docs/images/quiz.png`, `tela-inicial.png` e `exploracao.png` continuam descritos em [PRONTIDAO_EXPANSAO_CATALOGO.md](PRONTIDAO_EXPANSAO_CATALOGO.md). Não se atribuiu licença global a essas capturas, não se removeram imagens e não foi alterada a interface.

## 2. Direitos existentes e pendências atuais

Os registros de autoridade continuam sendo `content_policy.json`, `asset_rights.json`, proveniência, evidências por arquivo e ledgers. Os campos próprios `code`, `texts` e `collection` permanecem **`null`**. O novo inventário é relatório, sem criar confirmação, revisor ou evento.

- **Quiz:** 120 fotografias com documentação incompleta. Os snapshots públicos arquivados declaravam CC BY em 113 e CC0 em sete; a versão não ficou comprovada nesses snapshots. Sete comparações de crédito ainda exigem conferência. Essas contagens não demonstram violações.
- **Apresentação:** 109 fotografias com evidência por arquivo aceita pelo validador atual; as 109 revisões científicas continuam pendentes. A permissão fotográfica não aprova a identificação/representatividade.
- **Outros materiais:** sete achados de direitos ainda ativos, abrangendo três recursos de branding e quatro materiais educativos/de geração. A conferência de autoria/titularidade e a escolha de licença são decisões distintas.
- **Ciência:** dois bloqueios continuam ativos, relativos às observações Casual/cativeiro das fotos `707161720` e `659755043`. Não foi demonstrada identificação incorreta por essa condição isolada. Licenciar o código não resolve esse conflito de política.
- **Código/documentação:** ausência de licença própria expressa e de um mapa de titularidade confirmado; não há aprovação administrativa para aplicar uma das opções abaixo.

Os critérios de fotos permanecem CC0/CC BY por arquivo, com exceções somente documentadas. NC, SA e ND não se tornam autorização irrestrita por serem Creative Commons. Os [direitos por camada](DIREITOS_E_LICENCAS.md), [créditos](PHOTO_CREDITS.md) e [saneamento anterior](SANEAMENTO_DIREITOS.md) continuam aplicáveis. Não houve alteração dos 122 organismos, fontes científicas, créditos ou permissões.

## 3. Comparação das licenças do código próprio

As três opções admitem uso, estudo, modificação e redistribuição, inclusive comercial. O objetivo gratuito do mantenedor não cria uma cláusula de uso exclusivamente não comercial. A comparação se aplica ao **código que o projeto tiver direito de licenciar**, sem alterar termos de fotos, bases, marcas ou bibliotecas.

| Opção | Permissões e obrigações principais | Vantagens para o projeto | Limitações e manutenção |
| --- | --- | --- | --- |
| **MIT** | Uso, alteração e redistribuição, inclusive em produtos fechados. Preservar aviso de copyright e texto de permissão nas cópias/porções substanciais; manter a exclusão de garantia | Texto curto; pouca administração para mantenedor único; facilita reaproveitamento didático e integração em projetos variados | Não exige publicar melhorias; não oferece cláusula expressa de patente equivalente à Apache. Não dispensa obrigações de bibliotecas |
| **Apache-2.0** | Concessão de copyright e de patentes delimitada, com término desta última em certas ações de patente. Incluir licença, preservar avisos aplicáveis, indicar alterações e transportar atribuições de `NOTICE` quando existente | Patentes e contribuições tratadas expressamente; útil para futura colaboração institucional | Mais avisos/processos; não garante devolução pública das melhorias. Não concede marcas nem titularidade sobre contribuições sem autorização |
| **GPLv3** | Copyleft: ao transmitir obra coberta modificada/combinada, cumprir condições de licença, avisos e fonte correspondente. Distribuir executável exige uma via válida de acesso às fontes; requisitos adicionais podem aplicar-se a produtos de usuário | Favorece continuidade de derivados distribuídos como software livre; adequada se essa reciprocidade for prioridade do mantenedor | Exige organizar fontes/build/entrega e analisar combinações. Não é licença obrigatória para qualquer arquivo apenas por coexistir no repositório; agregados independentes e obras combinadas precisam ser distinguidos |

Fontes consultadas: [MIT — OSI, permissão e condição de aviso](https://opensource.org/license/mit); [Apache-2.0 — §§2–6](https://www.apache.org/licenses/LICENSE-2.0); [GPLv3 — §§1, 4–6, 10 e 14, texto da FSF disponibilizado pela OSI](https://opensource.org/license/gpl-3.0). Os textos GNU não responderam pelo domínio `gnu.org` nesta consulta; foram conferidos os textos disponibilizados pela OSI e as cópias do pacote local, sem presumir que a consulta falhou por conteúdo incorreto.

Para GPLv3, o mantenedor ainda deve escolher entre identificadores como `GPL-3.0-only` e `GPL-3.0-or-later`. Não há escolha implícita. Na Apache, a regra de contribuições da §5 só tem seu alcance nos termos efetivamente aplicáveis; este projeto não a ativou antecipadamente.

### Compatibilidade preliminar com as dependências

| Cenário efetivamente demonstrado no futuro | MIT | Apache-2.0 | GPLv3 |
| --- | --- | --- | --- |
| Qt/módulos sob uma opção LGPL aplicável, com todas as condições atendidas | Possível para a aplicação separável; conferir headers, biblioteca e forma de ligação | Mesma análise; não substituir LGPL pela Apache | Avaliar a combinação e cumprir as licenças/avisos aplicáveis |
| Componente GPLv3 necessário ao trabalho combinado, sem alternativa de licença | MIT isolada não resolve a distribuição do conjunto sob condições GPL | Não declarar o conjunto exclusivamente Apache; combinação depende das condições GPLv3 | Pode se alinhar ao copyleft do conjunto, sujeito às demais condições e compatibilidades |
| Mídias/dados/textos de terceiros | Manter licenças individuais e compatibilidade da forma de uso | Mesmo requisito | Mesmo requisito; separar licenças não elimina análise da obra combinada/agregada |

Essa tabela é uma hipótese de análise, **não aprovação de compatibilidade do pacote**. Apache permite incorporar seu código em trabalhos GPLv3 nas condições pertinentes; o sentido inverso não permite transformar código GPLv3 em Apache apenas por incorporá-lo. A compatibilidade com GPLv2 não deve ser presumida. [Explicação da ASF sobre compatibilidade GPL](https://www.apache.org/licenses/GPL-compatibility.html).

### Recomendação técnica para decisão

Para **baixa carga administrativa e reutilização ampla do código próprio**, recomendo discutir MIT primeiro, condicionada à titularidade confirmada e à demonstração de uma combinação lícita com as dependências. Se patentes e colaboração institucional forem prioridades, Apache-2.0 merece preferência na discussão. Se a prioridade for reciprocidade sobre derivados distribuídos, avaliar GPLv3. **Essa preferência técnica não seleciona uma licença e não autoriza aplicação automática.** Nenhuma opção resolve as pendências das mídias ou da cadeia Qt por si só.

## 4. Dependências: evidência exata do pacote existente

Foi lido, sem reexportar ou modificar, `package/compliance/dependencies.json`, inventariado em **2026-10-17T02:10:33.225963+00:00**. O snapshot público inclui seu SHA-256, componentes, versões, declarações originais, avisos/hashes, contagens e resultados do validador. O pacote é local, marcado para desenvolvimento e **não comprova o estado atual do executável**: continua ocorrendo `packaged_binary_or_compiled_sources_stale`.

São **1.346 arquivos inventariados**: 1.344 de terceiros, um executável do projeto e um ícone. Há **87 registros de avisos**, com hashes compatíveis na conferência. Os 23 componentes continuam com revisão de obrigações pendente; os 78 achados do validador se decompõem em 23 de obrigações não documentadas, 23 de ausência de revisão rastreável, nove de fonte não documentada e 23 de decisão não vinculada ao inventário. São achados cumulativos, não 78 bibliotecas diferentes.

| Pacote MSYS2, prefixo `mingw-w64-ucrt-x86_64-` omitido | Versão exata | Declaração do pacote, resumida sem escolher alternativa | Avisos |
| --- | --- | --- | --- |
| brotli | 1.2.0-1 | MIT | 1 |
| bzip2 | 1.0.8-4 | custom; conferir texto específico | 1 |
| double-conversion | 3.4.0-1 | BSD-3-Clause | 1 |
| freetype | 2.14.3-1 | GPL-2.0-or-later **OR** FTL | 2 |
| gcc-libs | 16.2.0-3 | GPL-3.0-or-later **WITH** GCC-exception-3.1 **AND** LGPL-2.1-or-later | 4 |
| gettext-runtime | 1.0-1 | GPL-3.0-or-later **AND** LGPL-2.1-or-later | 4 |
| glib2 | 2.88.3-1 | LGPL-2.1-or-later | 1 |
| graphite2 | 1.3.15-1 | LGPL-2.1-or-later | 2 |
| harfbuzz | 14.4.0-1 | MIT | 1 |
| icu | 78.3-4 | ICU na declaração; texto instalado identificado como Unicode License V3 | **0** |
| libb2 | 0.98.1-3 | custom:CC0 | 1 |
| libffi | 3.8.0-1 | MIT | 1 |
| libiconv | 1.19-1 | LGPL-2.1-or-later; documentação GPL-3.0-or-later | 4 |
| libjpeg-turbo | 3.2.0-1 | custom:BSD-like; conferir condições internas | 4 |
| libpng | 1.6.58-1 | custom; conferir texto específico | 1 |
| libwinpthread | 14.0.0.r302.gd7f3c5201-1 | MIT **AND** BSD-3-Clause-Clear | 1 |
| md4c | 0.5.3-1 | MIT | 1 |
| pcre2 | 10.47-1 | BSD-3-Clause; conferir eventuais exceções/partes | 2 |
| qt6-base | 6.11.2-2 | Expressão composta com LGPL/GPL, exceção Qt e terceiros; íntegra no JSON | 38 |
| qt6-declarative | 6.11.2-1 | Expressão composta com LGPL/GPL, exceção Qt e terceiros; íntegra no JSON | 14 |
| sqlite3 | 3.53.4-1 | PublicDomain na declaração | 1 |
| zlib | 1.3.2-2 | Zlib | 1 |
| zstd | 1.5.7-2 | BSD-3-Clause **OR** GPL-2.0-or-later | 1 |

`AND`, `OR` e `WITH` têm funções diferentes. Não foram substituídos por uma lista genérica de licenças autorizadas. Declarações de pacote podem reunir código, documentação e ferramentas sob condições diferentes; não demonstram que todo arquivo possui todas as opções. Aviso copiado não demonstra que todas as atribuições, concessões e obrigações foram atendidas.

### Qt, bibliotecas internas, fontes e ferramentas

O CMake solicita Core, Gui, Quick, Sql e Network. O pacote contém também OpenGL, Qml/Models/WorkerScript, QuickControls2 com estilos, Effects, Layouts, Shapes e Templates2, além de plugins de plataforma/imagem/SQLite e módulos QML. `qt6-base` responde por 16 DLLs; `qt6-declarative`, por 61 DLLs, 434 QML e **804 PNG**. A presença desses recursos no pacote exige conferir sua atribuição; não são imagens próprias do Glossata nem fotos do catálogo.

A [documentação Qt 6.11](https://doc.qt.io/qt-6.11/licensing.html) distingue opções comerciais/LGPL, módulos GPL e ferramentas. A expressão agregada do MSYS2 não demonstra uma licença comercial adquirida nem uma exceção aplicável a qualquer uso de Qt. O fato de uma ferramenta ser GPL também não basta para classificar todas as bibliotecas e saídas como GPL. Não foram distribuídos os executáveis de Python, Git, CMake, compilador ou `windeployqt` no inventário examinado; os runtimes GCC são outra camada.

A revisão interna deve cruzar arquivos realmente distribuídos com atribuições/configuração e fontes de **Qt 6.11.2 e revisões MSYS2 -2/-1**, incluindo patches. Componentes embutidos podem não aparecer como DLL separada. A lista oficial inclui código, dados e outros recursos de terceiros, com condições próprias; ela serve para investigação, sem provar a inclusão de todos os itens em nosso build. [Terceiros em Qt 6.11.2](https://doc.qt.io/qt-6.11/licenses-used-in-qt.html). O inventário atual não faz esse cruzamento completo.

Qt permite gerar SBOM por plataforma/configuração; obter um SBOM correspondente ao build efetivo ou produzir análise equivalente a partir das fontes e configuração. Um SBOM genérico de outra instalação não basta, e a documentação da versão não prova que o MSYS2 o gerou. [SBOM do Qt 6.11, seções de instalação e formatos](https://doc.qt.io/qt-6.11/sbom.html). Nenhum SBOM completo foi declarado concluído nesta tarefa.

Não foram encontrados arquivos TTF/OTF no inventário de distribuição nem nas mídias próprias; Segoe UI é solicitada ao sistema. Isso não dispensa conferir possíveis fontes, imagens ou dados embutidos no Qt. Avisos de exemplo, documentação GFDL e recursos específicos devem ser associados aos arquivos presentes, sem tratá-los como licença geral da interface.

### Lacuna comprovada do ICU

Para ICU **78.3-4**, o inventário registra zero avisos. O arquivo instalado `share/icu/78.3/LICENSE` existe, tem SHA-256 **`e55522d81edc687a341a4411e0776e54ca654e90147f354a90458aaced4116af`** e inicia com Unicode License V3. `dependencies()` procura avisos apenas em caminhos contendo `/share/licenses/`; por isso não recolhe esse caminho. A etiqueta `ICU` do pacote não foi trocada por outra licença no registro oficial. A ação futura é conferir o arquivo da versão exata, adaptar a coleta e transportá-lo com seus avisos/condições, sem marcar revisão concluída pela cópia. O relatório identifica somente um **candidato a aviso adicional**; o pacote e o ledger não mudaram.

### Fornecimento de fontes e substituição

Para bibliotecas sob LGPL, conferir avisos, textos de licença e as condições de modificação/recombinação, inclusive engenharia reversa para depuração dessas alterações. Uma ligação compartilhada adequada é uma das opções previstas; ter DLLs não prova sozinho o atendimento. Conferir também requisitos de informação de instalação quando aplicáveis. [LGPLv3, §4, texto da FSF na OSI](https://opensource.org/license/lgpl-3-0), [orientação oficial Qt](https://www.qt.io/development/open-source-lgpl-obligations).

Para cada obrigação de fonte, documentar a via aplicável à forma de distribuição, versões, patches e instruções de obtenção/reprodução. Uma oferta escrita genérica ou um link mutável ao upstream não demonstram cumprimento. O validador atual oferece `bundled_source` e `written_offer`; uma futura disponibilização de fonte em rede, quando juridicamente apropriada, exige análise e eventual extensão explícita do esquema com testes, sem registrar uma oferta inexistente para fazê-lo passar.

O validador também recusa `not_required` se a declaração agregada contiver `GPL`, inclusive em expressões com exceção ou partes distintas. É uma barreira conservadora, não prova de que cada DLL tem a mesma obrigação. Se uma análise fundamentada demonstrar dispensa ou outro mecanismo válido, propor ajuste delimitado do modelo/validador, com evidências e testes em PR própria. Não registrar método falso, ignorar o bloqueio ou alterar a declaração do pacote para satisfazê-lo.

GCC/libstdc++ possui exceção de runtime sujeita a condições; não é dispensa geral de obrigações sobre qualquer código GNU. Conferir os arquivos/notices e o processo de compilação aplicáveis. [Manual oficial GCC/libstdc++ e Runtime Library Exception 3.1](https://gcc.gnu.org/onlinedocs/libstdc++/manual/license.html). Para gettext/libiconv e outros pacotes compostos, mapear a licença à DLL e aos trechos realmente entregues antes de concluir exigências GPL/LGPL. O filtro textual `GPL` do gate é conservador e não é um interpretador jurídico de expressões SPDX.

## 5. Estrutura documental preparada e proposta

| Documento/registro | Situação nesta intervenção | Papel |
| --- | --- | --- |
| `README.md`, `DIREITOS_E_LICENCAS.md` | Avisos administrativos atualizados | Distinguir código público, intenção open source e licenças por material |
| `CONTRIBUTING.md` | Procedimento preparado, sem cessão/CLA ou escolha de licença | Receber propostas com origem, direitos e testes identificados |
| `INVENTARIO_LICENCIAMENTO.json` | Snapshot documental gerado offline | Classificação por arquivo e evidência exata das dependências |
| `docs/propostas/LICENCIAMENTO_CODIGO.md` | **Proposta não ativada** | Registrar decisões de escopo, titulares e condições para criar `LICENSE` |
| `PHOTO_CREDITS.md`, proveniência e ledgers | Autoridades preservadas | Atribuições/permissões e revisões reais por recurso |
| `LICENSE`, cabeçalhos SPDX próprios e eventual `NOTICE` | **Não criados/ativados** | Somente após escolha expressa e conferência de direitos |
| Avisos completos, fontes e instruções do pacote | Pendentes para distribuição | Cumprir condições das versões efetivas; não substituir por este plano |

A licença oficial escolhida deverá conservar seu texto canônico. O escopo e as exclusões ficam explicados nos documentos e mapeados por arquivo; não se reescreve MIT/Apache/GPL adicionando uma restrição de uso não comercial. Um futuro `NOTICE` deve conter avisos efetivos, sem alegar que uma lista parcial equivale a todas as obrigações. Não se cria um `NOTICE` fictício ou um `LICENSE` provisório que ferramentas possam interpretar como concessão ativa.

## 6. Plano anterior à distribuição pública

| Prioridade | Ação | Evidência/critério de conclusão | Responsável pela decisão |
| --- | --- | --- | --- |
| P0 | Conferir titularidade e decidir licença/escopo próprios | Registro expresso de escolha e de materiais cobertos, sem incluir direitos alheios | Mantenedor e titulares reais |
| P0 | Saneamento de fotos, textos e branding | Provas por arquivo e decisões legítimas no fluxo existente | Mantenedor; titulares quando necessário, sem contatos nesta tarefa |
| P0 | Resolver os dois conflitos científicos de origem | Decisão evidenciada de adequação/substituição/política, sem aprovação fictícia | Curadoria identificada |
| P0 | Mapear módulos/terceiros do Qt e demais DLLs | Versão/build/patches, avisos, opções realmente aplicáveis e compatibilidade documentados | Mantenedor, com apoio especializado quando necessário |
| P0 | Resolver aviso ausente do ICU e condições internas | Coleta correta, aviso íntegro e revisão vinculada; cópia isolada não encerra análise | Mantenedor |
| P0 | Preparar fontes e substituição | Mecanismo juridicamente aplicável, artefatos disponíveis e instruções testadas | Mantenedor |
| P1 | Gerar pacote correspondente ao código | Build-manifest, hashes e inventário atualizados em ambiente de preparação autorizado | Manutenção técnica |
| P1 | Conferir PR e documentação de contribuição | Direito de cada contribuição, licenças preservadas e CI técnica aprovada | Mantenedor/contribuidor |
| P0 antes de lançar | Executar gates completos e obter autorização de publicação | Sem erros obrigatórios de distribuição; não confundir check técnico com certificação | Mantenedor |

Não executar `package.ps1` como tentativa de encerrar este planejamento: o modo normal exige o gate, e `-Development` apenas prepara material local, sem autorizar redistribuição. Não se reexportou nem publicou o pacote nesta tarefa.

## 7. Decisões reservadas e implementação futura

O mantenedor deve decidir: licença do código e eventual modalidade GPL; titulares/anos comprovados; escopo por arquivo; política separada para documentação/textos/organização; tratamento de identidade visual e marcas; critérios de contribuições; opção aplicável por dependência; mecanismo de fornecimento/substituição e forma de distribuição. Questões que dependam de titularidade ou interpretação controversa podem exigir avaliação jurídica específica. Não há necessidade de simular outro revisor para registrar a decisão editorial do único mantenedor.

Após **autorização expressa da licença**:

1. Completar o registro de proposta com escolha real, escopo, titulares e referência da decisão. Não preencher responsáveis/datas fictícios.
2. Conferir trechos externos, direitos de emprego/contrato e contribuições anteriores. Segregar ou obter autorização quando necessário; não presumir titularidade pela posse do repositório.
3. Criar `LICENSE` com o texto oficial da opção escolhida, somente com avisos autorais fundamentados; mapear exclusões e recursos independentes. Conferir eventual obra combinada e preservar licenças de terceiros.
4. Atualizar de modo explícito os campos próprios de política/inventário, documentação, cabeçalhos aplicáveis e condições para novas contribuições. Não alterar licenças individuais de fotografias ou provas antigas.
5. Registrar a mudança administrativa em changelog; se afetar dados/política embarcada, atualizar identidade/versão do catálogo segundo o fluxo existente. Esta etapa futura não é executada nesta PR.
6. Completar conformidade do pacote, revisar avisos/entrega de fontes e registrar decisões reais pelo comando `distribution.py record-review`, vinculadas ao hash do inventário e às provas. Não registrar `not_required` para suprimir pendência sem análise.
7. Executar testes/gates, integrar por PR e somente publicar/taguear mediante autorização. Escolher a licença do código e autorizar uma release são atos distintos.

## 8. Reprodução e verificações

```powershell
# Apenas leitura dos materiais; grava relatórios em build, sem exportar pacote.
python scripts/licensing_inventory.py --package package --prefix C:\msys64\ucrt64
python scripts/distribution.py check --package package
# Quando for necessário registrar novo snapshot documental, sem ativar licença:
python scripts/licensing_inventory.py --package package --prefix C:\msys64\ucrt64 --archive
```

Sem pacote local, omitir `--package` e `--prefix`: o relatório explicita dependências não avaliadas, sem transformar ausência em zero pendências. A classificação é inventário de localização, não detector universal de copyright ou licença. Revalidar os componentes quando o pacote mudar; os números acima pertencem ao snapshot datado.

Verificações locais efetivamente executadas:

| Controle | Resultado |
| --- | --- |
| `test_rights_investigation.py` | **26 testes aprovados**, incluindo seis novos de classificação sem titularidade presumida, separação de mídia/metadados, declarações compostas, aviso ICU/versão instalada, ausência de gravações/aprovações e não contorno do gate |
| CTest de Release, RepositoryAudit, ProjectStatus, GovernanceHistory e RightsInvestigation | **5/5 suítes aprovadas**, 15,59 s |
| `audit_catalog.py --strict` | 122 registros, sem alteração de IDs, conteúdo ou elegibilidade e sem erros técnicos |
| `project_status.py check --build build` e `release.py check` | Metadados/documentação consistentes, sem criar versão, tag ou release |
| `audit-history` contra o SHA completo da base | Prefixos dos três ledgers e baseline preservados, sem eventos novos |
| Inventário e conferência do pacote | 23 componentes, 87 avisos registrados íntegros e candidato adicional ICU; nenhum pacote reexportado |
| Gate com pacote | Bloqueado: 120 fotos, sete achados de materiais, dois bloqueios científicos, 78 achados de dependências e um erro técnico de pacote desatualizado |

Também foram conferidos links locais, ausência de `LICENSE` e preservação dos arquivos de recursos e da política própria. Não houve nova compilação local ou validação visual: o aplicativo não foi modificado. A CI da PR executa seus próprios controles completos; consultar seu resultado no commit final. Não foi emitida aprovação jurídica, científica ou administrativa da licença.
