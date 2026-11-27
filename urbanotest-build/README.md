# Urbano Test APK build (branch temporary)

This branch is isolated from the repository default branch and contains the test project/build workflow. The existing Urbano-RP content has not been replaced. The source archive derives from third-party Huracan project files and is published publicly only because the repository owner authorized it. Check licenses before reuse or redistribution.

IP/port supplied by owner: 179.198.105.167:7101. This is a game server address only; the Huracan project still needs compatible owned file-cache/API endpoints and its server discovery / native connection flow verified before the game is functional. No APK is claimed until GitHub Actions completes successfully.


## Assinatura persistente

A keystore privada não é publicada no repositório. O workflow usa os Actions Secrets `URBANO_KEYSTORE_BASE64`, `URBANO_KEYSTORE_PASSWORD`, `URBANO_KEY_ALIAS` e `URBANO_KEY_PASSWORD`; consulte [SIGNING.md](SIGNING.md). A primeira build debug não pode ser atualizada por cima por uma release assinada e deve ser removida antes da primeira instalação release.

## Downloads do jogo

A API/cache Nexion ainda não foi integrada. Endpoints `.invalid` permanecem inativos; o host e porta `179.198.105.167:7101` são do servidor do jogo, não um endpoint HTTP/cache. Esta build não pode baixar o cache até que a integração de endpoints e formatos compatíveis seja feita.


## Cache Nexion autorizada para o teste

O responsável pelo projeto autorizou este teste a consumir `https://samp-mobile.shop/files.json`. O app baixa por HTTPS os arquivos do manifesto (316 entradas consultadas em 27/11/2026, aproximadamente 1,12 GB), verifica host, caminho e tamanho, salva nas pastas relativas esperadas e só marca o cache como pronto após concluir tudo. `.htaccess` é ignorado. Reserve aproximadamente o total faltante mais 64 MB livres. A lista não fornece hashes de conteúdo; a compatibilidade final da cache e o início do jogo ainda precisam de validação num aparelho.

Isso conecta apenas o download dos arquivos do jogo. As APIs para notícias/lista de servidores do launcher e a conexão de rede ao servidor `179.198.105.167:7101` não foram migradas da Nexion e não são declaradas funcionais por este ajuste.


## Diagnóstico de crash

O botão `Enviar log` agora cria `diagnostico_urbano.txt` com o `crash_log.txt`/`log.txt` nativos salvos pelo jogo e tenta coletar as últimas 2.000 linhas do `logcat`. Em seguida abre o seletor nativo de compartilhamento via FileProvider. O Android pode restringir linhas de outros processos; por isso, o log nativo do jogo é incluído primeiro. O relatório é local e só sai do aparelho se a pessoa escolher compartilhar; revisar antes de enviar porque pode conter dados do aparelho/apelido.
