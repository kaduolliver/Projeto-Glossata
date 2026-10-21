# Versionamento, captura e lançamento

## Fontes de verdade

- `VERSION.txt`: versão semântica do aplicativo, lida pelo CMake, `QCoreApplication` e metadados do `.exe`. PATCH corrige comportamento compatível; MINOR adiciona funcionalidades compatíveis; MAJOR altera contratos de forma incompatível.
- `resources/data/catalog_manifest.json`: versão independente e hashes do catálogo, revisões, evidências, políticas, mídias ativas e branding. Arquivos históricos locais fora do catálogo embarcado não entram nessa identidade; continuam na auditoria separada de direitos/distribuição. `scripts/release.py catalog --version X.Y.Z` atualiza explicitamente o snapshot, sem alterar datas ou aprovar registros. Pode-se completar a mesma versão **ainda não lançada**; um snapshot já lançado deve receber uma nova versão.
- `CHANGELOG.md` e `docs/CATALOGO_CHANGELOG.md`: mudanças do aplicativo e de dados/licenças, respectivamente, com pendências de curadoria explícitas. Um incremento de versão não é autorização de distribuição.
- `build/build-manifest.json`: versões do aplicativo e catálogo, hash do catálogo, fontes e hash do executável. O pacote inclui cópia em `compliance/` junto aos créditos, evidências e avisos. A tag anotada também vincula commit e hash do catálogo.
- `docs/PROJECT_STATUS.json` e blocos gerados no README/PROJECT_CONTEXT: projeções verificáveis das fontes, não novas autoridades. `project_status.py check --build build` confere a documentação, versão configurada e inventário real do CTest, sem executar testes ou aprovar distribuição. Consulte [ESTADO_DO_PROJETO.md](ESTADO_DO_PROJETO.md).

O diretório interno `glossata_quiz` e os bancos dos jogadores permanecem estáveis. Os testes, o smoke test e a captura utilizam bancos temporários. Nenhum dos scripts de lançamento faz push nem cria uma release no GitHub.

## Captura real da tela inicial

Após compilar, execute na raiz, com Qt no PATH:

```powershell
python scripts/capture_home.py
python scripts/release.py check
```

Quando a alteração visual estiver na seção inferior da Home, use `python scripts/capture_home.py --binary build/2.2.1-B-verified/projeto_glossata.exe --section discovery`. O próprio executável faz a rolagem no contêiner real e captura o quadro; o manifesto declara o enquadramento. Sem `--section`, o comportamento anterior de capturar o topo é preservado. A opção existe somente no modo isolado de captura, sem mudar a abertura normal ou os dados dos jogadores. Os quadros de testes deste refinamento têm registros próprios; a imagem oficial usa o executável de produção pelo script.

O script executa **o programa compilado**, com a interface e controlador de produção, perfil temporário e Discord desativado. Usa `QQuickWindow::grabWindow`, backend Qt software/offscreen, e grava `docs/images/tela-inicial.png`. Não redesenha, simula ou gera uma interface por IA. A ilustração decorativa que aparece dentro da janela é um recurso real da interface existente.

`docs/images/tela-inicial.json` registra data real da captura, dimensões, versão e hash do binário, hash da imagem e fontes da tela. Esse registro demonstra rastreabilidade técnica, não direitos sobre a arte. A imagem é da área de conteúdo da janela; o backend offscreen não desenha a moldura do Windows.

**Sempre inclua o PNG atualizado e seu registro no mesmo commit de uma alteração visual relevante da tela inicial**, preservando o caminho. O controle é conservador para `Main.qml`, `HomeView.qml`, componentes compartilhados e branding. Mudanças sem efeito visual que acionem esse controle devem ser reorganizadas ou justificadas por uma revisão humana; não altere pixels artificialmente para satisfazer o Git. Se não for possível executar/capturar, registre a pendência e não use mockup como substituto.

CI confere a correspondência dos hashes e, em pushes/PRs, os commits do intervalo enviado. Não compara pixels entre sistemas operacionais/fontes diferentes. Alterações visuais produzidas indiretamente por dados ou C++ também precisam de captura conforme a regra editorial, mesmo quando não reconhecidas pelo detector de caminhos.

## Sequência de trabalho

Integração de desenvolvimento e lançamento têm requisitos distintos: a proposta de proteção exige `offline-validation`, com PR e zero aprovações externas para manutenção individual. `distribution-permissions` permanece obrigatório para liberar distribuição, sem ser exigido para integrar correções das próprias pendências. A proteção remota depende de ativação autorizada; consulte [PROTECAO_REPOSITORIO.md](PROTECAO_REPOSITORIO.md).

1. Faça mudanças em lotes coerentes e verifique cada área afetada antes de fazer commit. Separe conteúdo, importação, interface, distribuição e documentação quando possível. Não inclua bancos pessoais, saídas de build ou credenciais.
2. Escolha o incremento SemVer adequado; ajuste `VERSION.txt`, históricos e a versão independente do catálogo quando aplicável. Atualize os hashes com `python scripts/release.py catalog --version X.Y.Z`. Não modifique aprovações históricas para fazer checks passarem.
3. Quando houver mudança de conteúdo, gere educação/créditos pelos mecanismos existentes. Configure CMake e atualize os blocos derivados com `python scripts/project_status.py write --build build` após alterações legítimas de versões/regras/testes; confira com `check --build build`. Compile e rode CTest, lint QML, auditoria técnica e `python scripts/release.py check`. Após mudanças na tela inicial, gere a captura real antes do commit.
4. Revise as pendências científicas e de licenciamento. `python scripts/distribution.py check` e o gate completo do pacote precisam passar. O pacote `-Development` não é evidência de liberação pública.
5. Quando houver intenção e autorização para um lançamento, complete as entradas com a data real `AAAA-MM-DD`, marque o catálogo como `released`, faça commit e deixe a árvore limpa. O catálogo pode manter a versão de um lançamento anterior se seus bytes/revisões forem idênticos.
6. Com Qt/CMake/Python no PATH, execute `python scripts/release.py tag`. O comando verifica metadados, versões anteriores, árvore limpa, permissões do repositório, build, todas as suítes, lint, auditoria, empacotamento, smoke test isolado e permissões do pacote. Só então cria **uma tag local anotada `vX.Y.Z`**. Falha em qualquer requisito impede sua criação. Tags existentes nunca são substituídas.
7. Confira o commit e o manifesto da tag. Somente com autorização específica faça push da branch/tag e publique uma release. Descreva as versões do aplicativo e do catálogo, mudanças de licenças, verificações e pendências aceitas conforme sua gravidade. Não publique um pacote de desenvolvimento.

Se um lançamento posterior mudar a tela inicial, repita a captura no commit visual. Os estados atuais declarados vêm dos blocos gerados e não comprovam publicação. A correção INV-10 não criou tag/release nem liberou distribuição. A preparação de tag também exige `project_status.py check --build build`, além dos gates existentes.

## Registros de etapas anteriores

Os parágrafos abaixo preservam associações e decisões das etapas documentadas, sem afirmar lançamento ou substituir as fontes atuais. O catálogo 2.0.0 mudou para MAJOR pela organização/esquema filogenético; 2.1.0 acrescentou mídias de exploração compatíveis (MINOR), preservando backbone de origem, identidades e dados históricos; consulte [CLASSIFICACAO_FILOGENETICA.md](CLASSIFICACAO_FILOGENETICA.md).

As ferramentas de múltiplos provedores do aplicativo 1.4.0 reutilizam o catálogo 2.1.0 sem modificar seus bytes, revisões ou hash. Candidatos em build/scientific-sources não são conteúdo publicado; um aumento do banco de candidatos não incrementa automaticamente a versão do catálogo nem concede aprovação.

As ferramentas de curadoria automatizada do aplicativo **1.7.0**, ainda não lançado, reutilizam o catálogo **2.4.0** sem modificar dados/revisões. Filas, Issues e estados do Project não incrementam a versão do catálogo nem aprovam recursos. Uma decisão humana que altere conteúdo publicado deve atualizar o catálogo pelo mesmo fluxo acima. A entrega mantém os bloqueios de distribuição; não cria tag, push ou release. Consulte [CURADORIA_AUTOMATIZADA.md](CURADORIA_AUTOMATIZADA.md).

O aplicativo 1.5.0 acompanha o catálogo 2.2.0: ampliação compatível de fotos, evidências e auditoria de cobertura. A versão dos dados do quiz e seus organismos não é reescrita por uma fotografia de apresentação. As mudanças visuais desta etapa afetam a exploração; a tela inicial é novamente capturada pelo programa real, preservando o caminho do README.
