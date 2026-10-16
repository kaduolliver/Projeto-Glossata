# Limpeza de fotografias legadas do repositório

## Escopo e motivo

Em 2026-10-16, foram removidas da árvore atual 37 fotografias antigas do iNaturalist (4.896.566 bytes), sem uso no catálogo ativo, no QRC ou no inventário de permissões. A auditoria as classificava como `not_bundled_permission_not_demonstrated`; isso registra ausência de comprovação, sem afirmar infração de direitos.

A remoção foi solicitada pelo mantenedor após a revisão do repositório. A lista abaixo preserva caminho, hash dos bytes removidos e referências existentes. O `.gitignore` impede a reinclusão acidental desses mesmos arquivos; novas propostas devem passar pelo fluxo de candidatos e evidências antes da incorporação.

## Preservação

- `catalog_replacements.json`, documentos de substituições e evidências continuam preservados; suas referências antigas são históricas e não exigem o binário antigo para migrar o SQLite.
- Os 122 organismos, as 120 fotografias ativas do quiz, as 109 imagens de apresentação, os créditos e os hashes do catálogo não mudam. Nenhum banco ou perfil de jogador foi alterado.
- Aplicativo 1.7.0 e catálogo 2.4.0 permanecem em preparação. Trata-se de limpeza de arquivos fora dos recursos publicados, sem nova versão ou lançamento.
- As pendências de direitos das 120 fotos ativas e dos sete outros materiais continuam abertas. Esta limpeza não concede permissão nem aprova revisão científica.
- O histórico Git não foi reescrito: os bytes antigos ainda podem ser consultados em commits anteriores. O registro de limpeza não é uma declaração de eliminação de todas as cópias públicas.

## Inventário removido

Baseline inspecionada: `7bf8fe81d83bb52ed2112fdcddcfc9a719a87f6f`.

| Arquivo em `resources/images/inaturalist/` | SHA-256 | Referências históricas preservadas |
| --- | --- | --- |
| `111492748.jpeg` | `9d6cfb8750c8a13f87b121ab25bcb3375830590450350f3180c10475caeaf4bd` | `resources/data/catalog_replacements.json` |
| `120009929.jpeg` | `9d8d592c3e7d2376ee1e84a38d4efc6dfc6387646faff905cb610ca5b29dbc1a` | Nenhuma encontrada na baseline |
| `137831916.jpeg` | `a55e59f45864733cf00362c2435a786b589f535500d7c7d4145c5fd9e7d9afcd` | Nenhuma encontrada na baseline |
| `144194751.jpg` | `806d108e60d39c633dbfb4ffe1a7c10d3000eb775cbe0e9f5ce6bea85338d1e1` | `resources/data/catalog_replacements.json` |
| `156970348.jpg` | `b0b1bb60d34da054c462a06bc99b86821cd16b8879f543742d2f39f6ecd944bc` | `resources/data/catalog_replacements.json` |
| `157125794.jpg` | `3c4f2635d936bfab53602436d88cc4b373bcc1c1f421ac1cc8598a7cf5305db0` | `resources/data/catalog_replacements.json` |
| `178409678.jpeg` | `b054166781eba51345c722d8bb5f28995bd994855f942f7c38dc2b5f0c95a062` | Nenhuma encontrada na baseline |
| `211654606.jpeg` | `ef446bd1b274e9c481fe5f7f48160851677bf1eaee99df67d23ebc26e7924df6` | `resources/data/catalog_replacements.json` |
| `234592351.jpeg` | `8bf0e020df8fcfd308299fa961b012b70aca8850d241593ba0e0e6a952bf0c3a` | `resources/data/catalog_replacements.json` |
| `243782262.jpeg` | `9f43271f0a316e1fda3e8726cbb6653e377898bad50156a9c010b334c6f4bc61` | Nenhuma encontrada na baseline |
| `291136956.jpg` | `dea904ed8ff56051f9c297e84d28fec2629ef50ffc459f7cb7cf1a45f479a361` | `resources/data/catalog_replacements.json` |
| `31018368.jpg` | `72b27e0842bcf52c121bc984f610b6203d587a62cb08a25b9de8601b4c80e504` | Nenhuma encontrada na baseline |
| `320712583.jpeg` | `1e5663f7ffcf1fab79951d542be9faedb2387785764353cf5edbf76e5a8ca43a` | `resources/data/catalog_replacements.json` |
| `321855344.jpg` | `53043652fab15d79c9cb2ab5df1eeb527b3a7e9a8a2bbbfe7549b668512c7c5f` | `resources/data/catalog_replacements.json` |
| `322085831.jpg` | `0b6bd4e27529028435c2f193f375c72dbbaaabb919fa1b6f68998e1792bfb072` | `resources/data/catalog_replacements.json` |
| `337864904.jpg` | `e34817d0339bbc66feafaf02cb9cb2c3314c1640fb7e67e53246677e107cf5b2` | Nenhuma encontrada na baseline |
| `344593119.jpeg` | `fdd27606776ba824f4b4b02d4f7d74b5cc5ac80f84b1a9e112d57c555d63385f` | `resources/data/catalog_replacements.json` |
| `356956710.jpg` | `4861fe304f9839a7b5f41f8cd0076bbd9959a17a326618c8dcb5a70747ee3d21` | `resources/data/catalog_replacements.json` |
| `357886873.jpg` | `00312bf6a531fac4275e99493d06691eaa72324935c2617cd464a5ee2b1b38a5` | `resources/data/catalog_replacements.json` |
| `368743189.jpg` | `b4e9517603a6ec04653561ea4724c805a5b2869eecdffab9399419c3fddb4430` | Nenhuma encontrada na baseline |
| `4016368.jpg` | `344b8754e5bd123d5227b74f1f58dc5587f17071572767a915a7d938919d5026` | Nenhuma encontrada na baseline |
| `448498951.jpeg` | `25143ae24eaa61dc89fbeb0a8504ac23e84d382415e7d8b42c923397f96f9537` | Nenhuma encontrada na baseline |
| `465907310.jpg` | `ecd57304e6d8e60a8845bfde61850c0a070d1fbbf775c1d9b3e1319b43fb3934` | `resources/data/catalog_replacements.json` |
| `469510982.jpg` | `a57a8e4928f9c1ed4da435fe656c639983dec1ff05be3123946ad49d4908ccc1` | Nenhuma encontrada na baseline |
| `471069278.png` | `75065cf007adb31256cac32554380d23f691e79f40592c548894cfcd6f2cba46` | `resources/data/catalog_replacements.json` |
| `51317278.jpeg` | `a70a58f01b7e0c2183e9896bea78b8063d492f17ac5a57e0d201e1e0f7585e2e` | `docs/REFINAMENTO_IDENTIFICACAO.md`, `resources/data/catalog_replacements.json` |
| `513524942.jpg` | `7ad82fcc8cd4ff6472f1e70540d2451f1834f7b750aef2849f7aff95388cfbf9` | Nenhuma encontrada na baseline |
| `513628641.jpg` | `e862ceb68814a787cf54a8b3934311202368337834e84dbb2ff01acb48e2b150` | `resources/data/catalog_replacements.json` |
| `523634183.jpg` | `6170305c8642f8b5a73bc95df4ba7bc45edfc3678ae5a85a5f87d147af60b080` | Nenhuma encontrada na baseline |
| `5777965.jpeg` | `7ec683e50cfb5ae0321c10aced492a53a2b883f032c94f4dfed8ddb62837c97a` | `docs/REFINAMENTO_IDENTIFICACAO.md`, `resources/data/catalog_replacements.json` |
| `583650058.jpg` | `3ad1de4f6cc066cbd74decc32ffb659e0e98ceaf818ad3380bb2b1a7f0d3baa6` | `resources/data/catalog_replacements.json` |
| `588568892.jpg` | `1202804b7de53237b8ffd71250d0b2468162ac83b9fc59e7a573dafc128be75a` | `resources/data/catalog_replacements.json` |
| `588721568.jpg` | `b7f7c55ba662251c4f407a785a506476ac3cf806c39d87c94bbc949109713262` | `resources/data/catalog_replacements.json` |
| `59270879.jpeg` | `168bee54c0adfd7561918efd3562e77e3349fa8c1b4b1315fabcbe75a9686fd9` | `resources/data/catalog_replacements.json` |
| `594387576.jpg` | `52096aef384b01ffb3298285bc7b77cf70be4cd4adddf600b4bb4a6f88d8bfc0` | `resources/data/catalog_replacements.json` |
| `654470799.jpg` | `4f88fe22efaa9b62671aa78574678adc668aed5f27019abae5da766ae4f5f023` | `docs/REFINAMENTO_IDENTIFICACAO.md` |
| `70152459.jpg` | `5cb7e5df1930835378700255b982170352253c246f09db0b0bdc4935551ec080` | `resources/data/catalog_replacements.json` |

## Verificação

Antes da remoção, foram conferidos os 37 hashes e a ausência em recursos ativos, inventário e entradas do manifesto. Depois, a comparação das entradas do catálogo confirmou igualdade byte a byte; a auditoria de governança preservou os diagnósticos ativos e reduziu apenas os arquivos fora do bundle. As demais verificações executadas estão registradas no commit e na entrega.

Verificações executadas nesta limpeza:

- `ctest --test-dir build --output-on-failure`: 15/15 suítes passaram com o build existente; não houve recompilação, pois nenhum recurso embarcado ou fonte mudou.
- `python scripts/audit_catalog.py --strict`: passou, com 122 organismos preservados.
- `python scripts/release.py check`: metadados e hashes íntegros.
- `python scripts/exploration_media.py`: zero erros; 109 imagens de apresentação preservadas.
- `python scripts/audit_repository.py --staged`: zero achados nos blobs preparados.
- `git diff --cached --check`: sem erros de espaçamento.
- `python scripts/distribution.py check`: bloqueado (código 2), conforme as pendências existentes; nenhuma tag, release ou liberação de pacote foi produzida.
