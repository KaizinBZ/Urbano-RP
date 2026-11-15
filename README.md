# Urbano RP — launcher SA-MP Android

## Atualizações por cima e dados

O `applicationId` da source-base é `com.imperio.city.migracao`. Para o Android atualizar por cima e manter os dados privados do app, todas as builds precisam manter esse mesmo ID e ser assinadas com a mesma chave privada. O workflow gera `versionCode` sempre crescente (`109 + número da execução`), então builds antigas são recusadas como downgrade em vez de substituir uma versão mais nova.

**Importante sobre uma instalação que já existe:** se ela foi instalada com outra assinatura, uma chave nova não pode atualizá-la por cima. É preciso recuperar a keystore original usada nessa instalação. Não é possível reconstruir sua chave privada a partir do APK. A documentação da source informa que o APK 109 anterior usou uma chave debug de teste e que a chave de produto não está no ZIP. Antes da primeira assinatura persistente, confirme se a chave antiga está disponível ou se a primeira instalação poderá ser feita como nova. Depois da primeira build com a chave permanente, mantenha essa mesma keystore e backups seguros; perdê-la impede futuras atualizações sobre essa instalação.

A keystore e senhas devem ser configuradas como GitHub Actions Secrets (nunca no código público):
- `URBANO_KEYSTORE_BASE64`
- `URBANO_KEYSTORE_PASSWORD`
- `URBANO_KEY_ALIAS`
- `URBANO_KEY_PASSWORD`

O workflow está configurado para usar essa assinatura no `assembleRelease`. A chave persistente ainda não foi gerada/configurada porque é necessário confirmar compatibilidade com qualquer APK já instalado.

## Source Android ainda incompleta

Este repositório contém o README, o workflow e os arquivos de integração preparados, mas ainda falta enviar a source Android completa: `app/`, `gradle/`, `gradlew`, `settings.gradle`, `build.gradle` e `gradle/wrapper/gradle-wrapper.properties`.

A source validada anteriormente chama-se `IMPERIO_NEXION_SOURCE_109`, usa o package `com.imperio.city.migracao` e descreve-se como migração parcial. Confirme que essa é a base autorizada/correta do Projeto Urbano antes de distribuir. Envie a source à raiz preservando os dois fontes de integração já publicados; não substitua os arquivos `MainActivity.java` e `main.cpp` preparados.

Quando a source estiver no repositório e os secrets da keystore forem configurados, use **Actions → Build Projeto Urbano APK → Run workflow**. A build também roda automaticamente quando a source ou os arquivos Gradle da raiz mudam. O artefato release estará na execução do workflow.

## Endpoint e interface

O endereço de conexão é fixo no cliente nativo e não é mostrado como IP na interface; a etiqueta padrão é `Urbano City`. O código do cliente sempre pode ser analisado, portanto a ofuscação não é segredo de segurança.

## Build

Java 21, Android SDK 36, Build Tools 36.0.0, NDK 26.2.11394342, CMake 3.22.1. Não publique tokens, senhas ou keystores.
