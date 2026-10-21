# Documentação da interface

Esta pasta registra a interface Qt 6/QML do Projeto Glossata. A [arquitetura](ARQUITETURA_UI.md) descreve as telas e a navegação; o [design system](DESIGN_SYSTEM.md) inventaria os estilos em uso e as oportunidades de centralização; o [histórico visual](CHANGELOG_UI.md) aponta para as versões documentadas.

A [linha de base anterior à reformulação](VERSOES/baseline/README.md) está ligada ao commit `b201e20c2d83fda6d790bc6a96217c0fdc75307e`. Suas capturas são imagens da janela Qt real, geradas pelo teste de interface com SQLite temporário. O [manifesto de capturas](VERSOES/baseline/capturas.json) registra hashes, dimensões, horário, ambiente e binário usado. O commit desta documentação preserva as imagens; o código da interface se recupera pelo commit de base, sem cópia integral nesta pasta.

## Como registrar uma mudança futura

Revisão atual: [2.2.2 — refinamentos finais de Explorar](VERSOES/2.2.2/rodada-final/README.md), preservando a [rodada 2](VERSOES/2.2.2/rodada-2/README.md), a [rodada 1](VERSOES/2.2.2/rodada-1/README.md) e a [entrega inicial](VERSOES/2.2.2/README.md), sobre a [Descoberta do Dia 2.2.1-B](VERSOES/2.2.1-B/README.md), o [Início fotográfico 2.2.1-A](VERSOES/2.2.1-A/README.md), a [navegação 2.1.3](VERSOES/2.1.3/README.md) e o [design system 2.1.2](VERSOES/2.1.2/README.md), com [contratos dos componentes](COMPONENTES.md). As versões anteriores permanecem históricas.

1. Antes de alterar QML, crie uma versão em `VERSOES/<identificador>/` com o commit exato da interface e as telas afetadas. Conserve as versões anteriores.
2. Recompile o aplicativo e gere capturas da **janela real** com banco temporário, registrando método, ambiente, horários, dimensões, hashes do binário e dos PNGs. Se a tela inicial mudar, execute `scripts/capture_home.py` e atualize também `docs/images/tela-inicial.png` e seu registro no **mesmo commit visual**.
3. Documente a diferença observável, as limitações da captura e o caminho do PR em `CHANGELOG_UI.md`. Inclua o commit final da mudança após criá-lo; se ainda não existir, deixe a referência como pendente, sem antecipar hash.
4. Verifique os links e hashes, execute os testes de interface pertinentes e confira as capturas. Capturas não substituem teste de acessibilidade nem inspeção em outros sistemas ou GPUs.

Os PNGs podem conter fotografias e identidade visual com direitos individuais. Seus registros de origem e atribuição continuam nos controles existentes; uma captura não altera o estado de distribuição de qualquer recurso.

## Planejamento das próximas subetapas — Issue #22

Sequência da evolução visual da Home e das próximas telas:

1. **2.2.1-A — Início fotográfico:** concluída no PR #30, integrado à main em `27294a18ffd0d1e533dcdeb47b44ee0011e97b35`.
2. **2.2.1-B — Descoberta do Dia e indicadores dinâmicos:** integrada pelo [PR #31](https://github.com/kaduolliver/Projeto-Glossata/pull/31), merge `9d047910ddb6b56051d1830cba4052ca03a4e520`. Implementação e verificação na [revisão própria](VERSOES/2.2.1-B/README.md).
3. **2.2.2 — Explorar:** entregue na [PR #32](https://github.com/kaduolliver/Projeto-Glossata/pull/32), independente e vinculada à Issue #22. Implementação e evidências nesta [revisão](VERSOES/2.2.2/README.md); sem merge ou lançamento nesta entrega.
4. **2.2.3 — Jogar:** futura reformulação, após Explorar e fora do escopo de 2.2.1-B.

Propostas ainda pendentes, sem identificador ou conclusão presumidos:

- **Etapa 3 — Enciclopédia Científica Interativa:** criação e revisão de descrições educativas completas dos grupos e organismos, com fontes e decisões rastreáveis no fluxo de curadoria. A PR #32 apenas reutiliza conteúdo elegível existente; não produz descrições em massa nem antecipa fichas completas.
- Refinar visualmente o diálogo de créditos, preservando atribuições, licenças, fontes e acessibilidade.
- Melhorar as descrições educativas do carrossel por meio da curadoria científica existente, com fontes e revisão das redações concretas. Preservar os textos originais e suas decisões; não substituir ou reaproveitar aprovações indevidamente.

O detalhamento e a implementação dependem das próximas subetapas da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22), sem antecipar sua conclusão.
