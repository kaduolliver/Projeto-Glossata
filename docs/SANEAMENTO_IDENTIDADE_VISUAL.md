# Proveniência e direitos da identidade visual — Issue #2, etapa 2

Investigação documental e visual assistida por IA em **2026-10-17**, sobre `d2280de6a6825125d04a8fd4579357838c684096`. O PR #12 foi confirmado como integrado à `main`. O workspace estava limpo; esta intervenção utiliza `docs/issue-2-branding-provenance`, sem descartar trabalhos anteriores. [Registro estruturado, hashes e comparação de pixels](SANEAMENTO_IDENTIDADE_VISUAL.json).

**Resultado: origem mais documentada; os três achados de direitos continuam pendentes.** Não se aprovou titularidade, autorização externa, licença gráfica ou distribuição. Não houve geração, conversão, substituição ou modificação dos ícones. Código Apache-2.0, fotos, textos educativos e regras do jogo permanecem separados desta investigação. Não é parecer jurídico ou revisão especializada independente.

## Evidências e limites

| Evidência | O que demonstra | O que não demonstra |
| --- | --- | --- |
| Primeiro registro Git dos três arquivos em `a9dc4ed`, 2026-10-14 | Presença dos bytes no snapshot inicial; hashes atuais coincidem | Data da criação gráfica, autoria exclusiva, ferramenta anterior ao Git, direitos de referências ou cessão |
| `resources/branding/README.md` | Fornecimento registrado do pacote `glossata_icon_pack` e finalidades dos arquivos | Licença, autorização externa ou método efetivo de geração |
| [Declaração original](../resources/compliance/branding/origin-declaration.json) | Mantenedor informou criação via ChatGPT; três hashes vinculados | Data, modelo/plano, termos históricos, entradas e direitos externos |
| [Nova declaração](../resources/compliance/branding/reference-disclosure.json) | Referência inicial aos Vagalumes de The Last of Us, adaptada para mariposa e grupos biológicos, expressamente informada pelo mantenedor | Arquivo/versão de referência, envio de imagem ou descrição textual, elementos incorporados ou autorização |
| Decodificação PNG/ICO e comparação RGBA | Dimensões, transparência e equivalência numérica dos quadros ICO a redimensionamentos do PNG principal | Sequência histórica, autoria, licença ou exclusividade |
| `scripts/set-icon.ps1`, presente desde o snapshot inicial | Ferramenta disponível: PNG 1024 × 1024 e ICO com interpolação bicúbica | Que tenha produzido estes arquivos: PNGs atuais têm 1254 × 1254 e os quadros ICO coincidem com Lanczos, não com a comparação bicúbica realizada |

A declaração anterior foi preservada **byte a byte**, no mesmo caminho. A nova a complementa, com contexto e desconhecidos explícitos. Não se publicaram conversas privadas, prompts, credenciais ou imagens de entrada. A data do registro não foi utilizada como data de criação. **Carlos Eduardo Lima de Oliveira** foi expressamente identificado como criador original da iniciativa; isso não autentica autoria gráfica exclusiva.

## Relação entre os arquivos

| Arquivo | Dados atuais | Relação demonstrada e limite | Situação individual |
| --- | --- | --- | --- |
| `glossata.png` | PNG RGBA, 1254 × 1254, 1.830.645 bytes; SHA-256 `6c83e4e175be354ce8e889b6548121ded4e9a1ae9685d2d44327c0a9f63bde4b` | Arte-base da comparação: mariposa com contorno dourado, antenas plumosas, cabeça circular e painéis biológicos ilustrados | `pending_permission`, `distribution_allowed: false`. Faltam entradas, condições históricas e decisão gráfica fundamentada |
| `glossata_discord.png` | PNG RGBA, 1254 × 1254, 1.929.682 bytes; SHA-256 `e0c34c105919bfc2e49fb02804ec52794241958793f0926b14939992282a2549` | Organização temática semelhante, mas enquadramento, contornos, detalhes e pixels diferem. Não foi demonstrado que seja mera remoção de fundo, nem a sequência entre variantes | `pending_permission`, `distribution_allowed: false`. Faltam processo, entradas e escopo de direitos da variante |
| `glossata.ico` | ICO, 150.153 bytes; SHA-256 `12f8139c5fcacad6451aad1185bdb62ca8c06e5acf90679a44efdf8bb4d93c06` | Sete quadros: 16, 24, 32, 48, 64, 128 e 256 pixels. Todos são pixel a pixel iguais ao PNG principal redimensionado em memória por Lanczos no Pillow 12.2.0 | `pending_permission`, `distribution_allowed: false`. Equivalência técnica não cria direitos nem comprova a ferramenta histórica |

Os hashes RGBA dos PNGs e de cada quadro estão no JSON. A comparação é exata, sem limiar percentual; não foi salva uma nova imagem. Mais de uma ferramenta pode implementar o mesmo algoritmo. Os motivos biológicos são ilustrações de identidade, não fotografias validadas ou identificações científicas de espécies.

## Referência aos Vagalumes: comparação qualitativa

Consultada a [publicação oficial da Naughty Dog de 28 de setembro de 2026](https://www.naughtydog.com/blog/celebrating-the-last-of-us-day-2026), seção **Steam Avatars**. A imagem vinculada apresenta o emblema dos Vagalumes ao centro. Foi inspecionada como referência comparativa, **não identificada como o arquivo histórico utilizado pelo mantenedor**. URL e hash estão no JSON. O arquivo externo ficou somente em cache ignorado de inspeção em `build/`; não integra Git, relatório ilustrado, QRC ou pacote. A publicação oficial não concede autorização de incorporação ao Glossata.

| Critério | Referência oficial observada | Glossata atual | Interpretação limitada |
| --- | --- | --- | --- |
| Silhueta | Inseto bilateral, asas laterais e prolongamento central descendente | Mariposa bilateral, asas abertas e região inferior alongada | Pode evocar a referência declarada; asas, simetria e insetos também são motivos naturais genéricos. Não se atribui exclusividade sobre eles |
| Geometria | Segmentos retos, pontas, formas angulares e vazios que definem o corpo | Curvas orgânicas, cabeça circular, antenas plumosas e contornos de folhas/asas | Não foi demonstrada identidade dos contornos ou reprodução exata; leitura visual não substitui exame de fontes e arquivos de criação |
| Organização | Corpo central e grupos laterais de formas geométricas, com poucos elementos no emblema | Eixo central, quatro regiões laterais e painel inferior com cenas biológicas | Distribuição bilateral compartilhada, preenchimento e organização fina diferentes |
| Composição geral | Símbolo gráfico de alto contraste, apresentado aqui em peça promocional ornamentada | Contorno dourado e ilustrações coloridas de animais, vegetação, fungos e microrganismos | Cor e detalhes diferenciam a aparência; não comprovam, sozinhos, independência ou autorização de uma eventual estrutura incorporada |
| Elementos reconhecíveis | Emblema identificado oficialmente com a franquia | Não aparecem o nome da franquia, sua tipografia ou reprodução identificada do emblema completo | A ausência não comprova inexistência de elemento protegido ou de associação; a entrada histórica é desconhecida |

Não se usou porcentagem arbitrária ou reconhecimento automatizado como prova jurídica. A comparação exata ICO/PNG é outra questão. A outra mariposa presente na peça oficial não foi tratada como referência histórica do Glossata: o mantenedor não declarou isso.

**Incerteza relevante:** a referência foi admitida, mas não existem, nas evidências consultadas, arquivo de entrada, prompt ou sequência de alterações para delimitar o que foi incorporado. A análise não demonstrou reprodução literal do emblema nem permite afirmar que toda semelhança se resume a formas genéricas. A origem declarada e a estrutura geral compartilhada justificam revisão humana, sem concluir infração ou ausência de risco. Não se realizou busca de marcas ou análise jurídica territorial.

## Recomendação e proposta de identidade independente

Recomenda-se **planejar uma identidade independente antes de consolidar o uso público permanente ou ampliar sua redistribuição**, reduzindo a dependência documental da referência de franquia. É recomendação de gestão de incerteza, não determinação jurídica de substituição. Neste PR, preservam-se os arquivos para manter o trabalho e o histórico; isso não oferece nova autorização para distribuí-los.

Brief textual, **sem imagem gerada e sujeito a autorização futura**:

1. Desenvolver esboços e composição próprios sobre observação da biodiversidade. Uma mariposa pode continuar sendo avaliada como motivo natural, sem presumir exclusividade alheia sobre o animal.
2. Não usar emblemas de franquias ou suas reconstruções como entradas, traçados ou moldes. Explorar geometria e espaços vazios próprios; evitar apenas recolorir ou preencher a estrutura angular distintiva dos Vagalumes.
3. Registrar entradas e respectivos direitos, ferramenta/modalidade, data real, arquivos de trabalho e transformações. Identificar assistência de IA sem prometer autoria exclusiva ou exclusividade da saída.
4. Conferir silhueta e legibilidade em tamanhos de ícone, além das cores, com revisão humana identificada. Testes visuais não são certificação jurídica; nome e eventual pesquisa de marcas são verificações separadas.
5. Gerar, substituir, exportar variantes e escolher licença gráfica somente após autorização expressa e análise do escopo disponível. Preservar a rastreabilidade e os arquivos históricos sem reescrever o Git.

Alternativas são completar o dossiê e obter análise/autorizações cabíveis para manter a arte, ou autorizar substituição documentada. Creditar a franquia, isoladamente, não é permissão. Nenhum contato externo foi enviado e nenhuma alternativa foi ativada automaticamente.

## Termos do serviço, direitos e marca

Os [termos atuais da OpenAI](https://openai.com/policies/row-terms-of-use/) foram consultados em 2026-10-17, com vigência exibida de 2026-01-01. **Content**, **Third party Services** e **Our IP rights** separam entradas, saídas e materiais externos. Entre as partes e dentro dos limites legais, atribuem ao usuário eventuais direitos da OpenAI sobre a saída e exigem direitos adequados sobre entradas. Saídas podem não ser únicas; a atribuição não abrange automaticamente saídas de outros usuários ou materiais de terceiros. Nome e logotipo da própria OpenAI têm condições separadas.

Isso não autentica o criador, garante exclusividade/proteção autoral de toda imagem gerada ou concede direitos da Sony, Naughty Dog e outros titulares. Modalidade/plano e termos aceitos na geração continuam desconhecidos; contratos empresariais podem ser diferentes. A consulta atual não foi tratada como prova do contrato histórico.

Direitos autorais sobre expressão gráfica, obrigações contratuais e direitos sobre sinais distintivos exigem análises separadas. **Não foi informado** registro do Glossata no INPI ou parecer jurídico especializado. Não se afirma registro, exclusividade ou inexistência de qualquer direito. A [Apache-2.0, §6](https://www.apache.org/licenses/LICENSE-2.0) não concede marcas de modo geral; a licença do código não licencia automaticamente estes grafismos. LICENSE, NOTICE e escopo do código foram preservados.

## Política proposta e decisões humanas

A [proposta independente de política](propostas/POLITICA_IDENTIDADE_VISUAL.md) identifica projeto/criador e arquivos; separa código e identidade; propõe análise de redistribuição e usos educativos; trata falsa afiliação/endosso e direitos de terceiros. **É rascunho não ativado**, sem autorização comercial ou proibição de terceiros por inferência. Um merge deste documento não concede licença gráfica.

O mantenedor ainda deve esclarecer as entradas e condições históricas quando houver registros, escolher entre manter a arte com dossiê/análise adequados ou autorizar uma identidade independente, e decidir condições gráficas dentro dos direitos disponíveis. Não preencher desconhecidos por inferência. Quando necessário, obter avaliação qualificada sobre elementos distintivos, titularidade ou marca; não simular uma equipe externa para registrar decisões editoriais reais.

## Governança e comparação antes/depois

Reutilizado `asset_rights.json`: `origin_statement` mantém a declaração anterior; `additional_origin_evidence` vincula a nova. O auditor comum confere existência, hash e vínculo dos arquivos nessas referências opcionais, sempre separados de `permission`. Uma declaração, mesmo alegando aprovação, não satisfaz a autorização de distribuição. Registros sem esses campos mantêm compatibilidade. A mudança **acrescenta** verificação de integridade, sem relaxar requisitos.

| Controle/achado | Antes | Depois |
| --- | ---: | ---: |
| Branding com permissão pendente | 3 | 3 |
| Outros materiais com permissão pendente, incluindo branding | 7 | 7 |
| Fotos do quiz com pendência de permissão | 120 | 120 |
| Bloqueios científicos prioritários | 2 | 2 |
| Erros técnicos de governança | 0 | 0 |
| Ícones modificados/substituídos | — | 0 |
| Permissões ou revisões humanas criadas | — | 0 |

O avanço é documental: referência explicitada, análise limitada registrada, vínculo PNG/ICO demonstrado e proposta preparada. **Nenhum dos três achados foi resolvido como direito demonstrado.** `pending_permission` e `distribution_allowed: false` permanecem em cada registro. Não se apagaram eventos nem alteraram revisores. A Issue #2 continua aberta.

O manifesto completa o catálogo 2.4.0 ainda não lançado para identificar a evidência, os vínculos e o README de branding. Não muda a versão do aplicativo, dados científicos, regras, fotografias ou redação educativa. O gate de distribuição permanece ativo.

## Validação

Verificações realizadas nesta etapa:

- Compilação Release e **26/26 suítes CTest aprovadas em 131,09 segundos**.
- **38 testes de investigação de direitos** e **23 de governança** aprovados. Novas fixtures sintéticas verificam que evidência de origem válida não libera distribuição, alegações de aprovação não substituem permissão, adulteração/vínculo errado/caminho inseguro são recusados e registros legados sem referência opcional continuam compatíveis.
- Auditoria estrita: 122 organismos, 120 fotos distintas do quiz, pool competitivo 80, sem erros técnicos. Dados científicos e pedagógicos inalterados.
- Hashes dos três ícones e bytes de 17 arquivos centrais preservados, incluindo evidência anterior, educação, dicas, política, baseline, LICENSE, NOTICE, VERSION e créditos. Nenhuma foto modificada; comparação Git das mídias sem diferenças.
- `licensing_inventory.py --check-code`, manifesto/status e verificação do histórico visual aprovados. Nenhuma alteração visual exige captura nova, pois os grafismos e a interface permanecem iguais.
- `recover-permissions` em conferência: consistente, sem replay. `audit-history` contra o commit-base completo: prefixos e baseline preservados, sem novos eventos.
- `distribution.py check`: **saída 2**, bloqueio esperado; zero erros técnicos, 120 pendências fotográficas, sete de outros materiais e dois bloqueios científicos. Nenhum gate foi alterado para aprovar o pacote.

Testes usaram fixtures/bancos temporários; não se consultaram bancos pessoais. O cache externo é ignorado e não integra a entrega. A execução remota de `offline-validation` é acompanhada no PR desta etapa. Testes técnicos não certificam direitos, autoria, ausência de semelhança juridicamente relevante ou revisão científica independente. O [saneamento anterior](SANEAMENTO_ISSUE_2.md) permanece registro da etapa anterior.
