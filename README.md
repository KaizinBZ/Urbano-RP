# Projeto Urbano — SA-MP Android

A source Android completa `IMPERIO_NEXION_SOURCE_109` foi enviada ao repositório como sete partes em `source/`. O workflow reconstrói o ZIP, verifica MD5 e CRC, restaura o projeto e preserva os dois arquivos de integração Urbano.

## Servidor e interface

O cliente nativo conecta ao IP/porta definidos para o Urbano; a interface exibe “Urbano City”, não o endereço numérico. O IP não pode ser secreto de alguém que analise o APK ou o tráfego, mas não é mostrado na tela do launcher.

## Assinatura e atualizações

O package da source é `com.imperio.city.migracao`. Para instalar por cima mantendo os dados, todas as builds precisam conservar esse package e usar a mesma keystore. Foi criada uma nova keystore persistente, guardada somente nos GitHub Actions Secrets: `URBANO_KEYSTORE_BASE64`, `URBANO_KEYSTORE_PASSWORD`, `URBANO_KEY_ALIAS`, `URBANO_KEY_PASSWORD`. O workflow atribui versionCode crescente para impedir downgrade.

A keystore antiga do APK 109 não estava na source. A primeira instalação assinada com a nova chave pode exigir remover um APK que tenha assinatura diferente e isso pode apagar dados privados. Faça backup antes dessa migração. Depois, não perca nem troque a nova chave.

## Build

Use **Actions → Build Projeto Urbano APK → Run workflow**. O build instala Java 21, Android SDK 36, Build Tools 36.0.0, NDK 26.2.11394342 e CMake 3.22.1. O APK release aparece como artefato do workflow. A source chama-se migração parcial Nexion 109, então o build e o teste no aparelho ainda são necessários antes de distribuir.
