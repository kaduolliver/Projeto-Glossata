# Instruções do projeto

- Preserve funcionamento offline, funcionalidades e os bancos/perfis/histórico dos jogadores. Use bancos temporários em testes e capturas.
- Organize alterações em commits pequenos e coerentes, com mensagens claras, depois das verificações pertinentes. Não faça push nem publique releases sem autorização específica do usuário.
- Use SemVer: PATCH para correções compatíveis, MINOR para novas funcionalidades compatíveis, MAJOR para mudanças incompatíveis. A versão do aplicativo (`VERSION.txt`) é independente da versão do catálogo (`resources/data/catalog_manifest.json`).
- Em cada lançamento, atualize `CHANGELOG.md` e o histórico do catálogo/licenças, registre a correspondência entre versões e crie tag anotada `vX.Y.Z` apenas após os checks obrigatórios. Consulte `docs/VERSIONAMENTO.md` e use `scripts/release.py tag`. Não contorne o gate de distribuição ou trate testes como certificação científica/jurídica.
- Toda alteração visual relevante da tela inicial deve incluir uma nova captura **real do programa** em `docs/images/tela-inicial.png`, junto ao registro de captura, no mesmo commit. Preserve esse caminho no README. Use `scripts/capture_home.py` após recompilar; se não puder executar/capturar, informe a pendência. Nunca substitua por imagem simulada.
- Não invente revisores, datas, autores, títulos, referências, licenças ou permissões. Registre pendências e decisões humanas separadamente. Preserve evidências e histórico de revisão.

## Higiene de arquivos em tarefas Codex

- Ao concluir uma tarefa ou preparar uma PR, faça uma **varredura não destrutiva** de arquivos temporários, caches, compilações redundantes, capturas locais duplicadas e artefatos não versionados. Resuma os candidatos com caminho, tamanho, motivo e risco.
- Quando disponível, execute `python scripts/cleanup_build.py` (modo padrão **dry-run**) para inventariar candidatos. Não interprete arquivos semelhantes ou hashes iguais como autorização para apagar.
- **Nunca exclua automaticamente** arquivos, mesmo ignorados pelo Git. Para `--apply`, peça autorização explícita do usuário e respeite os bloqueios do script (main limpa, somente após merge, idade mínima e proteção dos builds recentes). Não execute limpeza durante testes, builds ou PR aberta.
- Preserve integralmente `resources/`, `package/`, dados de usuários, SQLite, mídias de curadoria, histórico de governança, documentação, capturas e evidências versionadas, manifestos e arquivos necessários à rastreabilidade. Nunca remova dados para forçar aprovações científicas, legais ou da CI.
- Limpeza local é independente do escopo da PR: não adicione exclusões de arquivos versionados ou mudanças na governança como parte de uma correção de UI sem autorização expressa.
