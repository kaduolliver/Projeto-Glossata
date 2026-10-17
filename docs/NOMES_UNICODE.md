# Nomes de jogadores: política Unicode — INV-07

Correção local do desenvolvimento **1.7.0**, ainda não lançado. Complementa a [identidade permanente do INV-01](IDENTIDADE_JOGADORES.md); não muda IDs, regras dos quizzes, catálogo 2.4.0, pontuações ou decisões de curadoria.

## Diagnóstico e equivalência

O `COLLATE NOCASE` original não resolvia caixa em nomes como `João`/`JOÃO`. O INV-01 já introduziu normalização nas novas operações, mas duplicava a expressão, não registrava a política/versão Unicode do banco e não auditava bancos de esquema 4 na reabertura. O único `NOCASE` ainda usado para perfis ativos ordena a lista; não decide unicidade. Seus usos em arquivos lógicos e agrupamentos de resultados pré-ID conservam exclusivamente a interpretação histórica.

`src/domain/PlayerNamePolicy.h` centraliza **`nfc-simple-casefold-v1`**:

1. Aparar espaços Unicode externos com `QString::trimmed()`. Espaços internos, inclusive repetidos, permanecem significativos.
2. Para exibição, conservar exatamente o resultado do passo 1: caixa, acentos e sequência composta ou decomposta. O limite existente continua em 32 unidades UTF-16; não corresponde necessariamente a 32 caracteres visuais.
3. Para comparação, aplicar NFC, `QString::toCaseFolded()` e NFC novamente. Armazenar essa chave separadamente do nome exibido. A igualdade das chaves é binária, sem collation local do SQLite.

É **case folding simples**, independente de locale. Não se afirma implementar full case folding, transliteração ou detecção de caracteres visualmente confundíveis. Essa escolha reutiliza Qt Core, sem extensão ICU, tabelas Unicode externas ou nova dependência. A distinção entre folding simples e completo está documentada no arquivo oficial [CaseFolding do Unicode](https://www.unicode.org/Public/15.0.0/ucd/CaseFolding.txt); as operações usadas constam da documentação de [QString](https://doc.qt.io/qt-6/qstring.html#toCaseFolded) e [normalização](https://doc.qt.io/qt-6/qstring.html#normalized).

| Nomes | Resultado da política |
| --- | --- |
| `João`, `JOÃO`, `Joa` + U+0303 + `o` | Equivalentes. |
| `Maria`, `MARIA` | Equivalentes. |
| `Σίσυφος`, `ΣΊΣΥΦΟΣ`; `Иван`, `ИВАН` | Equivalentes dentro de cada par. Sigma final também é dobrado. |
| Maiúscula/minúscula Deseret U+10400/U+10428 | Equivalentes, inclusive fora do plano básico. |
| `Joao`, `João` | Distintos: não se removem acentos. |
| `A` latino, `А` cirílico | Distintos: sem equivalência por aparência ou alfabeto. |
| `Straße`, `STRASSE`; `İ`, `i` | Distintos nesta política simples; não se presume folding completo nem regra turca. |
| `Ana Maria`, `Ana  Maria` | Distintos: espaços internos preservados. |

Criação e renomeação usam o mesmo módulo. Uma mudança apenas de caixa ou representação canônica do próprio perfil é permitida se não houver conflito com outro perfil. Uma tentativa conflitante não altera nome nem ID. Perfis excluídos logicamente não reservam o nome; sua reutilização gera identidade nova, sem herança de resultados. O nome capturado no início de uma partida permanece intacto.

## Migração e auditoria de conflitos

O SQLite passa de **`user_version=4` para 5**. Bancos anteriores chegam ao esquema de identidade do INV-01 e à nova etapa dentro da mesma transação. A migração:

- Mantém UUIDs, grafias, exclusões, arquivos lógicos, vínculos, resultados, snapshots e datas históricas.
- Registra em `player_name_policy` o identificador do algoritmo, `QChar::currentUnicodeVersion()` e a versão Qt da coleta. O inteiro Unicode é o enum do Qt, não uma versão semântica escrita em decimal.
- Recalcula chaves de todos os registros `origin='profile'`, incluindo excluídos, a partir do nome. Não transforma os conjuntos `legacy_unresolved` nem os associa a pessoas.
- Guarda em `player_name_migration` uma linha por ID: nome original, chave anterior/nova, política, versão Unicode, data real da migração e indicador de colisão ativa naquele momento. Não é aprovação humana.
- Suspende somente os dois triggers de unicidade durante o recálculo, dentro da transação; reinstala-os antes do commit. IDs continuam protegidos. Falha faz rollback também do DDL, das chaves e dos triggers.
- Expõe `player_name_conflicts`, uma view das colisões **atuais** entre perfis ativos. O relatório da migração permanece mesmo após resolução explícita.

Não há índice único que obrigue apagar ou unir perfis antigos. Todos permanecem selecionáveis por ID. Triggers impedem novas criações, renomeações e reativações conflitantes. Para resolver uma colisão, o usuário escolhe o perfil por ID e o renomeia explicitamente; não se escolhe um “vencedor” automático. A API de compatibilidade por nome prefere uma correspondência exata única e recusa correspondências ambíguas. A interface atual seleciona por ID.

Em toda reabertura do esquema 5, o repositório confere a política/versão Unicode e todas as chaves. Chave inconsistente ou mudança da versão Unicode do Qt **bloqueia a abertura para escrita**, com diagnóstico, preservando o banco. Não renova silenciosamente a política nem aceita mistura de algoritmos: uma futura mudança de Unicode/folding exige migração explícita com nova auditoria de colisões. Atualização de Qt com a mesma versão Unicode não exige migração.

Inspeção em uma cópia local, com ferramenta SQLite somente leitura:

```sql
PRAGMA user_version;
PRAGMA foreign_key_check;
SELECT * FROM player_name_policy;
SELECT player_id, name, name_key FROM player_name_conflicts ORDER BY name_key, player_id;
SELECT player_id, old_key, new_key, active_collision, migrated_at FROM player_name_migration;
```

Não publique bancos, nomes ou relatórios pessoais. Feche o aplicativo e preserve backup antes de atualizar uma instalação. O binário anterior não abre esquema 5. Não remova triggers, edite IDs ou restaure um backup sobre partidas novas para resolver conflitos. Uma divergência sem causa demonstrada exige investigação em cópia do banco; não há reparação administrativa automática.

## Verificação e limites

Os bancos usados nesta intervenção são temporários. **Nenhum banco pessoal foi aberto ou migrado**, portanto não se afirma que existam ou não colisões na instalação do mantenedor. A análise de perfis existentes foi exercitada em fixtures pré-migração: `João` e `JOÃO` produziram duas linhas de conflito, conservaram IDs separados e o recorde permaneceu associado ao ID original. Após renomeação explícita, a view ficou vazia, conservando as duas ocorrências no relatório histórico.

`PlayerIdentityTests.cpp` cobre os pares da tabela, criação/renomeação, preservação de grafia decomposta, reinicialização, nomes reutilizados, migração v3/v4, correção de chave antiga, colisões preservadas, idempotência, rollback e recusa de política/chave incompatível. Os testes usam o driver SQLite real com `QTemporaryDir`; não acessam a Internet. Validação técnica não identifica pessoas e não permite atribuir resultados antigos por semelhança de nome.

Resultados locais em 2026-10-16 (Brasília), Qt **6.11.2**, Release C++17:

| Verificação | Resultado |
| --- | --- |
| Compilação | Aplicativo e executáveis de teste recompilados com a nova migração. |
| CTest completo | **20/20 suítes**, 82,18 s. |
| Regressão após ajustes finais | **3/3 suítes** de domínio/controlador, QML e identidade, 33,22 s. |
| Identidade/Unicode final | **25 verificações QtTest**, sem falhas, inclusive perfis excluídos pré-v5, reativação conflitante e guarda SQLite para inserção direta. |
| Domínio/controlador | **50 verificações QtTest**, sem falhas; pontuação e sessão continuam cobertas. |
| Catálogo | Auditoria estrita aprovada para os 122 organismos; nenhum insumo científico/licenciamento alterado. |
| Versionamento | `release.py check` e `check-history --base e4304c3` aprovados, sem criação de versão, tag ou release. |
| Pacote local | Preparado com `package.ps1 -Development`; teste de inicialização com PATH isolado e banco temporário aprovado. Mantém marcador de desenvolvimento e bloqueio de redistribuição. |

Não houve mudança visual, captura nova ou validação visual manual nesta correção. A suíte QML executou o programa real em renderização software com banco temporário; isso não equivale a uma inspeção visual manual de todas as telas.

O empacotador continua avisando sobre traduções Qt ausentes e componentes opcionais de Direct3D 12. O smoke test aprovado não é teste de instalação limpa nem de todas as GPUs. Nenhuma pendência de licenciamento foi aprovada ou contornada. Os comandos de empacotamento usaram `-ExecutionPolicy Bypass` somente nos processos filhos, sem alterar a política persistente do Windows.
