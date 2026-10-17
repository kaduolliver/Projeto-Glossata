# Identidade temporária

Arte fornecida pelo usuário para testes de identidade do Projeto Glossata.

- `glossata.png`: logo de 1254×1254 para o cabeçalho e o ícone da aplicação no Developer Portal.
- `glossata_discord.png`: variante de 1254×1254 preparada para Rich Presence.
- `glossata.ico`: ícone do Windows com tamanhos 16, 24, 32, 48, 64, 128 e 256 pixels.
- Os arquivos atuais foram copiados do pacote `glossata_icon_pack` fornecido pelo usuário, preservando seus bytes. A arte não representa uma espécie do catálogo.

## Proveniência — atualização de 2026-10-17

O mantenedor declarou criação via ChatGPT e referência inicial ao símbolo dos Vagalumes de The Last of Us, depois adaptada para mariposa e motivos biológicos. A [declaração anterior](../compliance/branding/origin-declaration.json) foi preservada e a [declaração complementar](../compliance/branding/reference-disclosure.json) não concede permissão. Data/modelo, arquivo de referência exato, método e direitos das entradas continuam desconhecidos.

A inspeção demonstrou equivalência de todos os quadros ICO a redimensionamentos Lanczos do PNG principal; isso não comprova a ferramenta histórica. Os dois PNGs não são idênticos em pixels. A existência do conversor abaixo não demonstra seu uso para produzir os arquivos atuais.

Os três registros continuam `pending_permission`, com `distribution_allowed: false`. Consulte a [avaliação e recomendação](../../docs/SANEAMENTO_IDENTIDADE_VISUAL.md) e a [política proposta, não ativada](../../docs/propostas/POLITICA_IDENTIDADE_VISUAL.md). Não houve geração ou substituição de arte, concessão de licença gráfica ou afirmação de marca registrada.

Para importar outro pacote pronto, substitua os três arquivos correspondentes, compile e atualize o pacote. O build recompila também o recurso nativo Windows quando o ICO muda. Para gerar um PNG 1024×1024 e um ICO a partir de uma imagem única, use `scripts/set-icon.ps1 -InputImage 'caminho da imagem'`; a variante de Discord deve ser atualizada separadamente. Esses arquivos são temporários; os direitos e créditos da arte precisam ser confirmados antes de uma distribuição pública.
