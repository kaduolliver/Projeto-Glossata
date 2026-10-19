# Documentação da interface

Esta pasta registra a interface Qt 6/QML do Projeto Glossata. A [arquitetura](ARQUITETURA_UI.md) descreve as telas e a navegação; o [design system](DESIGN_SYSTEM.md) inventaria os estilos em uso e as oportunidades de centralização; o [histórico visual](CHANGELOG_UI.md) aponta para as versões documentadas.

A [linha de base anterior à reformulação](VERSOES/baseline/README.md) está ligada ao commit `b201e20c2d83fda6d790bc6a96217c0fdc75307e`. Suas capturas são imagens da janela Qt real, geradas pelo teste de interface com SQLite temporário. O [manifesto de capturas](VERSOES/baseline/capturas.json) registra hashes, dimensões, horário, ambiente e binário usado. O commit desta documentação preserva as imagens; o código da interface se recupera pelo commit de base, sem cópia integral nesta pasta.

## Como registrar uma mudança futura

Revisão implementada: [2.1.2 — design system](VERSOES/2.1.2/README.md), com [contratos dos componentes](COMPONENTES.md) e capturas reais próprias. O baseline permanece histórico.

1. Antes de alterar QML, crie uma versão em `VERSOES/<identificador>/` com o commit exato da interface e as telas afetadas. Conserve as versões anteriores.
2. Recompile o aplicativo e gere capturas da **janela real** com banco temporário, registrando método, ambiente, horários, dimensões, hashes do binário e dos PNGs. Se a tela inicial mudar, execute `scripts/capture_home.py` e atualize também `docs/images/tela-inicial.png` e seu registro no **mesmo commit visual**.
3. Documente a diferença observável, as limitações da captura e o caminho do PR em `CHANGELOG_UI.md`. Inclua o commit final da mudança após criá-lo; se ainda não existir, deixe a referência como pendente, sem antecipar hash.
4. Verifique os links e hashes, execute os testes de interface pertinentes e confira as capturas. Capturas não substituem teste de acessibilidade nem inspeção em outros sistemas ou GPUs.

Os PNGs podem conter fotografias e identidade visual com direitos individuais. Seus registros de origem e atribuição continuam nos controles existentes; uma captura não altera o estado de distribuição de qualquer recurso.
