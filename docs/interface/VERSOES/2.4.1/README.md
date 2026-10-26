# Subetapa 2.4.1 — Perfil e ajustes

Base: `origin/main` em `4a2e2e4510f058eecd8388f7260d5ce93b2ac9df`, integração da PR #39. Branch `feat/profile-settings-2-4-1`. Aplicativo **2.3.0 ainda não lançado** (MINOR por preferências e áudio configuráveis), catálogo **2.4.0** e SQLite **8** preservados. O número da subetapa é independente da versão semântica do aplicativo. Sem merge, release ou tag.

A [correção pontual de persistência na mesma PR #40](correcao-persistencia/README.md) separa valores recuperáveis, alterações do usuário e arquivos malformados. O botão permite salvar as correções sem editar outra preferência e fica oculto quando o arquivo requer intervenção externa. Os registros da implementação inicial abaixo permanecem preservados; a correção possui suas próprias evidências.

O [refinamento sonoro na mesma PR](refinamento-audio/README.md) documenta a análise do WAV completo, o decaimento originalmente rápido e a possibilidade de parada indevida em mudanças da lista de dispositivos. O WAV atual tem 1,20 s; prévias informam reprodução/espera, com intervalo de 1,3 s e sem reiniciar um efeito ativo. Síntese, recursos anteriores e validação manual estão registrados separadamente.

## Organização e identidade

A página usa cinco seções compactas dentro da superfície do tema: Meu perfil; Interface e acessibilidade; Áudio e notificações; Integrações; Informações. Alturas seguem o conteúdo, com limite de leitura de 1.000 px, divisores discretos, tipografia/ícones existentes, switches com foco e slider acessível por teclado. A rolagem e o enquadramento automático do foco continuam no shell existente.

O nome e o UUID são projeções de `GameController`. Renomear chama exclusivamente `renameCurrentPlayerProfile`, com sua política de validação existente. O UUID, resultados, conquistas, estatísticas, ranking, histórico e payloads/checksums de checkpoints não são reescritos. Não há perfis adicionais, login, cadastro ou sincronização.

O acesso a créditos, fontes, transparência e licenciamento continua abrindo `CreditsView` pelo caminho existente. Não há nova cópia desses conteúdos na página.

## Preferências locais

`AppPreferences` é a única fonte de verdade em C++, exposta como `appPreferences`. QML lê propriedades notificáveis e solicita alterações nos setters. `Main.motionEnabled` é somente leitura e projeta a preferência para o mecanismo existente `Theme.motionEnabled`; não existe outro motor de animações. A redução de movimento das notificações permanece no componente da PR #39.

Persistência: `QSettings::IniFormat`, fallbacks desativados e sincronização atômica padrão. Caminho: `QStandardPaths::AppLocalDataLocation/preferences.ini`, normalmente `%LOCALAPPDATA%/glossata_quiz/preferences.ini` no Windows, no usuário do sistema operacional. Preferências pertencem à instalação local nesse contexto e não a uma conta ou UUID. O identificador `glossata_quiz`, a organização da aplicação e os caminhos dos bancos pessoais não mudam. Não há migração SQLite.

| Chave | Padrão | Validação |
| --- | --- | --- |
| `interface/motionEnabled` | `true` | booleano ou representação `true/false/1/0` |
| `audio/achievementSoundsEnabled` | `false` | booleano ou representação `true/false/1/0` |
| `audio/effectsVolume` | `35` | inteiro de 0 a 100 |

Ausência de arquivo/chave não provoca gravação inicial. Valores semanticamente inválidos usam o padrão da opção, mostram aviso e marcam `recoveryPending`, separado de `dirty_`. A ação `retrySave()` do botão confirma explicitamente a gravação das correções, preservando valores válidos e chaves desconhecidas. Enquanto essa confirmação estiver pendente, gravações automáticas por timer ou fechamento aguardam a ação do usuário, inclusive para outras alterações da sessão.

Preferências válidas mantêm gravação reunida em 250 ms e sincronização ao sair; setters sem mudança e novas tentativas após sucesso não regravam. Os setters continuam limitando volume a 0–100. Falhas de gravação mantêm os valores da sessão e permitem nova tentativa. Arquivo estruturalmente malformado mantém a proteção retida, usa padrões seguros e exige correção externa seguida de reabertura; `canRetryPersistence` é falso e o botão fica oculto. Não há restauração destrutiva automática.

Testes, smoke e captura recebem caminhos temporários explícitos. Os testes de interface usam uma saída gravadora em memória, sem criar `QSoundEffect` ou exigir dispositivo físico.

## Áudio e apresentação

`AchievementAudio` separa política de reprodução de `EffectOutput`. A saída de produção reutiliza um `QSoundEffect`, carrega o WAV embarcado e informa ausência de dispositivo, carregamento ou erro. Mudança de saída de áudio interrompe a reprodução corrente; não agenda repetição. Volume e desativação afetam imediatamente o efeito em andamento.

Único gatilho de conquistas: `AchievementNotificationLayer.notificationPresented(achievementId)`. Validação do evento, reconhecimento retroativo, deduplicação e FIFO continuam na camada existente. Não há novo observador de desbloqueios. O áudio ainda consome cada ID uma única vez na execução como defesa contra repetição do sinal. Não carrega conquistas antigas nem toca ao abrir Conquistas.

Há uma saída, sem sobreposição, e intervalo mínimo de **1.300 ms** entre solicitações, inclusive prévias, com proteção adicional pelo estado real de reprodução. Em um lote apresentado simultaneamente, somente a primeira elegível toca. As demais são consumidas silenciosamente, sem fila de áudio ou repetição posterior. Uma notificação que só alcançar uma vaga visual mais tarde poderá tocar naquele momento, respeitando a limitação. A fila visual, sua ordem e seus tempos não são alterados. O botão exibe reprodução/espera e fica indisponível enquanto não pode iniciar outra prévia.

A permissão de apresentação da camada controla também o áudio. Modal, minimização/inatividade e demais suspensões interrompem o som corrente. Cartões que já tocaram não tocam ao reaparecer. Eventos pendentes só podem tocar quando realmente apresentados; eventos expirados nunca chegam ao gatilho. Se a saída estiver indisponível ou carregando, a oportunidade é descartada, sem áudio atrasado.

O botão **Testar som** solicita uma prévia explícita, respeitando o volume e a limitação de sobreposição, mesmo com os sons automáticos desativados. Em 0% fica desabilitado. Não cria desbloqueios, não consulta critérios e não altera dados do jogador. Sons automáticos vêm desativados e o volume inicial é 35% para permitir experimentação voluntária.

Origem, síntese reproduzível, características e pendência de licença: [recurso de áudio](../../../../resources/audio/README.md). Qt Multimedia é uma dependência adicional obrigatória do build. Referências técnicas: [QSettings](https://doc.qt.io/qt-6/qsettings.html) e [QSoundEffect](https://doc.qt.io/qt-6/qsoundeffect.html).

## Discord e dependências Windows

`discord.json` continua com `enabled: true`. Na abertura normal, `main.cpp` continua chamando `presence.start()` segundo esse arquivo. O switch desativa/ativa apenas a sessão atual; a abertura seguinte volta ao comportamento existente. A interface informa essa duração. Persistir essa opção mudaria o comportamento de compartilhamento e exige decisão própria; não foi acrescentada às três preferências aprovadas. Smoke e capturas continuam sem iniciar Discord.

Ambiente de validação: MSYS2 UCRT64, Qt 6.11.2; instalado `mingw-w64-ucrt-x86_64-qt6-multimedia` 6.11.2-1, compatível com `qt6-base` 6.11.2-2. O pacote MSYS2 acrescenta seu backend FFmpeg e dependências transitivas. `CMakeLists.txt` exige `Multimedia` e liga `Qt6::Multimedia`; a CI instala o mesmo módulo. O WAV está em QRC e acompanha o executável, sem acesso à rede.

`scripts/package.ps1` mantém os gates e acrescenta `-BuildBinary` e `-PackageDirectory` para validar um binário em um diretório novo dentro do projeto. Recusa um diretório alternativo já existente e não modifica `package/` neste fluxo. O deploy usa `windeployqt6`, confere `Qt6Multimedia.dll`, gera `qt.conf` com caminhos relativos de QML/plugins e mantém a inspeção recursiva de DLLs e o inventário de obrigações. A ausência de `qt.conf` foi detectada pelo smoke isolado e corrigida; o script corrigido foi executado novamente do início ao fim, em outro diretório novo, com sucesso.

O pacote local de desenvolvimento contém `Qt6Multimedia.dll` e `multimedia/ffmpegmediaplugin.dll`. Iniciou com PATH limitado ao pacote e ao Windows, sem carregar módulos do MSYS2 externo. O backend registra FFmpeg 9.0.1, “GPL version 3 or later”; o inventário identifica 82 pacotes, cujas obrigações continuam pendentes de revisão. Isso não concede permissão de distribuição. Foram observados avisos de traduções Qt e DLLs opcionais de Direct3D 12 ausentes; o smoke offscreen/software passou e não valida esses recursos ou uma instalação Windows inteiramente limpa.

Reprodução do pacote isolado, escolhendo sempre um diretório de saída novo:

```powershell
powershell -File scripts/package.ps1 -Development -BuildBinary C:/glossata_game/build/profile-settings-2-4-1/projeto_glossata.exe -PackageDirectory C:/glossata_game/build/profile-settings-package-verified
powershell -File scripts/validate_package.ps1 -PackageDirectory C:/glossata_game/build/profile-settings-package-verified
```

Os novos headers auxiliares `tests/RecordingEffectOutput.h` e `tests/TestAppServices.h` também aguardam decisão de licença, sem herança automática da licença de outros arquivos. O inventário mecânico atual não classifica headers em `tests/`; esta pendência fica explicitamente registrada aqui, sem ampliar a concessão ou alterar a política de governança.

## Validação e limitações

As duas verificações de Perfil/áudio passaram em [100%](verificacoes/ui-100.txt), [125%](verificacoes/ui-125.txt) e [150%](verificacoes/ui-150.txt), sem casos ignorados. Cada execução verifica as três resoluções, formando as nove combinações requeridas. Testa nome inválido/renomeação com UUID, conquistas, métricas, pontuação e payloads/checksums de checkpoints preservados; persistência/reabertura; Tab/Space, foco, Home/End/setas, volume, prévia, alinhamento e enquadramento pela rolagem. A integração exercita lote real de desbloqueios, duplicado, suspensão, fila, ocultação/retomada e reavaliação histórica sem reprodução.

Foram preservados **27 PNGs reais**, com três enquadramentos em cada combinação: topo do perfil, áudio com foco de teclado e rodapé com Informações. [Manifesto com dimensões, horários reais e SHA-256 dos PNGs, fontes e executáveis](capturas.json). O harness usa QML/controlador de produção, banco e preferências temporários e saída de áudio gravadora em memória. Não são mockups. A [Home oficial](../../../images/tela-inicial.png) e seu [registro](../../../images/tela-inicial.json) foram renovados por `scripts/capture_home.py --binary build/profile-settings-2-4-1/projeto_glossata.exe --section discovery`, executando o aplicativo de produção recompilado, e acompanham o commit do shell.

| Resolução lógica | 100% | 125% | 150% |
| --- | --- | --- | --- |
| 1000×700 | [Perfil](capturas/100/profile-top-1000x700.png) · [Áudio/foco](capturas/100/audio-keyboard-focus-1000x700.png) · [Informações](capturas/100/information-bottom-1000x700.png) | [Perfil](capturas/125/profile-top-1000x700.png) · [Áudio/foco](capturas/125/audio-keyboard-focus-1000x700.png) · [Informações](capturas/125/information-bottom-1000x700.png) | [Perfil](capturas/150/profile-top-1000x700.png) · [Áudio/foco](capturas/150/audio-keyboard-focus-1000x700.png) · [Informações](capturas/150/information-bottom-1000x700.png) |
| 1240×820 | [Perfil](capturas/100/profile-top-1240x820.png) · [Áudio/foco](capturas/100/audio-keyboard-focus-1240x820.png) · [Informações](capturas/100/information-bottom-1240x820.png) | [Perfil](capturas/125/profile-top-1240x820.png) · [Áudio/foco](capturas/125/audio-keyboard-focus-1240x820.png) · [Informações](capturas/125/information-bottom-1240x820.png) | [Perfil](capturas/150/profile-top-1240x820.png) · [Áudio/foco](capturas/150/audio-keyboard-focus-1240x820.png) · [Informações](capturas/150/information-bottom-1240x820.png) |
| 1920×1080 | [Perfil](capturas/100/profile-top-1920x1080.png) · [Áudio/foco](capturas/100/audio-keyboard-focus-1920x1080.png) · [Informações](capturas/100/information-bottom-1920x1080.png) | [Perfil](capturas/125/profile-top-1920x1080.png) · [Áudio/foco](capturas/125/audio-keyboard-focus-1920x1080.png) · [Informações](capturas/125/information-bottom-1920x1080.png) | [Perfil](capturas/150/profile-top-1920x1080.png) · [Áudio/foco](capturas/150/audio-keyboard-focus-1920x1080.png) · [Informações](capturas/150/information-bottom-1920x1080.png) |

Reprodução da matriz com Qt no PATH e o binário compilado:

```powershell
$env:QT_QPA_PLATFORM='offscreen'
$env:QT_QUICK_BACKEND='software'
$env:QT_QUICK_CONTROLS_STYLE='Basic'
foreach ($scale in '1','1.25','1.5') {
    $env:QT_SCALE_FACTOR=$scale
    build/profile-settings-2-4-1/glossata_ui_tests.exe profileSettingsPreferencesKeyboardAndCaptures achievementSoundsFollowPresentationOnly
}
```

O renderizador offscreen produz quadros reais do programa, mas não comprova fluidez física, audibilidade, comportamento de todos os dispositivos ou leitores de tela. A percepção estética do som ainda deve ser avaliada pelo mantenedor em hardware real. Nenhuma aprovação jurídica ou científica é inferida dos testes.

## Resultados consolidados

Registro da implementação inicial, até `1d39072`. Os resultados da correção pontual estão no [registro específico](correcao-persistencia/README.md).

[Resumo técnico e hashes](verificacoes/resultados.json). A execução CTest completa aprovou 27 dos 28 grupos em 701,09 s; o caso de reabertura da interface criava um segundo engine sem os novos serviços temporários. Após corrigir esse caso, a **suíte inteira de interface** foi repetida e aprovada em 509,78 s, com **39 casos, zero falhas e zero ignorados**. Consolidam-se assim **28 grupos distintos aprovados**, sem falhas pendentes. [Execução inicial](verificacoes/ctest-initial.txt), [interface inicial](verificacoes/ui-full-initial.txt), [reexecução CTest](verificacoes/ctest-interface-final.txt) e [interface final](verificacoes/ui-full-final.txt).

| Área | Resultado |
| --- | --- |
| Regras/controlador, Quiz, Conquistas, Estatísticas e Ranking | 86 casos aprovados |
| Identidade local | 38 casos aprovados |
| Recuperação/checkpoints | 21 casos aprovados |
| Preferências e política de áudio | [16 casos aprovados](verificacoes/unit-final.txt), incluindo valores inválidos, arquivo malformado preservado e falha/nova tentativa de gravação |
| Interface completa | 39 casos aprovados |
| Perfil/áudio em escala | 100%, 125% e 150% aprovados; três resoluções por execução |
| QML/metadados/escopo de código/catálogo | Lint, release check, project status, inventário de código e auditoria técnica aprovados |
| Governança | [1.053 eventos e baseline preservados byte a byte](verificacoes/governance-history.json) frente à base da PR #39 |
| Pacote Windows local | [Smoke com PATH isolado aprovado](verificacoes/package-smoke.json), 91 módulos carregados, nenhum do MSYS2 externo |

Os relatórios iniciais foram mantidos. O teste de arquivo malformado identificou o cache compartilhado de parsing do QSettings; a falha de formato agora fica retida para impedir regravação do arquivo. Home/End foram implementados explicitamente no slider. Os ajustes restantes de teste tratam contexto de exploração, geometria em coordenadas da página, callbacks pendentes da Home e instalação dos serviços no engine de reabertura. A primeira falha de pacote por caminhos QML está [registrada](verificacoes/package-initial-failure.json). Uma tentativa de link encontrou o executável de teste em uso; o link foi repetido sem concorrência, e a matriz final foi refeita com o binário final.

O [gate de distribuição](verificacoes/distribution-gate.json) retorna **2 — bloqueado**, preservando as pendências de permissões, revisões e obrigações, inclusive as dependências adicionais. A nova licença do WAV e a inclusão expressa dos arquivos novos no escopo de código continuam pendentes. Integração de desenvolvimento e testes não liberam distribuição. Não há merge, tag, release ou implementação da 2.4.2.

## Arquivos alterados

[Lista de caminhos da entrega inicial](verificacoes/arquivos-modificados.txt). Os grupos principais são `AppPreferences`, `AchievementAudio` e `EffectOutput`; `SettingsView` e ligação em `Main`/`main.cpp`; recurso WAV/gerador; testes e serviços isolados; CMake/CI/empacotamento; versões, documentação, capturas e relatórios. `GameController`, catálogo, persistência SQLite, regras e pontuação não foram modificados. O [registro da correção](correcao-persistencia/README.md) identifica seus arquivos adicionais.

## Inventário sem exclusões

A varredura foi executada após encerrar builds e testes, antes de abrir a PR: [dry-run padrão](verificacoes/cleanup-dry-run.json), [inclusão de builds antigos](verificacoes/cleanup-old-builds-dry-run.json) e [inventário com tamanho/motivo/risco](verificacoes/inventario-local.json). Ambos os dry-runs encontraram **zero candidatos elegíveis** pelas proteções de idade e retenção. Nenhuma limpeza foi aplicada; os artefatos abaixo permanecem preservados para revisão humana.

| Caminho | Bytes | Motivo | Risco |
| --- | ---: | --- | --- |
| `build/profile-settings-2-4-1` | 323617479 | Executáveis usados na verificação e rastreabilidade | alto |
| `build/profile-settings-package` | 329033997 | Pacote inicial e diagnóstico de importação QML | alto |
| `build/profile-settings-package-verified` | 329034060 | Pacote corrigido e evidência do smoke isolado | alto |
| `build/2.3.2-cores-dicas` | 323614407 | Binários da primeira compilação, que herdou o diretório anterior do cache | alto |
| `previews/profile-settings` | 3024532 | Originais locais das capturas também versionadas nesta pasta | médio |
| `scripts/__pycache__` | 1363308 | Cache Python regenerável | baixo |
| `scripts/scientific_sources/__pycache__` | 221517 | Cache Python regenerável | baixo |

`resources/`, `package/`, dados de jogadores, SQLite, documentação, capturas, manifestos e histórico permanecem protegidos. Sem autorização explícita para exclusão, sem `--apply` e sem uso de limpeza para alterar resultados de governança.
