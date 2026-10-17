# Escopo da Apache License 2.0

Decisão administrativa de **2026-10-17**: o mantenedor autorizou expressamente Apache-2.0 para o **código próprio** do Projeto Glossata e forneceu **Carlos Eduardo Lima de Oliveira** como nome autoral do criador original. A concessão admite uso, modificação e redistribuição, inclusive comercial, conforme [LICENSE](../LICENSE). O propósito educacional e sem fins lucrativos da iniciativa oficial não acrescenta restrição à licença.

## Arquivos abrangidos

O [registro enumerado por arquivo](ESCOPO_LICENCIAMENTO.json), campo `files`, delimita a concessão vigente: **156 arquivos** de C++, QML próprio, scripts Python/PowerShell, testes, build, launcher e automação/configuração. A enumeração é explícita: um arquivo futuro não recebe licença por estar na mesma pasta. O código de testes não licencia as fixtures/dados que lê; o manifesto QRC não licencia os recursos que referencia. A concessão se limita aos direitos disponíveis sobre as partes próprias.

No saneamento da Issue #2 em **2026-10-17**, a redação de `build_education.py` foi extraída sem alteração para `resources/data/group_education.json`. A lógica Python separada foi inspecionada, sem avisos ou fragmentos externos identificados, e incluída explicitamente na concessão de código já autorizada pelo mantenedor. Os limites de titularidade abaixo continuam aplicáveis. A pendência de direitos da redação acompanha o novo recurso, com caminho/hash anteriores preservados no inventário. A extração não é aprovação dos textos. Consulte [SANEAMENTO_ISSUE_2.md](SANEAMENTO_ISSUE_2.md).

A inspeção utilizou a árvore e o histórico de `b48ab87` (PR #10 integrado), busca de avisos/licenças/origens e o inventário anterior. Não foram identificados código vendorizado, avisos de código de terceiros ou contribuições com termos externos nesse escopo. O Git declara `kaduolliver` e `Carlos Eduardo Lima de Oliveira` como autores de commits. **Esses campos não comprovam identidade, cessão ou titularidade exclusiva.** A autorização desta intervenção fornece a decisão de licença e a identificação do criador; não é uma auditoria de contratos de emprego ou direitos de coautores desconhecidos.

Não se declarou que todos os arquivos históricos são exclusivamente do criador. Se surgir uma contribuição/trecho externo, preserve seus avisos, documente a origem e os termos e trate a compatibilidade antes de incorporá-lo à concessão. A mera disponibilidade pública não permite relicenciá-lo. O inventário registra os limites da inspeção; não autentica autores nem certifica inexistência de direitos externos.

## Exclusões e pendências

| Material | Tratamento |
| --- | --- |
| `resources/data/group_education.json` | Redação extraída de `DEFINITIONS` e `PLAIN_NOTES`, integralmente fora da concessão Apache. Originalidade, direitos e licença editorial permanecem pendentes; a lógica separada de `scripts/build_education.py` integra a enumeração de código |
| Texto científico/educativo incorporado, carregado ou reproduzido por código | Excluído; não passa a Apache por ser uma string, parâmetro, fixture ou saída do programa. Rótulos técnicos de UI/erros próprios integram o código; o corpus educativo é outra camada |
| Dados científicos externos e fatos | Termos das fontes quando aplicáveis; não se reivindica exclusividade sobre fatos taxonômicos |
| Textos próprios, documentação e organização da coletânea | Sem escolha de licença nesta decisão. `texts` e `collection` continuam `null`; CC BY 4.0 é apenas uma opção futura |
| Fotos de quiz/exploração, créditos e evidências | Licença/permissão individual, autoria e proveniência preservadas. Nenhuma licença foi alterada nem foto baixada/modificada |
| Capturas do README, relatórios gerados e fixtures | Não recebem licença global; conservam os direitos dos elementos reproduzidos e suas pendências |
| Nome, logotipo, ícones e branding | Tratamento separado. Não há concessão automática de marca, afirmação de registro INPI ou titularidade exclusiva comprovada |
| Qt, DLLs, QML/recursos Qt, plugins e componentes MSYS2 | Licenças e obrigações individuais. A Apache do código próprio não substitui LGPL/GPL, exceções, avisos ou fornecimento de fontes |
| Qualquer arquivo não enumerado em `files` | Não incluído por esta concessão. Ausência da lista não prova violação nem domínio público |

O [NOTICE](../NOTICE) reconhece a criação original e distingue contribuidores posteriores. Não inventa um ano ou declaração de copyright exclusivo, nem exige atribuição permanente na interface. Preserve os avisos aplicáveis nas formas previstas no §4 da licença. A identificação do criador foi expressamente fornecida, sem dedução do nome de usuário. Não se presume endosso de fotógrafos, provedores ou instituições.

## Registros e verificação

`content_policy.json` registra a decisão administrativa e `own_material_licenses.code = "Apache-2.0"`; `asset_rights.json` mantém a mesma declaração. A referência de escopo é este registro enumerado. Não se criaram eventos de revisão científica, confirmação de fotos ou assinatura humana. A autorização é a solicitação expressa desta implementação, não um ato jurídico independente simulado.

`LICENSE` foi obtido de [ASF — texto oficial](https://www.apache.org/licenses/LICENSE-2.0.txt), sem edição, inclusive o apêndice padrão. SHA-256: `cfc7749b96f63bd31c3c42b5c471bf756814053e847c10f3eb003417bc523d30`. Os campos exemplificativos do apêndice oficial não são avisos nominais preenchidos pelo projeto. A atribuição real está no NOTICE.

```powershell
python scripts/licensing_inventory.py --check-code
```

O comando valida offline os bytes oficiais, NOTICE, enumeração de código, pendências e coerência dos registros. Não escolhe licença, não confirma titularidade e não aprova conteúdo. Integra `offline-validation`, com regressões em `test_rights_investigation.py`. O inventário geral pode ser refeito em `build/`; o snapshot de preparação `INVENTARIO_LICENCIAMENTO.json` permanece histórico, sem reescrever a situação anterior.

Optou-se por **não inserir cabeçalhos em massa**: arquivos QML/C++ que participam de capturas não tiveram alterações sem efeito visual. O validador de inventário e, após a extração da redação, o gerador Python separado receberam `SPDX-License-Identifier: Apache-2.0`. O recurso de textos não recebeu SPDX de código. A enumeração e os documentos de concessão identificam os demais arquivos sem remover avisos anteriores.

## Distribuição e contribuições

A Apache permite usos comerciais do código coberto. Ela não libera automaticamente um executável com mídias ou dependências pendentes. `distribution-permissions` permanece obrigatório para distribuir, separado do check técnico. O aviso ICU fora do coletor e as obrigações Qt/MSYS2 continuam trabalhos próprios; não foi reempacotado executável.

Contribuições futuras seguem [CONTRIBUTING.md](../CONTRIBUTING.md) e os termos aplicáveis do §5 da Apache. Autoria individual é preservada; não há CLA, transferência de copyright ou assentimento retrospectivo presumido. Consulte o [relatório desta implementação](RELATORIO_IMPLEMENTACAO_APACHE_2_0.md) para resultados reais e limitações.
