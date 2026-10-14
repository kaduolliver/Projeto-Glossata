# Expansão para os cinco reinos

Implementação em C:\glossata_game, branch feat/cinco-reinos. A interface existente, a partida contínua, os perfis e o ranking foram preservados.

## Navegação e perguntas

A Home apresenta a composição decorativa de cinco organismos e a contagem real do catálogo. Jogar oferece Todas as espécies e cinco cartões de reinos. Cada reino abre o mesmo explorador; os próximos grupos e o caminho clicável são gerados pela taxonomia, incluindo níveis intermediários.

Em qualquer grupo, Todas as espécies de [grupo] inicia uma partida com seus descendentes. O seletor Identificar oferece somente níveis com quatro alternativas válidas: espécie, gênero, família, ordem, classe, filo/divisão ou reino científico. Não se pergunta uma ordem quando todo o grupo pertence à mesma ordem. Grupos pequenos continuam acessíveis, com o início do quiz desabilitado.

Cada pergunta tem quatro respostas distintas no mesmo nível e uma resposta correta. Na modalidade textual, as alternativas de espécie não compartilham o gênero indicado pela pista. O modo misto intercala reinos proporcionalmente ao tamanho dos grupos, embaralha internamente e não repete espécies dentro de um ciclo. A partida continua até o jogador encerrar. O resultado permite retornar ao grupo escolhido.

As fotos são locais, preservam a proporção e exibem atribuição e origem. Na ausência de foto adequada, a pergunta usa classificação ou a curiosidade original; a arte genérica recebe a legenda Ilustração decorativa.

## Catálogo importado

| Reino didático | Originais | Novas | Total | CC BY | CC0 |
|---|---:|---:|---:|---:|---:|
| Animalia | 22 | 20 | 42 | 15 | 5 |
| Plantae | 0 | 20 | 20 | 17 | 3 |
| Fungi | 0 | 20 | 20 | 16 | 4 |
| Protista | 0 | 20 | 20 | 18 | 2 |
| Monera | 0 | 20 | 20 | 10 | 10 |
| Total | 22 | 100 | 122 | 76 | 24 |

As licenças referem-se às 100 fotografias importadas; as 22 espécies originais não tinham foto. Há 77 registros aprovados na triagem para perguntas visuais. Três usam pistas textuais por limitações da fotografia. Os 20 registros de Monera usam perguntas textuais; duas imagens de hospedeiros/sintomas são omitidas da interface.

IDs, nomes, curiosidades, dicas, famílias, regiões e imagens das 22 espécies originais foram comparados com a referência do primeiro commit, em tests/fixtures/legacy_species.json. Os campos anteriores permanecem iguais.

## Modelo didático e classificação da fonte

Os cinco reinos são a organização educacional solicitada. A árvore científica conserva nomes, ranks, ancestrais, identificadores e origem do iNaturalist. O enquadramento usa raízes explícitas, conferindo ID, nome e rank kingdom:

| Reino didático | Raízes do iNaturalist |
|---|---|
| Animalia | Animalia (1) |
| Plantae | Plantae (47126) |
| Fungi | Fungi (47170) |
| Protista | Protozoa (47686), Chromista (48222) |
| Monera | Bacteria (67333), Archaea (151817) |

Uma ancestralidade ausente, incompatível ou associada a mais de um reino didático é rejeitada. A associação didática não é apresentada como uma classificação científica moderna universal. A primeira amostra de Protista contém Protozoa; a de Monera contém Bacteria. Chromista e Archaea são suportados pelo mapeamento e pelos testes, mas não possuem registros nesta amostra.

As espécies originais receberam a linhagem Animalia → Arthropoda → Insecta → Lepidoptera → família → gênero → espécie. As linhagens foram conciliadas com a fonte apenas em correspondências únicas de nome, nível e reino, preservando os IDs locais. Taxonomias parciais permanecem parciais: Dendrostilbella smaragdina não recebe família ou gênero inventados.

## Importação e revisão

O script usa a biblioteca padrão do Python e a [API v1 do iNaturalist](https://api.inaturalist.org/v1/docs/). Seleciona observações de grau research, com identificação comunitária concordante com a espécie ativa. Exclui nomes informais, duplicados, fotos ocultas ou sinalizadas, imagens pequenas e licenças desconhecidas, NC, SA ou ND.

A licença é conferida em cada fotografia, independentemente da licença da observação ou da foto padrão do táxon. Essa separação consta na [documentação de licenças do iNaturalist](https://help.inaturalist.org/en/support/solutions/articles/151000175695). Autor, licença, URL do registro, URL da imagem, ID da foto e hash SHA-256 ficam registrados. [PHOTO_CREDITS.md](PHOTO_CREDITS.md) contém os créditos por arquivo.

Os links CC BY 4.0 e CC0 1.0 seguem o [módulo de licenças do iNaturalist](https://github.com/inaturalist/inaturalist/blob/main/app/models/shared/license_module.rb). A legenda da licença na interface abre o respectivo documento.

Os arquivos medium foram baixados sem recorte ou retoque. A triagem substituiu imagens de ambiente por imagens do organismo quando disponíveis. A revisão fica vinculada ao ID da fotografia em resources/data/photo_reviews.json: trocar uma fotografia ou importar outra não habilita automaticamente perguntas visuais. O grau research e a triagem visual não equivalem a validação taxonômica independente.

```powershell
python scripts/import_inaturalist.py --per-kingdom 20 --max-pages 5
python scripts/import_inaturalist.py --per-kingdom 40 --max-pages 5
python scripts/import_inaturalist.py --offline --per-kingdom 20
```

per-kingdom é a meta total de importadas por reino. Reexecutar com 20 não adiciona outras 100 espécies; aumentar para 40 tenta completar mais 20 por reino. Registros são deduplicados por nome e ID da fonte. O script limita a frequência da API, repete falhas de conexão e usa cache em build/import-cache. Fotos inválidas são descartadas com motivo.

Uma falha fatal da API antes de concluir não substitui os JSON existentes; fotos já baixadas podem permanecer na pasta de importação. O relatório final registra contagens, déficits, licenças e rejeições. Offline requer arquivos já disponíveis para completar metas novas; com a meta atingida, a reimportação não faz requisições.

## Persistência e migração

glossata_catalog.sqlite fica em QStandardPaths::AppDataLocation, separado de glossata_history.sqlite. Possui taxons com parent_id, unicidade source/source_taxon_id, espécies com chave estrangeira, índices de parentesco/origem e consultas recursivas. Curiosidade, dica, região, nomes, origem e créditos também são persistidos.

A carga inicial e as atualizações do conteúdo são transacionais, com pais antes dos filhos. Um hash dos JSON identifica mudanças, aplicadas por upsert. A adaptação de colunas é aditiva; tabelas e registros anteriores não são apagados. A abertura seguinte carrega o SQLite uma vez para um índice em memória, incluindo registros previamente armazenados. Navegar não relê JSON nem acessa a API.

O banco de jogadores mantém caminho e schema anteriores. O ranking continua com o resultado mais recente por nome. Estatísticas e conquistas continuam relativas à partida atual/última partida, como antes; não foi criada persistência cumulativa de progresso.

## Arquivos e validação

- Domínio: Taxon, Species, Question e elegibilidade das alternativas.
- Aplicação: GameService e TaxonomyIndex para filtros, níveis, contagens e navegação.
- Infraestrutura: JsonSpeciesCatalog e SqliteSpeciesCatalog para carga inicial, migração e cache.
- Apresentação: GameController, telas existentes e TaxonomyView; ExploreCard, KingdomArtwork, LifeArtwork e SpeciesPortrait reutilizados.
- Conteúdo: species.json, taxonomy.json, photo_reviews.json, import_report.json, 100 fotos locais e créditos.
- Ferramentas: import_inaturalist.py, testes C++/QML/Python e CMake.

CTest cobre regras antigas, filtros, níveis válidos, grupos pequenos, quatro opções distintas, mistura de reinos, preservação de perfis/ranking em migrações, reabertura do catálogo, fotos locais, rejeição de licenças, hashes e reimportação offline sem duplicatas. O teste de interface percorre cinco reinos, subgrupos, caminho de volta, quiz e retorno ao explorador, capturando telas em 1240×820 e 1000×700 e rejeitando avisos QML.

## Limitações

Os novos textos descrevem a classificação recebida, com fontes acessíveis na ficha. Distribuição, habitat, alimentação e outras curiosidades precisam de curadoria; não foram gerados como fatos sem fonte. As 22 curiosidades anteriores ainda precisam da revisão científica já prevista.

A seleção favorece observações populares com fotos licenciadas e não representa toda a diversidade de cada reino. Identificações do iNaturalist podem mudar; algumas classificações são incompletas. Os quizzes visuais são exercícios educativos dentro do catálogo.

O jogo empacotado usa recursos embarcados e funciona offline. Discord permanece opcional. Falta validar a distribuição em um Windows sem Qt/MSYS2 instalado, como já previsto.
