# Urbano Test APK build (branch temporary)

This branch is isolated from the repository default branch and contains the test project/build workflow. The existing Urbano-RP content has not been replaced. The source archive derives from third-party Huracan project files and is published publicly only because the repository owner authorized it. Check licenses before reuse or redistribution.

IP/port supplied by owner: 179.198.105.167:7101. This is a game server address only; the Huracan project still needs compatible owned file-cache/API endpoints and its server discovery / native connection flow verified before the game is functional. No APK is claimed until GitHub Actions completes successfully.


## Assinatura persistente

A keystore privada não é publicada no repositório. O workflow usa os Actions Secrets `URBANO_KEYSTORE_BASE64`, `URBANO_KEYSTORE_PASSWORD`, `URBANO_KEY_ALIAS` e `URBANO_KEY_PASSWORD`; consulte [SIGNING.md](SIGNING.md). A primeira build debug não pode ser atualizada por cima por uma release assinada e deve ser removida antes da primeira instalação release.

## Downloads do jogo

A API/cache Nexion ainda não foi integrada. Endpoints `.invalid` permanecem inativos; o host e porta `179.198.105.167:7101` são do servidor do jogo, não um endpoint HTTP/cache. Esta build não pode baixar o cache até que a integração de endpoints e formatos compatíveis seja feita.
