# Como contribuir com o Projeto Glossata

O projeto tem um mantenedor e está em desenvolvimento experimental. O código próprio enumerado está sob [Apache License 2.0](LICENSE), com [escopo e exclusões por arquivo](docs/ESCOPO_LICENCIAMENTO.md). Fotos, dados, textos, branding e dependências têm condições separadas. Preservam-se o reconhecimento do criador original, Carlos Eduardo Lima de Oliveira, em [NOTICE](NOTICE), e a autoria das contribuições posteriores. Não há cessão de copyright, CLA ou revisão independente presumida.

## Propostas e Pull Requests

Use os [formulários de Issues](https://github.com/kaduolliver/Projeto-Glossata/issues/new/choose) para erros científicos, fotografias, sugestões educativas e licenciamento. Explique o problema, o recurso/ID e a evidência pública disponível. Para mudanças maiores, alinhe o escopo com a manutenção antes de implementar.

Trabalhe em branch própria e envie PR para `main`, com commits pequenos e mensagens claras. Descreva problema, comportamento resultante, arquivos sensíveis, verificações executadas e limitações. Preserve o funcionamento offline e os IDs, perfis, histórico, pontos e conquistas. Não inclua bancos pessoais, credenciais, saídas de build ou evidências confidenciais.

Contribuições de código intencionalmente submetidas para incorporação seguem os termos aplicáveis do §5 da Apache-2.0, salvo indicação expressa em contrário. Elas devem ser compatíveis com a licença e com as condições de submissão. Se propuser termos diferentes ou não tiver direitos suficientes, indique isso antes da integração: a proposta permanece em discussão/rascunho até esclarecer a compatibilidade. Não se presume assentimento retrospectivo de contribuições antigas, nem transferência de copyright. Informe autoria/origem, coautores, direitos de empregador e avisos externos; não copie código público supondo que publicidade autorize reutilização.

Novos arquivos de código precisam de inclusão explícita em `docs/ESCOPO_LICENCIAMENTO.json` ou justificativa de pendência, após conferir sua origem. Use `SPDX-License-Identifier: Apache-2.0` somente onde essa licença efetivamente abranja o código; não remova avisos anteriores nem aplique cabeçalho integral a conteúdo misto/terceiro. Preserve os avisos aplicáveis de LICENSE/NOTICE e indique modificações conforme o §4, sem impor exibição permanente na interface. Textos educativos e materiais externos não entram nessa concessão por estarem em um PR de código.

## Código e testes

Preserve C++17/Qt 6/QML e as fronteiras existentes de domínio, aplicação, apresentação e infraestrutura. Reutilize políticas/validadores; não duplique regras de pontuação ou aprovação. Evite dependências desnecessárias e documente versões/obrigações de qualquer dependência proposta.

Registre os resultados efetivos, sem declarar compilação, execução visual ou teste que não realizou. Para alterações de código, os controles técnicos do projeto incluem:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
python scripts/licensing_inventory.py --check-code
python scripts/project_status.py check --build build
python scripts/audit_catalog.py --strict
python scripts/release.py check
python scripts/audit_repository.py --staged
```

Qt/CMake/Python precisam estar no PATH, conforme o [README](README.md). Execute qmllint quando QML for afetado e os geradores/auditores pertinentes quando houver conteúdo derivado. A CI usa o check técnico **`offline-validation`**; confira seu resultado no commit final do PR. `distribution-permissions` é gate de lançamento: pode permanecer bloqueado por pendências legítimas e não deve ser desativado para integrar melhorias.

Para documentação sem alteração de comportamento, registre validação de links, consistência e checks pertinentes; não invente testes de execução. Alteração visual relevante da tela inicial exige captura **real** atualizada por `scripts/capture_home.py`, com registro, no mesmo commit e no caminho `docs/images/tela-inicial.png`. Utilize bancos temporários para testes/capturas. Consulte [VERSIONAMENTO.md](docs/VERSIONAMENTO.md).

## Conteúdo científico e educativo

Informe IDs, táxon e fontes que sustentem cada afirmação, com seção/página/registro e data real de consulta quando disponível. Diferencie ocorrência fotográfica, distribuição conhecida e afirmações da espécie; preserve incertezas e divergências. Não crie ranks, referências, etimologias ou estados de conservação para preencher lacunas.

Novos organismos entram como candidatos, pelo [fluxo de admissão](docs/PRONTIDAO_EXPANSAO_CATALOGO.md), sem copiar diretamente para o catálogo ativo. Conteúdo assistido por IA entra como rascunho. Submeta textos próprios/sínteses com origem declarada e verifique se há trechos reproduzidos. Uma citação não concede licença para copiar o texto citado.

A [política editorial de licenciamento misto](docs/propostas/LICENCIAMENTO_TEXTOS.md) adota CC BY 4.0 como padrão para redações próprias elegíveis, inclusive futuras contribuições com termos compatíveis e direitos demonstrados. Declare origem, autores, transformações e condições da sua contribuição; não se presume cessão de direitos. Textos externos, traduções e adaptações preservam os requisitos da fonte, incluindo ShareAlike, NC e ND quando aplicáveis. Fonte científica e origem da redação são registros distintos; a escolha do padrão não aprova nem licencia automaticamente o material enviado.

A revisão editorial do mantenedor é distinta de revisão por especialista. Não invente revisores, identidade humana, datas, especialidade ou aprovações; uma resposta de API, URL, teste ou fechamento de Issue não conclui uma revisão. Alterações relevantes exigem nova revisão vinculada ao hash atual. Use os mecanismos existentes para decisões e preserve eventos/evidências anteriores.

## Fotografias e materiais de terceiros

Apresente a fotografia específica, seu ID, origem, crédito fornecido, título quando existente, licença e versão comprovadas, evidência datada e modificações. Priorize CC0/CC BY conforme a política. Não generalize licença de uma observação, dataset, site ou outra foto; não substitua o fotógrafo pelo observador. Preserve os créditos também para CC0.

Não envie imagens sem licença/permissão demonstrável nem presuma que gratuidade autoriza uso. NC, SA, ND e exceções exigem análise explícita; não renomeie licenças ou preencha permissões para satisfazer um validador. Identificação/adequação visual deve ser analisada separadamente do direito de redistribuição. Não publique coordenadas sensíveis ou informações ocultadas.

## Declaração de origem no PR

Informe, de maneira factual e sem criar um contrato de cessão:

- Quais partes você produziu e quais vieram de terceiros, incluindo referências a trechos, imagens e datasets.
- Qual licença/permissão já se aplica a cada material externo, com avisos e provas compatíveis com publicação pública/offline.
- Se há direitos de empregador, coautor ou outra restrição conhecida que dependa de autorização.
- Qual assistência automatizada/IA foi utilizada e quais itens ainda não foram revisados.
- Quais decisões ou condições de incorporação continuam pendentes; não declarar consenso, consentimento ou qualificação sem evidência.

O mantenedor conferirá o escopo e poderá solicitar esclarecimentos. Não anexe contatos privados, contratos integrais ou provas confidenciais a Issues públicas; use o [procedimento de relato/remoção](docs/REPORTAR_PROBLEMA.md) para orientar uma abordagem minimizada.

Integração de desenvolvimento, escolha de licença e autorização de distribuição são etapas diferentes. Não faça force push na `main`, não remova bloqueios de conformidade e não publique pacote/tag/release sem autorização específica.
