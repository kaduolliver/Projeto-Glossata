# Teste do ícone e da atividade no Discord

O jogo usa a aplicação `1559712085250801685`. A conexão é local com o Discord desktop, via RPC/IPC. Não exige token de bot, Client Secret nem login dentro do jogo.

## Imagem no Developer Portal

1. Abra [sua aplicação](https://discord.com/developers/applications/1559712085250801685).
2. Em **General Information**, use `resources/branding/glossata.png` como ícone da aplicação para testar também a identidade da aplicação no Discord.
3. Em **Rich Presence → Art Assets**, cadastre `resources/branding/glossata_discord.png` como um novo asset com a chave **glossata_icon_v2** e salve. A configuração usa essa nova chave para separar a imagem atual da anterior. A chave deve corresponder a `largeImage` no JSON.
4. Mantenha o Discord desktop aberto e conectado e inicie `package/Glossata Quiz.lnk`.
5. Em **Perfis e ajustes**, confira o estado **Atividade publicada no Discord**. Veja a atividade no seu perfil do Discord; a configuração de compartilhamento de atividades do Discord precisa permitir a exibição.

O ícone da aplicação no Portal, a imagem grande do Rich Presence e o ícone do executável Windows são configurações distintas. As variantes atuais têm 1254×1254 e vieram do pacote fornecido pelo usuário. O Discord [recomenda 1024×1024 para assets](https://docs.discord.com/developers/discord-social-sdk/development-guides/setting-rich-presence#uploading-assets). Reinicie o jogo após trocar o asset para conferir a nova imagem.

## Configuração local

`package/discord.json` contém:

```json
{
  "enabled": true,
  "applicationId": "1559712085250801685",
  "largeImage": "glossata_icon_v2"
}
```

Reinicie o jogo depois de editar esse arquivo. Para manter alterações nos próximos empacotamentos, edite também `resources/data/discord.json`.

A atividade exibe o nome do jogo, **Explorando os cinco reinos biológicos**, **Uma descoberta por vez** e o tempo desde a abertura do aplicativo. Não publica o nome do perfil local ou o ranking. O controle **Atividade no Discord** desliga a presença para a sessão atual; `enabled: false` no JSON a desliga por padrão. Fechar o jogo remove a atividade.

Se o Discord não estiver disponível, o jogo continua funcionando e tenta conectar novamente a cada 15 segundos. O status de conexão não confirma que o asset foi cadastrado: a imagem deve ser conferida visualmente depois do upload no Portal.

## Launcher e diagnóstico

Use **Glossata Quiz.lnk** para iniciar com o ícone próprio. O atalho usa um arquivo ICO em `package/branding/` cujo nome inclui o hash da imagem, e o empacotamento notifica o Explorer sobre a alteração do atalho. Isso evita depender do ícone do executável guardado no cache do Windows. O executável também contém o ícone. O `.cmd` continua disponível, mas sua aparência no Explorer segue a associação de arquivos `.cmd` do Windows. Cópias antigas ou atalhos fixados devem ser substituídos pelo atalho atualizado.

Para testar somente a conexão, sem abrir uma janela do jogo:

```powershell
$testProcess = Start-Process -FilePath '.\package\glossata_quiz.exe' -ArgumentList '--discord-test' -WindowStyle Hidden -Wait -PassThru
$testProcess.ExitCode
Get-Content '.\package\discord-test.log'
```

O teste publica brevemente a atividade e a remove ao sair. O código 0 indica publicação confirmada; 2 indica tempo esgotado; 3 indica configuração desativada ou impossibilidade de criar o log. O teste deve rodar no ambiente normal do usuário: sandboxes podem bloquear o canal local do Discord.

A implementação segue o [protocolo RPC/IPC documentado pelo Discord](https://docs.discord.com/developers/topics/rpc). Para uma integração futura com amigos, convites ou outras funções sociais, considerar o Social SDK oficial.
