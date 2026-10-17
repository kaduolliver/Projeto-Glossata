# Identidade visual do Projeto Glossata

## Decisão e representação

Em 2026-10-17, o mantenedor decidiu preservar os três ícones atuais como identidade visual oficial do Projeto Glossata. A composição representa uma mariposa estilizada e a biodiversidade, sem identificar uma espécie do catálogo. Essa decisão administrativa trata da apresentação da iniciativa, sem constituir aprovação jurídica ou científica.

Carlos Eduardo Lima de Oliveira é o criador original do projeto, conforme identificação expressamente fornecida pelo mantenedor. Sua participação na direção criativa e o uso declarado do ChatGPT como ferramenta de geração são registrados sem presumir autoria exclusiva dos elementos gráficos. Não se afirma exclusividade, registro de marca ou inexistência de direitos de terceiros.

## Arquivos preservados e finalidades

| Arquivo | Finalidade | SHA-256 dos bytes preservados |
| --- | --- | --- |
| `resources/branding/glossata.png` | Identificação visual da aplicação; imagem utilizada na configuração do aplicativo no Developer Portal | `6c83e4e175be354ce8e889b6548121ded4e9a1ae9685d2d44327c0a9f63bde4b` |
| `resources/branding/glossata_discord.png` | Variante de apresentação para Rich Presence no Discord | `e0c34c105919bfc2e49fb02804ec52794241958793f0926b14939992282a2549` |
| `resources/branding/glossata.ico` | Ícone Windows do executável, launcher e janela, conforme a configuração existente | `12f8139c5fcacad6451aad1185bdb62ca8c06e5acf90679a44efdf8bb4d93c06` |

Os PNGs têm 1254 × 1254 pixels. O ICO contém tamanhos de 16, 24, 32, 48, 64, 128 e 256 pixels. Imagens, cores, transparência, geometria e proporções permanecem intactas. Esta documentação não altera a configuração remota do Discord.

## Proveniência e limites

O [registro original de origem](../resources/compliance/branding/origin-declaration.json) preserva a declaração do mantenedor: “Criei através do ChatGPT”. Os arquivos foram fornecidos no pacote `glossata_icon_pack`, conforme documentação existente. O [registro administrativo desta decisão](IDENTIDADE_VISUAL.json) vincula a adoção aos mesmos hashes, sem substituir a evidência anterior ou acrescentar eventos de permissão.

Os registros disponíveis não permitem reconstruir integralmente o processo de geração, as entradas utilizadas e as condições aplicáveis à época. Não se declara ausência de referências externas. A declaração da ferramenta não demonstra, isoladamente, titularidade suficiente ou autorização de terceiros. Documentos privados de criação não são publicados por esta intervenção; eventual conferência deve preservar a privacidade e registrar somente evidência pública necessária.

## Direitos e distribuição

O código próprio enumerado permanece sob Apache-2.0; essa concessão não licencia os ícones, a marca, os textos educativos ou os materiais externos. Consulte [escopo do código](ESCOPO_LICENCIAMENTO.md) e [direitos por camada](DIREITOS_E_LICENCAS.md).

Os três ícones continuam como `pending_permission`, com `distribution_allowed: false` no inventário existente. Sua manutenção no repositório e adoção oficial não resolvem a pendência de direitos. A distribuição continua submetida ao gate existente, sem exceção criada nesta etapa. A [proposta de condições de uso](propostas/USO_IDENTIDADE_VISUAL.md) é um rascunho para decisão futura, sem licença gráfica ativada.

Para regularizar: conferir os registros de criação e os termos efetivamente aplicáveis; identificar componentes e direitos de terceiros quando existentes; delimitar quais direitos podem ser concedidos; decidir expressamente condições de uso; arquivar a evidência necessária e utilizar o fluxo de governança documentado. Dúvidas permanecem pendentes, sem inventar titulares ou aprovações. Relatos podem ser encaminhados pelo [canal de correção e remoção](REPORTAR_PROBLEMA.md).

## Verificações desta intervenção

Verificações locais em 2026-10-17, comparadas à base `d2280de6a6825125d04a8fd4579357838c684096`:

| Verificação | Resultado |
| --- | --- |
| Comparação dos três ícones com a base e com os hashes registrados | Bytes idênticos |
| LICENSE, NOTICE, política, inventário de direitos, proveniência e declaração original | Preservados, sem novas permissões |
| `licensing_inventory.py --check-code` | Escopo Apache-2.0 consistente |
| `audit_catalog.py --strict` e `review_hints.py` | 122 organismos preservados; dicas válidas |
| `build_education.py` e geração de créditos | Saídas educativas e créditos sem diferenças em relação à base |
| Recuperação de permissões e `audit-history --base` | Estado consistente; histórico e baseline preservados |
| `release.py check`, `check-history` e `project_status.py check --build build` | Manifestos, versões e histórico consistentes |
| Auditorias de repositório, exploração, cobertura, assistência, manutenção e direitos | Execução concluída sem erro técnico; pendências de conteúdo preservadas |
| `qmllint` | Sem erros |
| Compilação Release C++/Qt e `ctest --test-dir build --output-on-failure` | Compilação aprovada; 26/26 suítes passaram, em 135,59 segundos |
| `distribution.py check` | Bloqueado, código de saída 2; zero erros técnicos, 120 registros fotográficos com pendências, sete materiais pendentes e dois bloqueios científicos prioritários |

O manifesto foi regenerado por `scripts/release.py catalog --version 2.4.0`: somente o hash do README de branding e o agregado mudaram. Aplicativo 1.7.0 e catálogo 2.4.0 continuam não lançados. Não houve alteração visual da aplicação nem necessidade de nova captura da tela inicial. Testes técnicos não certificam autoria, direitos ou revisão científica.
