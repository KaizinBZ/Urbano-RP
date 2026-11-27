# Urbano Test — assinatura de release

A build teste usa `applicationId` `com.kaizin.urbanotest`. Para que releases futuras possam atualizar uma instalação assinada, mantenha esse identificador, a mesma keystore e o `versionCode` crescente.

## Localização segura da chave

A chave privada e suas senhas **não ficam neste repositório público**. O workflow consome os GitHub Actions Secrets já configurados no repositório `KaizinBZ/Urbano-RP`:

- `URBANO_KEYSTORE_BASE64` — keystore PKCS#12 codificada em Base64;
- `URBANO_KEYSTORE_PASSWORD` — senha da keystore;
- `URBANO_KEY_ALIAS` — alias do certificado;
- `URBANO_KEY_PASSWORD` — senha da chave privada.

O workflow restaura a keystore apenas no runner temporário, configura assinatura PKCS#12 e gera uma release assinada. Não imprima nem copie valores secretos para logs, arquivos públicos, commits ou issues. A chave persistente existente pertence à configuração privada de release do projeto Urbano nesse repositório; ela está sendo reutilizada para a identidade de teste separada.

## Migração da build atual

A build `1.0-test` inicialmente distribuída era debug e foi assinada com uma chave temporária do runner. Ela não pode ser atualizada por cima pela primeira release persistente. Remova a build debug antes de instalar a primeira release assinada; isso pode apagar os dados do app. Depois dessa migração, mantenha a keystore e use sempre o workflow de release para atualizações por cima.

As execuções futuras incrementam o `versionCode`; nunca reduza nem reutilize um número de versão.

## Limite funcional atual

A source é baseada no launcher Huracan. A API/cache da Nexion não foi integrada e os endpoints `.invalid` estão inativos; esta nota de assinatura não significa que o download dos arquivos do jogo ou a conexão ao servidor já funcionem.