# Exploração científica e fotografias dos cartões

A exploração mantém Bacteria, Archaea e Eukaryota como portas principais. Os atalhos Animalia, Plantae e Fungi abrem os mesmos nós da hierarquia, com seus identificadores originais: Animalia e Fungi em Opisthokonta; Plantae no sentido do backbone adotado em Archaeplastida. Não são novas raízes nem uma classificação alternativa. Os caminhos clicáveis permitem voltar aos ancestrais; grupos intermediários ausentes não são inventados.

Em Eukaryota, os cartões mostram Amoebozoa, Archaeplastida, Discoba, Opisthokonta e SAR. Os grupos filhos aparecem antes dos controles de quiz para favorecer a exploração. Os filtros de quiz usam o nó selecionado e seus descendentes, preservando as exigências de revisão, fotografia adequada ao nível e quatro alternativas distintas. Um grupo vazio, como Archaea ou Discoba nesta etapa, continua navegável, sem inventar organismos ou liberar um quiz. Consulte as [fontes e divergências filogenéticas](CLASSIFICACAO_FILOGENETICA.md).

## Imagens e escopo

O manifesto editável `resources/data/exploration_photos.json` associa explicitamente os identificadores dos grupos às fotografias. Não se propaga a imagem de um ancestral indiscriminadamente para seus descendentes. Os 122 organismos, suas 120 fotos de quiz, identificadores, revisões, pontos, perfis e conquistas permanecem preservados. As oito novas imagens são recursos de apresentação e não concedem elegibilidade para identificação visual.

| Representante | Uso nos cartões | Fonte e licença por arquivo |
| --- | --- | --- |
| *Danaus plexippus* | Jogo misto, Eukaryota e ramos correspondentes de lepidópteros | [kcthetc1 — CC0 1.0](https://commons.wikimedia.org/wiki/File:Danaus_plexippus_100029190.jpg) |
| *Panthera leo* | Opisthokonta, Animalia e ancestrais correspondentes do leão | [Kevin Pluck — CC BY 2.0](https://commons.wikimedia.org/wiki/File:Lion_waiting_in_Namibia.jpg) |
| *Helianthus annuus* | Archaeplastida, Plantae e Tracheophyta | [S.G.S. — CC0 1.0](https://commons.wikimedia.org/wiki/File:Sunflowers_public_domain_helianthus_annuus_IMG_0815.jpg) |
| *Amanita muscaria* | Fungi e ramos correspondentes do cogumelo | [Bernard Spragg. NZ — CC0 1.0](https://commons.wikimedia.org/wiki/File:Amanita_muscaria_(1)_(8692325426).jpg) |
| *Macrocystis pyrifera* | SAR, Stramenopiles e ramos correspondentes da alga | [Gonzalo Bravo — CC BY 4.0](https://commons.wikimedia.org/wiki/File:Macrocystis_pyrifera_671118520.jpg) |
| *Physarum polycephalum* | Amoebozoa e Mycetozoa | [Katja Schulz — CC BY 4.0](https://commons.wikimedia.org/wiki/File:Physarum_polycephalum_10744588.jpg) |
| *Escherichia coli* | Bacteria; micrografia eletrônica colorizada, com recorte publicado pela fonte | [NIAID — CC BY 2.0](https://commons.wikimedia.org/wiki/File:E._coli_bacteria_(cropped).jpg) |
| *Sulfolobus* sp. A20, nome da fonte | Archaea; micrografia eletrônica de transmissão. A página registra a atualização para *Saccharolobus* | [Dai e colaboradores — CC BY 4.0](https://commons.wikimedia.org/wiki/File:Fmicb-07-01902-g001-v1.jpg) |

Os créditos completos, inclusive a lista de autores fornecida para a micrografia de Archaea, são gerados em [PHOTO_CREDITS.md](PHOTO_CREDITS.md) e apresentados em **Perfis e ajustes → Créditos e transparência**. O cartão identifica o organismo, a autoria, a licença e as limitações em sua descrição acessível e ao manter o ponteiro ou foco. *P. polycephalum* não é apresentado como fotografia de *P. psittacinum*, organismo do catálogo. As micrografias de domínio também não são atribuídas às espécies do quiz.

## Permissão e rastreabilidade

`scripts/exploration_media.py` consulta metadados **do arquivo específico** no Commons, verifica CC0 ou CC BY e sua versão/URL antes de solicitar a imagem. Ausência, incompatibilidade, restrições, SA, ND e NC são recusadas neste fluxo. A licença geral do site não serve como licença da foto. O autor indicado pela fonte é preservado; não se presume que o carregador ou observador seja o fotógrafo.

As evidências selecionadas da resposta da API ficam em `resources/compliance/exploration/`: identificador do arquivo, revisão da página, URLs original e da miniatura, SHA-1 original informado pela fonte, descrição, crédito e licença, data real da coleta. O manifesto registra também SHA-256 local, hash da evidência, dimensões, tamanho e modificações. O inventário `asset_rights.json` vincula o recurso à mesma evidência. Não se arquivam coordenadas ou páginas integrais com dados pessoais desnecessários. A revisão da página pode ser consultada no histórico do Commons; os metadados arquivados não simulam uma revisão jurídica ou independente.

O processo mantém créditos também para CC0, indica as alterações e não sugere endosso. Qualquer futura troca de arquivo exige nova evidência e novo hash; não sobrescreva a prova anterior em um lançamento já publicado. Git preserva as coletas anteriores. Os bloqueios de distribuição dos recursos legados continuam em vigor.

## Renderização e funcionamento offline

As oito miniaturas fornecidas pela fonte têm 330 pixels de largura e somam 276.666 bytes. São embarcadas no QRC, com manifesto e evidências; a execução normal não faz downloads. A região das imagens continua no alto à direita dos cartões (96 × 96; até 250 × 115 no cartão misto), sem moldura.

`FeatheredPhoto.qml` usa Canvas.Image e composição `destination-in`, disponíveis no Qt instalado, com gradientes horizontais e verticais nas bordas da fotografia ajustada proporcionalmente. O centro permanece opaco; o organismo não é recortado para preencher o espaço. A máscara acompanha os limites da foto, incluindo formatos retrato. A pintura ocorre no carregamento e no redimensionamento, sem animação contínua. Não requer um efeito exclusivo de GPU. Consulte a documentação do [Canvas](https://doc.qt.io/qt-6/qml-qtquick-canvas.html) e do [Context2D](https://doc.qt.io/qt-6/qml-qtquick-context2d.html).

Sem associação explícita ou com arquivo/evidência inválidos, o componente mantém a ilustração anterior. Discoba e vários ramos menores não têm fotografia selecionada nesta etapa. A ausência é preferível a uma representação incorreta. Os textos e indicadores ficam separados visualmente da imagem; foco de teclado e descrições acessíveis são preservados.

## Verificação e pendências humanas

Execute `python scripts/exploration_media.py`, `python tests/test_exploration_media.py`, `python scripts/content_governance.py credits`, compilação, CTest, qmllint e auditoria estrita do catálogo. A integração contínua usa os mesmos controles offline. Os testes verificam permissão por arquivo, crédito e evidência adulterados, associação incompatível, ausência de foto, micrografias, filtros e acesso aos 122 organismos. Os testes QML capturam o programa real e verificam transparência progressiva com centro opaco no renderizador software.

As associações visuais foram preparadas por IA a partir da identificação da fonte e dos caminhos do catálogo. O campo `scientific_review` explicita essa limitação; escopos externos ao catálogo permanecem rascunhos. Não houve nova revisão por especialista, alteração de revisão científica, concessão do mantenedor ou aprovação humana presumida. Cabe à curadoria conferir identificação, representatividade e qualidade pedagógica, em especial micrografias e mixomicetos. Licenças demonstradas na coleta e testes técnicos não certificam regularização integral do projeto. Relate problemas pelo [canal de correção e remoção](REPORTAR_PROBLEMA.md).
