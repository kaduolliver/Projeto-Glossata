# Auditoria do repositório e da integração contínua

Auditoria iniciada em 16/10/2026 por automação/IA, sem certificação jurídica ou científica. Base local `3bcbd4f` (`feat/classificacao-filogenetica`), árvore inicialmente limpa; origem `https://github.com/kaduolliver/Projeto-Glossata.git`.

## Git e preservação

`git status`, `git ls-files --stage -z`, inventário de extensões/diretórios e inspeção de padrões conhecidos de credenciais foram executados. Havia **650 arquivos versionados**, sem executáveis, DLLs, objetos, bancos, caches, logs ou contêineres conhecidos de credenciais indevidamente incluídos. A inspeção de nomes no histórico alcançável também não identificou esses artefatos. Não foi feita varredura forense universal de todos os conteúdos históricos ou serviços externos.

Fontes, documentação, recursos científicos, fotos de quiz/apresentação, evidências e configurações de launcher foram preservados. Não houve exclusão de arquivos, limpeza de build ou reescrita de histórico. O Application ID público do Discord não é token/Client Secret.

`.gitignore` já protegia build, pacote gerado, bancos, logs e configurações locais. Acrescentadas extensões de temporários/downloads incompletos, dumps e contêineres de chaves, sem ignorar globalmente JSON ou mídias científicas.

`scripts/audit_repository.py` lê **os blobs do índice**, não apenas os arquivos de trabalho: um segredo já staged continua detectável mesmo se o arquivo local for substituído por texto inofensivo. O relatório registra somente caminho/tipo, nunca valores. Arquivos forçados com `git add -f` também são conferidos. O auditor não apaga nada, não altera dados de jogadores e não verifica identidades de pessoas.

O hook `.githooks/pre-commit` executa esse controle; instalação local por `git config --local core.hooksPath .githooks`. CI repete a verificação, independentemente do hook. Padrões cobrem chaves privadas, formatos conhecidos de tokens GitHub, access keys AWS e tokens MFA Discord; não substituem secret scanning especializado. Credenciais expostas exigiriam revogação pelo titular e tratamento separado, sem mostrar seus valores.

## Evidência do GitHub

Consulta somente de leitura ao repositório e ao [run 38820019422](https://github.com/kaduolliver/Projeto-Glossata/actions/runs/38820019422). A `main` remota estava em `dd69feb3e4af01ce06726c80cf5828e8b8750d15`, anterior às alterações locais. Nada foi enviado, executado novamente ou mesclado no GitHub.

| Job remoto | Evidência | Diagnóstico e ação |
| --- | --- | --- |
| `offline-validation`, 116714052713 | Log: `git: command not found`, saída 127 | Git ausente no ambiente MSYS2. Adicionado `git` a `install` e `git --version` antes de gerar dados. A [documentação oficial do setup-msys2](https://github.com/msys2/setup-msys2#default-shell) inclui a instalação explícita desse pacote |
| `distribution-permissions`, 116714052499 | Etapa de bloqueio recusou distribuição e guardou relatório | O bloqueio não foi desativado nem convertido artificialmente em sucesso. Diagnóstico local separa integridade, documentação e direitos |

O GitHub CLI local não estava autenticado; o conector GitHub e a API pública permitiram consultar estado/jobs/logs. Não foi necessário solicitar credencial. Uma execução futura no runner é necessária para confirmar o workflow corrigido no GitHub; não houve push ou dispatch nesta etapa.

## Direitos: classes distintas de pendências

| Classe | Situação inicial local | Tratamento |
| --- | --- | --- |
| Integridade técnica | Zero erros técnicos no gate | Manter verificações obrigatórias |
| Documentação incompleta | 120 fotos legadas sem versão de licença comprovada, coleta original conhecida e confirmação vinculada | Preservar evidências e pontos históricos; não inventar datas, autores ou permissões. A ausência não prova licença incompatível |
| Metadados divergentes | Sete créditos legados divergem da evidência; três hashes antigos do inventário de materiais próprios | Distinguir identidade/crédito e inventário desatualizado de licença efetivamente incompatível; não conceder direitos ao atualizar hash |
| Materiais próprios | Ícones/arte, textos e organização sem decisão de licença/permissão documentada | Decisão do mantenedor permanece pendente |
| Incompatibilidade efetiva | Nenhuma foto do catálogo atual sinalizada como `license_outside_policy` nesta auditoria | Fotos fora da política continuam recusadas antes do download; não generalizar licença de uma observação ou do site |
| Apresentação | 98 fotos com licença pública CC0/CC BY evidenciada por arquivo | Isso não aprova identificação científica ou resolve materiais legados |

Referências e pendências detalhadas permanecem em [CONFORMIDADE.md](CONFORMIDADE.md), [DIREITOS_E_LICENCAS.md](DIREITOS_E_LICENCAS.md) e [PHOTO_CREDITS.md](PHOTO_CREDITS.md). Nenhuma aprovação humana foi criada.

## Validação e limites

Linha de base: build existente e **9/9 suítes CTest**, 62,41 s. Quatro testes novos usam repositórios temporários para verificar artefatos forçados, preservação das evidências/fotos, detecção do blob staged e ausência de vazamento do segredo no relatório. O workflow integra essa suíte ao CTest e guarda `build/repository-audit.json`.

Não se declara validação de runner remoto ou de Windows limpo. O bloqueio de distribuição permanece enquanto faltarem permissões/obrigações demonstráveis. O registro da execução e o próximo passo ficam em [PLANO_EVOLUCAO.md](PLANO_EVOLUCAO.md).

Conclusão local da etapa 1: **10/10 suítes CTest**, 51,76 s. Os três hashes obsoletos foram atualizados como integridade de inventário, com histórico e sem alterar permissões; catálogo 2.2.1. Continuam seis faltas de permissão dos materiais próprios e as pendências fotográficas legadas. Não houve correção fictícia de licenças.

Preflight adicional em bash MSYS2 local: Git for Windows 2.51.2 fornecido explicitamente no PATH, com exceção de ownership limitada à pasta deste workspace e somente ao processo de validação (sem modificar safe.directory global). Isso verifica a disponibilidade do comando e o auditor em shell MSYS2, não instala nem simula o runner GitHub.
