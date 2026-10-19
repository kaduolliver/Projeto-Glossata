# Linha de base visual anterior à reformulação

Interface de produção no commit `b201e20c2d83fda6d790bc6a96217c0fdc75307e` (`git checkout b201e20c2d83fda6d790bc6a96217c0fdc75307e` em checkout separado para consultar o código). Aplicativo 1.7.0 em desenvolvimento; este registro **não é um lançamento**. As imagens são da área de conteúdo de `QQuickWindow`, sem moldura do Windows.

As 14 imagens abaixo foram produzidas em 2026-10-19 pelo teste existente `QmlInterfaceTests.navigateAndConfirmAnswers`: `QQmlApplicationEngine` carregou o `Main.qml` de produção, com `GameController` real e SQLite em `QTemporaryDir`, e `QQuickWindow::grabWindow` gravou cada PNG. O teste percorreu telas/estados reais, inclusive partida e resultados de teste; o perfil e o ranking capturados são temporários. Discord não foi iniciado. Backend Qt offscreen/software, escala 1, 96 DPI, Qt Quick Controls Basic. O [manifesto](capturas.json) inclui SHA-256, dimensão 1240 × 820, horário UTC por arquivo, nome de saída do teste e hash do executável de teste.

O teste inicia e interrompe sessões em sequência. Por isso, algumas capturas posteriores, como o painel de dicas, exibem um aviso de sessão recuperável; ele é parte do estado real alcançado pelo fluxo de teste, não um estado inicial da tela.

| Estado capturado | Imagem |
| --- | --- |
| Início | [inicio.png](capturas/inicio.png) |
| Exploração e filtros | [explorar.png](capturas/explorar.png) |
| Navegação do domínio Eukaryota | [dominio-eukaryota.png](capturas/dominio-eukaryota.png) |
| Atalho Animalia | [grupo-animalia.png](capturas/grupo-animalia.png) |
| Quiz de quatro alternativas | [quiz.png](capturas/quiz.png) |
| Painel de dicas | [painel-dicas.png](capturas/painel-dicas.png) |
| Resposta correta | [resposta-correta.png](capturas/resposta-correta.png) |
| Resposta incorreta | [resposta-incorreta.png](capturas/resposta-incorreta.png) |
| Ficha de espécie | [ficha-especie.png](capturas/ficha-especie.png) |
| Resultado de partida de teste | [resultado.png](capturas/resultado.png) |
| Estatísticas e ranking de teste | [estatisticas.png](capturas/estatisticas.png) |
| Conquistas | [conquistas.png](capturas/conquistas.png) |
| Perfis e ajustes | [perfis-ajustes.png](capturas/perfis-ajustes.png) |
| Créditos e transparência | [creditos.png](capturas/creditos.png) |

## Direitos e atribuição das imagens reproduzidas

As capturas reproduzem fotografias do quiz e dos cartões de exploração, além da identidade visual do aplicativo. Consulte os [créditos fotográficos completos](../../../PHOTO_CREDITS.md) para autoria informada pela fonte, arquivo específico, licença, alterações e pendências de cada fotografia; a [política de direitos](../../../DIREITOS_E_LICENCAS.md) trata separadamente os ícones e os demais materiais. A imagem estática não preserva os links nem as dicas interativas de crédito dos cartões. Por isso, o documento de créditos deve acompanhar o acesso público a estas capturas.

Na [captura de exploração](capturas/explorar.png), os cartões visíveis incluem *Danaus plexippus* (kcthetc1, [arquivo no Commons](https://commons.wikimedia.org/wiki/File:Danaus_plexippus_100029190.jpg), [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/)), *Sulfolobus* sp. A20 (Xin Dai e colaboradores; autores completos nos créditos, [arquivo no Commons](https://commons.wikimedia.org/wiki/File:Fmicb-07-01902-g001-v1.jpg), [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)) e *Escherichia coli* (NIAID, [arquivo no Commons](https://commons.wikimedia.org/wiki/File:E._coli_bacteria_(cropped).jpg), [CC BY 2.0](https://creativecommons.org/licenses/by/2.0/)). Nos cartões, as imagens são ajustadas proporcionalmente, recortadas e suavizadas por gradiente. A [captura do quiz](capturas/quiz.png) mostra a foto [322926715 de *Danaus plexippus*](https://www.inaturalist.org/photos/322926715), com crédito declarado a **Leila Dasher** e licença declarada **CC BY de versão ainda não comprovada**. Os créditos completos e as lacunas constam no documento citado; não se infere CC BY 4.0 da página de créditos gerada quando a evidência da versão falta. Estes exemplos não são um inventário exaustivo dos elementos das 14 capturas.

Atribuir uma fotografia não resolve, por si só, a comprovação da licença específica e das condições de redistribuição. Estas capturas **não confirmam autorização para redistribuir todos os elementos nelas presentes**, nem concedem licença coletiva às fotografias ou aos ícones. As pendências de versão/permissionamento das fotos do quiz e os direitos próprios do branding permanecem sob os registros de governança existentes. Nenhum autor, iNaturalist ou Wikimedia Commons é apresentado como apoiador do projeto. Antes de reutilizar uma captura fora deste contexto, identifique cada elemento nela visível, confira seu registro individual e mantenha as atribuições e avisos de alterações aplicáveis. A [orientação da Creative Commons sobre atribuição](https://wiki.creativecommons.org/index.php?title=Recommended_practices_for_attribution) recomenda crédito adequado ao meio, com fonte, licença e alterações quando fornecidos.

Para reproduzir: configure e compile `glossata_ui_tests`, defina as variáveis do manifesto, execute `build/tests/glossata_ui_tests.exe navigateAndConfirmAnswers` **a partir de `build/tests`** e confira `build/tests/previews/`. O teste exercita fluxos completos; alguns estados dependem dos dados de catálogo do commit. A captura oficial da tela inicial no README principal permanece em `docs/images/tela-inicial.png` e tem registro próprio. Estas imagens não são conceitos gerados e não substituem aquela captura.

Limitações: o renderizador offscreen não representa molduras do sistema operacional, fontes/antialiasing de toda GPU ou comportamento de leitor de tela. Foram arquivadas imagens em 1240 × 820; o teste também gera estados compactos e largura maior em `build/tests/previews/`, mas estes não integram o conjunto fixo.
