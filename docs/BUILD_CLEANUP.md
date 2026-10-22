# Higiene de compilações e arquivos gerados

O Glossata usa `build/` para compilação CMake/Ninja, testes, capturas temporárias e relatórios. Não se deve excluir artefatos enquanto uma PR ou suíte de testes estiver em andamento.

## Auditoria (segura, sem exclusões)

```powershell
python scripts/cleanup_build.py
python scripts/cleanup_build.py --include-old-builds --min-age-days 14 --retain 2
```

O comando apresenta JSON com candidatos, bytes estimados e motivos pelos quais outros diretórios são protegidos. Por padrão, apenas conjuntos antigos de `build/tests/previews/` podem ser propostos. Com `--include-old-builds`, o script também avalia subpastas versionadas nomeadas como `build/2.2.3-...`, mantendo as duas mais recentes **por versão** e recusando conjuntos que contêm arquivos possivelmente importantes.

## Aplicação manual, somente após merge

A limpeza não é agendada e **nunca ocorre automaticamente em prompts Codex**. Depois de revisar os candidatos e confirmar que resultados necessários foram preservados, o mantenedor pode autorizar explicitamente:

```powershell
python scripts/cleanup_build.py --include-old-builds --min-age-days 14 --retain 2 --apply
```

O modo de exclusão só opera em uma `main` limpa. Nunca exclui a raiz `build/`, `package/`, `resources/`, `docs/`, `CMakeFiles/`, executáveis do build principal, dados pessoais, mídias e evidências versionadas. Não substitui backup. Feche o aplicativo, VS Code builds e testes antes de autorizar a operação.

## Varredura do Codex ao terminar cada tarefa

O `AGENTS.md` estabelece auditoria não destrutiva ao finalizar prompts. O Codex deve listar arquivos desnecessários ou duplicados com motivo e tamanho, separar arquivos *versionados* de *locais*, e solicitar aprovação antes de qualquer exclusão. Não alterar a CI, conteúdo científico, evidências ou políticas de licenciamento para reduzir o tamanho da pasta.

## Limitações

O script não busca arquivos arbitrários em todo o disco e não identifica duplicatas por hash: mesmo bytes idênticos podem ser necessários a testes diferentes. O tamanho indicado é uma estimativa do que os diretórios candidatos ocupam, não uma promessa de espaço recuperável. As pastas ignoradas não são respaldadas pelo Git; confirme que não contêm dados únicos antes de excluí-las.
