# Transporte HTTPS de manutenção — INV-06

## Escopo e resultado

O cliente `PublicClient`, antes definido em `scripts/review_updates.py`, passa a ser implementado em [maintenance_http.py](../scripts/maintenance_http.py). O módulo original reexporta a classe para preservar os chamadores existentes. A correção trata o cliente usado por `propose_updates`, pelo comando `review_updates.py --online` e pelo caminho de compatibilidade `refine_photos.audit`/`--online`.

A execução normal do jogo permanece offline. Este trabalho não substitui os outros clientes HTTP do projeto nem declara que todos os caminhos de rede foram auditados. Não foram alterados organismos, fotografias, fontes científicas, licenças, avaliações, decisões de curadoria ou dados dos jogadores.

O comportamento anterior que seguia um redirecionamento público HTTPS até HTTP local foi substituído por conexão direta a IP validado e acompanhamento manual dos saltos. Os testes usam DNS, sockets e TLS simulados; não demonstram nem pressupõem uma invasão anterior.

| Arquivo alterado/adicionado | Finalidade |
| --- | --- |
| `scripts/maintenance_http.py` | Política, resolução limitada, conexão HTTPS fixada e controle de saltos/leitura |
| `scripts/maintenance_http_policy.json` | Lista explícita de hosts e porta |
| `scripts/review_updates.py` | Reexportação compatível e registro de destinos recusados como propostas |
| `tests/test_maintenance_http.py` | Testes sem rede real, incluindo cobertura dos hosts atuais e recusa de novos destinos não enumerados |
| `tests/test_content_governance.py` | Regressão de preservação do conteúdo após recusa |
| `tests/CMakeLists.txt` | Registro da 17ª suíte |
| `docs/TRANSPORTE_MANUTENCAO.md`, `docs/RELATORIO_INVESTIGATIVO.md`, `CHANGELOG.md` | Política, limites, resultados e histórico da correção |

## Política explícita

[maintenance_http_policy.json](../scripts/maintenance_http_policy.json) contém os **hosts exatos** permitidos, cobrindo as referências atuais, aliases canônicos selecionados e rotas editoriais previstas para DOI. A lista é a fonte de verdade; sua contagem não é duplicada neste documento. Não há wildcard, confiança automática em novos hosts do catálogo ou descoberta de permissões por resposta HTTP.

Na conferência da Issue #2 em 2026-10-18 foram acrescentados explicitamente os
destinos bibliográficos ANBG (`anbg.gov.au`), Smithsonian/STRI
(`biogeodb.stri.si.edu`), EOL (`eol.org`), Lepiforum (`lepiforum.org`), CDN da IUCN
(`nc.iucnredlist.org`), eFloras (`www.efloras.org`), FNCV (`www.fncv.org.au`),
Montpellier (`www.montpellier.fr`) e San Francisco Zoo (`www.sfzoo.org`). O exame
das referências está documentado no [lote consolidado](SANEAMENTO_ISSUE_2.md).
Não foram relaxados protocolo/porta, DNS público fixado, validação de saltos,
limites ou restrições de acesso. A inclusão permite apenas tentar a consulta
segura; não aprova a ciência, os direitos ou a redistribuição da fonte.

| Aspecto | Regra |
| --- | --- |
| Protocolo e porta | HTTPS, somente 443; porta explícita 443 também é aceita |
| Autoridade | Host presente na lista, sem usuário/senha, IP literal, representação numérica alternativa, zona IPv6 ou hostname ambíguo |
| URL | Limite de 8.192 caracteres; rejeita controles, espaços crus e barras invertidas; caminho/query são codificados para a requisição |
| Redirecionamentos | Apenas 301, 302, 303, 307 e 308; relativos e entre hosts permitidos; cada destino é validado antes de resolver e conectar |
| Quantidade | Até cinco redirecionamentos e seis requisições por `get`; ausência/duplicidade de `Location`, ciclo e excesso são recusados |
| DNS | Todos os A/AAAA retornados precisam ser públicos e adequados; mistura de IP público e proibido é recusada integralmente |
| IPs | Rejeita loopback, privados, link-local, não especificados, multicast, reservados e demais endereços sem alcance global; também faixas de transição/tradução e protocolo inadequadas |
| IPv6 | Unicast global nativo em `2000::/3`, sujeito aos filtros adicionais; IPv4 mapeado, Teredo, 6to4 e NAT64 não são aceitos por esta política |
| TLS | Contexto padrão com verificação de certificado e hostname, SNI para o host original; nenhum modo inseguro ou fallback HTTP |
| Estado HTTP | Somente GET, sem cookies, autenticação, Referer ou reaproveitamento de conexão entre saltos |
| Proxies | Variáveis de proxy do ambiente não são usadas; não há CONNECT ou caminho alternativo que substitua o endpoint validado |

A restrição do HTML da **LPSN** continua sendo aplicada antes do DNS, inclusive em redirecionamentos. Listar um host em política de rede não comprova direito de coleta ou redistribuição de seu conteúdo. Uma nova fonte ou destino editorial exige revisão explícita da lista e dos termos pertinentes; não se libera qualquer domínio apenas porque um DOI aponta para ele.

## Resolução e conexão sem segunda consulta DNS

Cada salto resolve o hostname uma vez, valida todas as respostas e produz endpoints numéricos. O socket conecta diretamente ao IP/porta aprovados; não chama um conector que resolva novamente o hostname. O peer conectado deve corresponder ao endpoint aprovado. O hostname original continua no cabeçalho Host e no SNI/verificação do certificado TLS.

Uma mudança de DNS entre validação e conexão não troca o endpoint já fixado. Em um novo salto, inclusive para o mesmo hostname, o DNS é consultado e validado novamente. Respostas mistas ou endereços inesperados interrompem o processo antes de uma conexão proibida. Isso reduz a janela de rebinding; não substitui a confiança no sistema operacional, nas rotas de rede e na cadeia de CAs configurada na máquina.

São usadas as bibliotecas padrão [http.client](https://docs.python.org/3.11/library/http.client.html), [socket](https://docs.python.org/3.11/library/socket.html) e [ssl](https://docs.python.org/3.11/library/ssl.html). O parser HTTP é reutilizado, com ciclo de vida controlado para manter o socket disponível ao watchdog até o fechamento do corpo, inclusive quando o servidor envia `Connection: close`.

## Limites e recursos

| Limite | Valor padrão/máximo |
| --- | --- |
| Timeout de conexão/I/O | 15 segundos por operação |
| Orçamento total de um `get` | 45 segundos, incluindo espera, DNS, saltos e leitura |
| DNS | Espera limitada pelo menor prazo restante/15 s; no máximo uma resolução pendente por cliente |
| Respostas DNS | Até 64 registros; todos são validados, não truncados para ignorar registros inseguros |
| Tentativas TCP/TLS | Até quatro endpoints já validados por salto, dentro do mesmo orçamento |
| Ritmo | Intervalo de pelo menos 1,1 s entre requisições de um cliente usado sequencialmente |
| JSON | Até 4.000.000 bytes; lê no máximo um byte adicional para detectar excesso, sem descompressão automática |
| Link bibliográfico | Até 1.024 bytes do corpo, como antes; não é download integral de artigo ou imagem |

O socket tem timeout e um watchdog associado ao prazo total, que tenta interromper comunicação pendente por shutdown. O leitor confere o prazo durante e após a leitura; uma resposta vencida não é aceita. Timers, respostas e sockets são fechados ao concluir, redirecionar ou falhar. Não há retry de resposta HTTP; HTTP 429 conserva `Retry-After` e mantém a interrupção existente do gerador de propostas.

O resolvedor nativo não oferece cancelamento portável. A espera ocorre em worker daemon: após timeout, novas resoluções no mesmo cliente são recusadas enquanto aquela chamada nativa continuar pendente, evitando acúmulo ilimitado de workers. O worker pode terminar mais tarde; sua resposta vencida não é usada. Os prazos são controles de orçamento e I/O, sujeitos ao escalonamento e às chamadas nativas do sistema, não garantias de tempo real.

## Compatibilidade e resultados da manutenção

As fontes atuais de instituições, museus, periódicos, Commons, Creative Commons e iNaturalist estão cobertas, mantendo LPSN HTML como consulta manual/API configurada. Rotas selecionadas de DOI para Nature, Elsevier e outros destinos da lista podem ser seguidas; um redirecionamento adicional para host desconhecido requer análise, sem fallback para uma política permissiva.

`propose_updates` registra `destination_refused`, motivo controlado e `requires_human_check` quando a política recusa um destino. Isso não é evidência de erro taxonômico, indisponibilidade permanente, licença revogada ou aprovação científica. As demais falhas continuam sendo propostas para conferência humana. O modo padrão permanece offline, e nenhum teste solicita `--online` com transporte real.

## Testes e validação

```powershell
python tests/test_maintenance_http.py
python tests/test_content_governance.py
python tests/test_scientific_sources.py
ctest --test-dir build --output-on-failure
```

A suíte dedicada exercita o parser HTTP de produção sobre sockets/TLS/DNS simulados, bloqueando DNS real durante os testes. Cobre:

- JSON do iNaturalist, Host/SNI originais, peer e conexão numérica sem segunda resolução;
- DOI para editora, caminhos relativos, cadeias legítimas e os cinco status suportados;
- downgrade, protocolos inválidos, credenciais, portas indevidas, aliases numéricos e host desconhecido;
- IPv4/IPv6 privados, reservados, multicast, tradução/transição e respostas DNS mistas;
- mudança de DNS entre saltos, peer divergente, certificado inválido e contexto TLS inseguro;
- cinco redirecionamentos aceitos, sexto recusado, ciclos, fragmentos e `Location` ambíguo;
- HTTP 429 sem retry, JSON excessivo, leitura parcial de link, orçamento total e watchdog;
- fallback limitado, worker DNS pendente, proxies do ambiente e fechamento de recursos;
- cobertura das referências atuais e manutenção da restrição da LPSN.

A regressão de governança também verifica que recusas de destino geram propostas sem modificar nenhum byte em `resources/`. Não houve teste online de disponibilidade dos 111 hosts, validação de certificados de servidores reais ou teste físico de DNS rebinding. A correção local não equivale a uma certificação de segurança de todos os transportes do projeto.

## Resultado executado

Em 16/10/2026, configuração CMake Release/build incremental e **17/17 suítes CTest passaram**, em 79,60 s. A suíte nova tem **20 testes**, além de um novo caso na governança que comprova a preservação dos arquivos científicos após recusas. Houve validação das rotas atuais contra a política sem fazer DNS ou HTTP real para essas fontes. Consultas de disponibilidade/certificados reais e execução remota da CI não foram realizadas nesta correção.

A aplicação 1.7.0 continua em preparação; esta é uma correção compatível de manutenção, sem lançamento, incremento do catálogo, nova permissão ou mudança visual. Compilar/testar não altera as pendências de direitos e curadoria que bloqueiam distribuição.
