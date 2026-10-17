# Como contribuir com o Projeto Glossata

O projeto tem um mantenedor e está em desenvolvimento experimental. O código é público, mas sua licença própria **ainda não foi escolhida**. Fotos, dados, textos e dependências têm condições separadas. Este procedimento não concede licença geral, não exige cessão de direitos e não presume aceitação de uma licença futura. Consulte o [plano de licenciamento](docs/PLANO_LICENCIAMENTO.md) e os [direitos por camada](docs/DIREITOS_E_LICENCAS.md).

## Propostas e Pull Requests

Use os [formulários de Issues](https://github.com/kaduolliver/Projeto-Glossata/issues/new/choose) para erros científicos, fotografias, sugestões educativas e licenciamento. Explique o problema, o recurso/ID e a evidência pública disponível. Para mudanças maiores, alinhe o escopo com a manutenção antes de implementar.

Trabalhe em branch própria e envie PR para `main`, com commits pequenos e mensagens claras. Descreva problema, comportamento resultante, arquivos sensíveis, verificações executadas e limitações. Preserve o funcionamento offline e os IDs, perfis, histórico, pontos e conquistas. Não inclua bancos pessoais, credenciais, saídas de build ou evidências confidenciais.

Enquanto não houver licença própria definida, enviar código não significa autorizar seu uso sob qualquer licença futura. Informe autoria/origem e condições expressamente aplicáveis à contribuição. A incorporação de material protegível precisa de condições documentadas compatíveis com o uso pretendido; se faltarem, a proposta permanece em discussão/rascunho. Não copie código público de terceiros supondo que publicidade autorize reutilização.

## Código e testes

Preserve C++17/Qt 6/QML e as fronteiras existentes de domínio, aplicação, apresentação e infraestrutura. Reutilize políticas/validadores; não duplique regras de pontuação ou aprovação. Evite dependências desnecessárias e documente versões/obrigações de qualquer dependência proposta.

Registre os resultados efetivos, sem declarar compilação, execução visual ou teste que não realizou. Para alterações de código, os controles técnicos do projeto incluem:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
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
