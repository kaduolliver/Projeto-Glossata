# Revisão visual 2.1.2 — design system

Subetapa 2.1.2 da [Issue #22](https://github.com/kaduolliver/Projeto-Glossata/issues/22). Base anterior: `15cc212b097eb2dc72ecdec84c869cf141e56da4`. 2.1.2 identifica a etapa documental; o aplicativo continua 1.7.0 em desenvolvimento, sem lançamento.

Tokens QML locais centralizam cores, tipografia, espaçamentos, bordas e movimento em quatro componentes compartilhados. Botões têm fundos mais escuros, navegação usa dourado discreto na seleção e cartões estatísticos ganham bordas e texto mais legíveis. Layout, fotografias, ícones e regras são preservados. Não há carrossel, nova exploração, quiz reformulado ou novas conquistas.

As capturas são renderizações reais: início pelo executável de produção (scripts/capture_home.py), demais telas pelo teste Qt com Main.qml e controlador reais. Bancos temporários, Discord desativado. [capturas.json](capturas.json) registra hashes, horário, dimensões, binário e fontes. A sequência do teste pode exibir sessões recuperáveis criadas no banco temporário. As imagens do [baseline](../baseline/README.md) permanecem intactas.

| Tela afetada | Captura |
| --- | --- |
| Início — botão, navegação e StatCard | [inicio.png](capturas/inicio.png) |
| Estatísticas — cartões e botões | [estatisticas.png](capturas/estatisticas.png) |
| Perfis e ajustes — botões e TextAction | [perfis-ajustes.png](capturas/perfis-ajustes.png) |

Limites: Qt software/offscreen em 1240 × 820, sem moldura Windows; não representa inspeção de todas as GPUs, escalas ou leitores de tela. A referência do mantenedor orientou cores/acabamento, sem importar fotografias, ícones ou classificação da imagem conceitual.

Direitos: capturas não concedem autorização coletiva sobre seus elementos. Consulte [PHOTO_CREDITS.md](../../../PHOTO_CREDITS.md), a [política de direitos](../../../DIREITOS_E_LICENCAS.md) e a ressalva do baseline. Os gates de distribuição seguem independentes dos testes. Theme.qml contém somente código de apresentação próprio, enumerado no escopo Apache; isso não concede direitos sobre imagens.

Validação local: compilação Release concluída; 26 suítes CTest exercitadas. A última execução encontrou somente o snapshot de CHANGELOG.md desatualizado em PROJECT_STATUS.json; ele foi regenerado por `project_status.py write`, e a suíte GlossataProjectStatusTests passou na reexecução. A suíte de interface teve sete casos aprovados, incluindo inicialização/finalização, contraste/estados, navegação, fotografias e recuperação. Qmllint, auditoria estrita dos 122 organismos, escopo Apache, verificação de metadados e hashes das capturas passaram. O gate de distribuição permaneceu bloqueado: 120 pendências fotográficas e 275 blocos científicos, zero erros técnicos; não houve liberação de direitos.

Para reproduzir: `cmake --build build -j 4`, `ctest --test-dir build --output-on-failure`, `python scripts/capture_home.py`. A execução de GlossataInterfaceTests salva as demais capturas em `build/tests/previews/`; esta revisão arquiva statistics.png e settings.png. A opção de redução de movimento e os estados de teclado são exercitados pelo teste específico, sem acessar perfis reais.
