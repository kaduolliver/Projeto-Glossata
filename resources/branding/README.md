# Identidade temporária

Arte fornecida pelo usuário para testes de identidade do Projeto Glossata.

- `glossata.png`: logo de 1254×1254 para o cabeçalho e o ícone da aplicação no Developer Portal.
- `glossata_discord.png`: variante de 1254×1254 preparada para Rich Presence.
- `glossata.ico`: ícone do Windows com tamanhos 16, 24, 32, 48, 64, 128 e 256 pixels.
- Os arquivos atuais foram copiados do pacote `glossata_icon_pack` fornecido pelo usuário, preservando seus bytes. A arte não representa uma espécie do catálogo.

Para importar outro pacote pronto, substitua os três arquivos correspondentes, compile e atualize o pacote. O build recompila também o recurso nativo Windows quando o ICO muda. Para gerar um PNG 1024×1024 e um ICO a partir de uma imagem única, use `scripts/set-icon.ps1 -InputImage 'caminho da imagem'`; a variante de Discord deve ser atualizada separadamente. Esses arquivos são temporários; os direitos e créditos da arte precisam ser confirmados antes de uma distribuição pública.
