# Implementação da política de identidade visual reservada

Conferência posterior em **2026-10-18**: os termos oficiais do serviço e as conclusões individuais foram acrescentados a [IDENTIDADE_VISUAL.md](IDENTIDADE_VISUAL.md), com fonte pública e limites. O mantenedor confirmou outubro de 2026, conta pessoal e ausência de componentes externos adicionais. Após a recusa inicial da revisão automática, autorizou expressamente o registro limitado ao uso oficial. Os três ícones passam a `documented_official_use`, com `distribution_allowed: true` somente no escopo `official_project_distribution`, vinculado aos hashes preservados. Não há licença gráfica geral, exclusividade ou certificação jurídica; textos, fotografias e pacote continuam bloqueados. O gate agora exige autorização específica e evidência vinculada aos arquivos, além dos fatos e termos consultados. Os resultados atuais estão em [SANEAMENTO_ISSUE_2.md](SANEAMENTO_ISSUE_2.md).

## Registro histórico da implementação da política em 2026-10-17

O texto a seguir preserva a situação anterior à confirmação adicional de uso oficial; os estados pendentes nas tabelas históricas não descrevem o inventário atual.

Atualização de proveniência após o PR #15: o mantenedor respondeu “Apenas descrição de texto” sobre a geração da arte. O [registro complementar](../resources/compliance/branding/text-only-declaration.json) preserva essa informação sem alterar a declaração anterior, distinguindo-a de eventuais edições, preparação de variantes e conversão para ICO. Não se afirma geração textual independente dos três arquivos finais; etapas posteriores não declaradas continuam desconhecidas. As demais conclusões abaixo são o registro da implementação da política. A [situação atual da Issue #2](SANEAMENTO_ISSUE_2.md) delimita o que ainda falta para distribuição, sem exigir exclusividade, registro de marca ou publicação de conversas privadas.

## Decisão e alcance

Em 2026-10-17, o mantenedor escolheu expressamente **identidade visual reservada ao Projeto Glossata**, com intenção de uso dos três ícones nas versões oficiais. A [política adotada](propostas/USO_IDENTIDADE_VISUAL.md) substitui o estado de proposta não escolhida; o caminho anterior foi mantido para preservar referências. A reserva limita-se aos direitos efetivamente disponíveis, sem afirmar autoria exclusiva de resultados de IA, registro de marca ou ausência de direitos de terceiros.

Não foi concedida licença gráfica geral de reutilização, redistribuição ou sublicenciamento por projetos independentes. Isso não acrescenta restrições ao código próprio Apache-2.0 nem proíbe visualização, fork no GitHub ou usos legalmente permitidos. Recomenda-se identidade própria para derivados independentes, sem obrigação de exibir permanentemente nome ou logotipo do Glossata na interface. LICENSE, NOTICE e enumeração do código permanecem intactos.

O PR #14 foi confirmado como integrado em `13fda255ac64387c9d3b90b92782d49bfe01b8a2`, base desta branch `docs/politica-branding-reservado`. A árvore estava limpa. A atualização da `main` local foi somente avanço direto; a implementação está na nova branch.

## Documentos e governança

Foram atualizados `docs/propostas/USO_IDENTIDADE_VISUAL.md`, `docs/IDENTIDADE_VISUAL.md` e seu JSON, `docs/DIREITOS_E_LICENCAS.md`, README principal e README de branding. A decisão inicial de adoção foi preservada no JSON, acrescentando-se `policy_decision` para a escolha posterior da política. Os resultados anteriores do PR #14 permanecem identificados como históricos, sem reescrita retrospectiva.

O inventário `asset_rights.json` vincula cada ícone à política e ao registro administrativo, com hash do registro. Esses metadados são informativos: não preenchem `permission.holder` ou `permission.evidence_path` e não têm efeito autorizador no validador existente. A declaração original `origin-declaration.json`, os hashes dos arquivos gráficos e os ledgers foram preservados. Não foram criados eventos de autorização, revisores, assinaturas ou avaliações especializadas. A política de conteúdo e as licenças de textos/coletânea permanecem inalteradas.

## Situação individual dos três recursos

| Recurso | SHA-256 conferido | Evidência existente e uso pretendido | Evidência ainda necessária | Estado final |
| --- | --- | --- | --- | --- |
| `glossata.png` | `6c83e4e175be354ce8e889b6548121ded4e9a1ae9685d2d44327c0a9f63bde4b` | Pacote fornecido pelo mantenedor; declaração de criação via ChatGPT; direção criativa e política administrativas. Cabeçalho e identidade oficial | Conferir registros de geração e condições históricas; identificar entradas/componentes externos e direitos aplicáveis; demonstrar escopo legítimo de distribuição destes bytes | `pending_permission`; distribuição `false` |
| `glossata_discord.png` | `e0c34c105919bfc2e49fb02804ec52794241958793f0926b14939992282a2549` | Mesma declaração de origem vinculada especificamente a este hash; variante oficial para Rich Presence | Conferir origem da variante e direitos sobre seus componentes/transformações; demonstrar escopo aplicável ao asset e à redistribuição offline | `pending_permission`; distribuição `false` |
| `glossata.ico` | `12f8139c5fcacad6451aad1185bdb62ca8c06e5acf90679a44efdf8bb4d93c06` | Pacote fornecido pelo mantenedor e declaração de origem vinculada a este hash; uso oficial no Windows | Confirmar cadeia documental da arte incorporada e de sua conversão; demonstrar direitos para incorporação no executável e distribuição do ICO | `pending_permission`; distribuição `false` |

O titular suficiente para autorizar distribuição não está comprovado nos campos de permissão. Também não há autorização externa arquivada nesses campos. Essas lacunas não comprovam, por si só, infração. A geração declarada com ChatGPT e a escolha reservada não suprem automaticamente a análise de entradas, condições históricas e direitos disponíveis. Não se exige aprovação externa fictícia; o mantenedor pode realizar conferência editorial fundamentada, distinguindo-a de certificação jurídica ou especializada.

## Substituição técnica em projetos derivados

| Integração atual | Como um fork utiliza identidade própria |
| --- | --- |
| `resources.qrc` inclui PNG principal e ICO; `Main.qml` carrega `qrc:/resources/branding/glossata.png`; `src/app/main.cpp` define o ícone da janela a partir do ICO | Fornecer arquivos próprios nos mesmos caminhos preserva as referências e não exige mudar a lógica C++/QML |
| `CMakeLists.txt` define `GLOSSATA_ICON_FILE`; `src/app/windows/app.rc.in` incorpora o ICO nativo | Substituir o ICO no fork e recompilar; `OBJECT_DEPENDS` obriga recompilar o recurso quando ele muda. A variável não é uma opção de cache; manter o caminho evita alteração do build |
| Discord: `discord.json` é copiado no configure e pode ser lido ao lado do executável antes do fallback QRC | Usar aplicação/asset próprios, com direitos demonstrados, ou desativar a presença por `enabled: false`. Não reutilizar por padrão a configuração oficial ao publicar um projeto independente |
| `glossata_discord.png` não está no QRC | É um arquivo de preparação do asset, sem upload ou download automático pelo jogo; seu cadastro remoto é uma ação separada |
| `scripts/package.ps1` copia o ICO e usa hash no nome do ícone do atalho | Após substituição legítima no fork, o caminho do atalho acompanha os novos bytes; o gate de distribuição continua obrigatório |

Procedimento para o fork: preparar sua própria arte com direitos documentados; substituir arquivos nos caminhos existentes em seu repositório; ajustar identificação pública e configuração Discord quando aplicável; registrar proveniência/condições próprias no inventário, sem reaproveitar autorizações dos ícones originais; atualizar manifestos pelo fluxo de versionamento e recompilar. A Apache preserva seus avisos aplicáveis, sem obrigar o uso da identidade oficial na interface. Esta orientação técnica não é autorização para redistribuir recursos pendentes.

Não foi encontrado obstáculo que exija alterar substancialmente o código para substituir os ícones. Nenhuma parametrização nova, alteração de CMake, QRC, scripts de empacotamento ou aparência oficial foi necessária. O empacotador foi apenas inspecionado, sem geração de pacote nesta intervenção.

## Validação

Verificações locais de 2026-10-17:

| Verificação | Resultado |
| --- | --- |
| `python tests/test_rights_investigation.py` | 36 testes passaram; inclui novo recurso sintético cuja intenção de uso oficial e política reservada não suprem a permissão ausente, mesmo em execuções repetidas; sem escrita em registros reais |
| Comparação com a base, SHA-256 e vínculos administrativos | Três ícones byte a byte preservados; hashes de origem/decisão conferidos; somente `administrative_policy` acrescentado aos três registros do inventário |
| `licensing_inventory.py --check-code` | Aprovado; LICENSE, NOTICE e escopo Apache-2.0 idênticos à base |
| `audit_catalog.py --strict` e `review_hints.py` | Aprovados; 122 organismos, 80 elegíveis no pool competitivo e dicas preservados |
| `release.py check`, `check-history` e `project_status.py check --build build` | Aprovados; manifestos e versões consistentes |
| `recover-permissions` e `audit-history --base 13fda255ac64387c9d3b90b92782d49bfe01b8a2` | Consistentes; baseline e históricos preservados, nenhum evento criado |
| Auditorias de repositório, proteção, exploração, assistência e relatório de direitos | Executadas sem erro técnico; nenhuma decisão automática |
| `qmllint` e links locais dos documentos | Aprovados |
| Compilação Release (`cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release`; `cmake --build build --parallel 4`) | Aprovada |
| `ctest --test-dir build --output-on-failure` | 26/26 suítes aprovadas, em 178,50 segundos; inclui governança, direitos, perfis, checkpoints e backups em fixtures temporárias |
| Gerador educativo e créditos (`build_education.py`; `content_governance.py credits`) | Saídas idênticas à base; textos e créditos preservados |
| `distribution.py check` | Saída 2, distribuição bloqueada: zero erros técnicos, 120 registros fotográficos com pendências, sete outros materiais e dois bloqueios científicos prioritários |

O manifesto foi regenerado por `scripts/release.py catalog --version 2.4.0` para acompanhar apenas o inventário de recursos e o README de branding; os dados científicos não mudaram. O estado do projeto foi atualizado pelo gerador existente. Aplicativo 1.7.0 e catálogo 2.4.0 continuam não lançados. Não houve alteração visual da tela inicial ou nova captura. O pacote não foi gerado ou validado nesta intervenção; as obrigações de dependências permanecem separadas.

A reprovação do gate por pendências legítimas não equivale a regressão técnica e não é substituída pela escolha da política. Os testes não autenticam autoria nem certificam direitos ou revisão científica.

## Próximas providências

O mantenedor deve reunir evidências proporcionais à origem e ao uso pretendido de cada arquivo, conferir as condições efetivamente aplicáveis e registrar somente conclusões fundamentadas no fluxo de governança existente. Preservar privacidade dos registros de criação. Se houver dúvida relevante sobre direitos externos, manter a pendência e obter orientação apropriada antes de distribuir. Qualquer futura autorização deve delimitar recurso, bytes, responsável declarado, fundamento e escopo; não basta escolher a política ou encerrar uma Issue.

Os três achados gráficos permanecem abertos. Nenhuma fotografia, texto, classificação, dica, pontuação, dado de jogador, licença de terceiro ou evento histórico é alterado por esta decisão. Não houve tag, release, publicação de pacote, encerramento da Issue #2 ou merge automático.
