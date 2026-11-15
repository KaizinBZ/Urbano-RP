# Urbano RP — launcher SA-MP

Repositório de código do Projeto Urbano. A origem da source Android completa precisa ser adicionada antes de rodar o build.

## Estado

O repositório já tem os arquivos de integração do IP padrão `179.198.105.167:7101` e um workflow do GitHub Actions. O IP é montado por ofuscação simples no cliente; isso não o mantém secreto para quem analisar o APK ou o tráfego.

**Ainda falta enviar a source Android completa** para a raiz deste repositório. A source validada anteriormente é `IMPERIO_NEXION_SOURCE_109`, package `com.imperio.city.migracao`, e descreve-se como migração parcial. Confirme que essa é realmente a base autorizada do Projeto Urbano antes de publicar/compilar.

## Adicionar source completa

Envie a source íntegra preservando a estrutura da raiz: `app/`, `gradle/`, `gradlew`, `settings.gradle`, `build.gradle` e `gradle/wrapper/gradle-wrapper.properties`. Não substitua os dois arquivos de integração já presentes: `app/src/main/java/com/lurion/launcher/activity/MainActivity.java` e `app/src/main/cpp/samp/main.cpp`. Resolva os conflitos escolhendo esses arquivos preparados para o endpoint Urbano.

Depois do upload dos arquivos Gradle da raiz, o workflow inicia automaticamente. Também pode ser executado em **Actions → Build Projeto Urbano APK → Run workflow** depois que a source estiver toda no repositório. O APK debug aparece como artefato da execução.

## Build

Java 21, Android SDK 36, Build Tools 36.0.0, NDK 26.2.11394342 e CMake 3.22.1. A assinatura debug do runner é temporária; não é uma assinatura de produção.

Não envie tokens, senhas ou keystores ao repositório público.
